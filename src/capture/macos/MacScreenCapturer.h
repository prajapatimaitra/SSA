#pragma once

#include "capture/interfaces/IScreenCapturer.h"
#include <memory>

class MacScreenCapturerPrivate;

namespace ssa::capture {

class MacScreenCapturer : public IScreenCapturer {
public:
    MacScreenCapturer();
    ~MacScreenCapturer() override;

    void captureScreenshot(const QString& displayId, const QString& outputPath, std::function<void(bool)> callback) override;

private:
    std::unique_ptr<MacScreenCapturerPrivate> m_private;
};

} // namespace ssa::capture
