#include "mainwindow.h"
#include "trendchart.h"
#include "hwconfig.h"

#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_worker(0)
    , m_fanOk(false)
{
    setWindowTitle(QStringLiteral("智慧农业环境监测与控制系统"));
    setMinimumSize(760, 460);

    QWidget *central = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(8, 6, 8, 4);
    root->setSpacing(5);

    /* ================= 顶栏 ================= */
    QHBoxLayout *top = new QHBoxLayout;
    QLabel *title = new QLabel(QStringLiteral("智慧农业环境监测与控制系统"));
    title->setObjectName("title");
    m_clock = new QLabel;
    m_clock->setObjectName("clock");
    m_clock->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_modeTag = new QLabel(QStringLiteral("手动"));
    m_modeTag->setObjectName("modeTag");
    m_modeTag->setStyleSheet(QStringLiteral("background:#ffb74d;"));
    QPushButton *btnQuit = new QPushButton(QStringLiteral("退出"));
    connect(btnQuit, SIGNAL(clicked()), this, SLOT(close()));
    top->addWidget(title);
    top->addStretch(1);
    top->addWidget(m_clock);
    top->addSpacing(12);
    top->addWidget(m_modeTag);
    top->addSpacing(6);
    top->addWidget(btnQuit);
    root->addLayout(top);

    /* ================= 中部: 数值卡片 + 控制面板 ================= */
    QHBoxLayout *mid = new QHBoxLayout;
    mid->setSpacing(6);

    QVBoxLayout *cards = new QVBoxLayout;
    cards->setSpacing(5);

    QWidget *c1 = buildSensorCard(QStringLiteral("温 度"), "card");
    QWidget *c2 = buildSensorCard(QStringLiteral("湿 度"), "card");
    QWidget *c3 = buildSensorCard(QStringLiteral("光 照"), "card");
    cards->addWidget(c1);
    cards->addWidget(c2);
    cards->addWidget(c3);
    mid->addLayout(cards, 1);

    /* 控制面板 */
    QWidget *panel = new QWidget;
    panel->setObjectName("panel");
    panel->setFixedWidth(300);
    QVBoxLayout *pl = new QVBoxLayout(panel);
    pl->setContentsMargins(12, 8, 12, 8);
    pl->setSpacing(6);

    QLabel *fanTitle = new QLabel(QStringLiteral("风扇控制"));
    fanTitle->setStyleSheet(QStringLiteral("color:#4dd0e1;font-size:15px;font-weight:bold;"));
    pl->addWidget(fanTitle);

    QHBoxLayout *modeRow = new QHBoxLayout;
    m_radManual = new QRadioButton(QStringLiteral("手动"));
    m_radAuto   = new QRadioButton(QStringLiteral("自动控温"));
    m_radManual->setChecked(true);          /* 注意: 此处先不 connect, 控件未建完 */
    modeRow->addWidget(m_radManual);
    modeRow->addWidget(m_radAuto);
    modeRow->addStretch(1);
    pl->addLayout(modeRow);

    QHBoxLayout *thrRow = new QHBoxLayout;
    QLabel *thrLbl = new QLabel(QStringLiteral("控温阈值"));
    thrLbl->setObjectName("caption");
    m_spinThr = new QSpinBox;
    m_spinThr->setRange(20, 40);
    m_spinThr->setValue(28);
    m_spinThr->setSuffix(QStringLiteral(" ℃"));
    connect(m_spinThr, SIGNAL(valueChanged(int)), this, SLOT(onThresholdChanged(int)));
    /* 全部控件就绪后再连模式切换, 并同步一次初始使能状态 */
    connect(m_radManual, SIGNAL(toggled(bool)), this, SLOT(onModeChanged()));
    connect(m_radAuto,   SIGNAL(toggled(bool)), this, SLOT(onModeChanged()));
    thrRow->addWidget(thrLbl);
    thrRow->addWidget(m_spinThr);
    thrRow->addStretch(1);
    pl->addLayout(thrRow);

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(0);
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
    pl->addWidget(m_slider);

    QHBoxLayout *dutyRow = new QHBoxLayout;
    QLabel *outLbl = new QLabel(QStringLiteral("风扇输出"));
    outLbl->setObjectName("caption");
    m_dutyBig = new QLabel(QStringLiteral("0%"));
    m_dutyBig->setObjectName("dutyBig");
    m_dutyLabel = new QLabel(QStringLiteral("PWM 状态: 未知"));
    m_dutyLabel->setObjectName("stInfo");
    dutyRow->addWidget(outLbl);
    dutyRow->addWidget(m_dutyBig);
    dutyRow->addStretch(1);
    pl->addLayout(dutyRow);
    pl->addWidget(m_dutyLabel);

    pl->addSpacing(2);
    QLabel *ledCaption = new QLabel(QStringLiteral("板载 LED 状态"));
    ledCaption->setObjectName("caption");
    pl->addWidget(ledCaption);
    QHBoxLayout *ledRow = new QHBoxLayout;
    m_dotLed1 = makeDot(14, "#3a4453");
    m_dotLed2 = makeDot(14, "#3a4453");
    m_dotLed3 = makeDot(14, "#3a4453");
    QLabel *tLed1 = new QLabel(QStringLiteral("运行"));
    QLabel *tLed2 = new QLabel(QStringLiteral("告警"));
    QLabel *tLed3 = new QLabel(QStringLiteral("风扇"));
    tLed1->setObjectName("ledTxt");
    tLed2->setObjectName("ledTxt");
    tLed3->setObjectName("ledTxt");
    ledRow->addWidget(m_dotLed1); ledRow->addWidget(tLed1);
    ledRow->addSpacing(14);
    ledRow->addWidget(m_dotLed2); ledRow->addWidget(tLed2);
    ledRow->addSpacing(14);
    ledRow->addWidget(m_dotLed3); ledRow->addWidget(tLed3);
    ledRow->addStretch(1);
    pl->addLayout(ledRow);
    pl->addStretch(1);

    mid->addWidget(panel);
    root->addLayout(mid, 1);

    /* ================= 曲线 ================= */
    m_chart = new TrendChart;
    m_chart->setMinimumHeight(120);
    root->addWidget(m_chart);

    /* ================= 状态行 ================= */
    m_stBar = new QLabel(QStringLiteral("正在启动..."));
    m_stBar->setObjectName("stBar");
    m_stBar->setWordWrap(false);
    m_stSummary = new QLabel;
    m_stSummary->setObjectName("stInfo");
    root->addWidget(m_stSummary);
    root->addWidget(m_stBar);

    setCentralWidget(central);

    /* 初始控件状态(手动模式: 滑块可用, 阈值禁用) */
    m_slider->setEnabled(true);
    m_spinThr->setEnabled(false);
    onModeChanged();

    /* ================= 硬件初始化 ================= */
    m_fanOk = m_fan.init();
    if (m_fanOk)
    {
        m_dutyLabel->setText(QStringLiteral("PWM 状态: 正常 (%1)").arg(Hw::fanPwmChip()));
        setDutyUi(0);
    }
    else
    {
        m_dutyLabel->setText(QStringLiteral("PWM 状态: %1").arg(m_fan.lastError()));
        m_slider->setEnabled(false);
        m_stBar->setText(m_fan.lastError());
    }
    m_radAuto->setEnabled(m_fanOk);

    m_led.init();
    m_led.setOn(1, true);           /* led1 运行指示 */
    setDotColor(m_dotLed1, QColor("#69f0ae"));

    /* ================= 采集线程 ================= */
    m_worker = new SensorWorker;
    m_worker->moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), m_worker, SLOT(start()));
    connect(m_worker, SIGNAL(sampleReady(Sample)), this, SLOT(onSample(Sample)));
    connect(m_worker, SIGNAL(statusMsg(QString)), this, SLOT(onStatusMsg(QString)));
    m_thread.start();

    /* ================= 时钟 ================= */
    QTimer *clockTimer = new QTimer(this);
    clockTimer->setInterval(1000);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(onTickClock()));
    clockTimer->start();
    onTickClock();
}

