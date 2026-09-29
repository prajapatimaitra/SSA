#include "workspace/SystemTrayController.h"
#include "capture/CaptureController.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>

namespace ssa::workspace {

SystemTrayController::SystemTrayController(ssa::capture::CaptureController* captureController, QObject* parent)
    : QObject(parent), m_captureController(captureController) {
    createActions();
    createTrayIcon();
    
    if (m_captureController) {
        connect(m_captureController, &ssa::capture::CaptureController::recordingStarted, this, [this]() {
            updateRecordingState(true);
        });
        connect(m_captureController, &ssa::capture::CaptureController::recordingStopped, this, [this]() {
            updateRecordingState(false);
        });
    }
}

SystemTrayController::~SystemTrayController() = default;

void SystemTrayController::initialize() {
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon->show();
        updateIcon();
    } else {
        qWarning() << "System tray is not available on this system.";
    }
}

void SystemTrayController::updateRecordingState(bool isRecording) {
    m_isRecording = isRecording;
    if (m_recordAction) {
        m_recordAction->setText(m_isRecording ? tr("Stop Recording") : tr("Record Screen"));
    }
    updateIcon();
}

void SystemTrayController::createActions() {
    m_trayMenu = std::make_unique<QMenu>();
    
    m_recordAction = new QAction(tr("Record Screen"), this);
    connect(m_recordAction, &QAction::triggered, this, &SystemTrayController::onRecordScreenClicked);
    
    m_screenshotAction = new QAction(tr("Screenshot"), this);
    connect(m_screenshotAction, &QAction::triggered, this, &SystemTrayController::onScreenshotClicked);
    
    m_quitAction = new QAction(tr("Quit"), this);
    connect(m_quitAction, &QAction::triggered, this, &SystemTrayController::onQuitClicked);
    
    m_trayMenu->addAction(m_recordAction);
    m_trayMenu->addAction(m_screenshotAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);
}

void SystemTrayController::createTrayIcon() {
    m_trayIcon = std::make_unique<QSystemTrayIcon>(this);
    m_trayIcon->setContextMenu(m_trayMenu.get());
    m_trayIcon->setToolTip(tr("SSA Workspace"));
    
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated, this, &SystemTrayController::onTrayIconActivated);
}

void SystemTrayController::updateIcon() {
    // For a real application, you would load proper icons from resources.
    // We'll use a basic fallback here or an empty icon for now if none exists.
    if (!m_trayIcon) return;
    
    if (m_isRecording) {
        // Red dot icon indicating recording
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(Qt::red);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(8, 8, 16, 16);
        m_trayIcon->setIcon(QIcon(pixmap));
    } else {
        // Normal icon
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::black);
        painter.drawEllipse(8, 8, 16, 16);
        m_trayIcon->setIcon(QIcon(pixmap));
    }
}

void SystemTrayController::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        // Could open the main library window here
    }
}

void SystemTrayController::onRecordScreenClicked() {
    if (!m_captureController) return;
    
    if (m_isRecording) {
        m_captureController->stopCapture();
    } else {
        // For simplicity, trigger a full display capture
        m_captureController->startCapture("0", "");
    }
}

void SystemTrayController::onScreenshotClicked() {
    if (!m_captureController) return;
    m_captureController->captureScreenshot();
}

void SystemTrayController::onQuitClicked() {
    QCoreApplication::quit();
}

} // namespace ssa::workspace
