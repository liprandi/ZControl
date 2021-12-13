#ifndef PLCS_H
#define PLCS_H

#include <QThread>
#include <QSettings>
#include <QList>
#include <mutex>
#include "zplc.h"

class Plcs : public QThread
{
    Q_OBJECT
private:
    struct Addr
    {
        QByteArray name;       // name of plc
        QByteArray ip;
        int     backplane;
        int     slot;
    };
    struct AreaReadFromPlc
    {
        int        id;
        QByteArray   plc;       // name of plc
        QByteArray tag;
        int        len;
    };
    struct AreaReadToPlc
    {
        int        id;
        QByteArray plc;       // name of plc
        QByteArray tag;
        CIP_TYPE   type;
        int        len;
    };
    struct ExternalCommand
    {
        int byte;
        unsigned value;
    };

public:
    explicit Plcs(QSettings& settings, QObject *parent = nullptr);
    ~Plcs();

    void setCommand(int byte, unsigned value);
    void resetAllCommand();

private:
    void externalCommand(QByteArray &data);
    void configPlc(const QByteArray& name, ZPlc& plc);
    bool compareBuffers(const QByteArray& data);
    bool compareBuffer(int base, int size, const QByteArray& data, QList<short>& msg, QList<short>& diff);
    virtual void run() override;
signals:
    void newMsgSx(const QList<short> msg);
    void newMsgDx(const QList<short> msg);
    void newMsgGeneral(const QList<short> msg);
public:
    ZPlc m_a;
    ZPlc m_b;
    ZPlc m_skids;
private:
    QList<Addr> m_addr;     // address for the 3 plcs
    QList<AreaReadFromPlc>  m_in;  // area to read from ferratura A
    QList<AreaReadToPlc>    m_out;  // area to read from ferratura A
    QList<ExternalCommand>  m_command; // command qrriving from others sources but HMI
    QList<short>  m_msgSx;        // messages of different data hinge left
    QList<short>  m_msgDx;        // messages of different data hinge right
    QList<short>  m_msgGeneral;   // messages of different data general fault
    QList<short>  m_diffSx;        // messages of different data hinge left
    QList<short>  m_diffDx;        // messages of different data hinge right
    QList<short>  m_diffGeneral;   // messages of different data general fault

    bool m_run;
    bool m_quit;
    std::mutex m_mutexCommand;
};

#endif // PLCS_H
