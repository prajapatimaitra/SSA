#include "NativeIntegrationManager.h"
#include "core/logging/Logger.h"
#include <QWindow>

#ifdef __APPLE__
#include "macos/MacNativeIntegration.h"
#endif

namespace ssa::workspace {

static NativeIntegrationManager* s_instance = nullptr;

NativeIntegrationManager::NativeIntegrationManager(QObject* parent)
    : QObject(parent) {
    s_instance = this;
    
#ifdef __APPLE__
    m_impl = std::make_unique<macos::MacNativeIntegration>();
#else
    core::logging::Logger::error("Native integration not implemented for this platform");
#endif
}

NativeIntegrationManager::~NativeIntegrationManager() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

NativeIntegrationManager* NativeIntegrationManager::instance() {
    return s_instance;
}

void NativeIntegrationManager::revealInOS(const QString& path) {
    if (m_impl) {
        m_impl->revealInOS(path);
    }
}

void NativeIntegrationManager::shareFile(const QString& path) {
    if (m_impl) {
        m_impl->shareFile(path);
    }
}

void NativeIntegrationManager::setWindowCaptureProtected(QObject* windowObject, bool protect) {
    void* winPtr = nullptr;
    if (windowObject) {
        if (auto qwindow = qobject_cast<QWindow*>(windowObject)) {
            winPtr = reinterpret_cast<void*>(qwindow->winId());
        }
    }
    if (m_impl) {
        m_impl->setWindowCaptureProtected(winPtr, protect);
    }
}

void NativeIntegrationManager::setAppCaptureProtected(bool protect) {
    if (m_impl) {
        m_impl->setAppCaptureProtected(protect);
    }
}

} // namespace ssa::workspace
