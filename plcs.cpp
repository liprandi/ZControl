#include "plcs.h"

Plcs::Plcs(QSettings &settings, QObject *parent) :
    QThread(parent)
  , m_run(false)
  , m_quit(false)
{
    bool ok = true;
    m_addr.clear();
    for(int i = 0; ok; i++)
    {
        QString prefix = QString("plc_%1").arg(i+1, 2, 10, QChar('0'));
        Plcs::Addr tmp;
        tmp.name = settings.value(prefix + "/name", prefix).toByteArray();
        tmp.ip = settings.value(prefix + "/ip", "0.0.0.0").toByteArray();
        tmp.backplane = settings.value(prefix + "/backplane", -1).toInt();
        tmp.slot = settings.value(prefix + "/slot", -1).toInt();

        if(tmp.slot < 0 || tmp.backplane < 0)
            ok = false;
        else
            m_addr.append(tmp);
    }
    ok = true;
    for(int i = 0; ok; i++)
    {
        QString prefix = QString("area_in_%1").arg(i+1, 2, 10, QChar('0'));
        Plcs::AreaReadFromPlc tmp;
        tmp.plc = settings.value(prefix + "/plc", "").toByteArray();
        tmp.id = settings.value(prefix + "/id", 0).toInt();
        tmp.tag = settings.value(prefix + "/tag", "").toByteArray();
        tmp.len = settings.value(prefix + "/length", 0).toInt();

        ok = false;
        for(const auto& i: m_addr)
        {
            if(!i.name.compare(tmp.plc))
            {
                ok = true;
                break;
            }
        }
        if(ok)
            m_in.append(tmp);
    }
    ok = true;
    for(int i = 0; ok; i++)
    {
        QString prefix = QString("area_out_%1").arg(i+1, 2, 10, QChar('0'));
        Plcs::AreaReadToPlc tmp;
        tmp.plc = settings.value(prefix + "/plc", "").toByteArray();
        tmp.id = settings.value(prefix + "/id", 0).toInt();
        tmp.tag = settings.value(prefix + "/tag", "").toByteArray();
        tmp.type = CIP_TYPE(settings.value(prefix + "/type", "").toInt());
        tmp.len = settings.value(prefix + "/length", 0).toInt();

        ok = false;
        for(const auto& i: m_addr)
        {
            if(!i.name.compare(tmp.plc))
            {
                ok = true;
                break;
            }
        }
        if(ok)
            m_out.append(tmp);
    }
    configPlc("LFA", m_a);
    configPlc("LFB", m_b);
    configPlc("SKIDS", m_skids);
    start();
}

Plcs::~Plcs()
{
    m_quit = true;
    if(m_run)
        msleep(500);
}

void Plcs::configPlc(const QByteArray& name, ZPlc& plc)
{
    for(const auto& addr: m_addr)
    {
        if(!addr.name.compare(name))  // ferratura line 1
        {
            for(const auto& area: m_in)
            {
                if(!area.plc.compare(addr.name))  // ferratura line 1
                {
                    plc.setAreaIn(area.id, area.tag, area.len, 100ms);
                }
            }
            plc.setAddress(addr.ip, addr.backplane, addr.slot);
            break;
        }
    }
}
void Plcs::run()
{
    m_run = true;
    while(!m_quit)
    {
        m_a.cycleRead();
        m_b.cycleRead();
        m_skids.cycleRead();
        QByteArray from = m_b.getData(1);
        QByteArray to = m_skids.getData(2);
        QByteArray speedA = m_skids.getData(3);
        QByteArray speedB = m_skids.getData(4);
        QByteArray alive = m_skids.getData(5);
        externalCommand(to);
        if(from.length() > 0 && to.length() > 0 && m_out.count() >=  5)
        {
            if(!m_out[0].plc.compare("SKIDS"))
                m_skids.writeData(m_out[0].tag, m_out[0].type, from);
            if(!m_out[1].plc.compare("LFB"))
                m_b.writeData(m_out[1].tag, m_out[1].type, to);
            if(!m_out[2].plc.compare("LFA"))
                m_a.writeData(m_out[2].tag, m_out[2].type, speedA);
            if(!m_out[3].plc.compare("LFB"))
                m_b.writeData(m_out[3].tag, m_out[3].type, speedB);
            if(!m_out[4].plc.compare("LFA") && alive.length() >= 2)
            {
                QByteArray t(4, 0);
                t[0] = alive[0];
                t[1] = alive[1];
                m_a.writeData(m_out[4].tag, m_out[4].type, t);
            }
        }
        m_a.cycleWrite();
        m_b.cycleWrite();
        m_skids.cycleWrite();
        if(compareBuffers(from))
        {
            if(m_diffSx.count() > 0)
                emit newMsgSx(m_diffSx);
            if(m_diffDx.count() > 0)
                emit newMsgDx(m_diffSx);
            if(m_diffGeneral.count() > 0)
                emit newMsgGeneral(m_diffGeneral);
        }
    }
    m_run = false;
}
void  Plcs::setCommand(int byte, unsigned value)
{
    const std::lock_guard<std::mutex> lock(m_mutexCommand);
    bool found = false;
    for(int i = 0; i < m_command.length(); i++)
    {
        const ExternalCommand& cmd = m_command[i];
        if(cmd.byte == byte)
        {
            if(value == 0)
              m_command.remove(i);
            found = true;
            break;
        }
    }
    if(value != 0 && !found)
    {
        m_command.append({byte, value});
    }
}

bool Plcs::compareBuffers(const QByteArray& data)
{
    bool ret = false;
    ret |= compareBuffer(10, 10, data, m_msgSx, m_diffSx);
    ret |= compareBuffer(50, 10, data, m_msgDx, m_diffDx);
    ret |= compareBuffer(80, 5,  data, m_msgGeneral, m_diffGeneral);
    return ret;
}
bool Plcs::compareBuffer(int base, int size, const QByteArray& data, QList<short>& msg, QList<short>& diff)
{
    QList<short> err;
    for(int i = 0; i < size; i++)
    {
        auto f = (short*)(data.data() + (i * 2 + base));
        if((*f) > 0 && ((*f) > 128 || base > 50))
        {
            err.append(*f);
        }
    }
    diff.clear();
    for(auto u: err)
    {
        bool found = false;
        for(auto v: msg)
        {
            if(u == v)
            {
                found = true;
                break;
            }
        }
        if(!found)
            diff.append(u);
    }
    for(auto v: msg)
    {
        bool found = false;
        for(auto u: err)
        {
            if(v == u)
            {
                found = true;
                break;
            }
        }
        if(!found)
            diff.append(-v);
    }
    msg = err;
    return diff.count();
}
void  Plcs::resetAllCommand()
{
    const std::lock_guard<std::mutex> lock(m_mutexCommand);
    m_command.clear();
}

void Plcs::externalCommand(QByteArray& data)
{
    const std::lock_guard<std::mutex> lock(m_mutexCommand);
    for(auto cmd: m_command)
    {
        if(cmd.byte < data.length())
        {
            data[cmd.byte] = cmd.value;
        }
    }
}
