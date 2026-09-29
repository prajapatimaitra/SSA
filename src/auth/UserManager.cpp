#include "auth/UserManager.h"
#include "core/logging/Logger.h"
#include <QSqlRecord>

using namespace ssa::core;

namespace ssa::auth {

UserManager::UserManager(QObject* parent)
    : QObject(parent) {
}

void UserManager::initialize(const QString& dbPath) {
    m_dbPath = dbPath;
    
    QSqlDatabase db;
    if (QSqlDatabase::contains("UserDatabaseConnection")) {
        db = QSqlDatabase::database("UserDatabaseConnection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "UserDatabaseConnection");
        db.setDatabaseName(m_dbPath);
    }

    if (!db.open()) {
        logging::Logger::error("Failed to open user database at: " + m_dbPath.toStdString() + " Error: " + db.lastError().text().toStdString());
        return;
    }

    logging::Logger::info("User Database opened successfully at: " + m_dbPath.toStdString());
    createTables();
    refreshStats();
}

void UserManager::createTables() {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);

    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT UNIQUE NOT NULL,
            username TEXT NOT NULL,
            password_hash TEXT,
            tier INTEGER DEFAULT 1,
            tenant_id TEXT DEFAULT 'main_tenant',
            registered_at TEXT,
            last_active_at TEXT,
            total_exports INTEGER DEFAULT 0
        );
    )";

    if (!query.exec(createTableQuery)) {
        logging::Logger::error("Failed to create users table: " + query.lastError().text().toStdString());
    } else {
        logging::Logger::info("Users database table verified/created.");
    }

    QString createTenantsConfigQuery = R"(
        CREATE TABLE IF NOT EXISTS tenants_config (
            tenant_id TEXT PRIMARY KEY,
            daily_export_limit INTEGER DEFAULT 10,
            max_duration_mins INTEGER DEFAULT 10,
            max_resolution_height INTEGER DEFAULT 2160
        );
    )";
    query.exec(createTenantsConfigQuery);

    // Migration fallback for existing tables
    query.exec("ALTER TABLE users ADD COLUMN tenant_id TEXT DEFAULT 'main_tenant'");

    seedDemoData();
}

void UserManager::registerOrUpdateUser(const QString& email, const QString& username, const QString& passwordHash, int tier, const QString& tenantId) {
    if (email.trimmed().isEmpty()) return;

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id, username, tier FROM users WHERE email = :email");
    checkQuery.bindValue(":email", email.trimmed());

    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (checkQuery.exec() && checkQuery.next()) {
        // User exists -> update last active timestamp
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE users SET last_active_at = :now, username = CASE WHEN :username != '' THEN :username ELSE username END WHERE email = :email");
        updateQuery.bindValue(":now", now);
        updateQuery.bindValue(":username", username);
        updateQuery.bindValue(":email", email.trimmed());
        updateQuery.exec();
    } else {
        // New User -> Insert
        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO users (email, username, password_hash, tier, registered_at, last_active_at, total_exports) "
                            "VALUES (:email, :username, :hash, :tier, :now, :now, 0)");
        insertQuery.bindValue(":email", email.trimmed());
        insertQuery.bindValue(":username", username.isEmpty() ? email.section('@', 0, 0) : username);
        insertQuery.bindValue(":hash", passwordHash);
        insertQuery.bindValue(":tier", tier);
        insertQuery.bindValue(":now", now);
        
        if (!insertQuery.exec()) {
            logging::Logger::error("Failed to register new user: " + insertQuery.lastError().text().toStdString());
        } else {
            logging::Logger::info("Registered new user in database: " + email.toStdString());
        }
    }

    refreshStats();
}

int UserManager::getUserTier(const QString& email) {
    if (email.trimmed().isEmpty()) return 1; // Default Free

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT tier FROM users WHERE email = :email");
    query.bindValue(":email", email.trimmed());

    if (query.exec() && query.next()) {
        return query.value("tier").toInt();
    }
    return 1;
}

