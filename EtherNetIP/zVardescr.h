#ifndef VARDESCR_H
#define VARDESCR_H

#include <QVariant>

class VarDescr
{
    enum Type
    {
        boolType    = 1,
        byteType,
        wordType,
        dwordType,
        realType,
        stringType,
        numOfTypes
    };
public:
    QString         m_id;       // id of variable
    int             m_type;     // type of variable
    int             m_offset;   // offset in byte
    int             m_option;   // option info (bit in case of Type = bool and leght in case of string)
    QVariant        m_value;
public:
    VarDescr(const QString& id, int type, int offset, int option = 0);
    VarDescr(const QString& id, const QString& type, int offset, int option = 0);

    void setValue(const QByteArray& data);
private:
    static const QString m_strType[];
};

#endif // VARDESCR_H
