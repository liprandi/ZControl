#include <QDebug>
#include "zmariadb.h"

#define errdb {qDebug() <<  mysql_error(m_sql); return false;}

ZMariaDB::ZMariaDB(QObject *parent) : QAbstractTableModel(parent)
  , m_sql(nullptr)
  , m_res(nullptr)
{
    m_sql = mysql_init(nullptr);
    int err = mysql_optionsv(m_sql, MYSQL_READ_DEFAULT_FILE, "my.cnf");
    if(err)
    {
       qDebug() <<  mysql_error(m_sql);
    }
}

ZMariaDB::~ZMariaDB()
{
    if(m_res)
    {
        mysql_free_result(m_res);
        delete m_res;
        m_res = nullptr;
    }
    if(m_sql)
        mysql_close(m_sql);
}

bool ZMariaDB::connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database, int port)
{
    if(!m_sql)
        return false;
    try
    {
        m_host = host;
        m_user = user;
        m_password = password;
        m_database = database;
        m_port = port;
        // Establish a MySQL connection
        if(!mysql_real_connect(
                m_sql,
                host.data(), user.data(),
                password.data(), database.data(),
                port, nullptr, 0))
        {
            errdb
        }
    }
    catch (char *e)
    {
        qDebug() <<  e;
        return false;
    }

    // Execute a sql statement
    query("SHOW TABLES");

    return true;
}

bool ZMariaDB::reconnect()
{
    if(!m_sql)
        return false;
    try
    {
        // Establish a MySQL connection
        if(!mysql_real_connect(
                m_sql,
                m_host.data(), m_user.data(),
                m_password.data(), m_database.data(),
                m_port, nullptr, 0))
        {
            errdb
        }
    }
    catch (char *e)
    {
        qDebug() <<  e;
        return false;
    }

    return true;
}

bool ZMariaDB::query(const std::string& qString)
{
    if(m_res)
    {
        mysql_free_result(m_res);
        delete m_res;
        m_res = nullptr;
    }
    if(mysql_query(m_sql, qString.data()))
    {
        if(reconnect())
        {
            if(mysql_query(m_sql, qString.data()))
            {
                errdb
            }
        }
    }
    m_res = mysql_use_result(m_sql);
    MYSQL_FIELD *fields;
    m_fields.clear();

    int num_fields = mysql_num_fields(m_res);
    fields = mysql_fetch_fields(m_res);

    for(int i = 0; i < num_fields; i++)
    {
        m_fields.push_back(fields[i].name);
    }
    return true;
}
const std::vector<std::string>& ZMariaDB::getFields() const
{
    return m_fields;
}
const std::vector<std::string>& ZMariaDB::getNextRecord()
{
    MYSQL_ROW row;
    m_record.clear();
    if(!m_sql || !m_res)
        return m_record;
    if((row = mysql_fetch_row(m_res)))
    {
       for(int i = 0; i < m_fields.size(); i++)
       {
           auto v = row[i];
           m_record.push_back(v ? v : "NULL");
       }
    }
    else
    {
        mysql_free_result(m_res);
        m_res = nullptr;
    }
    return m_record;
}
const std::vector<std::map<std::string, std::string> > &ZMariaDB::getAllRecords()
{
    m_records.clear();
    if(!m_sql || !m_res)
        return m_records;

    MYSQL_ROW row;
    while((row = mysql_fetch_row(m_res)))
    {
       std::map<std::string, std::string> rec;
       for(int i = 0; i < m_fields.size(); i++)
       {
           std::string v = row[i];
           rec[m_fields[i]] = (v.empty() ? "NULL" : v);
       }
       m_records.push_back(rec);
    }
    mysql_free_result(m_res);
    m_res = nullptr;

    return m_records;
}
int ZMariaDB::rowCount(const QModelIndex &parent) const
{
    return m_records.size();
}


int ZMariaDB::columnCount(const QModelIndex &parent) const
{
    return m_fields.size();
}

QVariant ZMariaDB::data(const QModelIndex &index, int role) const
{
    auto rec = m_records[index.row()];
    std::string c = m_fields[index.column()];
    return QVariant(rec[c].data());
}
