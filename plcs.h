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
    virtual void run() override;
signals:

public:
    ZPlc m_a;
    ZPlc m_b;
    ZPlc m_skids;
private:
    QList<Addr> m_addr;     // address for the 3 plcs
    QList<AreaReadFromPlc> m_in;  // area to read from ferratura A
    QList<AreaReadToPlc> m_out;  // area to read from ferratura A
    QList<ExternalCommand> m_command; // command qrriving from others sources but HMI
    bool m_run;
    bool m_quit;
    std::mutex m_mutexCommand;
};

#endif // PLCS_H