QString UserManager::getUserTenant(const QString& email) {
    if (email.trimmed().isEmpty()) return "main_tenant";

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT COALESCE(tenant_id, 'main_tenant') AS tenant_id FROM users WHERE email = :email");
    query.bindValue(":email", email.trimmed());

    if (query.exec() && query.next()) {
        return query.value("tenant_id").toString();
    }
    return "main_tenant";
}

bool UserManager::setUserTier(const QString& email, int tier) {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("UPDATE users SET tier = :tier WHERE email = :email");
    query.bindValue(":tier", tier);
    query.bindValue(":email", email.trimmed());

    bool ok = query.exec();
    if (ok) {
        logging::Logger::info("Updated tier for user " + email.toStdString() + " to " + std::to_string(tier));
        emit userTierUpdated(email.trimmed(), tier);
        refreshStats();
    }
    return ok;
}

bool UserManager::deleteUser(const QString& email) {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE email = :email");
    query.bindValue(":email", email.trimmed());

    bool ok = query.exec();
    if (ok) {
        logging::Logger::info("Deleted user from database: " + email.toStdString());
        refreshStats();
    }
    return ok;
}

void UserManager::recordUserExport(const QString& email) {
    if (email.trimmed().isEmpty()) return;

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("UPDATE users SET total_exports = total_exports + 1, last_active_at = :now WHERE email = :email");
    query.bindValue(":now", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":email", email.trimmed());
    query.exec();

    refreshStats();
}

QVariantList UserManager::getUsersList() {
    QVariantList list;
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT email, username, tier, COALESCE(tenant_id, 'main_tenant') AS tenant_id, registered_at, last_active_at, total_exports FROM users ORDER BY registered_at DESC");

    if (query.exec()) {
        while (query.next()) {
            QVariantMap map;
            map["email"] = query.value("email").toString();
            map["username"] = query.value("username").toString();
            map["tier"] = query.value("tier").toInt();
            map["tierName"] = query.value("tier").toInt() == 2 ? "Pro" : (query.value("tier").toInt() == 1 ? "Free" : "Guest");
            map["tenantId"] = query.value("tenant_id").toString();
            map["registeredAt"] = query.value("registered_at").toString();
            map["lastActiveAt"] = query.value("last_active_at").toString();
            map["totalExports"] = query.value("total_exports").toInt();
            list.append(map);
        }
    }
    return list;
}

QVariantList UserManager::getSoloUsersList() {
    QVariantList list;
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT email, username, tier, COALESCE(tenant_id, 'main_tenant') AS tenant_id, registered_at, last_active_at, total_exports FROM users WHERE tenant_id IS NULL OR tenant_id = '' OR tenant_id = 'main_tenant' OR tenant_id = 'Solo' ORDER BY registered_at DESC");

    if (query.exec()) {
        while (query.next()) {
            QVariantMap map;
            map["email"] = query.value("email").toString();
            map["username"] = query.value("username").toString();
            map["tier"] = query.value("tier").toInt();
            map["tierName"] = query.value("tier").toInt() == 2 ? "Pro" : (query.value("tier").toInt() == 1 ? "Free" : "Guest");
            map["tenantId"] = query.value("tenant_id").toString();
            map["registeredAt"] = query.value("registered_at").toString();
            map["lastActiveAt"] = query.value("last_active_at").toString();
            map["totalExports"] = query.value("total_exports").toInt();
            list.append(map);
        }
    }
    return list;
}

QVariantList UserManager::getTenantsList() {
    QVariantList list;
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT COALESCE(tenant_id, 'main_tenant') AS tenant_id, 
               COUNT(*) AS user_count, 
               SUM(CASE WHEN tier = 2 THEN 1 ELSE 0 END) AS pro_count,
               SUM(total_exports) AS exports_count
        FROM users 
        WHERE tenant_id NOT IN ('main_tenant', 'Solo', '') AND tenant_id IS NOT NULL
        GROUP BY tenant_id 
        ORDER BY user_count DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            QVariantMap map;
            map["tenantId"] = query.value("tenant_id").toString();
            map["tenantName"] = query.value("tenant_id").toString();
            map["userCount"] = query.value("user_count").toInt();
            map["proCount"] = query.value("pro_count").toInt();
            map["exportsCount"] = query.value("exports_count").toInt();
            list.append(map);
        }
    }
    return list;
}

