#include "ztelegramservice.h"

using namespace std::chrono_literals;

ZTelegramService::ZTelegramService(QSettings &settings, QObject *parent) : QObject(parent)
  , m_reply(nullptr)
  , m_offset(0)
  , m_finished(true)
{
    m_http = settings.value("telegram/http").toString();
    m_prefix = settings.value("telegram/prefix").toString();
}
ZTelegramService::~ZTelegramService() = default;

void ZTelegramService::check()
{
    if(!hasFinished())
    {
        auto start = std::chrono::system_clock::now();
        auto diff = start - m_lasttime;
        if(diff > 10s)
        {
            if(m_reply)
            {
                m_reply->disconnect();
                delete m_reply;
                m_reply = nullptr;
            }
            m_data = QJsonDocument::fromJson("{\"ok\":false}");
            m_finished = true;
        }
    }
    else
    {
        if(m_messages.count() > 0)
        {
            const auto msg = m_messages.front();
            startMessage(msg);
            m_messages.pop_front();
        }
        else
        {
            if(m_data["ok"].toBool())
            {
                auto v = m_data["result"];
                if(v.isUndefined())
                    m_offset = 0;
                else
                {
                    if(v.isArray() && v.toArray().count() > 0)
                    {
                        m_offset = v[0]["update_id"].toInteger() + 1;
                        auto msg = v[0]["message"];
                        auto c = msg["text"];
                        if(c.isString())
                        {
                            auto cmd = c.toString();
                            addMessage(QString("I've received %1 command from %2 %3").arg(cmd.mid(1)).arg(msg["from"]["first_name"].toString()).arg(msg["from"]["last_name"].toString()));
                            emit requestStatus(cmd);
                        }
                    }
                }
            }
            startRequest(m_offset > 0 ? QString("getUpdates?offset=%1").arg(m_offset): "getUpdates");
        }
    }
}
void ZTelegramService::addMessage(const QString &message)
{
    m_messages.append(message);
}

void ZTelegramService::startMessage(const QString &message)
{
    startRequest(m_prefix + message);
}
void ZTelegramService::startRequest(const QString &request)
{
    if(m_reply)
    {
       m_reply->disconnect();
        delete m_reply;
        m_reply = nullptr;
    }
    m_data = QJsonDocument::fromJson("{\"ok\":false}");
    connect(&m_qnam, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *, QAuthenticator *authenticator)
    {
        authenticator->setUser("user");
        authenticator->setPassword("password");
    });
    QUrl url = QUrl(m_http + request);
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    // Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
    req.setRawHeader("User-Agent", "ZControl v.0.1");
    req.setRawHeader("X-Custom-User-Agent", "ZControl v.0.1");
    req.setRawHeader("Content-Type", "application/json");

    m_reply = m_qnam.get(req);

    if(m_reply)
    {
        connect(m_reply, &QIODevice::readyRead, [&]()
        {
            m_data = QJsonDocument::fromJson(m_reply->readAll());
        });


    #if QT_CONFIG(ssl)
        connect(m_reply, &QNetworkReply::sslErrors, [&](const QList<QSslError> &errors)
        {
            m_reply->ignoreSslErrors();
         });
    #endif
        connect(m_reply, &QNetworkReply::finished, [&]()
        {
            m_reply->disconnect();
            delete m_reply;
            m_reply = nullptr;
            m_finished = true;
        });
    }
}

