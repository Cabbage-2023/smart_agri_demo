#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "sensorworker.h"
#include "fancontroller.h"
#include "ledcontroller.h"

class QLabel;
class QSlider;
class QSpinBox;
class QRadioButton;
class TrendChart;

/* 智慧农业环境监测与控制系统 主窗口 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onSample(const Sample &s);
    void onStatusMsg(const QString &text);
    void onSliderChanged(int value);
    void onModeChanged();
    void onThresholdChanged(int value);
    void onTickClock();

private:
    QWidget *buildSensorCard(const QString &caption, const QString &objName);
    QLabel  *makeDot(int size, const char *colorOff);

    void applyAutoDuty(const Sample &s);   /* 自动控温规则 */
    void refreshLeds(const Sample &s);
    void refreshStatusLine(const Sample &s);
    void setDutyUi(int duty);
    void setDotColor(QLabel *dot, const QColor &c);
    void stopWorker();

    /* UI */
    QLabel *m_valTemp, *m_valHum, *m_valLux;
    QLabel *m_dotTemp, *m_dotHum, *m_dotLux;
    QLabel *m_dotLed1, *m_dotLed2, *m_dotLed3;
    QLabel *m_clock, *m_modeTag;
    QLabel *m_dutyLabel, *m_dutyBig;
    QLabel *m_stSummary, *m_stBar;
    QRadioButton *m_radManual, *m_radAuto;
    QSlider *m_slider;
    QSpinBox *m_spinThr;
    TrendChart *m_chart;

    /* 设备 */
    QThread        m_thread;
    SensorWorker  *m_worker;
    FanController  m_fan;
    LedController  m_led;

    bool m_fanOk;
    Sample m_last;
};

#endif // MAINWINDOW_H
