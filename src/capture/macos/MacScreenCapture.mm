#include "capture/interfaces/IPlatformCapture.h"
#include "core/logging/Logger.h"
#include "media/VideoFrame.h"

#import <Foundation/Foundation.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#import <CoreMedia/CoreMedia.h>
#import <AVFoundation/AVFoundation.h>

using namespace ssa::capture;
using namespace ssa::media;

@interface SCKStreamDelegate : NSObject <SCStreamDelegate, SCStreamOutput>
@property (nonatomic, assign) std::function<void(const VideoFrame&)> frameCallback;
@property (nonatomic, strong) AVAssetWriter *assetWriter;
@property (nonatomic, strong) AVAssetWriterInput *videoInput;
@property (nonatomic, strong) AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor;

@property (nonatomic, strong) AVAssetWriter *audioAssetWriter;
@property (nonatomic, strong) AVAssetWriterInput *audioInput;

@property (nonatomic, assign) BOOL isRecording;
@property (nonatomic, assign) BOOL hasStartedSession;
@property (nonatomic, assign) BOOL hasStartedAudioSession;
@end

@implementation SCKStreamDelegate

- (void)stream:(SCStream *)stream didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer ofType:(SCStreamOutputType)type {
    if (type == SCStreamOutputTypeScreen) {
        if (self.isRecording && self.assetWriter && self.videoInput) {
            if (self.assetWriter.status == AVAssetWriterStatusUnknown) {
                BOOL started = [self.assetWriter startWriting];
                if (!started) {
                    ssa::core::logging::Logger::error(std::string("AVAssetWriter startWriting failed: ") + [[self.assetWriter.error localizedDescription] UTF8String]);
                    self.isRecording = NO;
                    return;
                }
                [self.assetWriter startSessionAtSourceTime:CMSampleBufferGetPresentationTimeStamp(sampleBuffer)];
                self.hasStartedSession = YES;
            }
            
            if (self.assetWriter.status == AVAssetWriterStatusWriting && self.videoInput.readyForMoreMediaData) {
                CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
                if (imageBuffer) {
                    [self.pixelBufferAdaptor appendPixelBuffer:imageBuffer withPresentationTime:CMSampleBufferGetPresentationTimeStamp(sampleBuffer)];
                }
            }
        }

        if (self.frameCallback) {
            CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
            if (imageBuffer) {
                size_t width = CVPixelBufferGetWidth(imageBuffer);
                size_t height = CVPixelBufferGetHeight(imageBuffer);
                
                CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
                uint64_t timestamp = CMTimeGetSeconds(pts) * 1000000.0;
                
                VideoFrame frame;
                frame.timestamp = timestamp;
                frame.width = static_cast<int>(width);
                frame.height = static_cast<int>(height);
                frame.format = PixelFormat::BGRA;
                frame.colorSpace = ColorSpace::UNKNOWN;
                frame.nativeBuffer = imageBuffer;
                self.frameCallback(frame);
            }
        }
    } else if (type == SCStreamOutputTypeAudio) {
        if (self.isRecording && self.audioAssetWriter && self.audioInput) {
            if (self.audioAssetWriter.status == AVAssetWriterStatusUnknown) {
                BOOL started = [self.audioAssetWriter startWriting];
                if (started) {
                    [self.audioAssetWriter startSessionAtSourceTime:CMSampleBufferGetPresentationTimeStamp(sampleBuffer)];
                    self.hasStartedAudioSession = YES;
                }
            }
            
            if (self.audioAssetWriter.status == AVAssetWriterStatusWriting && self.audioInput.readyForMoreMediaData) {
                [self.audioInput appendSampleBuffer:sampleBuffer];
            }
        }
    }
}

- (void)stream:(SCStream *)stream didStopWithError:(NSError *)error {
    ssa::core::logging::Logger::error(std::string("ScreenCaptureKit Stream Error: ") + [[error localizedDescription] UTF8String]);
}

@end

namespace ssa::platform {

class MacScreenCapture : public IPlatformCapture {
public:
    MacScreenCapture() {
        m_delegate = [[SCKStreamDelegate alloc] init];
    }
    
