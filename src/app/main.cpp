#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core/logging/Logger.h"
#include "core/state/ApplicationState.h"
#include "platform/PlatformFactory.h"

#include "capture/CaptureController.h"
#include "editor/EditorController.h"
#include "workspace/SystemTrayController.h"
#include "workspace/NativeIntegrationManager.h"
#include "workspace/ShortcutManager.h"
#include "workspace/ClipboardManager.h"
#include "export/ExportQueueManager.h"

#include <QSurfaceFormat>
#include <QQuickWindow>
#include <QQuickStyle>

#ifdef __APPLE__
#include "capture/macos/MacScreenCapturer.h"
#endif

#include "library/LibraryManager.h"
#include "library/LibraryModel.h"
#include "library/LibraryFilterModel.h"
#include "project/ProjectBinManager.h"
#include "auth/AuthManager.h"
#include "auth/UserQuotaManager.h"
#include "auth/UserManager.h"
#include <QDir>
#include <QStandardPaths>

using namespace ssa::core;
using namespace ssa::platform;
using namespace ssa::capture;
using namespace ssa::project;
using namespace ssa::workspace;
using namespace ssa::library;

static QString getAppDataDir() {
    QDir current(QDir::currentPath());
    if (current.exists(".recordings") || current.exists("CMakeLists.txt") || current.exists("cursors")) {
        return current.absolutePath();
    }
    QDir parentDir = current;
    while (parentDir.cdUp()) {
        if (parentDir.exists(".recordings") || parentDir.exists("CMakeLists.txt") || parentDir.exists("cursors")) {
            return parentDir.absolutePath();
        }
    }
    return QDir::currentPath();
}

int main(int argc, char *argv[]) {
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); // Keep running in system tray when windows are closed
    QQuickStyle::setStyle("Basic");
    
    logging::Logger::init();
    logging::Logger::info("Starting SSA Application...");

    // Setup LibraryManager
    LibraryManager libraryManager;
    QString appDataDir = getAppDataDir();
    QDir().mkpath(appDataDir + "/.recordings");
    QDir().mkpath(appDataDir + "/.screenshots");
    libraryManager.initialize(appDataDir + "/ssa_library.db");
    libraryManager.scanExistingFiles(appDataDir + "/.recordings", appDataDir + "/.screenshots");

    // Setup Library Models
    LibraryModel libraryModel(&libraryManager);
    LibraryFilterModel libraryFilterModel;
    libraryFilterModel.setSourceModel(&libraryModel);

    // Setup platform implementations
    auto displayManager = createDisplayManager();
    auto gpuDetector = createGpuDetector();
    auto capabilityDetector = createCapabilityDetector();
    auto captureEngine = createScreenCapture();
    auto inputTracker = createInputTracker();
    
    std::unique_ptr<IScreenCapturer> screenCapturer;
#ifdef __APPLE__
    screenCapturer = std::make_unique<MacScreenCapturer>();
#endif

    // Log platform data
    auto gpuInfo = gpuDetector->getPrimaryGpu();
    logging::Logger::info("GPU: " + gpuInfo.name + (gpuInfo.isHardwareAccelerated ? " (Hardware Accelerated)" : " (Software)"));

    auto displays = displayManager->enumerateDisplays();
    logging::Logger::info("Detected " + std::to_string(displays.size()) + " displays.");
    QString firstDisplayId = "";
    for (const auto& d : displays) {
        logging::Logger::info(" - " + d.name + " (" + std::to_string(d.width) + "x" + std::to_string(d.height) + ")");
        if (firstDisplayId.isEmpty()) {
            firstDisplayId = QString::fromStdString(d.id);
        }
    }

    // App state & Controllers
    state::ApplicationState appState;
    CaptureController captureController(std::move(captureEngine), std::move(screenCapturer), std::move(inputTracker));
    captureController.setAppDataDir(appDataDir);
    captureController.setLibraryManager(&libraryManager);
    
    ssa::editor::EditorController editorController;
    
    QObject::connect(&captureController, &CaptureController::subtitlesReady, &editorController, [&editorController](const QString& vttPath) {
        editorController.loadSubtitles(vttPath);
    });

    // System Tray integration
    SystemTrayController systemTray(&captureController);
    systemTray.initialize();

    // Global Shortcuts
    ShortcutManager shortcutManager(&captureController);
    shortcutManager.initialize();
    
    // Clipboard Manager
    ClipboardManager clipboardManager;
    
    ssa::workspace::SystemTrayController trayController(&captureController);
    ssa::workspace::NativeIntegrationManager nativeIntegration;
    ssa::export_engine::ExportQueueManager exportQueueManager;

    // Authentication, User & Quota Managers
    ssa::auth::UserManager userManager;
    userManager.initialize(appDataDir + "/ssa_library.db");

    ssa::auth::AuthManager authManager;
    authManager.setUserManager(&userManager);
    ssa::auth::UserQuotaManager quotaManager(&authManager);
    quotaManager.setUserManager(&userManager);

    // Project Bins & Asset Database Manager (Phase 1)
    ProjectBinManager projectBinManager;
    projectBinManager.initialize(appDataDir + "/ssa_library.db", appDataDir);

    QQmlApplicationEngine engine;
    
    // Register QML context properties
    engine.rootContext()->setContextProperty("appDataDir", appDataDir);
    engine.rootContext()->setContextProperty("appDataUrl", QUrl::fromLocalFile(appDataDir).toString());
    engine.rootContext()->setContextProperty("appState", &appState);
    engine.rootContext()->setContextProperty("captureController", &captureController);
    engine.rootContext()->setContextProperty("editorController", &editorController);
    engine.rootContext()->setContextProperty("clipboardManager", &clipboardManager);
    engine.rootContext()->setContextProperty("libraryManager", &libraryManager);
    engine.rootContext()->setContextProperty("projectBinManager", &projectBinManager);
    engine.rootContext()->setContextProperty("nativeIntegration", &nativeIntegration);
    engine.rootContext()->setContextProperty("exportQueueManager", &exportQueueManager);
    engine.rootContext()->setContextProperty("authManager", &authManager);
    engine.rootContext()->setContextProperty("quotaManager", &quotaManager);
    engine.rootContext()->setContextProperty("userManager", &userManager);
    engine.rootContext()->setContextProperty("libraryModel", &libraryModel);
    engine.rootContext()->setContextProperty("libraryFilterModel", &libraryFilterModel);
    engine.rootContext()->setContextProperty("firstDisplayId", firstDisplayId);
    engine.rootContext()->setContextProperty("gpuName", QString::fromStdString(gpuInfo.name));
    engine.rootContext()->setContextProperty("displayCount", QVariant::fromValue(static_cast<int>(displays.size())));

    const QUrl url("qrc:/MainWindow.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    // Apply Recordly System-Level Window Protection (NSWindowSharingNone) automatically
    nativeIntegration.setAppCaptureProtected(true);

    return app.exec();
}
