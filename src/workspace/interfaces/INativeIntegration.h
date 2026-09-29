#pragma once

#include <QString>

namespace ssa::workspace::interfaces {

class INativeIntegration {
public:
    virtual ~INativeIntegration() = default;
    
    // Reveals the file at the given path in the native file explorer (Finder, Windows Explorer, etc.)
    // Note: The file itself should be selected/highlighted, not just its parent folder.
    virtual void revealInOS(const QString& path) = 0;
    
    // Spawns the native share sheet or dialog for the given file path
    virtual void shareFile(const QString& path) = 0;

    // Sets window capture protection (NSWindowSharingNone on macOS / WDA_EXCLUDEFROMCAPTURE on Windows)
    virtual void setWindowCaptureProtected(void* windowPtr, bool protect) = 0;

    // Sets capture protection for all app windows
    virtual void setAppCaptureProtected(bool protect) = 0;
};

} // namespace ssa::workspace::interfaces
