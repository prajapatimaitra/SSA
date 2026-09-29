#pragma once

#include <QObject>
#include <QString>

namespace ssa::workspace {

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    explicit ClipboardManager(QObject* parent = nullptr);

    /**
     * Copies an image from a local file path or file:// URL to the system clipboard.
     * @param localFileUrl The local path or URL to the image file.
     * @return true if successful, false otherwise.
     */
    Q_INVOKABLE bool copyImageToClipboard(const QString& localFileUrl);
};

} // namespace ssa::workspace
