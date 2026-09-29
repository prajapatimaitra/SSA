#include "MacNativeIntegration.h"
#include "core/logging/Logger.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

namespace ssa::workspace::macos {

MacNativeIntegration::MacNativeIntegration() {
}

MacNativeIntegration::~MacNativeIntegration() {
}

void MacNativeIntegration::revealInOS(const QString& path) {
    @autoreleasepool {
        QString cleanPath = path;
        if (cleanPath.startsWith("file://")) {
            cleanPath = cleanPath.mid(7);
        }
        
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cleanPath.toUtf8().constData()]];
        if (fileURL) {
            [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[fileURL]];
            core::logging::Logger::info("Revealed file in Finder: " + cleanPath.toStdString());
        } else {
            core::logging::Logger::error("Failed to create URL for reveal: " + cleanPath.toStdString());
        }
    }
}

void MacNativeIntegration::shareFile(const QString& path) {
    @autoreleasepool {
        QString cleanPath = path;
        if (cleanPath.startsWith("file://")) {
            cleanPath = cleanPath.mid(7);
        }
        
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cleanPath.toUtf8().constData()]];
        if (!fileURL) {
            core::logging::Logger::error("Failed to create URL for sharing: " + cleanPath.toStdString());
            return;
        }
        
        // We must dispatch to main thread for UI operations
        dispatch_async(dispatch_get_main_queue(), ^{
            NSView *contentView = [NSApp keyWindow].contentView;
            if (contentView) {
                NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:@[fileURL]];
                // Show in the center of the window
                NSRect rect = NSMakeRect(NSMidX(contentView.bounds), NSMidY(contentView.bounds), 1, 1);
                [picker showRelativeToRect:rect ofView:contentView preferredEdge:NSRectEdgeMinY];
                core::logging::Logger::info("Summoned macOS Share Sheet for: " + cleanPath.toStdString());
            } else {
                core::logging::Logger::error("No key window available to present Share Sheet.");
            }
        });
    }
}

void MacNativeIntegration::setWindowCaptureProtected(void* windowPtr, bool protect) {
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            NSWindow *targetWindow = nil;
            if (windowPtr) {
                id obj = (__bridge id)windowPtr;
                if ([obj isKindOfClass:[NSWindow class]]) {
                    targetWindow = (NSWindow *)obj;
                } else if ([obj isKindOfClass:[NSView class]]) {
                    targetWindow = [(NSView *)obj window];
                }
            }
            if (!targetWindow) {
                targetWindow = [NSApp keyWindow];
            }
            if (targetWindow) {
                // NSWindowSharingNone (0) excludes window from all OS screenshots/recordings (QuickTime, Cmd+Shift+4, Zoom, OBS)
                // NSWindowSharingReadOnly (1) is default normal window sharing
                [targetWindow setSharingType:protect ? NSWindowSharingNone : NSWindowSharingReadOnly];
                core::logging::Logger::info("MacNativeIntegration: Applied window capture protection (NSWindowSharingNone) = " + std::to_string(protect));
            } else {
                core::logging::Logger::error("MacNativeIntegration: No target window available to apply capture protection.");
            }
        });
    }
}

static id s_windowCreatedObserver = nil;

void MacNativeIntegration::setAppCaptureProtected(bool protect) {
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            for (NSWindow *window in [NSApp windows]) {
                [window setSharingType:protect ? NSWindowSharingNone : NSWindowSharingReadOnly];
            }
            
            if (protect && !s_windowCreatedObserver) {
                s_windowCreatedObserver = [[NSNotificationCenter defaultCenter]
                    addObserverForName:NSWindowDidBecomeKeyNotification
                                object:nil
                                 queue:[NSOperationQueue mainQueue]
                            usingBlock:^(NSNotification * _Nonnull note) {
                                NSWindow *w = note.object;
                                if (w) {
                                    [w setSharingType:NSWindowSharingNone];
                                }
                            }];
            } else if (!protect && s_windowCreatedObserver) {
                if (s_windowCreatedObserver) {
                    [[NSNotificationCenter defaultCenter] removeObserver:s_windowCreatedObserver];
                    s_windowCreatedObserver = nil;
                }
            }
            
            core::logging::Logger::info("MacNativeIntegration: Applied capture protection to all NSApp windows & installed observer = " + std::to_string(protect));
        });
    }
}

} // namespace ssa::workspace::macos
