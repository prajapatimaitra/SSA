#include "MacScreenCapturer.h"
#include "core/logging/Logger.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

class MacScreenCapturerPrivate {
public:
    void captureScreenshot(const QString& displayId, const QString& outputPath, std::function<void(bool)> callback) {
        ssa::core::logging::Logger::info("MacScreenCapturer: captureScreenshot called.");
        
        // COPY the reference to a local value so the block captures it safely by value
        QString pathCopy = outputPath;
        
        if (@available(macOS 14.0, *)) {
            ssa::core::logging::Logger::info("MacScreenCapturer: Using SCScreenshotManager (macOS 14.0+).");
            [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *content, NSError *error) {
                ssa::core::logging::Logger::info("MacScreenCapturer: getShareableContent callback fired.");
                if (error || !content) {
                    ssa::core::logging::Logger::error("Failed to get SCShareableContent for screenshot.");
                    if (callback) callback(false);
                    return;
                }
                
                SCDisplay *targetDisplay = content.displays.firstObject;
                if (!targetDisplay) {
                    ssa::core::logging::Logger::error("No target display found.");
                    if (callback) callback(false);
                    return;
                }
                
                pid_t currentPid = [[NSProcessInfo processInfo] processIdentifier];
                NSMutableArray<SCRunningApplication *> *excludedApps = [NSMutableArray array];
                for (SCRunningApplication *app in content.applications) {
                    if (app.processID == currentPid) {
                        [excludedApps addObject:app];
                    }
                }
                
                SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:targetDisplay excludingApplications:excludedApps exceptingWindows:@[]];
                SCStreamConfiguration *config = [[SCStreamConfiguration alloc] init];
                config.width = targetDisplay.width;
                config.height = targetDisplay.height;
                config.showsCursor = YES;
                
                ssa::core::logging::Logger::info("MacScreenCapturer: Calling captureImageWithFilter...");
                [SCScreenshotManager captureImageWithFilter:filter configuration:config completionHandler:^(CGImageRef sampleBuffer, NSError *error) {
                    ssa::core::logging::Logger::info("MacScreenCapturer: captureImageWithFilter callback fired.");
                    if (error || !sampleBuffer) {
                        ssa::core::logging::Logger::error("SCScreenshotManager failed to capture image.");
                        if (callback) callback(false);
                        return;
                    }
                    
                    saveCGImageToPath(sampleBuffer, pathCopy);
                    ssa::core::logging::Logger::info("MacScreenCapturer: Image saved successfully.");
                    if (callback) callback(true);
                }];
            }];
        } else {
            ssa::core::logging::Logger::error("macOS 14.0+ is required for taking screenshots via ScreenCaptureKit.");
            if (callback) callback(false);
        }
    }
    
    void saveCGImageToPath(CGImageRef image, const QString& outputPath) {
        NSString *path = [NSString stringWithUTF8String:outputPath.toStdString().c_str()];
        NSURL *url = [NSURL fileURLWithPath:path];
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL((__bridge CFURLRef)url, CFSTR("public.png"), 1, NULL);
        if (destination) {
            CGImageDestinationAddImage(destination, image, nil);
            CGImageDestinationFinalize(destination);
            CFRelease(destination);
        } else {
            ssa::core::logging::Logger::error("Failed to create CGImageDestinationRef.");
        }
    }
};

namespace ssa::capture {

MacScreenCapturer::MacScreenCapturer() : m_private(std::make_unique<MacScreenCapturerPrivate>()) {
}

MacScreenCapturer::~MacScreenCapturer() = default;

void MacScreenCapturer::captureScreenshot(const QString& displayId, const QString& outputPath, std::function<void(bool)> callback) {
    m_private->captureScreenshot(displayId, outputPath, callback);
}

} // namespace ssa::capture
