#ifndef ZMARIADB_H
#define ZMARIADB_H

#include <QAbstractTableModel>
#include <map>
#include <vector>
#include <string>
#include <mariadb/mysql.h>

class ZMariaDB : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ZMariaDB(QObject *parent = nullptr);
    ~ZMariaDB();

    bool connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database, int port = 3306);
signals:

public:
    bool query(const std::string &qString);
    const std::vector<std::string>& getFields() const;
    const std::vector<std::string>& getNextRecord();
    const std::vector<std::map<std::string, std::string>>& getAllRecords();

public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:    // data used by MariaDb library
    MYSQL* m_sql;
    MYSQL_RES *m_res;
private:    // data used for interface
    std::vector<std::string> m_fields;
    std::vector<std::string> m_record;
    std::vector<std::map<std::string, std::string>> m_records;
};

#endif // ZMARIADB_H
