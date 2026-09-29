#include "input/interfaces/IInputTracker.h"
#include "core/logging/Logger.h"
#include <memory>

namespace ssa::platform {

class WindowsInputTracker : public input::IInputTracker {
public:
    void startTracking() override {
        core::logging::Logger::error("Windows Input Tracker is a compilation stub.");
    }
    void stopTracking() override {}
    void setEventCallback(std::function<void(const input::MouseEvent&)> callback) override {}
};

std::unique_ptr<input::IInputTracker> createInputTracker() {
    return std::make_unique<WindowsInputTracker>();
}

} // namespace ssa::platform
