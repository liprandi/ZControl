#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "zmariadb.h"
#include "plcs.h"
#include "http/zhttpservice.h"
#include "http/ztelegramservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void readMessageStepsAndFails();
    virtual void timerEvent(QTimerEvent* event) override;
    void newMsg(const QString& header, const QList<short>& msgs, const QMap<int, QString>& descr);
public slots:

private:
    Ui::MainWindow *ui;
private: // databases
    QSettings m_settings;
    ZMariaDB m_local;
    ZMariaDB m_remote;
    ZHttpService* m_serverRemote;
    ZTelegramService* m_telegram;
    Plcs m_plcs;
    QString m_addrLocal;
    QString m_userLocal;
    QString m_passwordLocal;
    QString m_dbLocal;
    QString m_addrRemote;
    QString m_userRemote;
    QString m_passwordRemote;
    QString m_dbRemote;
    QMap<int, QString> m_steps[2];      // 0 left, 1 right
    QMap<int, QString> m_messages[2];   // 0 left, 1 right
    QMap<int, QString> m_failures;
};
#endif // MAINWINDOW_H
