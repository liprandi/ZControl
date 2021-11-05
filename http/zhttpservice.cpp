#include "zhttpservice.h"

ZHttpService::ZHttpService(QObject *parent) : QObject(parent)
  , m_finished(false)
  , m_reply(nullptr)
{

}
ZHttpService::~ZHttpService() = default;

void ZHttpService::startRequest(const QUrl &requestedUrl)
{
    m_finished = false;
    m_data.clear();
    if(m_reply)
    {
        delete m_reply;
        m_reply = nullptr;
    }

    connect(&qnam, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *, QAuthenticator *authenticator)
    {
        authenticator->setUser("paolo@liprandi.com");
        authenticator->setPassword("0713D0504l");
    });

    QNetworkRequest req(requestedUrl);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    // Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
    req.setRawHeader("User-Agent", "ZControl v.0.1");
    req.setRawHeader("X-Custom-User-Agent", "ZControl v.0.1");
    req.setRawHeader("Content-Type", "text/html");

    m_reply = qnam.get(req);

    if(m_reply)
    {
        connect(m_reply, &QIODevice::readyRead, [&]()
        {
            m_data += m_reply->readAll();
        });


    #if QT_CONFIG(ssl)
        connect(m_reply, &QNetworkReply::sslErrors, [&](const QList<QSslError> &errors)
        {
            m_reply->ignoreSslErrors();
         });
    #endif
        connect(m_reply, &QNetworkReply::finished, [&]()
        {
            QNetworkReply::NetworkError error = m_reply->error();
            delete m_reply;
            m_reply = nullptr;
            if(error != QNetworkReply::NoError)
            {
                m_data += "Error in http request: ";
            }
            m_finished = true;
        });
    }
}

void ZHttpService::cancelDownload()
{
    m_reply->abort();
}
