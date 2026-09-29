#include "workspace/ClipboardManager.h"
#include "core/logging/Logger.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QImage>
#include <QUrl>
#include <QFileInfo>

namespace ssa::workspace {

ClipboardManager::ClipboardManager(QObject* parent) : QObject(parent) {
}

bool ClipboardManager::copyImageToClipboard(const QString& localFileUrl) {
    QString filePath = localFileUrl;
    
    // Handle file:// protocol if present
    if (filePath.startsWith("file://")) {
        QUrl url(filePath);
        filePath = url.toLocalFile();
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        core::logging::Logger::error("ClipboardManager: File does not exist: " + filePath.toStdString());
        return false;
    }
    
    QImage image(filePath);
    if (image.isNull()) {
        core::logging::Logger::error("ClipboardManager: Failed to load image from: " + filePath.toStdString());
        return false;
    }
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setImage(image);
        core::logging::Logger::info("ClipboardManager: Copied image to clipboard.");
        return true;
    }
    
    core::logging::Logger::error("ClipboardManager: Clipboard is not available.");
    return false;
}

} // namespace ssa::workspace
