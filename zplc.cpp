#include "zplc.h"

ZPlc::ZPlc(QObject *parent) :
    QThread()
  , m_plc(nullptr)
{
    m_run = false;
    m_quit = false;
}

ZPlc::~ZPlc()
{
    m_quit = true;
    while(m_run)
        msleep(100);
    if(m_plc)
        delete m_plc;
}
ZPlc::In::In(int id, const QByteArray &tag, int len, std::chrono::duration<__int64, std::milli> msec):
    m_id(id)
  , m_tag(tag)
  , m_len(len)
  , m_time(msec)
{
    m_last =  std::chrono::steady_clock::now();
    m_last -= m_time;
    m_data = QByteArray(len, '\0');
}
ZPlc::In::~In() = default;

ZPlc::Out::Out(const QByteArray& tag, CIP_TYPE type, const QByteArray& data):
      m_tag(tag)
    , m_type(type)
    , m_data(data)
{
}
ZPlc::Out::~Out() = default;

void ZPlc::setAddress(const QString& ip, int backplane, int slot)
{
    m_ip = ip;
    m_backplane = backplane;
    m_slot = slot;
    start();
}

void ZPlc::setAreaIn(int id, const QByteArray &tag, int len, std::chrono::duration<__int64, std::milli> msec)
{
    const std::lock_guard<std::mutex> lock(m_mutexRead);
    m_in.push_back(new In(id, tag, len, msec));
}

void ZPlc::writeData(const QByteArray& tag, CIP_TYPE type, const QByteArray& data)
{
    const std::lock_guard<std::mutex> lock(m_mutexWrite);
    m_out.push_back(new Out(tag, type, data));
}

QByteArray ZPlc::getData(int id)
{
    const std::lock_guard<std::mutex> lock(m_mutexRead);
    for(auto i: m_in)
    {
        if(i->m_id == id)
        {
            return i->m_data;
            break;
        }
    }
    return QByteArray();
}

void ZPlc::run()
{
    bool ok = true;
    m_run = true;

    while(!m_quit)
    {
        ok = true;
        if(!m_plc || !m_plc->isConnected())
        {
            delete m_plc;
            m_plc = new ZEthernetIp(m_ip, m_backplane, m_slot);
        }
        while(!m_quit && ok)
        {
            // read data
            {
                auto t = std::chrono::steady_clock::now();
                const std::lock_guard<std::mutex> lock(m_mutexRead);
                for(auto i: m_in)
                {
                    auto diff = t - i->m_last;
                    if(diff > i->m_time)
                    {
                        i->m_last = t;
                        if(m_plc->reqReadPlc(i->m_tag, i->m_len))
                            if(m_plc->getReadPlc())
                                i->m_data = m_plc->readData();
                    }
                }
            }
            // write data
            {
                if(m_mutexWrite.try_lock())
                {
                    while(m_out.size() > 0)
                    {
                        auto o = m_out.front();
                        if(!m_plc->writePlc(o->m_tag, o->m_type, o->m_data))
                            ok = false;
                        m_out.pop_front();
                    }
                    m_mutexWrite.unlock();
                }
            }
            ok = m_plc->isConnected();
            msleep(100);
        }
        delete m_plc;
        m_plc = nullptr;
        sleep(3);
    }
    m_run = false;
    m_quit = false;
}
