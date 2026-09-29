#include "platform/interfaces/ICapabilityDetector.h"

namespace ssa::platform {

class WindowsCapabilityDetector : public ICapabilityDetector {
public:
    bool hasScreenRecordingPermission() override { return true; }
    bool hasInputMonitoringPermission() override { return true; }
    void requestScreenRecordingPermission() override {}
    void requestInputMonitoringPermission() override {}
};

} // namespace ssa::platform
