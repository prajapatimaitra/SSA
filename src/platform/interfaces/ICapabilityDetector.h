#pragma once

namespace ssa::platform {

class ICapabilityDetector {
public:
    virtual ~ICapabilityDetector() = default;
    virtual bool hasScreenRecordingPermission() = 0;
    virtual bool hasInputMonitoringPermission() = 0;
    virtual void requestScreenRecordingPermission() = 0;
    virtual void requestInputMonitoringPermission() = 0;
};

} // namespace ssa::platform
