#include "cyclewindow.h"
#include "ui_cyclewindow.h"
#include <QPushButton>
#include <QPainter>
#include <QBrush>
#include <QColor>

CycleWindow::CycleWindow(QWidget *parent) :
    QWidget(parent)
  , ui(new Ui::CycleWindow)
  , m_cycle(0)
  , m_plcs(nullptr)
  , m_db(nullptr)
{
    ui->setupUi(this);
    connect(ui->btnNext, &QToolButton::clicked, [&](bool checked)
    {
        (void)checked;
        if(m_plcs && m_cycle > 0 && m_steps.count() > 0)
        {
            int base = (m_cycle == 211) ? 6: 26;
            m_plcs->setCommand(base, m_step + 2);
        }
    });
    connect(ui->btnPrev, &QToolButton::clicked, [&](bool checked)
    {
        (void)checked;
        if(m_plcs && m_cycle > 0 && m_steps.count() > 0)
        {
            int base = (m_cycle == 211) ? 6: 26;
            if(m_step > 0)
                m_plcs->setCommand(base, m_step);
        }
    });
    connect(ui->btnNext, &QToolButton::released, [&]()
    {
        int base = (m_cycle == 211) ? 6: 26;
        m_plcs->setCommand(base, 0);
    });
    connect(ui->btnPrev, &QToolButton::released, [&]()
    {
        int base = (m_cycle == 211) ? 6: 26;
        m_plcs->setCommand(base, 0);
    });
}
CycleWindow::~CycleWindow()
{
    delete ui;
}
// activate one cycle to show
void CycleWindow::setCycle(int num, ZMariaDB *db, Plcs *plcs)
{
    m_cycle = num;
    m_db = db;
    m_plcs = plcs;
    if(m_plcs)
        m_plcs->resetAllCommand();
    recreateButtons();
}
// update data to show
void CycleWindow::updateData(const QByteArray& data)
{
    updateHeader(data);
    updateList(data);
    updateLeds(data);
    ui->lvMessages->resizeColumnsToContents();
    ui->lvMessages->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}
void CycleWindow::recreateButtons()
{
    QList<QAbstractButton*> toremove;
    for (int i = 0; i < ui->btnLayout->count(); ++i)
    {
        QAbstractButton *btn = reinterpret_cast<QAbstractButton *>(ui->btnLayout->itemAt(i)->widget());
        if(btn != NULL)
        {
            toremove.append(btn);
        }
    }
    for(auto btn : toremove)
    {
        ui->btnLayout->removeWidget(btn);
        delete btn;
    }
    if(m_db)
    {
        if(m_db->query(QString("SELECT * FROM `button` where `equipment`=%1 order by `idx`").arg(m_cycle).toStdString()))
        {
            const auto recs = m_db->getAllRecords();
            int cntfwd = 0;
            int cntbwd = 0;
            for(const auto r: recs)
            {
                auto desc = r.at("description");
                int col = std::stoi(r.at("idx"));
                int row = 2 - std::stoi(r.at("fwd_bwd"));
                auto p = new CycleWindow::PushButton();
                p->setText(QString::fromStdString(desc));
                p->setIcon(QIcon(":/pict/gray"));
                p->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                p->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                p->setMinimumHeight(64);
                p->setIconSize(QSize(32, 32));
                p->setUserData(col, row);
                ui->btnLayout->addWidget(p, row, col);
                connect(p, &QPushButton::pressed, this, &CycleWindow::pressedButton);
                connect(p, &QPushButton::released, this, &CycleWindow::releasedButton);
            }
        }
        if(m_db->query(QString("SELECT * FROM `step` where `equipment`=%1 order by `idx`").arg(m_cycle).toStdString()))
        {
            m_steps.clear();
            const auto recs = m_db->getAllRecords();
            for(const auto r: recs)
            {
                m_steps[std::stoi(r.at("idx"))] = QString::fromStdString(r.at("description"));
            }
        }
        if(m_db->query(QString("SELECT * FROM `fault` where `equipment`=%1 order by `idx`").arg(m_cycle).toStdString()))
        {
            m_messages.clear();
            const auto recs = m_db->getAllRecords();
            for(const auto r: recs)
            {
                m_messages[std::stoi(r.at("idx"))+1] = QString::fromStdString(r.at("description"));
            }
        }
        if(m_db->query("SELECT * FROM `fault` where `equipment`=0 order by `idx`"))
        {
            m_failures.clear();
            const auto recs = m_db->getAllRecords();
            for(const auto r: recs)
            {
                m_failures[std::stoi(r.at("idx"))+1] = QString::fromStdString(r.at("description"));
            }
        }
    }
}
void CycleWindow::pressedButton()
{
    actCommand(true);
}
void CycleWindow::releasedButton()
{
    actCommand(false);
}
void CycleWindow::actCommand(bool set)
{
    auto p = reinterpret_cast<PushButton*>(sender());
    if(p)
    {
        int base = (m_cycle == 211) ? 2: 22;
        if(p->m_row == 0)
            base += 2;
        int byte = base;
        int bit = p->m_col;
        if(set)
            m_plcs->setCommand(byte, 1 << bit);
        else
            m_plcs->setCommand(byte, 0);
    }
}

