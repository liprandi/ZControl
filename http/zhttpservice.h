#ifndef ZHTTPSERVICE_H
#define ZHTTPSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QScopedPointerDeleteLater>
#include <QUrl>
#include <QFile>

class ZHttpService : public QObject
{
    Q_OBJECT
public:
    explicit ZHttpService(QObject *parent = nullptr);
    ~ZHttpService();

    void startRequest(const QUrl &requestedUrl);
private slots:
    void cancelDownload();
public:
    const QByteArray& data(){return m_data;}
    bool hasFinished(){return m_finished;}
signals:

private:
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply* m_reply;
    QByteArray m_data;
    bool m_finished;
};

#endif // ZHTTPSERVICE_H
