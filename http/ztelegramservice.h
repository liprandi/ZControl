#ifndef ZTELEGRAMSERVICE_H
#define ZTELEGRAMSERVICE_H

#include <QObject>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QScopedPointerDeleteLater>
#include <QUrl>
#include <chrono>

class ZTelegramService : public QObject
{
    Q_OBJECT
public:
    explicit ZTelegramService(QSettings &settings, QObject *parent = nullptr);
    ~ZTelegramService();
    void check();
    bool hasFinished()
    {
        return m_finished;
    }

public slots:
    void addMessage(const QString& message);
private:
    void startMessage(const QString& message);
    void startRequest(const QString& request);
public:
    const QJsonDocument& data(){return m_data;}
signals:
    void requestStatus(const QString& cmd);
private:
    QString                 m_http;
    QString                 m_prefix;
    QNetworkAccessManager   m_qnam;
    QNetworkReply*          m_reply;
    QJsonDocument           m_data;
    QStringList             m_messages;
    std::chrono::time_point<std::chrono::system_clock> m_lasttime;
    long long               m_offset;
    bool                    m_finished;
};

#endif // ZHTTPSERVICE_H