MainWindow::~MainWindow()
{
}

/* 传感器数值卡片: caption 卡内小字, 数值+单位由调用方再布局 */
QWidget *MainWindow::buildSensorCard(const QString &caption, const QString &objName)
{
    QWidget *card = new QWidget;
    card->setObjectName(objName);
    QHBoxLayout *row = new QHBoxLayout(card);
    row->setContentsMargins(14, 6, 14, 6);

    QVBoxLayout *lcol = new QVBoxLayout;
    lcol->setSpacing(0);
    QLabel *cap = new QLabel(caption);
    cap->setObjectName("caption");
    QLabel *dot = makeDot(10, "#3a4453");
    QHBoxLayout *capRow = new QHBoxLayout;
    capRow->setSpacing(6);
    capRow->addWidget(cap);
    capRow->addWidget(dot);
    capRow->addStretch(1);
    lcol->addLayout(capRow);

    QHBoxLayout *valRow = new QHBoxLayout;
    valRow->setSpacing(6);
    QLabel *val = new QLabel("--");
    QLabel *unit = new QLabel;
    unit->setObjectName("unit");
    valRow->addWidget(val);
    valRow->addWidget(unit);
    valRow->addStretch(1);
    lcol->addLayout(valRow);

    row->addLayout(lcol);
    row->addStretch(1);

    /* 登记到成员, 按卡片类别赋值 */
    if (caption.contains(QStringLiteral("温")))
    {
        m_valTemp = val; m_dotTemp = dot;
        val->setStyleSheet(QStringLiteral("color:#ff8a65;font-size:32px;font-weight:bold;"));
        unit->setText(QStringLiteral("℃"));
    }
    else if (caption.contains(QStringLiteral("湿")))
    {
        m_valHum = val; m_dotHum = dot;
        val->setStyleSheet(QStringLiteral("color:#4fc3f7;font-size:32px;font-weight:bold;"));
        unit->setText(QStringLiteral("%RH"));
    }
    else
    {
        m_valLux = val; m_dotLux = dot;
        val->setStyleSheet(QStringLiteral("color:#ffd54f;font-size:32px;font-weight:bold;"));
        unit->setText(QStringLiteral("lux"));
    }
    return card;
}

