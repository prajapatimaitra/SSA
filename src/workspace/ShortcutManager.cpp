#include "workspace/ShortcutManager.h"
#include "capture/CaptureController.h"
#include "core/logging/Logger.h"

#ifdef __APPLE__
#include "workspace/macos/MacShortcutProvider.h"
#endif

namespace ssa::workspace {

ShortcutManager::ShortcutManager(ssa::capture::CaptureController* captureController, QObject* parent)
    : QObject(parent), m_captureController(captureController) {
    
#ifdef __APPLE__
    m_provider = std::make_unique<MacShortcutProvider>();
#else
    // Stub for other platforms
#endif

    if (m_captureController) {
        connect(m_captureController, &ssa::capture::CaptureController::recordingStarted, this, [this]() {
            m_isRecording = true;
        });
        connect(m_captureController, &ssa::capture::CaptureController::recordingStopped, this, [this]() {
            m_isRecording = false;
        });
    }
}

ShortcutManager::~ShortcutManager() = default;

void ShortcutManager::initialize() {
    registerDefaultShortcuts();
}

void ShortcutManager::registerDefaultShortcuts() {
    if (!m_provider || !m_captureController) return;

    // Cmd + Option + Shift + R
    auto recordModifiers = ShortcutModifier::Command | ShortcutModifier::Alt | ShortcutModifier::Shift;
    m_recordShortcutId = m_provider->registerShortcut(ShortcutKey::Key_R, recordModifiers, [this]() {
        if (m_isRecording) {
            core::logging::Logger::info("Global Shortcut Triggered: Stop Recording");
            m_captureController->stopCapture();
        } else {
            core::logging::Logger::info("Global Shortcut Triggered: Start Recording");
            m_captureController->startCapture("0", "");
        }
    });

    // Cmd + Option + Shift + S
    auto screenshotModifiers = ShortcutModifier::Command | ShortcutModifier::Alt | ShortcutModifier::Shift;
    m_screenshotShortcutId = m_provider->registerShortcut(ShortcutKey::Key_S, screenshotModifiers, [this]() {
        core::logging::Logger::info("Global Shortcut Triggered: Screenshot");
        m_captureController->captureScreenshot();
    });
}

} // namespace ssa::workspace
