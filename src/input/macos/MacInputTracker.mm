#include "input/interfaces/IInputTracker.h"
#include "core/logging/Logger.h"
#include "core/coordinates/CoordinateMapper.h"

#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/AppKit.h>
#import <CommonCrypto/CommonDigest.h>
#include <chrono>
#include <unordered_map>

static std::string getCursorHash(NSData* data) {
    unsigned char result[CC_MD5_DIGEST_LENGTH];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CC_MD5(data.bytes, (CC_LONG)data.length, result);
#pragma clang diagnostic pop
    char hex[33];
    snprintf(hex, sizeof(hex), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            result[0], result[1], result[2], result[3], 
            result[4], result[5], result[6], result[7],
            result[8], result[9], result[10], result[11],
            result[12], result[13], result[14], result[15]);
    return std::string(hex);
}

static std::string getCursorNameFromHash(const std::string& hash) {
    static const std::unordered_map<std::string, std::string> cursorMap = {
        {"c1601f6caf96128fa5aec1ee86504f29", "resizeLeftRight"},
        {"462910aaf576cf1bb13ee7cab5e7c469", "resizeDown"},
        {"a417da2fbb660140961facff55e4e29d", "dragCopy"},
        {"abd1e94d6b9f670dfde5405e3b86d64a", "iBeamCursorForVerticalLayout"},
        {"695fa9b7f3a3701c9c5acba7cc4239e1", "resizeUp"},
        {"c518be5b7015d6324a77e51e337b5e3d", "pointingHand"},
        {"860691cf03dcbb6f413642ea6f7b34bb", "arrow"},
        {"2bc14cc94697456bf6e6709a21f026ea", "crosshair"},
        {"393906485cb09d1967ae8d04db389458", "resizeRight"},
        {"44ce05e00b5b9e8e97e8267ec3f88ead", "closedHand"},
        {"ed485bf63f1a2dbe4a62530707460b8f", "operationNotAllowed"},
        {"190f21f902534c95ecdce630a6faa0be", "dragLink"},
        {"2b99485d8c8036d2e26a2912a8d5362b", "openHand"},
        {"551d86af38c893113f93c03ac20b52d1", "disappearingItem"},
        {"f539149b97021980a4393bd802f5fc49", "contextualMenu"},
        {"85f61d21a2d37d4a9eb08c089e1095f3", "resizeUpDown"},
        {"6b6f08ad231d9d082b63aa71504c042d", "iBeam"},
        {"c0e96ab847246a7f9f51fd92ad3e043c", "resizeLeft"}
    };
    auto it = cursorMap.find(hash);
    if (it != cursorMap.end()) {
        return it->second;
    }
    return "arrow";
}

using namespace ssa::input;

namespace ssa::platform {

class MacInputTracker : public IInputTracker {
public:
    MacInputTracker() : m_runLoopSource(nullptr), m_eventTap(nullptr) {}

    ~MacInputTracker() {
        stopTracking();
    }

    void startTracking() override {
        if (m_eventTap) return;

        CGEventMask eventMask = (1 << kCGEventMouseMoved) |
                                (1 << kCGEventLeftMouseDown) |
                                (1 << kCGEventLeftMouseUp) |
                                (1 << kCGEventRightMouseDown) |
                                (1 << kCGEventRightMouseUp) |
                                (1 << kCGEventLeftMouseDragged) |
                                (1 << kCGEventRightMouseDragged);

        m_eventTap = CGEventTapCreate(
            kCGHIDEventTap,
            kCGHeadInsertEventTap,
            kCGEventTapOptionListenOnly,
            eventMask,
            &MacInputTracker::eventCallback,
            this
        );

        if (!m_eventTap) {
            core::logging::Logger::error("Failed to create CGEventTap. Accessibility permissions missing?");
            return;
        }

        m_runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, m_eventTap, 0);
        CFRunLoopAddSource(CFRunLoopGetMain(), m_runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(m_eventTap, true);
        
        core::logging::Logger::info("MacInputTracker started.");
    }

    void stopTracking() override {
        if (m_eventTap) {
            CGEventTapEnable(m_eventTap, false);
            if (m_runLoopSource) {
                CFRunLoopRemoveSource(CFRunLoopGetMain(), m_runLoopSource, kCFRunLoopCommonModes);
                CFRelease(m_runLoopSource);
                m_runLoopSource = nullptr;
            }
            CFRelease(m_eventTap);
            m_eventTap = nullptr;
            core::logging::Logger::info("MacInputTracker stopped.");
        }
    }

    void setEventCallback(std::function<void(const MouseEvent&)> callback) override {
        m_callback = callback;
    }

private:
    static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
        MacInputTracker* tracker = static_cast<MacInputTracker*>(refcon);
        if (!tracker->m_callback) return event;

        CGPoint location = CGEventGetLocation(event);
        
        MouseEvent mouseEvent;
        // High precision timestamp
        auto now = std::chrono::high_resolution_clock::now();
        mouseEvent.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        
        mouseEvent.x = core::coordinates::CoordinateMapper::mapX(location.x);
        mouseEvent.y = core::coordinates::CoordinateMapper::mapY(location.y);

        @autoreleasepool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            NSCursor *cursor = [NSCursor currentSystemCursor];
#pragma clang diagnostic pop
            if (cursor) {
                NSImage *image = [cursor image];
                NSData *tiff = [image TIFFRepresentation];
                if (tiff) {
                    std::string hash = getCursorHash(tiff);
                    mouseEvent.cursorType = getCursorNameFromHash(hash);
                }
                
                // Get dynamic logical size and hotspot points
                NSSize size = [image size];
                NSPoint hotSpot = [cursor hotSpot];
                
                mouseEvent.cursorWidth = size.width;
                mouseEvent.cursorHeight = size.height;
                mouseEvent.cursorHotspotX = hotSpot.x;
                mouseEvent.cursorHotspotY = hotSpot.y;
            }
        }

        if (type == kCGEventLeftMouseDown) {
            mouseEvent.button = MouseButton::Left;
            mouseEvent.type = MouseEventType::Down;
        } else if (type == kCGEventLeftMouseUp) {
            mouseEvent.button = MouseButton::Left;
            mouseEvent.type = MouseEventType::Up;
        } else if (type == kCGEventRightMouseDown) {
            mouseEvent.button = MouseButton::Right;
            mouseEvent.type = MouseEventType::Down;
        } else if (type == kCGEventRightMouseUp) {
            mouseEvent.button = MouseButton::Right;
            mouseEvent.type = MouseEventType::Up;
        } else if (type == kCGEventLeftMouseDragged || type == kCGEventRightMouseDragged) {
            mouseEvent.button = (type == kCGEventLeftMouseDragged) ? MouseButton::Left : MouseButton::Right;
            mouseEvent.type = MouseEventType::Drag;
        } else {
            mouseEvent.button = MouseButton::None;
            mouseEvent.type = MouseEventType::Move;
        }

        tracker->m_callback(mouseEvent);
        return event;
    }

    CFMachPortRef m_eventTap;
    CFRunLoopSourceRef m_runLoopSource;
    std::function<void(const MouseEvent&)> m_callback;
};

std::unique_ptr<IInputTracker> createInputTracker() {
    return std::make_unique<MacInputTracker>();
}

} // namespace ssa::platform
