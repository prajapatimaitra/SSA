#pragma once

#include "workspace/interfaces/INativeIntegration.h"
#include <QString>

namespace ssa::workspace::macos {

class MacNativeIntegration : public interfaces::INativeIntegration {
public:
    MacNativeIntegration();
    ~MacNativeIntegration() override;

    void revealInOS(const QString& path) override;
    void shareFile(const QString& path) override;
    void setWindowCaptureProtected(void* windowPtr, bool protect) override;
    void setAppCaptureProtected(bool protect) override;
};

} // namespace ssa::workspace::macos
