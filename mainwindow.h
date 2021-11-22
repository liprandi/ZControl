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

   virtual void timerEvent(QTimerEvent* event) override;
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
};
#endif // MAINWINDOW_H
