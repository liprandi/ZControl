#include "zethernetip.h"

/* ----------------------------------------------------------
   Prima operazione: registrazione
   la comunicazione in Ethernet/IP richiede la registrazione
   del client
   ---------------------------------------------------------- */
ZEthernetIp::ZEthernetIp(const QString& ip, int backplane, int slot)
{
    m_session = 0;
    m_backplane = backplane;
    m_slot = slot;
    memset(dbg_error, 0, sizeof(dbg_error));
    if(!m_ip.setAddress(ip))
        return; // throw(std::invalid_argument("indirizzo IP non conforme"));
    m_socket.connectToHost(m_ip, 0xaf12);
    if(m_socket.state() != 0)
    {
        ENCAPH tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.command = 0x65;         // comando per la registrazione del client
        tmp.length = 0x04;
        quint32 num = 1;
        QByteArray sendBuffer;
        sendBuffer.append((const char *)&tmp, sizeof(tmp));
        sendBuffer.append((const char *)&num, sizeof(num));

        m_socket.readAll();
        if(m_socket.write(sendBuffer) == sendBuffer.length())
        {
            if(m_socket.waitForBytesWritten(1000))
            {
                if(m_socket.waitForReadyRead(1000))
                {
                    if(m_socket.read((char *)&tmp, sizeof(tmp)) == sizeof(tmp))
                    {
                        char data[tmp.length];
                        m_socket.read(data, tmp.length);
                        if(tmp.length == 4 && memcmp(data, "\0\0\0\0", tmp.length))
                            m_session = tmp.session;
                    }
                    else
                        qDebug() << QObject::tr("error read data from Tcp/Ip socket");
                }
                else
                    qDebug() << QObject::tr("error no data replied");
            }
            else
                qDebug() << QObject::tr("error no data written");
        }
        else
            qDebug() << QObject::tr("error in written data size");
    }
}

/* ----------------------------------------------------------
   Ultima operazione: deregistrazione
   chiusura del servizio di comunicazione con ControlLogix
   ---------------------------------------------------------- */
ZEthernetIp::~ZEthernetIp()
{
    if(isConnected())
    {
        ENCAPH tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.command = 0x66;                 // comando per la chiusura della registrazione
        tmp.session = m_session;
        QHostAddress addr = m_socket.localAddress();
        tmp.context[0] = addr.toIPv4Address();
        m_socket.readAll();
        if(m_socket.write((const char *)&tmp, sizeof(tmp)) == sizeof(tmp))
        {
            if(m_socket.waitForBytesWritten(1000))
            {
                if(m_socket.waitForReadyRead(1000))
                {
                    if(m_socket.read((char *)&tmp, sizeof(tmp)) == sizeof(tmp))
                    {
                        char data[tmp.length];
                        m_socket.read(data, tmp.length);
                    }
                    else
                        qDebug() << QObject::tr("error read data from Tcp/Ip socket");
                }
                else
                    qDebug() << QObject::tr("error no data replied");
            }
            else
                qDebug() << QObject::tr("error no data written");
        }
        else
            qDebug() << QObject::tr("error in written data size");
    }
}

/* ----------------------------------------------------------
  esecuzione della lettura di un buffer da PLC
   ---------------------------------------------------------- */
bool ZEthernetIp::getReadPlc()
{
    bool ret = false;
    if(isConnected())
    {
        ENCAPH tmp;
        memset(&tmp, 0, sizeof(tmp));

        if(m_socket.waitForReadyRead(1000))
        {
            if(m_socket.bytesAvailable() >= (qint64)sizeof(tmp))
            {
                if(m_socket.read((char *)&tmp, sizeof(tmp)) == sizeof(tmp))
                {
                    char data[tmp.length];
                    m_socket.read(data, tmp.length);
                    m_dataIn.clear();
                    if(tmp.length > 22)
                    {
                        memmove(&m_readType, &data[20], 2);
                        if(m_readType != CIP_STRUCT)
                        {
                            m_readHandle = 0;
                            m_dataIn.append(&data[22], tmp.length - 22);
                        }
                        else
                        {
                            memmove(&m_readHandle, &data[22], 2);
                            m_dataIn.append(&data[24], tmp.length - 24);
                        }
                        ret = true;
                    }
                    else
                        qDebug() << QObject::tr("error size data less than 22");
                }
                else
                    qDebug() << QObject::tr("error in read data size");
            }
            else
                qDebug() << QObject::tr("error in read data available");
        }
    }

    return ret;
}
/* ----------------------------------------------------------
  richiesta di lettura di un buffer da PLC
   ---------------------------------------------------------- */
