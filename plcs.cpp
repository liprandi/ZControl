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
        externalCommand(to);
        if(from.length() > 0 && to.length() > 0 && m_out.count() >=  2)
        {
            if(!m_out[0].plc.compare("SKIDS"))
                m_skids.writeData(m_out[0].tag, m_out[0].type, from);
            if(!m_out[1].plc.compare("LFB"))
                m_b.writeData(m_out[1].tag, m_out[1].type, to);
        }
        m_a.cycleWrite();
        m_b.cycleWrite();
        m_skids.cycleWrite();
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
