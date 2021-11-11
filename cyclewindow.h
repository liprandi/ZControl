#ifndef CYCLEWINDOW_H
#define CYCLEWINDOW_H

#include <QWidget>
#include <QListView>
#include <QToolButton>
#include "zmariadb.h"
#include "plcs.h"

namespace Ui {
class CycleWindow;
}


class CycleWindow : public QWidget
{
    Q_OBJECT

public:
    class PushButton : public QToolButton
    {        
    public:
        void setUserData(int col, int row)
        {
            m_col = col;
            m_row = row;
        }
    public:
        int m_col;
        int m_row;
    };
public:
    class ListMessages : public QAbstractTableModel
    {
    public:
        ListMessages(const QByteArray& data, int cycle, const CycleWindow* super):
            m_data(data)
          , m_super(super)
          , m_cycle(cycle)
        {
            int i;
            for(i = 0; i < 5; i++)
            {
                auto v = (short*)(m_data.data() + (i * 2 + 80));
                if(*v <= 0)
                    break;
            }
            m_cntFailures = i;

            int base = (m_cycle == 211) ? 10: 50;
            for(i = 0; i < 10; i++)
            {
                auto v = (short*)(m_data.data() + (i * 2 + base));
                if(*v <= 0)
                    break;
            }
            m_cntMessages = i;
        }

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    private:
        const QByteArray m_data;
        const CycleWindow* m_super;
        int m_cycle;
        int  m_cntFailures;
        int  m_cntMessages;
    };

public:
    explicit CycleWindow(QWidget *parent = nullptr);
    ~CycleWindow();

    void setCycle(int num, ZMariaDB* db, Plcs* plcs);
    void updateData(const QByteArray &data);
signals:
    void setCommand(int byte, int bit);
    void resetCommand(int byte, int bit);
private slots:
    void pressedButton();
    void releasedButton();
private:
    void recreateButtons();
    void updateHeader(const QByteArray &data);
    void updateList(const QByteArray &data);
    void updateLeds(const QByteArray &data);
    void actCommand(bool set);

private:
    Ui::CycleWindow *ui;
    int m_cycle;
    int m_step;
    ZMariaDB* m_db;
    Plcs*     m_plcs;
    QMap<int, QString> m_steps;
    QMap<int, QString> m_messages;
    QMap<int, QString> m_failures;
};

#endif // CYCLEWINDOW_H
