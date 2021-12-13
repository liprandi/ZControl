#include "zhttpservice.h"

using namespace std::chrono_literals;

ZHttpService::ZHttpService(QSettings &settings, QObject *parent) : QObject(parent)
  , m_finished(true)
  , m_reply(nullptr)
  , m_lasttime(std::chrono::system_clock::now())
{
    m_http = settings.value("server/http").toString();
    m_url = QUrl(m_http);
}
ZHttpService::~ZHttpService() = default;

void ZHttpService::check()
{
    auto start = std::chrono::system_clock::now();
    auto diff = start - m_lasttime;

    if(hasFinished() && diff > 3s)
    {
        m_lasttime = start;
        qnam.clearAccessCache();
        qnam.clearConnectionCache();
        startRequest();
    }
}

void ZHttpService::startRequest()
{
    m_data.clear();

    connect(&qnam, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *, QAuthenticator *authenticator)
    {
        authenticator->setUser("paolo@liprandi.com");
        authenticator->setPassword("0713D0504l");
    });

    QNetworkRequest req(m_url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    // Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
    req.setRawHeader("User-Agent", "ZControl v.0.1");
    req.setRawHeader("X-Custom-User-Agent", "ZControl v.0.1");
    req.setRawHeader("Content-Type", "application/json");

    m_reply = qnam.get(req);

    if(m_reply)
    {
        m_finished = false;
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
            m_lasttime = std::chrono::system_clock::now();
        });
    }
    else
    {
        m_finished = true;
        m_lasttime = std::chrono::system_clock::now();
    }
}