// update top bar
void CycleWindow::updateHeader(const QByteArray &data)
{
    int base = (m_cycle == 211) ? 0: 40;
    auto alive = reinterpret_cast<const short*>(&data.data()[base]);
    auto step = reinterpret_cast<const short*>(&data.data()[base + 2]);
    m_step = *step;
    ui->leLiveCounter->setText(QString::number(*alive));
    ui->leStepNumber->setText(QString::number(*step));
    ui->leStepDescription->setText(m_steps[*step]);
}
// update list of messages
void CycleWindow::updateList(const QByteArray &data)
{
    auto old1 = ui->lvMessages->selectionModel();
    ui->lvMessages->setModel(new ListMessages(data, m_cycle, this));
    delete old1;
}
// update color of buttons
void CycleWindow::updateLeds(const QByteArray &data)
{
    for (int i = 0; i < ui->btnLayout->count(); ++i)
    {
        auto btn = reinterpret_cast<PushButton *>(ui->btnLayout->itemAt(i)->widget());
        if(btn != NULL && btn->m_col < 5 && data.length() >= 80)
        {
            int base = (m_cycle == 211) ? 30: 70;

            if(btn->m_row == 0)
                btn->setIcon(data[btn->m_col * 2 + base + 1] & 0x02 ? QIcon(":/pict/green"): QIcon(":/pict/gray"));
            else
                btn->setIcon(data[btn->m_col * 2 + base] & 0x02 ? QIcon(":/pict/green"): QIcon(":/pict/gray"));
            if(btn->m_col == 1 && btn->m_row == 0 && data[btn->m_col * 2 + base] & 0x20)
                btn->setIcon(QIcon(":/pict/yellow"));
        }
    }
}

int CycleWindow::ListMessages::rowCount(const QModelIndex &parent) const
{
    return m_cntFailures + m_cntMessages;
}


int CycleWindow::ListMessages::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant CycleWindow::ListMessages::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    int r = index.row();
    int c = index.column();
    int base = (m_cycle == 211) ? 10: 50;
    if(r < m_cntFailures)
        base = 80;
    auto f = (short*)(m_data.data() + (r * 2 + base));
    if(role == Qt::DisplayRole)
    {
        if(c == 0)
            ret = QVariant((*f) % 128);
        else if(m_super)
        {
            if(r < m_cntFailures)
                ret = m_super->m_failures[(*f) % 128];
            else
                ret = m_super->m_messages[(*f) % 128];
        }
    }
    else if(role == Qt::BackgroundRole)
    {
        if(*f > 128)
            ret = QBrush(Qt::yellow);
        else if(r < m_cntFailures)
            ret = QBrush(Qt::red);
        else
            ret = QBrush(Qt::green);
    }
    return ret;
}
