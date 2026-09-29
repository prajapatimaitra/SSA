#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <memory>

// Forward declarations
namespace ssa::capture {
    class CaptureController;
}

namespace ssa::workspace {

class SystemTrayController : public QObject {
    Q_OBJECT

public:
    explicit SystemTrayController(ssa::capture::CaptureController* captureController, QObject* parent = nullptr);
    ~SystemTrayController();

    void initialize();

public slots:
    void updateRecordingState(bool isRecording);

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onRecordScreenClicked();
    void onScreenshotClicked();
    void onQuitClicked();

private:
    void createActions();
    void createTrayIcon();
    void updateIcon();

    ssa::capture::CaptureController* m_captureController = nullptr;
    
    std::unique_ptr<QSystemTrayIcon> m_trayIcon;
    std::unique_ptr<QMenu> m_trayMenu;
    
    QAction* m_recordAction = nullptr;
    QAction* m_screenshotAction = nullptr;
    QAction* m_quitAction = nullptr;
    
    bool m_isRecording = false;
};

} // namespace ssa::workspace