    ~MacScreenCapture() {
        stopCapture();
        m_delegate = nil;
    }

    void startCapture(const CaptureConfig& config) override {
        if (@available(macOS 12.3, *)) {
            NSString *nsDisplayId = [NSString stringWithUTF8String:config.displayId.c_str()];
            NSString *nsOutputFilePath = [NSString stringWithUTF8String:config.outputFilePath.c_str()];
            NSString *nsSystemAudioPath = config.systemAudioPath.empty() ? nil : [NSString stringWithUTF8String:config.systemAudioPath.c_str()];

            [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *content, NSError *error) {
                if (error) return;

                SCDisplay *targetDisplay = nil;
                for (SCDisplay *display in content.displays) {
                    NSString *currentId = [NSString stringWithUTF8String:std::to_string(display.displayID).c_str()];
                    if ([currentId isEqualToString:nsDisplayId]) {
                        targetDisplay = display;
                        break;
                    }
                }
                if (!targetDisplay) targetDisplay = content.displays.firstObject;
                if (!targetDisplay) return;

                // Video AVAssetWriter
                if ([[NSFileManager defaultManager] fileExistsAtPath:nsOutputFilePath]) {
                    [[NSFileManager defaultManager] removeItemAtPath:nsOutputFilePath error:nil];
                }
                
                NSError *writerError = nil;
                AVAssetWriter *assetWriter = [AVAssetWriter assetWriterWithURL:[NSURL fileURLWithPath:nsOutputFilePath] fileType:AVFileTypeMPEG4 error:&writerError];
                
                int outWidth = targetDisplay.width;
                int outHeight = targetDisplay.height;
                
                if (config.isCustomRegion && config.regionWidth > 0 && config.regionHeight > 0) {
                    outWidth = config.regionWidth;
                    outHeight = config.regionHeight;
                }
                
                // Scale resolution if not Original/Native
                if (config.captureResolution == "4K") {
                    outWidth = (outWidth * 2160) / outHeight; outHeight = 2160;
                } else if (config.captureResolution == "1440p") {
                    outWidth = (outWidth * 1440) / outHeight; outHeight = 1440;
                } else if (config.captureResolution == "1080p") {
                    outWidth = (outWidth * 1080) / outHeight; outHeight = 1080;
                } else if (config.captureResolution == "720p") {
                    outWidth = (outWidth * 720) / outHeight; outHeight = 720;
                }

                // Ensure even dimensions for H.264
                outWidth = (outWidth / 2) * 2;
                outHeight = (outHeight / 2) * 2;
                
                NSDictionary *videoSettings = @{
                    AVVideoCodecKey: AVVideoCodecTypeH264,
                    AVVideoWidthKey: @(outWidth),
                    AVVideoHeightKey: @(outHeight)
                };

                AVAssetWriterInput *videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:videoSettings];
                videoInput.expectsMediaDataInRealTime = YES;
                [assetWriter addInput:videoInput];

                NSDictionary *sourcePixelBufferAttributes = @{
                    (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
                    (id)kCVPixelBufferWidthKey: @(outWidth),
                    (id)kCVPixelBufferHeightKey: @(outHeight),
                    (id)kCVPixelBufferMetalCompatibilityKey: @YES
                };

                AVAssetWriterInputPixelBufferAdaptor *adaptor = [AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:videoInput sourcePixelBufferAttributes:sourcePixelBufferAttributes];

                m_delegate.assetWriter = assetWriter;
                m_delegate.videoInput = videoInput;
                m_delegate.pixelBufferAdaptor = adaptor;
                
                // Audio AVAssetWriter (if requested)
                if (nsSystemAudioPath) {
                    if ([[NSFileManager defaultManager] fileExistsAtPath:nsSystemAudioPath]) {
                        [[NSFileManager defaultManager] removeItemAtPath:nsSystemAudioPath error:nil];
                    }
                    AVAssetWriter *audioWriter = [AVAssetWriter assetWriterWithURL:[NSURL fileURLWithPath:nsSystemAudioPath] fileType:AVFileTypeAppleM4A error:nil];
                    
                    AudioChannelLayout acl;
                    bzero(&acl, sizeof(acl));
                    acl.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
                    NSData *channelLayoutData = [NSData dataWithBytes:&acl length:sizeof(acl)];
                    
                    NSDictionary *audioSettings = @{
                        AVFormatIDKey: @(kAudioFormatMPEG4AAC),
                        AVSampleRateKey: @(48000.0),
                        AVNumberOfChannelsKey: @(2),
                        AVChannelLayoutKey: channelLayoutData,
                        AVEncoderBitRateKey: @(128000)
                    };
                    
                    AVAssetWriterInput *audioInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio outputSettings:audioSettings];
                    audioInput.expectsMediaDataInRealTime = YES;
                    [audioWriter addInput:audioInput];
                    
                    m_delegate.audioAssetWriter = audioWriter;
                    m_delegate.audioInput = audioInput;
                }

                m_delegate.isRecording = YES;
                m_delegate.hasStartedSession = NO;
                m_delegate.hasStartedAudioSession = NO;

                SCStreamConfiguration *streamConfig = [[SCStreamConfiguration alloc] init];
                streamConfig.width = outWidth;
                streamConfig.height = outHeight;
                streamConfig.minimumFrameInterval = CMTimeMake(1, config.fps > 0 ? config.fps : 30);
                
                if (config.isCustomRegion && config.regionWidth > 0 && config.regionHeight > 0) {
                    streamConfig.sourceRect = CGRectMake(config.regionX, config.regionY, config.regionWidth, config.regionHeight);
                }
                streamConfig.showsCursor = false;
                streamConfig.pixelFormat = kCVPixelFormatType_32BGRA;
                if (nsSystemAudioPath) {
                    streamConfig.capturesAudio = YES;
                }
                
                if (config.isCustomRegion) {
                    streamConfig.sourceRect = CGRectMake(config.regionX, config.regionY, config.regionWidth, config.regionHeight);
                    streamConfig.width = config.regionWidth;
                    streamConfig.height = config.regionHeight;
                }

                pid_t currentPid = [[NSProcessInfo processInfo] processIdentifier];
                NSMutableArray<SCRunningApplication *> *excludedApps = [NSMutableArray array];
                for (SCRunningApplication *app in content.applications) {
                    if (app.processID == currentPid) {
                        [excludedApps addObject:app];
                    }
                }

                SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:targetDisplay excludingApplications:excludedApps exceptingWindows:@[]];

                m_stream = [[SCStream alloc] initWithFilter:filter configuration:streamConfig delegate:m_delegate];
                
                [m_stream addStreamOutput:m_delegate type:SCStreamOutputTypeScreen sampleHandlerQueue:dispatch_get_main_queue() error:nil];
                if (nsSystemAudioPath) {
                    [m_stream addStreamOutput:m_delegate type:SCStreamOutputTypeAudio sampleHandlerQueue:dispatch_get_main_queue() error:nil];
                }
                
                [m_stream startCaptureWithCompletionHandler:nil];
            }];
        }
    }

    void stopCapture() override {
        if (@available(macOS 12.3, *)) {
            if (m_stream) {
                [m_stream stopCaptureWithCompletionHandler:nil];
                m_stream = nil;
            }
            
            if (m_delegate.isRecording) {
                m_delegate.isRecording = NO;
                
                if (m_delegate.assetWriter && m_delegate.assetWriter.status == AVAssetWriterStatusWriting) {
                    [m_delegate.videoInput markAsFinished];
                    [m_delegate.assetWriter finishWritingWithCompletionHandler:^{}];
                }
                
                if (m_delegate.audioAssetWriter && m_delegate.audioAssetWriter.status == AVAssetWriterStatusWriting) {
                    [m_delegate.audioInput markAsFinished];
                    [m_delegate.audioAssetWriter finishWritingWithCompletionHandler:^{}];
                }
            }
        }
    }

    void setFrameCallback(std::function<void(const media::VideoFrame&)> callback) override {
        m_delegate.frameCallback = callback;
    }

private:
    SCKStreamDelegate* m_delegate = nil;
    SCStream* m_stream = nil;
};

std::unique_ptr<capture::IPlatformCapture> createScreenCapture() {
    return std::make_unique<MacScreenCapture>();
}

} // namespace ssa::platform
