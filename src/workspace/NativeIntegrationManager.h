#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include "interfaces/INativeIntegration.h"

namespace ssa::workspace {

class NativeIntegrationManager : public QObject {
    Q_OBJECT

public:
    explicit NativeIntegrationManager(QObject* parent = nullptr);
    ~NativeIntegrationManager() override;

    // Singleton access if needed from C++
    static NativeIntegrationManager* instance();

    Q_INVOKABLE void revealInOS(const QString& path);
    Q_INVOKABLE void shareFile(const QString& path);
    Q_INVOKABLE void setWindowCaptureProtected(QObject* windowObject = nullptr, bool protect = true);
    Q_INVOKABLE void setAppCaptureProtected(bool protect = true);

private:
    std::unique_ptr<interfaces::INativeIntegration> m_impl;
};

} // namespace ssa::workspace
