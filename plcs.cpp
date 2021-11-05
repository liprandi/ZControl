#include "plcs.h"

Plcs::Plcs(QSettings &settings, QObject *parent) :
    QObject(parent)
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
                    plc.setAreaIn(area.id, area.tag, area.len);
                }
            }
            plc.setAddress(addr.ip, addr.backplane, addr.slot);
            break;
        }
    }
}
