#ifndef PLCS_H
#define PLCS_H

#include <QThread>
#include <QSettings>
#include <QList>
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

public:
    explicit Plcs(QSettings& settings, QObject *parent = nullptr);
    ~Plcs();

private:
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

    bool m_run;
    bool m_quit;
};

#endif // PLCS_H