bool ZEthernetIp::reqReadPlc(const QByteArray& tag, int len)
{
    bool ret = false;

    m_readTag = tag;
    m_readLength = len;
    if(isConnected())
    {
        ENCAPH tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.command = 0x6f;         // comando per l'invio di un comando CIP incapsulato
        tmp.session = m_session;
        QHostAddress addr = m_socket.localAddress();
        tmp.context[0] = addr.toIPv4Address();
        qint8 buffer[16];
        memset(&buffer, 0, sizeof(buffer));
        buffer[6] = 2;
        buffer[12] = 0xb2;
        QByteArray test;
        preapareCipReadTable(test, m_readTag, m_readLength, m_backplane, m_slot);
        buffer[14] = test.length() % 0x100;
        buffer[15] = test.length() / 0x100;
        tmp.length = sizeof(buffer) + test.length();

        QByteArray sendBuffer;
        sendBuffer.append((const char *)&tmp, sizeof(tmp));
        sendBuffer.append((const char *)buffer, sizeof(buffer));
        sendBuffer.append(test);

        m_socket.readAll();
        if(m_socket.write(sendBuffer) == sendBuffer.length())
        {
            if(m_socket.waitForBytesWritten(1000))
            {
                ret = true;
            }
            else
                qDebug() << QObject::tr("error no data written");
        }
        else
            qDebug() << QObject::tr("error in written data size");
    }
    return ret;
}
/* ----------------------------------------------------------
  esecuzione della scrittura di un buffer su PLC
   ---------------------------------------------------------- */
bool ZEthernetIp::writePlc(const QByteArray &tag, CIP_TYPE type, const QByteArray &data)
{
    bool ret = false;
    m_dataOut = data;
    m_writeTag = tag;
    m_writeType = type;
    m_writeLength = m_dataOut.length() / bytesOfType(type);
    if(isConnected())
    {
        ENCAPH tmp;
        qint8 buffer[16];
        memset(&buffer, 0, sizeof(buffer));
        memset(&tmp, 0, sizeof(tmp));
        tmp.command = 0x6f;     // comando per l'invio di un comando CIP incapsulato
        tmp.length = sizeof(buffer);
        tmp.session = m_session;
        QHostAddress addr = m_socket.localAddress();
        tmp.context[0] = addr.toIPv4Address();
        buffer[6] = 2;
        buffer[12] = 0xb2;
        QByteArray test;
        preapareCipWriteTable(test, m_writeTag, (CIP_TYPE)m_writeType, m_writeLength, m_dataOut, m_backplane, m_slot);
        buffer[14] = test.length() % 0x100;
        buffer[15] = test.length() / 0x100;
        tmp.length = sizeof(buffer) + test.length();

        QByteArray sendBuffer;
        sendBuffer.append((const char *)&tmp, sizeof(tmp));
        sendBuffer.append((const char *)buffer, sizeof(buffer));
        sendBuffer.append(test);

        m_socket.readAll();
        if(m_socket.write(sendBuffer) == sendBuffer.length())
        {
            if(m_socket.waitForReadyRead(1000))
            {
                if (m_socket.bytesAvailable() >= (qint64)sizeof(tmp))
                {

                    if(m_socket.read((char *)&tmp, sizeof(tmp)) == sizeof(tmp))
                    {
                        char data[tmp.length];
                        ret = true;
                        m_socket.read(data, tmp.length);
                        dbg_error[44]++;
                    }
                    else
                        dbg_error[42]++;
                }
                else
                    dbg_error[45]++;
            }
            else
                dbg_error[41]++;
        }
        else
            dbg_error[40]++;
    }
    return ret;
}

/* ----------------------------------------------------------
   Preparazione del pacchetto Cip incapslulato nel messagio Ethernet/Ip
   per la lettura di una variabile
   ---------------------------------------------------------- */