QLabel *MainWindow::makeDot(int size, const char *colorOff)
{
    QLabel *dot = new QLabel;
    dot->setFixedSize(size, size);
    dot->setStyleSheet(QStringLiteral("background:%1;border-radius:%2px;")
                       .arg(QLatin1String(colorOff)).arg(size / 2));
    return dot;
}

void MainWindow::setDotColor(QLabel *dot, const QColor &c)
{
    int r = dot->width() / 2;
    dot->setStyleSheet(QStringLiteral("background:%1;border-radius:%2px;")
                       .arg(c.name()).arg(qMax(1, r)));
}

/* =============== 采样到达 =============== */
void MainWindow::onSample(const Sample &s)
{
    m_last = s;

    if (s.ahtOk)
    {
        m_valTemp->setText(QString::number(s.tempC, 'f', 1));
        m_valHum->setText(QString::number(s.humRH, 'f', 1));
        bool hot = s.tempC > 38.0f;
        bool wet = s.humRH > 90.0f;
        setDotColor(m_dotTemp, hot ? QColor("#ff5252") : QColor("#69f0ae"));
        setDotColor(m_dotHum, wet ? QColor("#ff5252") : QColor("#69f0ae"));
    }
    else
    {
        m_valTemp->setText("--");
        m_valHum->setText("--");
        setDotColor(m_dotTemp, QColor("#3a4453"));
        setDotColor(m_dotHum, QColor("#3a4453"));
    }
    if (s.bhOk)
    {
        m_valLux->setText(QString::number(s.lux));
        setDotColor(m_dotLux, QColor("#69f0ae"));
    }
    else
    {
        m_valLux->setText("--");
        setDotColor(m_dotLux, QColor("#3a4453"));
    }

    m_chart->addSample(s.ahtOk ? s.tempC : qQNaN(),
                       s.ahtOk ? s.humRH : qQNaN(),
                       s.bhOk ? s.lux : -1);

    if (m_radAuto->isChecked())
        applyAutoDuty(s);
    refreshLeds(s);
    refreshStatusLine(s);
}

