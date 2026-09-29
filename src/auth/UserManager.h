#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

namespace ssa::auth {

class UserManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int totalUsersCount READ totalUsersCount NOTIFY usersChanged)
    Q_PROPERTY(int proUsersCount READ proUsersCount NOTIFY usersChanged)
    Q_PROPERTY(int freeUsersCount READ freeUsersCount NOTIFY usersChanged)
    Q_PROPERTY(int totalExportsCount READ totalExportsCount NOTIFY usersChanged)
    Q_PROPERTY(int tenantCount READ tenantCount NOTIFY usersChanged)

public:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager() override = default;

    void initialize(const QString& dbPath);

    int totalUsersCount() const { return m_totalUsersCount; }
    int proUsersCount() const { return m_proUsersCount; }
    int freeUsersCount() const { return m_freeUsersCount; }
    int totalExportsCount() const { return m_totalExportsCount; }
    int tenantCount() const { return m_tenantCount; }

    Q_INVOKABLE QVariantList getUsersList();
    Q_INVOKABLE QVariantList getSoloUsersList();
    Q_INVOKABLE QVariantList getTenantsList();
    Q_INVOKABLE QVariantList getTenantUsers(const QString& tenantId);
    Q_INVOKABLE QVariantList getTenantsSummary();
    Q_INVOKABLE bool setUserTier(const QString& email, int tier);
    Q_INVOKABLE bool setUserTenant(const QString& email, const QString& tenantId);
    Q_INVOKABLE bool createTenant(const QString& tenantName);
    Q_INVOKABLE bool deleteUser(const QString& email);
    Q_INVOKABLE void registerOrUpdateUser(const QString& email, const QString& username, const QString& passwordHash, int tier, const QString& tenantId = "main_tenant");
    Q_INVOKABLE int getUserTier(const QString& email);
    Q_INVOKABLE QString getUserTenant(const QString& email);
    Q_INVOKABLE void recordUserExport(const QString& email);
    Q_INVOKABLE QVariantMap getTenantLimits(const QString& tenantId);
    Q_INVOKABLE bool setTenantLimits(const QString& tenantId, int dailyLimit, int maxDurationMins, int maxResolutionHeight);
    Q_INVOKABLE QVariantMap getSoloPolicy();
    Q_INVOKABLE bool setSoloPolicy(int dailyLimit, int maxDurationMins, int maxResolutionHeight);
    Q_INVOKABLE void refreshStats();
    Q_INVOKABLE void seedDemoData();

signals:
    void usersChanged();
    void userTierUpdated(const QString& email, int tier);

private:
    void createTables();

    int m_totalUsersCount = 0;
    int m_proUsersCount = 0;
    int m_freeUsersCount = 0;
    int m_totalExportsCount = 0;
    int m_tenantCount = 0;
    QString m_dbPath;
};

} // namespace ssa::auth