QVariantList UserManager::getTenantUsers(const QString& tenantId) {
    QVariantList list;
    if (tenantId.isEmpty()) return list;

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT email, username, tier, COALESCE(tenant_id, 'main_tenant') AS tenant_id, registered_at, last_active_at, total_exports FROM users WHERE tenant_id = :tenantId ORDER BY registered_at DESC");
    query.bindValue(":tenantId", tenantId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap map;
            map["email"] = query.value("email").toString();
            map["username"] = query.value("username").toString();
            map["tier"] = query.value("tier").toInt();
            map["tierName"] = query.value("tier").toInt() == 2 ? "Pro" : (query.value("tier").toInt() == 1 ? "Free" : "Guest");
            map["tenantId"] = query.value("tenant_id").toString();
            map["registeredAt"] = query.value("registered_at").toString();
            map["lastActiveAt"] = query.value("last_active_at").toString();
            map["totalExports"] = query.value("total_exports").toInt();
            list.append(map);
        }
    }
    return list;
}

bool UserManager::createTenant(const QString& tenantName) {
    QString trimmed = tenantName.trimmed();
    if (trimmed.isEmpty()) return false;

    // Seed a placeholder admin/owner for the new tenant so it registers immediately
    QString placeholderEmail = QString("admin@%1.com").arg(trimmed.toLower().remove(' '));
    QString placeholderUser = QString("%1 Admin").arg(trimmed);
    registerOrUpdateUser(placeholderEmail, placeholderUser, "tenant_pass", 2, trimmed);
    logging::Logger::info("Created new tenant organization: " + trimmed.toStdString());
    return true;
}

QVariantList UserManager::getTenantsSummary() {
    return getTenantsList();
}

bool UserManager::setUserTenant(const QString& email, const QString& tenantId) {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("UPDATE users SET tenant_id = :tenantId WHERE email = :email");
    query.bindValue(":tenantId", tenantId.trimmed().isEmpty() ? "main_tenant" : tenantId.trimmed());
    query.bindValue(":email", email.trimmed());

    bool ok = query.exec();
    if (ok) {
        logging::Logger::info("Updated tenant for user " + email.toStdString() + " to " + tenantId.toStdString());
        refreshStats();
    }
    return ok;
}

void UserManager::seedDemoData() {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery countQuery(db);
    if (countQuery.exec("SELECT COUNT(*) FROM users") && countQuery.next()) {
        if (countQuery.value(0).toInt() >= 3) {
            // Already populated with users
            return;
        }
    }

    logging::Logger::info("Seeding realistic Multi-Tenant demo data for Admin management...");

    struct SeedUser {
        QString email;
        QString username;
        int tier;
        QString tenantId;
        int exports;
    };

    QList<SeedUser> seedUsers = {
        {"john.doe@acmecorp.com", "John Doe (Owner)", 2, "Acme Corp", 14},
        {"sarah.smith@acmecorp.com", "Sarah Smith", 1, "Acme Corp", 3},
        {"mark.tech@acmecorp.com", "Mark Tech", 1, "Acme Corp", 1},
        {"alex.creator@techlabs.io", "Alex Creator (Owner)", 2, "TechLabs Inc", 28},
        {"lisa.v@techlabs.io", "Lisa Vance", 2, "TechLabs Inc", 9},
        {"david.dev@devstudio.org", "David Dev", 1, "DevStudio", 4},
        {"emma.design@devstudio.org", "Emma Design", 1, "DevStudio", 2},
        {"user@gmail.com", "Demo User", 1, "Main Tenant", 5},
        {"admin@gmail.com", "Admin", 2, "Main Tenant", 12}
    };

    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    for (const auto& su : seedUsers) {
        QSqlQuery insert(db);
        insert.prepare("INSERT OR REPLACE INTO users (email, username, password_hash, tier, tenant_id, registered_at, last_active_at, total_exports) "
                       "VALUES (:email, :username, 'demo_hash', :tier, :tenant, :now, :now, :exports)");
        insert.bindValue(":email", su.email);
        insert.bindValue(":username", su.username);
        insert.bindValue(":tier", su.tier);
        insert.bindValue(":tenant", su.tenantId);
        insert.bindValue(":now", now);
        insert.bindValue(":exports", su.exports);
        insert.exec();
    }

    refreshStats();
}

