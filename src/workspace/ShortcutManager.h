#pragma once

#include "workspace/interfaces/IGlobalShortcutProvider.h"
#include <memory>
#include <QObject>

namespace ssa::capture {
    class CaptureController;
}

namespace ssa::workspace {

class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(ssa::capture::CaptureController* captureController, QObject* parent = nullptr);
    ~ShortcutManager();

    void initialize();

private:
    void registerDefaultShortcuts();

    ssa::capture::CaptureController* m_captureController = nullptr;
    std::unique_ptr<IGlobalShortcutProvider> m_provider;
    
    int m_recordShortcutId = -1;
    int m_screenshotShortcutId = -1;
    bool m_isRecording = false;
};

} // namespace ssa::workspace
