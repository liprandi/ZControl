#ifndef ZHTTPSERVICE_H
#define ZHTTPSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QScopedPointerDeleteLater>
#include <QUrl>
#include <QSettings>

class ZHttpService : public QObject
{
    Q_OBJECT
public:
    explicit ZHttpService(QSettings &settings, QObject *parent = nullptr);
    ~ZHttpService();

    void startRequest();
private slots:

public:
    const QByteArray& data(){return m_data;}
    void check();
    bool hasFinished(){return m_finished;}
signals:

private:
    QUrl m_url;
    QNetworkAccessManager qnam;
    QNetworkReply* m_reply;
    QByteArray m_data;
    QString    m_http;
    std::chrono::time_point<std::chrono::system_clock> m_lasttime;
    bool m_finished;
};

#endif // ZHTTPSERVICE_H