void UserManager::refreshStats() {
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);

    m_totalUsersCount = 0;
    m_proUsersCount = 0;
    m_freeUsersCount = 0;
    m_totalExportsCount = 0;
    m_tenantCount = 0;

    if (query.exec("SELECT COUNT(*) FROM users")) {
        if (query.next()) m_totalUsersCount = query.value(0).toInt();
    }

    if (query.exec("SELECT COUNT(*) FROM users WHERE tier = 2")) {
        if (query.next()) m_proUsersCount = query.value(0).toInt();
    }

    if (query.exec("SELECT COUNT(*) FROM users WHERE tier = 1")) {
        if (query.next()) m_freeUsersCount = query.value(0).toInt();
    }

    if (query.exec("SELECT SUM(total_exports) FROM users")) {
        if (query.next()) m_totalExportsCount = query.value(0).toInt();
    }

    if (query.exec("SELECT COUNT(DISTINCT tenant_id) FROM users")) {
        if (query.next()) m_tenantCount = query.value(0).toInt();
    }

    emit usersChanged();
}

QVariantMap UserManager::getTenantLimits(const QString& tenantId) {
    QVariantMap map;
    QString targetTenant = tenantId.trimmed().isEmpty() ? "main_tenant" : tenantId.trimmed();
    
    // Default fallback limits
    map["dailyLimit"] = 10;
    map["maxDurationMins"] = 10;
    map["maxResolutionHeight"] = 2160;

    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare("SELECT daily_export_limit, max_duration_mins, max_resolution_height FROM tenants_config WHERE tenant_id = :tenantId");
    query.bindValue(":tenantId", targetTenant);

    if (query.exec() && query.next()) {
        map["dailyLimit"] = query.value("daily_export_limit").toInt();
        map["maxDurationMins"] = query.value("max_duration_mins").toInt();
        map["maxResolutionHeight"] = query.value("max_resolution_height").toInt();
    }

    return map;
}

bool UserManager::setTenantLimits(const QString& tenantId, int dailyLimit, int maxDurationMins, int maxResolutionHeight) {
    QString targetTenant = tenantId.trimmed().isEmpty() ? "main_tenant" : tenantId.trimmed();
    QSqlDatabase db = QSqlDatabase::database("UserDatabaseConnection");
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT OR REPLACE INTO tenants_config (tenant_id, daily_export_limit, max_duration_mins, max_resolution_height)
        VALUES (:tenantId, :dailyLimit, :maxDurationMins, :maxResHeight)
    )");
    query.bindValue(":tenantId", targetTenant);
    query.bindValue(":dailyLimit", dailyLimit);
    query.bindValue(":maxDurationMins", maxDurationMins);
    query.bindValue(":maxResHeight", maxResolutionHeight);

    bool ok = query.exec();
    if (ok) {
        logging::Logger::info("Updated custom export limits for tenant: " + targetTenant.toStdString());
        emit usersChanged();
    }
    return ok;
}

QVariantMap UserManager::getSoloPolicy() {
    return getTenantLimits("SOLO_GLOBAL_POLICY");
}

bool UserManager::setSoloPolicy(int dailyLimit, int maxDurationMins, int maxResolutionHeight) {
    return setTenantLimits("SOLO_GLOBAL_POLICY", dailyLimit, maxDurationMins, maxResolutionHeight);
}

} // namespace ssa::auth
