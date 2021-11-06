#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <chrono>
#include <QDesktopServices>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings("zcontrol.ini", QSettings::IniFormat)
    , m_serverRemote(nullptr)
    , m_plcs(m_settings)
{
    m_addrLocal = m_settings.value("local/address", "localhost").toString();
    m_userLocal = m_settings.value("local/user", "root").toString();
    m_passwordLocal = m_settings.value("local/password", "root").toString();
    m_dbLocal = m_settings.value("local/db", "ferratura").toString();
    m_addrRemote = m_settings.value("remote/address", "liprandi.com").toString();
    m_userRemote = m_settings.value("remote/user", "liprandi_root").toString();
    m_passwordRemote = m_settings.value("remote/password", "0713D0504l").toString();
    m_dbRemote = m_settings.value("remote/db", "liprandi_log").toString();

    m_local.connect(m_addrLocal.toStdString(), m_userLocal.toStdString(), m_passwordLocal.toStdString(), m_dbLocal.toStdString());
    m_remote.connect(m_addrRemote.toStdString(), m_userRemote.toStdString(), m_passwordRemote.toStdString(), m_dbRemote.toStdString());
    auto res_local = m_local.getAllRecords();
    auto res_remote = m_remote.getAllRecords();
    ui->setupUi(this);
    auto old1 = ui->tblLocal->selectionModel();
    ui->tblLocal->setModel(&m_local);
    delete old1;
    auto old2 = ui->tblRemote->selectionModel();
    ui->tblRemote->setModel(&m_remote);
    delete old2;
    connect(ui->btnLogo, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(0);
       }
    });
    connect(ui->btnHingeLeft, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(1);
       }
    });
    connect(ui->btnHingeRight, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(1);
       }
    });
    connect(ui->btnDebug, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(2);
       }
    });
    connect(ui->btnTelegram, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(3);
           if(!m_serverRemote)
               m_serverRemote = new ZHttpService();
           m_serverRemote->startRequest(QUrl("http://www.liprandi.com/projects/betim/zServicePHP.php")); //"http://www.liprandi.com/projects/betim/zServicePHP.php"
       }
    });
    startTimer(1s);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_settings.setValue("local/address", m_addrLocal);
    m_settings.value("local/user", m_userLocal);
    m_settings.value("local/password", m_passwordLocal);
    m_settings.value("local/db", m_dbLocal);
    m_settings.value("remote/address", m_addrRemote);
    m_settings.value("remote/user", m_userRemote);
    m_settings.value("remote/password", m_passwordRemote);
    m_settings.value("remote/db", m_dbRemote);
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    if(m_serverRemote)
    {
        if(m_serverRemote->hasFinished())
        {
            ui->logTelegram->setPlainText(m_serverRemote->data());
            delete m_serverRemote;
            m_serverRemote = nullptr;
        }
    }
    if(ui->stackedWidget->currentIndex() == 2)
    {
        ui->toPc->clear();
        QByteArray t = m_plcs.m_b.getData(1);
        int i = 0;
        for(auto v: t)
        {
            if(v)
            {
                ui->toPc->addItem(QString("topc[%1] = %2").arg(i, 2, 10, QChar(' ')).arg(v, 4, 16, QChar('0')));
            }
            i++;
        }
        QByteArray f = m_plcs.m_skids.getData(2);
        i = 0;
        for(auto v: f)
        {
            if(v)
            {
                ui->toPc->addItem(QString("frompc[%1] = %2").arg(i, 2, 10, QChar(' ')).arg(v, 4, 16, QChar('0')));
            }
            i++;
        }
    }
}
