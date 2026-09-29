#pragma once

#include "input/MouseEvent.h"
#include <functional>

namespace ssa::input {

class IInputTracker {
public:
    virtual ~IInputTracker() = default;

    virtual void startTracking() = 0;
    virtual void stopTracking() = 0;
    
    // Callback for mouse events
    virtual void setEventCallback(std::function<void(const MouseEvent&)> callback) = 0;
};

} // namespace ssa::input