void preapareCipReadTable(QByteArray& buffer, const QByteArray& tag, int sizeInByte, int backplane, int slot)
{
    QByteArray ioi;
    QByteArrayList tagList = tag.split('.');
    for(int i = 0; i < tagList.count(); i++)
    {
        QByteArray name;
        int idx_array[2];
        qint8 index = -1;

        idx_array[0] = tagList[i].indexOf('[');
        if(idx_array[0] > 0)
            idx_array[1] = tagList[i].indexOf(']', idx_array[0]);
        if(idx_array[0] > 0 && idx_array[1] > idx_array[0])
        {
            name = tagList[i].left(idx_array[0]);
            index = tagList[i].mid(idx_array[0] + 1, idx_array[1] - idx_array[0] - 1).toInt();
        }
        else
            name = tagList[i];
        bool tagIsOdd = (name.length() % 2 == 1);

        ioi.append('\x91');
        ioi.append(name.length() % 0x100);
        ioi.append(name);
        if(tagIsOdd)
            ioi.append('\x00');
        if(index >= 0)
        {
            ioi.append('\x28');
            ioi.append(index);
        }
    }
    int words = ioi.length() / 2;
    ioi.append(sizeInByte % 0x100);
    ioi.append(sizeInByte / 0x100);

    buffer.clear();
    buffer.append('\x52');       // ! Unconnected Send service
    buffer.append('\x02');       // ! request path size: 2 words
    buffer.append('\x20');       // ! request path: class ID:
    buffer.append('\x06');       // !				0x06 (Connection Manager Object)
    buffer.append('\x24');       // !				instance ID:
    buffer.append('\x01');       // !				0x01
    buffer.append('\x0A');       // ! request data: priority/time tick
    buffer.append('\xF0');       // !				time-out_ticks
    buffer.append('\x00');       // !	[8]			message_request_size (LoByte) = 12 bytes
    buffer.append('\x00');       // !	[9]			message_request_size (HiByte)
    buffer.append('\x4C');       // !				CIP Read Data service
    buffer.append(words);        // !				IOI string (4 words)
    buffer.append(ioi);          // !				IOI string
    buffer[8] = (buffer.length() - 10) % 0x100;
    buffer[9] = (buffer.length() - 10) / 0x100;
    buffer.append('\x01');       // ! route_path_size: 1 word
    buffer.append('\x00');       // ! reserved
    buffer.append(backplane);    // ! route_path: Backplane
    buffer.append(slot);         // ! Slot 3 (it was 0)
}
/* ----------------------------------------------------------
   Preparazione del pacchetto Cip incapslulato nel messagio Ethernet/Ip
   per la scrittura di una variabile
   ---------------------------------------------------------- */
void preapareCipWriteTable(QByteArray& buffer, const QByteArray &tag, CIP_TYPE type, int lenInType, const QByteArray data, int backplane, int slot)
{
    QByteArray ioi;
    QByteArrayList tagList = tag.split('.');
    for(int i = 0; i < tagList.count(); i++)
    {
        QByteArray name;
        int idx_array[2];
        qint8 index = -1;

        idx_array[0] = tagList[i].indexOf('[');
        if(idx_array[0] > 0)
            idx_array[1] = tagList[i].indexOf(']', idx_array[0]);
        if(idx_array[0] > 0 && idx_array[1] > idx_array[0])
        {
            name = tagList[i].left(idx_array[0]);
            index = tagList[i].mid(idx_array[0] + 1, idx_array[1] - idx_array[0] - 1).toInt();
        }
        else
            name = tagList[i];

        bool tagIsOdd = (name.length() % 2 == 1);

        ioi.append('\x91');
        ioi.append(name.length() % 0x100);
        ioi.append(name);
        if(tagIsOdd)
            ioi.append('\x00');
        if(index >= 0)
        {
            ioi.append('\x28');
            ioi.append(index);
        }
    }
    int words = ioi.length() / 2;
    ioi.append(type % 0x100);
    ioi.append(type / 0x100);
    ioi.append(lenInType % 0x100);
    ioi.append(lenInType / 0x100);
    ioi.append(data);

    buffer.clear();
    buffer.append('\x52');       // ! Unconnected Send service
    buffer.append('\x02');       // ! request path size: 2 words
    buffer.append('\x20');       // ! request path: class ID:
    buffer.append('\x06');       // !				0x06 (Connection Manager Object)
    buffer.append('\x24');       // !				instance ID:
    buffer.append('\x01');       // !				0x01
    buffer.append('\x0A');       // ! request data: priority/time tick
    buffer.append('\xF0');       // !				time-out_ticks
    buffer.append('\x00');       // !	[8]			message_request_size (LoByte) = 12 bytes
    buffer.append('\x00');       // !	[9]			message_request_size (HiByte)
    buffer.append('\x4D');       // !				CIP Write Data service
    buffer.append(words);        // !				IOI string (4 words)
    buffer.append(ioi);          // !				IOI string
    buffer[8] = (buffer.length() - 10) % 0x100;
    buffer[9] = (buffer.length() - 10) / 0x100;
    buffer.append('\x01');       // ! route_path_size: 1 word
    buffer.append('\x00');       // ! reserved
    buffer.append(backplane);    // ! route_path: Backplane
    buffer.append(slot);         // ! Slot 3 (it was 0)
}
/* ----------------------------------------------------------
   Conversione dal tipo di variabile CIP alla lunghezza in bytes
   del suo elemento
   ---------------------------------------------------------- */
int  bytesOfType(int type)
{
    int ret = 1;
    switch(type)
    {
        case CIP_BOOL:
        case CIP_CHAR:
            ret = 1;
            break;
        case CIP_SHORT:
            ret = 2;
            break;
        case CIP_LONG:
        case CIP_FLOAT:
        case CIP_32BITS:
            ret = 4;
            break;
    }
    return ret;
}

