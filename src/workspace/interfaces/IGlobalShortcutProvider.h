#pragma once

#include <functional>
#include <string>

namespace ssa::workspace {

enum class ShortcutModifier {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,     // Option on Mac
    Command = 1 << 3  // Meta/Super on Windows/Linux
};

inline ShortcutModifier operator|(ShortcutModifier a, ShortcutModifier b) {
    return static_cast<ShortcutModifier>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(ShortcutModifier a, ShortcutModifier b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

enum class ShortcutKey {
    Key_R,
    Key_S,
    // Add more as needed
};

class IGlobalShortcutProvider {
public:
    virtual ~IGlobalShortcutProvider() = default;

    // Registers a global shortcut. The callback is invoked when the shortcut is pressed.
    // Returns an ID that can be used to unregister it, or -1 if it failed.
    virtual int registerShortcut(ShortcutKey key, ShortcutModifier modifiers, std::function<void()> callback) = 0;
    
    virtual bool unregisterShortcut(int id) = 0;
};

} // namespace ssa::workspace
