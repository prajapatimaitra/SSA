#pragma once

#include <memory>
#include "platform/interfaces/IDisplayManager.h"
#include "platform/interfaces/IGpuDetector.h"
#include "platform/interfaces/ICapabilityDetector.h"
#include "capture/interfaces/IPlatformCapture.h"
#include "input/interfaces/IInputTracker.h"

namespace ssa::platform {

std::unique_ptr<IDisplayManager> createDisplayManager();
std::unique_ptr<IGpuDetector> createGpuDetector();
std::unique_ptr<ICapabilityDetector> createCapabilityDetector();
std::unique_ptr<capture::IPlatformCapture> createScreenCapture();
std::unique_ptr<input::IInputTracker> createInputTracker();

} // namespace ssa::platform