void MainWindow::applyAutoDuty(const Sample &s)
{
    if (!s.ahtOk || !m_fanOk)
        return;
    int thr = m_spinThr->value();
    int duty = 0;
    if (s.tempC > thr)
    {
        if (s.tempC >= thr + 8)
            duty = 100;
        else
            duty = (int)((s.tempC - thr) * 100.0 / 8.0 + 0.5);
    }
    if (m_fan.setDuty(duty))
        setDutyUi(duty);
    else
        m_stBar->setText(m_fan.lastError());
}

/* =============== 用户操作 =============== */
void MainWindow::onSliderChanged(int value)
{
    if (m_radAuto->isChecked())
        return;                 /* 自动模式下滑块只做显示 */
    if (!m_fanOk)
        return;
    if (m_fan.setDuty(value))
        setDutyUi(value);
    else
        m_stBar->setText(m_fan.lastError());
}

void MainWindow::onModeChanged()
{
    bool autoMode = m_radAuto->isChecked();
    m_slider->setEnabled(!autoMode);
    m_spinThr->setEnabled(autoMode);
    m_modeTag->setText(autoMode ? QStringLiteral("自动") : QStringLiteral("手动"));
    m_modeTag->setStyleSheet(autoMode ? QStringLiteral("background:#4dd0e1;color:#102027;")
                                      : QStringLiteral("background:#ffb74d;color:#3e2723;"));
    if (autoMode)
        applyAutoDuty(m_last);
}

void MainWindow::onThresholdChanged(int)
{
    if (m_radAuto->isChecked())
        applyAutoDuty(m_last);
}

void MainWindow::setDutyUi(int duty)
{
    m_dutyBig->setText(QStringLiteral("%1%").arg(duty));
    m_slider->blockSignals(true);
    m_slider->setValue(duty);
    m_slider->blockSignals(false);
    refreshLeds(m_last);
}

/* =============== LED 逻辑 =============== */
void MainWindow::refreshLeds(const Sample &s)
{
    bool alarm = s.ahtOk && (s.tempC > 38.0f || s.humRH > 90.0f);
    m_led.setOn(2, alarm);
    setDotColor(m_dotLed2, alarm ? QColor("#ff5252") : QColor("#3a4453"));

    bool fanOn = m_fan.duty() > 0;
    m_led.setOn(3, fanOn);
    setDotColor(m_dotLed3, fanOn ? QColor("#ba68c8") : QColor("#3a4453"));
}

/* =============== 状态行 =============== */
void MainWindow::refreshStatusLine(const Sample &s)
{
    QString txt;
    txt += s.ahtOk ? QStringLiteral("AHT20 在线") : QStringLiteral("AHT20 离线");
    txt += QStringLiteral("  |  ");
    txt += s.bhOk ? QStringLiteral("BH1726 在线") : QStringLiteral("BH1726 离线");
    txt += QStringLiteral("  |  PWM ");
    txt += m_fanOk ? QStringLiteral("正常") : QStringLiteral("异常");
    txt += QStringLiteral("  |  日志 ");
    txt += Hw::logFilePath();
    bool allOk = s.ahtOk && s.bhOk && m_fanOk;
    m_stSummary->setText(txt);
    m_stSummary->setObjectName(allOk ? "stInfo" : "stBar");
    /* objectName 变更后需刷新样式 */
    m_stSummary->style()->unpolish(m_stSummary);
    m_stSummary->style()->polish(m_stSummary);
}

void MainWindow::onStatusMsg(const QString &text)
{
    m_stBar->setText(text);
}

void MainWindow::onTickClock()
{
    m_clock->setText(QDateTime::currentDateTime()
                     .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
}

/* =============== 退出 =============== */
void MainWindow::stopWorker()
{
    if (!m_worker)
        return;
    QMetaObject::invokeMethod(m_worker, "stop", Qt::BlockingQueuedConnection);
    m_thread.quit();
    m_thread.wait(3000);
    delete m_worker;            /* 线程已停, 回主线程释放 */
    m_worker = 0;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    stopWorker();
    m_fan.shutdown();
    m_led.allOff();
    e->accept();
    QMainWindow::closeEvent(e);
}
