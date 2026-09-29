#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QDate>
#include "auth/AuthManager.h"

namespace ssa::auth {

class UserQuotaManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int dailyExportCount READ dailyExportCount NOTIFY quotaChanged)
    Q_PROPERTY(int dailyExportLimit READ dailyExportLimit NOTIFY quotaChanged)
    Q_PROPERTY(qint64 maxDurationMs READ maxDurationMs NOTIFY quotaChanged)
    Q_PROPERTY(QString maxDurationFormatted READ maxDurationFormatted NOTIFY quotaChanged)
    Q_PROPERTY(int maxResolutionHeight READ maxResolutionHeight NOTIFY quotaChanged)
    Q_PROPERTY(int remainingDailyExports READ remainingDailyExports NOTIFY quotaChanged)

public:
    explicit UserQuotaManager(AuthManager* authManager, QObject* parent = nullptr);
    ~UserQuotaManager() override = default;

    void setUserManager(class UserManager* userManager) { m_userManager = userManager; }

    int dailyExportCount() const { return m_dailyExportCount; }
    int dailyExportLimit() const;
    qint64 maxDurationMs() const;
    QString maxDurationFormatted() const;
    int maxResolutionHeight() const;
    int remainingDailyExports() const;

    Q_INVOKABLE bool checkExportAllowed(qint64 durationMs, int height);
    Q_INVOKABLE QString getExportBlockingReason(qint64 durationMs, int height);
    Q_INVOKABLE void recordExport(qint64 durationMs);

signals:
    void quotaChanged();
    void exportBlocked(const QString& title, const QString& message);

private slots:
    void onAuthAccountTierChanged();

private:
    void checkAndResetDailyCounter();
    void loadQuotaData();
    void saveQuotaData();

    AuthManager* m_authManager = nullptr;
    class UserManager* m_userManager = nullptr;
    int m_dailyExportCount = 0;
    QDate m_lastExportDate;
    QSettings m_settings;
};

} // namespace ssa::auth
