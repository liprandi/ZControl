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
    , m_telegram(nullptr)
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
    auto old1 = ui->tblLocal->model();
    ui->tblLocal->setModel(&m_local);
    delete old1;
    auto old2 = ui->tblRemote->model();
    ui->tblRemote->setModel(&m_remote);
    delete old2;
    readMessageStepsAndFails();
    if(!m_telegram)
        m_telegram = new ZTelegramService(m_settings);
    m_telegram->addMessage("⚜<b>Inicio Programa!</b>");
    if(!m_serverRemote)
        m_serverRemote = new ZHttpService(m_settings);
    connect(m_telegram, &ZTelegramService::requestCommand, [&](const QString cmd)
    {
        if(cmd.indexOf("status", 0, Qt::CaseInsensitive) >= 0)
        {
            QByteArray t = m_plcs.m_b.getData(1);
            auto stepl = reinterpret_cast<const short*>(&t.data()[2]);
            auto stepr = reinterpret_cast<const short*>(&t.data()[42]);
            QString l =  QString("👈<b>Dobradiças Esquerdas</b>\n[<b>%1</b>]:<i>%2</i>").arg(*stepl).arg(m_steps[0][*stepl]);
            QString r =  QString("\n👉<b>Dobradiças Direitas</b>\n[<b>%1</b>]:<i>%2</i>").arg(*stepr).arg(m_steps[0][*stepr]);
            m_telegram->addMessage(l + r);
        }
    });
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
           ui->hingeWidget->setCycle(211, &m_local, &m_plcs, &m_steps[0], &m_messages[0], &m_failures);
       }
    });
    connect(ui->btnHingeRight, &QAbstractButton::toggled, [&](bool checked)
    {
       if(checked)
       {
           ui->stackedWidget->setCurrentIndex(1);
           ui->hingeWidget->setCycle(212, &m_local, &m_plcs, &m_steps[1], &m_messages[1], &m_failures);
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
           if(!m_telegram)
               m_telegram = new ZTelegramService(m_settings);
           m_telegram->addMessage("👍<b>Check Telegram!</b>");
       }
    });
    connect(&m_plcs, &Plcs::newMsgSx, [&](const QList<short> msgs)
    {
        QByteArray t = m_plcs.m_b.getData(1);
        auto step = reinterpret_cast<const short*>(&t.data()[2]);
        QString header =  QString("👈<b>Dobradiças Esquerdas</b>\n[<b>%1</b>]:<i>%2</i>").arg(*step).arg(m_steps[0][*step]);
        newMsg(header, msgs, m_messages[0]);
    });
    connect(&m_plcs, &Plcs::newMsgDx, [&](const QList<short> msgs)
    {
        QByteArray t = m_plcs.m_b.getData(1);
        auto step = reinterpret_cast<const short*>(&t.data()[42]);
        QString header =  QString("👉<b>Dobradiças Direitas</b>\n[<b>%1</b>]:<i>%2</i>").arg(*step).arg(m_steps[1][*step]);
        newMsg(header, msgs, m_messages[1]);
    });
    connect(&m_plcs, &Plcs::newMsgGeneral, [&](const QList<short> msgs)
    {
        newMsg("❗<b>Falhas Gerais</b>", msgs, m_failures);
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
void MainWindow::readMessageStepsAndFails()
{
    if(m_local.query("SELECT * FROM `step` where `equipment`=211 order by `idx`"))
    {
        m_steps[0].clear();
        const auto recs = m_local.getAllRecords();
        for(auto& r: recs)
        {
            m_steps[0][std::stoi(r.at("idx"))] = QString::fromStdString(r.at("description"));
        }
    }
    if(m_local.query("SELECT * FROM `step` where `equipment`=212 order by `idx`"))
    {
        m_steps[1].clear();
        const auto recs = m_local.getAllRecords();
        for(auto& r: recs)
        {
            m_steps[1][std::stoi(r.at("idx"))] = QString::fromStdString(r.at("description"));
        }
    }
    if(m_local.query("SELECT * FROM `fault` where `equipment`=211 order by `idx`"))
    {
        m_messages[0].clear();
        const auto recs = m_local.getAllRecords();
        for(auto& r: recs)
        {
            m_messages[0][std::stoi(r.at("idx"))+1] = QString::fromStdString(r.at("description"));
        }
    }
    if(m_local.query("SELECT * FROM `fault` where `equipment`=212 order by `idx`"))
    {
        m_messages[1].clear();
        const auto recs = m_local.getAllRecords();
        for(auto& r: recs)
        {
            m_messages[1][std::stoi(r.at("idx"))+1] = QString::fromStdString(r.at("description"));
        }
    }
    if(m_local.query("SELECT * FROM `fault` where `equipment`=0 order by `idx`"))
    {
        m_failures.clear();
        const auto recs = m_local.getAllRecords();
        for(auto& r: recs)
        {
            m_failures[std::stoi(r.at("idx"))+1] = QString::fromStdString(r.at("description"));
        }
    }
}
void MainWindow::newMsg(const QString& header, const QList<short>& msgs, const QMap<int, QString>& descr)
{
    QString outStr;
    if(m_telegram && msgs.count() > 0)
    {
        outStr = header;
        for(auto v: msgs)
        {
            if(v > 0)
            {
                outStr += "\n❌" + descr[v];
            }
            else
            {
                outStr += "\n✅<s>" + descr[-v] + "</s>";
            }
        }
        m_telegram->addMessage(outStr);
    }
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    if(m_serverRemote)
    {
        m_serverRemote->check();
    }
    if(m_telegram)
    {
        m_telegram->check();
    }
    if(ui->stackedWidget->currentIndex() == 1)
    {
        QByteArray t = m_plcs.m_b.getData(1);
        ui->hingeWidget->updateData(t);
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
