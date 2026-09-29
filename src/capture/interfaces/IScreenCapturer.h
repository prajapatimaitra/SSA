#pragma once

#include <QString>
#include <functional>

namespace ssa::capture {

class IScreenCapturer {
public:
    virtual ~IScreenCapturer() = default;

    /**
     * Captures a screenshot of the specified display and saves it to the output path.
     * @param displayId The ID of the display to capture (e.g., "0" for main display).
     * @param outputPath The file path where the image should be saved.
     * @param callback Called with true if successful, false otherwise.
     */
    virtual void captureScreenshot(const QString& displayId, const QString& outputPath, std::function<void(bool)> callback) = 0;
};

} // namespace ssa::capture
