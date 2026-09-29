#include "export/interfaces/IMediaEncoder.h"
#include "export/interfaces/IMediaDecoder.h"
#include "core/logging/Logger.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#include <QImage>
#include <QDebug>

namespace ssa::export_engine {

class MacMediaDecoder : public IMediaDecoder {
public:
    MacMediaDecoder() : m_asset(nil), m_generator(nil) {}

    ~MacMediaDecoder() override {
        if (m_generator) { [m_generator release]; m_generator = nil; }
        if (m_asset) { [m_asset release]; m_asset = nil; }
    }

    bool initialize(const std::string& videoPath) override {
        NSString *nsPath = [NSString stringWithUTF8String:videoPath.c_str()];
        NSURL *url = [NSURL fileURLWithPath:nsPath];
        m_asset = [[AVURLAsset URLAssetWithURL:url options:nil] retain];
        if (!m_asset) return false;

        m_generator = [[AVAssetImageGenerator assetImageGeneratorWithAsset:m_asset] retain];
        if (!m_generator) return false;
        
        m_generator.requestedTimeToleranceBefore = kCMTimeZero;
        m_generator.requestedTimeToleranceAfter = kCMTimeZero;
        m_generator.appliesPreferredTrackTransform = YES;
        
        return true;
    }

    QImage getFrameAtTime(qint64 timeMs) override {
        if (!m_generator) return QImage();

        CMTime time = CMTimeMake(timeMs, 1000);
        NSError *error = nil;
        CGImageRef cgImage = [m_generator copyCGImageAtTime:time actualTime:NULL error:&error];
        if (!cgImage) {
            core::logging::Logger::error("Failed to extract frame at " + std::to_string(timeMs) + "ms: " + (error ? std::string([[error localizedDescription] UTF8String]) : "Unknown error"));
            return QImage();
        }
        
        size_t width = CGImageGetWidth(cgImage);
        size_t height = CGImageGetHeight(cgImage);
        
        // QImage defaults to ARGB32, which matches BGRA internally on little endian
        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(Qt::black); // Ensure clear memory

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        // Use kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host for QImage::Format_ARGB32 mapping
        CGContextRef context = CGBitmapContextCreate(image.bits(), width, height, 8, image.bytesPerLine(), colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        
        if (context) {
            // Draw image into the QImage memory block
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
            CGContextRelease(context);
        } else {
            core::logging::Logger::error("Failed to create CGBitmapContext for frame extraction");
        }
        
        CGColorSpaceRelease(colorSpace);
        CGImageRelease(cgImage);
        
        return image;
    }

private:
    AVURLAsset* m_asset;
    AVAssetImageGenerator* m_generator;
};

std::unique_ptr<IMediaDecoder> createMediaDecoder() {
    return std::make_unique<MacMediaDecoder>();
}

class MacMediaEncoder : public IMediaEncoder {
public:
    MacMediaEncoder() : m_writer(nil), m_videoInput(nil), m_adaptor(nil) {}

    ~MacMediaEncoder() override {
        finish();
    }

