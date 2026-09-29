#include "auth/UserQuotaManager.h"
#include "auth/UserManager.h"
#include "core/logging/Logger.h"

using namespace ssa::core;

namespace ssa::auth {

UserQuotaManager::UserQuotaManager(AuthManager* authManager, QObject* parent)
    : QObject(parent),
      m_authManager(authManager),
      m_settings("ScreenStudio", "SSA_Quota") {
    loadQuotaData();
    checkAndResetDailyCounter();

    if (m_authManager) {
        connect(m_authManager, &AuthManager::accountTierChanged, this, &UserQuotaManager::onAuthAccountTierChanged);
    }
}

int UserQuotaManager::dailyExportLimit() const {
    if (!m_authManager) return 2;
    if (m_authManager->isAdmin() || m_authManager->accountTier() == AuthManager::AccountTier::Pro) {
        return -1; // Unlimited for Admin & Pro
    }

    if (m_authManager->isLoggedIn() && m_userManager) {
        QString email = m_authManager->userEmail();
        QString tenantId = m_userManager->getUserTenant(email);
        if (!tenantId.isEmpty() && tenantId != "main_tenant" && tenantId != "Solo") {
            QVariantMap limits = m_userManager->getTenantLimits(tenantId);
            return limits.value("dailyLimit", 10).toInt();
        }
    }

    if (m_userManager) {
        QVariantMap soloPolicy = m_userManager->getSoloPolicy();
        return soloPolicy.value("dailyLimit", 2).toInt();
    }

    return 2;
}

qint64 UserQuotaManager::maxDurationMs() const {
    if (!m_authManager) return 120000; // 2 min default
    if (m_authManager->isAdmin() || m_authManager->accountTier() == AuthManager::AccountTier::Pro) {
        return -1; // Unlimited for Admin & Pro
    }

    int mins = 2;
    if (m_authManager->isLoggedIn() && m_userManager) {
        QString email = m_authManager->userEmail();
        QString tenantId = m_userManager->getUserTenant(email);
        if (!tenantId.isEmpty() && tenantId != "main_tenant" && tenantId != "Solo") {
            QVariantMap limits = m_userManager->getTenantLimits(tenantId);
            mins = limits.value("maxDurationMins", 10).toInt();
        } else {
            QVariantMap soloPolicy = m_userManager->getSoloPolicy();
            mins = soloPolicy.value("maxDurationMins", 2).toInt();
        }
    } else if (m_userManager) {
        QVariantMap soloPolicy = m_userManager->getSoloPolicy();
        mins = soloPolicy.value("maxDurationMins", 2).toInt();
    }

    if (mins < 0) return -1;
    return static_cast<qint64>(mins) * 60000;
}

QString UserQuotaManager::maxDurationFormatted() const {
    qint64 maxMs = maxDurationMs();
    if (maxMs < 0) return "Unlimited";
    int minutes = static_cast<int>(maxMs / 60000);
    return QString("%1 Min").arg(minutes);
}

int UserQuotaManager::maxResolutionHeight() const {
    if (!m_authManager) return 1080;
    if (m_authManager->isAdmin() || m_authManager->accountTier() == AuthManager::AccountTier::Pro) {
        return 2160; // 4K for Admin & Pro
    }

    if (m_authManager->isLoggedIn() && m_userManager) {
        QString email = m_authManager->userEmail();
        QString tenantId = m_userManager->getUserTenant(email);
        if (!tenantId.isEmpty() && tenantId != "main_tenant" && tenantId != "Solo") {
            QVariantMap limits = m_userManager->getTenantLimits(tenantId);
            return limits.value("maxResolutionHeight", 2160).toInt();
        }
    }

    if (m_userManager) {
        QVariantMap soloPolicy = m_userManager->getSoloPolicy();
        return soloPolicy.value("maxResolutionHeight", 1080).toInt();
    }

    return 1080;
}

int UserQuotaManager::remainingDailyExports() const {
    int limit = dailyExportLimit();
    if (limit < 0) return 999;
    int remaining = limit - m_dailyExportCount;
    return remaining > 0 ? remaining : 0;
}

bool UserQuotaManager::checkExportAllowed(qint64 durationMs, int height) {
    checkAndResetDailyCounter();
    
    if (m_authManager && m_authManager->accountTier() == AuthManager::AccountTier::Pro) {
        return true;
    }

    int limit = dailyExportLimit();
    if (limit >= 0 && m_dailyExportCount >= limit) {
        QString title = "Daily Export Limit Reached";
        QString msg = QString("You have reached your limit of %1 daily exports for your %2 account.\nSign in with Google or upgrade to Pro for higher limits.")
                        .arg(limit)
                        .arg(m_authManager ? m_authManager->accountTierName() : "Guest");
        emit exportBlocked(title, msg);
        return false;
    }

    qint64 maxMs = maxDurationMs();
    if (maxMs >= 0 && durationMs > maxMs) {
        int maxMins = static_cast<int>(maxMs / 60000);
        int currentMins = static_cast<int>(durationMs / 60000) + 1;
        QString title = "Video Length Limit Exceeded";
        QString msg = QString("Your video length is ~%1 min, but the maximum allowed export duration for your %2 plan is %3 min.\nSign in with Google or upgrade to Pro to unlock longer exports.")
                        .arg(currentMins)
                        .arg(m_authManager ? m_authManager->accountTierName() : "Guest")
                        .arg(maxMins);
        emit exportBlocked(title, msg);
        return false;
    }

    int maxRes = maxResolutionHeight();
    if (maxRes > 0 && height > maxRes) {
        QString title = "Resolution Limit Exceeded";
        QString msg = QString("Exporting at %1p requires a Pro plan. Your current %2 plan is limited to %3p.")
                        .arg(height)
                        .arg(m_authManager ? m_authManager->accountTierName() : "Guest")
                        .arg(maxRes);
        emit exportBlocked(title, msg);
        return false;
    }

    return true;
}

QString UserQuotaManager::getExportBlockingReason(qint64 durationMs, int height) {
    checkAndResetDailyCounter();

    if (m_authManager && m_authManager->accountTier() == AuthManager::AccountTier::Pro) {
        return "";
    }

    int limit = dailyExportLimit();
    if (limit >= 0 && m_dailyExportCount >= limit) {
        return QString("Daily export limit reached (%1/%2 exports used today)").arg(m_dailyExportCount).arg(limit);
    }

    qint64 maxMs = maxDurationMs();
    if (maxMs >= 0 && durationMs > maxMs) {
        int maxMins = static_cast<int>(maxMs / 60000);
        return QString("Video duration exceeds your plan limit (%1 min max)").arg(maxMins);
    }

    int maxRes = maxResolutionHeight();
    if (maxRes > 0 && height > maxRes) {
        return QString("Target resolution %1p exceeds plan limit (%2p max)").arg(height).arg(maxRes);
    }

    return "";
}

void UserQuotaManager::recordExport(qint64 durationMs) {
    Q_UNUSED(durationMs);
    checkAndResetDailyCounter();
    m_dailyExportCount++;
    saveQuotaData();
    emit quotaChanged();
    logging::Logger::info("Recorded export. Daily export count is now: " + std::to_string(m_dailyExportCount));
}

void UserQuotaManager::onAuthAccountTierChanged() {
    emit quotaChanged();
}

void UserQuotaManager::checkAndResetDailyCounter() {
    QDate today = QDate::currentDate();
    if (m_lastExportDate != today) {
        m_dailyExportCount = 0;
        m_lastExportDate = today;
        saveQuotaData();
        emit quotaChanged();
    }
}

void UserQuotaManager::loadQuotaData() {
    m_dailyExportCount = m_settings.value("dailyExportCount", 0).toInt();
    QString dateStr = m_settings.value("lastExportDate", "").toString();
    m_lastExportDate = QDate::fromString(dateStr, Qt::ISODate);
    if (!m_lastExportDate.isValid()) {
        m_lastExportDate = QDate::currentDate();
    }
}

void UserQuotaManager::saveQuotaData() {
    m_settings.setValue("dailyExportCount", m_dailyExportCount);
    m_settings.setValue("lastExportDate", m_lastExportDate.toString(Qt::ISODate));
    m_settings.sync();
}

} // namespace ssa::auth
