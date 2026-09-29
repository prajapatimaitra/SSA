#include "workspace/macos/MacShortcutProvider.h"
#import <AppKit/AppKit.h>
#include <map>
#include <iostream>

struct ShortcutDef {
    int id;
    ssa::workspace::ShortcutKey key;
    ssa::workspace::ShortcutModifier modifiers;
    std::function<void()> callback;
};

class MacShortcutProviderPrivate {
public:
    int nextId = 1;
    std::map<int, ShortcutDef> shortcuts;
    id globalMonitor = nil;
    id localMonitor = nil;

    void setupMonitors() {
        if (globalMonitor) return;

        auto handler = ^(NSEvent *event) {
            handleEvent(event);
        };

        globalMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:handler];
        localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent *(NSEvent *event) {
            handleEvent(event);
            return event;
        }];
    }

    void teardownMonitors() {
        if (globalMonitor) {
            [NSEvent removeMonitor:globalMonitor];
            globalMonitor = nil;
        }
        if (localMonitor) {
            [NSEvent removeMonitor:localMonitor];
            localMonitor = nil;
        }
    }

    void handleEvent(NSEvent *event) {
        if ([event isARepeat]) return;

        unsigned short keyCode = [event keyCode];
        NSEventModifierFlags flags = [event modifierFlags];
        
        // R is 15, S is 1
        ssa::workspace::ShortcutKey pressedKey;
        if (keyCode == 15) pressedKey = ssa::workspace::ShortcutKey::Key_R;
        else if (keyCode == 1) pressedKey = ssa::workspace::ShortcutKey::Key_S;
        else return;

        ssa::workspace::ShortcutModifier pressedMods = ssa::workspace::ShortcutModifier::None;
        if (flags & NSEventModifierFlagShift) pressedMods = pressedMods | ssa::workspace::ShortcutModifier::Shift;
        if (flags & NSEventModifierFlagControl) pressedMods = pressedMods | ssa::workspace::ShortcutModifier::Control;
        if (flags & NSEventModifierFlagOption) pressedMods = pressedMods | ssa::workspace::ShortcutModifier::Alt;
        if (flags & NSEventModifierFlagCommand) pressedMods = pressedMods | ssa::workspace::ShortcutModifier::Command;

        for (const auto& pair : shortcuts) {
            const auto& def = pair.second;
            if (def.key == pressedKey && def.modifiers == pressedMods) {
                if (def.callback) def.callback();
            }
        }
    }
};

namespace ssa::workspace {

MacShortcutProvider::MacShortcutProvider() : m_private(std::make_unique<MacShortcutProviderPrivate>()) {
}

MacShortcutProvider::~MacShortcutProvider() {
    m_private->teardownMonitors();
}

int MacShortcutProvider::registerShortcut(ShortcutKey key, ShortcutModifier modifiers, std::function<void()> callback) {
    if (m_private->shortcuts.empty()) {
        m_private->setupMonitors();
    }
    
    int id = m_private->nextId++;
    m_private->shortcuts[id] = {id, key, modifiers, callback};
    return id;
}

bool MacShortcutProvider::unregisterShortcut(int id) {
    auto it = m_private->shortcuts.find(id);
    if (it != m_private->shortcuts.end()) {
        m_private->shortcuts.erase(it);
        if (m_private->shortcuts.empty()) {
            m_private->teardownMonitors();
        }
        return true;
    }
    return false;
}

} // namespace ssa::workspace
