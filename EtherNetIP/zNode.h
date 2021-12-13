#ifndef NODE_H
#define NODE_H

#include <QThread>
#include "ethernetIP.h"
#include "variabledata.h"

class Variable;

class Node : public QThread
{
public:
    Node(const QString& name, const QString& ip, int backplane, int slot, int sampleTime);
    ~Node();

    int addVariable(VariableData* var);      // add a variable to the list
    int findVariable(const QString& id);                   // find index of variable
    bool addField(const QString& id, const QString& field, const QString& type, int offset, int option = 0);   // add a field to variable
public: // inline function
    const QString& name(){return m_name;}
    QVariantMap dataMap();
    QVariantList variableDataMap();
private:
   void run();                      // Thread function
private:
    QString         m_name;           // id of node
    QString         m_ip;           // ip address interface
    int             m_backplane;    // backplane number (usually 1)
    int             m_slot;         // slot number (first position = 0)
    int             m_sampleTime;   // sample time in msec
    EthernetIp*     m_comm;         // interfaccia di comunicazione col PLC
    bool            m_quit;         // resquest to quit the thread
private:
    QList<VariableData*> m_vars;         // variable list to read

    friend class Variable;
};

#endif // NODE_H