    bool initialize(const std::string& outputPath, int width, int height, int fps, const EncoderConfig& config) override {
        if (@available(macOS 10.15, *)) {
            NSString *nsOutputPath = [NSString stringWithUTF8String:outputPath.c_str()];
            NSURL *fileUrl = [NSURL fileURLWithPath:nsOutputPath];

            if ([[NSFileManager defaultManager] fileExistsAtPath:nsOutputPath]) {
                [[NSFileManager defaultManager] removeItemAtPath:nsOutputPath error:nil];
            }

            NSError *error = nil;
            m_writer = [[AVAssetWriter assetWriterWithURL:fileUrl fileType:AVFileTypeMPEG4 error:&error] retain];
            
            if (error || !m_writer) {
                core::logging::Logger::error("Failed to create AVAssetWriter for export: " + std::string([[error localizedDescription] UTF8String]));
                return false;
            }

            AVVideoCodecType codecType = AVVideoCodecTypeH264;
            if (config.codec == "hevc" || config.codec == "HEVC") {
                codecType = AVVideoCodecTypeHEVC;
            }

            int bitrate = 10000000; // 10 Mbps default
            if (config.quality == "Low") bitrate = 2500000;
            else if (config.quality == "Medium") bitrate = 5000000;
            else if (config.quality == "High") bitrate = 10000000;

            NSDictionary *videoSettings = @{
                AVVideoCodecKey: codecType,
                AVVideoWidthKey: @(width),
                AVVideoHeightKey: @(height),
                AVVideoCompressionPropertiesKey: @{
                    AVVideoAverageBitRateKey: @(bitrate)
                }
            };

            m_videoInput = [[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:videoSettings] retain];
            m_videoInput.expectsMediaDataInRealTime = NO; // Offline export is faster than real-time

            if ([m_writer canAddInput:m_videoInput]) {
                [m_writer addInput:m_videoInput];
            } else {
                core::logging::Logger::error("Failed to add video input to AVAssetWriter");
                return false;
            }

            NSDictionary *sourcePixelBufferAttributes = @{
                (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
                (id)kCVPixelBufferWidthKey: @(width),
                (id)kCVPixelBufferHeightKey: @(height)
            };

            m_adaptor = [[AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:m_videoInput sourcePixelBufferAttributes:sourcePixelBufferAttributes] retain];

            if (![m_writer startWriting]) {
                core::logging::Logger::error("Failed to start writing: " + std::string([[m_writer.error localizedDescription] UTF8String]));
                return false;
            }

            [m_writer startSessionAtSourceTime:kCMTimeZero];
            m_fps = fps;
            
            core::logging::Logger::info("MacMediaEncoder initialized for " + outputPath);
            return true;
        }
        return false;
    }

    bool appendFrame(const QImage& frame, uint64_t ptsMicroseconds) override {
        if (!m_writer || !m_adaptor || !m_videoInput) return false;
        
        if (frame.isNull() || frame.width() == 0 || frame.height() == 0) {
            core::logging::Logger::error("Attempted to append an empty or invalid frame");
            return false;
        }

        int waitCount = 0;
        while (!m_videoInput.readyForMoreMediaData) {
            usleep(5000); // Sleep 5ms and wait
            waitCount++;
            if (waitCount > 2000) { // Wait 10 seconds max
                core::logging::Logger::error("Timeout waiting for AVAssetWriterInput to be ready");
                return false;
            }
        }

        QImage formattedFrame = frame.convertToFormat(QImage::Format_ARGB32);

        CVPixelBufferRef pixelBuffer = NULL;
        CVReturn status = kCVReturnError;
        
        if (m_adaptor.pixelBufferPool) {
            status = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, m_adaptor.pixelBufferPool, &pixelBuffer);
        } else {
            NSDictionary *options = @{
                (id)kCVPixelBufferCGImageCompatibilityKey: @YES,
                (id)kCVPixelBufferCGBitmapContextCompatibilityKey: @YES
            };
            status = CVPixelBufferCreate(kCFAllocatorDefault, formattedFrame.width(), formattedFrame.height(), kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef)options, &pixelBuffer);
        }
        
        if (status != kCVReturnSuccess || !pixelBuffer) {
            return false;
        }

        CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        void *pxdata = CVPixelBufferGetBaseAddress(pixelBuffer);
        size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

        if (!pxdata) {
            CVPixelBufferRelease(pixelBuffer);
            return false;
        }

        for (int y = 0; y < formattedFrame.height(); ++y) {
            memcpy((uint8_t*)pxdata + y * bytesPerRow, formattedFrame.constScanLine(y), formattedFrame.width() * 4);
        }

        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

        CMTime ptsCM = CMTimeMake(ptsMicroseconds, 1000000);
        BOOL success = [m_adaptor appendPixelBuffer:pixelBuffer withPresentationTime:ptsCM];
        
        if (!success) {
            core::logging::Logger::error("appendPixelBuffer failed. Writer status: " + std::to_string(m_writer.status) + ", Error: " + std::string(m_writer.error ? [[m_writer.error localizedDescription] UTF8String] : "None"));
        }
        
        CVPixelBufferRelease(pixelBuffer);
        return success;
    }

    void finish() override {
        if (m_writer && m_writer.status == AVAssetWriterStatusWriting) {
            [m_videoInput markAsFinished];
            
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            
            [m_writer finishWritingWithCompletionHandler:^{
                if (m_writer.status == AVAssetWriterStatusFailed) {
                    core::logging::Logger::error("MacMediaEncoder failed to finish: " + std::string([[m_writer.error localizedDescription] UTF8String]));
                } else {
                    core::logging::Logger::info("MacMediaEncoder finished writing video.");
                }
                dispatch_semaphore_signal(sema);
            }];
            
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        }
        
        if (m_adaptor) { [m_adaptor release]; m_adaptor = nil; }
        if (m_videoInput) { [m_videoInput release]; m_videoInput = nil; }
        if (m_writer) { [m_writer release]; m_writer = nil; }
    }

private:
    AVAssetWriter *m_writer;
    AVAssetWriterInput *m_videoInput;
    AVAssetWriterInputPixelBufferAdaptor *m_adaptor;
    int m_fps = 60;
};

std::unique_ptr<IMediaEncoder> createMediaEncoder() {
    return std::make_unique<MacMediaEncoder>();
}

} // namespace ssa::export_engine
