#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpServer>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

namespace ssa::auth {

class AuthManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userProfileChanged)
    Q_PROPERTY(QString userEmail READ userEmail NOTIFY userProfileChanged)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY userProfileChanged)
    Q_PROPERTY(AccountTier accountTier READ accountTier NOTIFY accountTierChanged)
    Q_PROPERTY(QString accountTierName READ accountTierName NOTIFY accountTierChanged)
    Q_PROPERTY(bool isAuthenticating READ isAuthenticating NOTIFY isAuthenticatingChanged)
    Q_PROPERTY(QString authError READ authError NOTIFY authErrorChanged)
    Q_PROPERTY(QString pendingEmail READ pendingEmail NOTIFY pendingEmailChanged)
    Q_PROPERTY(bool requiresUsernameChoice READ requiresUsernameChoice NOTIFY requiresUsernameChoiceChanged)
    Q_PROPERTY(QString senderEmail READ senderEmail WRITE setSenderEmail NOTIFY mailConfigChanged)
    Q_PROPERTY(QString mailApiKey READ mailApiKey WRITE setMailApiKey NOTIFY mailConfigChanged)
    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY isAdminChanged)

public:
    enum class AccountTier {
        Guest = 0,
        Free = 1,
        Pro = 2
    };
    Q_ENUM(AccountTier)

    explicit AuthManager(QObject* parent = nullptr);
    ~AuthManager() override;

    bool isLoggedIn() const { return m_isLoggedIn; }
    bool isAdmin() const { return m_isLoggedIn && (m_userEmail.toLower() == "admin@gmail.com" || m_userName.toLower() == "admin"); }
    QString userName() const { return m_userName; }
    QString userEmail() const { return m_userEmail; }
    QString avatarUrl() const { return m_avatarUrl; }
    AccountTier accountTier() const { return m_accountTier; }
    QString accountTierName() const;
    bool isAuthenticating() const { return m_isAuthenticating; }
    QString authError() const { return m_authError; }
    QString pendingEmail() const { return m_pendingEmail; }
    bool requiresUsernameChoice() const { return m_requiresUsernameChoice; }
    QString senderEmail() const { return m_senderEmail; }
    QString mailApiKey() const { return m_mailApiKey; }

    void setSenderEmail(const QString& email);
    void setMailApiKey(const QString& key);
    void setUserManager(class UserManager* userManager);

    Q_INVOKABLE void loginWithGoogle();
    Q_INVOKABLE bool loginWithAdmin(const QString& usernameOrEmail, const QString& password);
    Q_INVOKABLE bool requestEmailVerification(const QString& email);
    Q_INVOKABLE bool requestEmailVerificationWithPassword(const QString& email, const QString& password);
    Q_INVOKABLE bool verifyCode(const QString& code);
    Q_INVOKABLE bool setCustomUsername(const QString& newUsername);
    Q_INVOKABLE void saveMailConfig(const QString& senderEmail, const QString& apiKey);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void setSimulatedTier(int tierIndex);
    Q_INVOKABLE void clearAuthError();

signals:
    void isLoggedInChanged(bool isLoggedIn);
    void userProfileChanged();
    void accountTierChanged(AccountTier accountTier);
    void isAdminChanged();
    void isAuthenticatingChanged(bool isAuthenticating);
    void authErrorChanged(const QString& error);
    void pendingEmailChanged();
    void requiresUsernameChoiceChanged();
    void mailConfigChanged();
    void verificationCodeSent(const QString& email, const QString& code);
    void loginSuccess(const QString& userName, const QString& userEmail);

private slots:
    void handleNewTcpConnection();
    void onTokenExchangeFinished(QNetworkReply* reply);
    void onUserInfoFinished(QNetworkReply* reply);

private:
    void startLocalRedirectServer();
    void stopLocalRedirectServer();
    void parseAuthorizationCode(const QString& requestHeader);
    void exchangeCodeForToken(const QString& code);
    void fetchUserInfo(const QString& accessToken);
    void saveSession();
    void loadSession();
    void setAuthError(const QString& error);
    void sendEmailCodeToUser(const QString& recipientEmail, const QString& code);

    bool m_isLoggedIn = false;
    QString m_userName;
    QString m_userEmail;
    QString m_userHashedPassword;
    QString m_avatarUrl;
    AccountTier m_accountTier = AccountTier::Guest;
    bool m_isAuthenticating = false;
    QString m_authError;
    bool m_requiresUsernameChoice = false;

    QString m_pendingEmail;
    QString m_pendingHashedPassword;
    QString m_pendingVerificationCode;

    QString m_senderEmail;
    QString m_mailApiKey;

    QTcpServer* m_tcpServer = nullptr;
    quint16 m_serverPort = 8089;
    QString m_codeVerifier;
    QString m_authState;
    
    QNetworkAccessManager* m_networkManager = nullptr;
    class UserManager* m_userManager = nullptr;
    QSettings m_settings;
};

} // namespace ssa::auth
