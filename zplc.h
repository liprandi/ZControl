#ifndef ZPLC_H
#define ZPLC_H

#include <QObject>
#include <chrono>
#include <mutex>
#include <EtherNetIP/zethernetip.h>

using namespace std::chrono_literals;

class ZPlc : public QObject
{
    Q_OBJECT
public:
    class In
    {
    public:
        In(int id, const QByteArray &tag, int len, std::chrono::duration<__int64, std::milli> msec);
        ~In();
    private:
        int m_id;
        QByteArray m_tag;
        int m_len;
        QByteArray m_data;
        std::chrono::duration<__int64, std::milli> m_time;
        std::chrono::time_point<std::chrono::steady_clock> m_last;
        friend class ZPlc;
    };
    class Out
    {
    public:
        Out(const QByteArray& tag, CIP_TYPE type, const QByteArray& data);
        ~Out();
    private:
        QByteArray m_tag;
        CIP_TYPE m_type;
        QByteArray m_data;
        friend class ZPlc;
    };

public:
    explicit ZPlc(QObject *parent = nullptr);
    virtual ~ZPlc();

    void setAddress(const QString& ip, int backplane, int slot);
    void setAreaIn(int id, const QByteArray &tag, int len, std::chrono::duration<__int64, std::milli> msec = 500ms);
    void writeData(const QByteArray& tag, CIP_TYPE type, const QByteArray& data);
    QByteArray getData(int id);
    void cycleRead();
    void cycleWrite();
protected:
//    virtual void run() override;

private:
//    bool m_run;
//    bool m_quit;
    std::list<In*> m_in;
    std::list<Out*> m_out;

    ZEthernetIp* m_plc;

    QString m_ip;
    int m_backplane;
    int m_slot;
    std::mutex m_mutexRead;
    std::mutex m_mutexWrite;
};

#endif // ZPLC_H
