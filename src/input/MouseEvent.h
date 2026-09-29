#pragma once

#include <cstdint>
#include <string>

namespace ssa::input {

enum class MouseButton {
    None,
    Left,
    Right,
    Middle
};

enum class MouseEventType {
    Move,
    Down,
    Up,
    Drag
};

struct MouseEvent {
    uint64_t timestamp = 0; // Microseconds since epoch
    double x;
    double y;
    MouseButton button;
    MouseEventType type;
    std::string cursorType = "arrow";
    double cursorWidth = 14.0;
    double cursorHeight = 20.0;
    double cursorHotspotX = 5.0;
    double cursorHotspotY = 5.0;
    double anchorX = 0.34;
    double anchorY = 0.24;
    double velocityX = 0.0;
    double velocityY = 0.0;
    double clickProgress = 0.0;
};

} // namespace ssa::input
