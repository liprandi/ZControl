#ifndef VARIABLEDATA_H
#define VARIABLEDATA_H

#include <QByteArray>
#include "vardescr.h"
#include "ethernetIP.h"

class VariableData
{
public:
    VariableData();
    VariableData(const VariableData& src);
    VariableData(const QString& plc, const QString& id, const QString& info, const QString &description, const QRect &rect, int size = 1);

    void set(const QByteArray& source);
    void addDescr(const QString& id, const QString& type, int offset, int option = 0);
    QVariantMap values();
    QVariantMap dataMap();
public: // inline function
    const QString& info(){return m_info;}
    const QString& description(){return m_description;}

    virtual bool afterRead(EthernetIp*){return false;} // function to call after read
public:
    QString     m_id;           // id of the variable
    CIP_TYPE    m_type;         // variable type
    int         m_size;         // variable size
    QByteArray  m_data;         // data of element
    bool        m_return;       // return code of active function
private:
    QList<VarDescr> m_descr;    // description of data
    QString     m_plc;          // plc aggregate
    QString     m_info;         // info about type variable
    QString     m_description;  // description of variable
    QRect       m_rect;         // position on canvas
};

#endif // VARIABLEDATA_H
