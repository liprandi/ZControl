#ifndef ZETHERNETIP_H
#define ZETHERNETIP_H

#include <QByteArray>
#include <QTcpSocket>
#include <QDebug>

enum CIP_TYPE
{
    CIP_BOOL   = 0xc1,
    CIP_CHAR   = 0xc2,
    CIP_SHORT  = 0xc3,
    CIP_LONG   = 0xc4,
    CIP_FLOAT  = 0xca,
    CIP_32BITS = 0xd3,
    CIP_STRUCT = 0x02a0
};

void preapareCipReadTable(QByteArray& buffer, const QByteArray &tag, int sizeInByte, int backplane, int slot);
void preapareCipWriteTable(QByteArray& buffer, const QByteArray& tag, CIP_TYPE type, int lenInType, const QByteArray data, int backplane, int slot);
int  bytesOfType(int type);

typedef struct  encaph
{
    quint16 command;     /* Command code */
    quint16 length;      /* Total transaction length */
    quint32 session;     /* Session identifier */
    quint32 status;      /* Status code */
    quint32 context[2];  /* Context information */
    quint32 opt;         /* Options flags */
}ENCAPH;

class ZEthernetIp
{
public:
    ZEthernetIp(const QString& ip, int backplane, int slot);        // apertura del servizio (registrazione)
    ~ZEthernetIp();

    bool reqReadPlc(const QByteArray &tag, int len);
    bool getReadPlc();
    bool writePlc(const QByteArray& tag, CIP_TYPE type, const QByteArray& data);
public:   // inline function
    QByteArray& readData(){return m_dataIn;}
    CIP_TYPE readType(){return (CIP_TYPE)m_readType;}
    QByteArray& writeData(){return m_dataOut;}
    bool isConnected(){return m_socket.state() == QAbstractSocket::ConnectedState;}
public:
    int             dbg_error[100];      // contatori di errore comunicazione
private:
    QTcpSocket      m_socket;           // socket di comunicazione
    QString         m_err;              // ultimo errore riscontrato
    QHostAddress    m_ip;               // indirizzo IP
    int             m_backplane;        // backplane (sempre 1)
    int             m_slot;             // slot in cui è collocata la cpu
    QByteArray      m_readTag;          // variabile di lettura divisa in lista di stringhe
    quint16         m_readType;         // lunghezza dell'area di scrittura
    quint16         m_readHandle;       // user defined struct handle: gestore struttura utente
    int             m_readLength;       // lunghezza dell'area di lettura espressa in bytes
    QByteArray      m_writeTag;         // variabile di scrittura divisa in lista di stringhe
    quint16         m_writeType;        // lunghezza dell'area di scrittura
    quint16         m_writeHandle;       // user defined struct handle: gestore struttura utente
    int             m_writeLength;      // lunghezza dell'area di scrittura espressa in elementi
    unsigned long   m_session;          // sessione aperta dalla registrazione
    QByteArray      m_dataIn;           // dati da PLC a PC
    QByteArray      m_dataOut;          // dati da PC a PLC
};
#endif // ZETHERNETIP_H
