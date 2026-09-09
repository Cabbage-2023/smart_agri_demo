#ifndef SENSORWORKER_H
#define SENSORWORKER_H

#include <QObject>
#include <QFile>
#include "aht20device.h"
#include "bh1726device.h"

/* 一次采样结果(跨线程值传递, 已注册元类型) */
struct Sample
{
    float   tempC = -100.0f;    /* 温度 ℃, 无效时为 -100 */
    float   humRH = -1.0f;      /* 湿度 %RH, 无效时为 -1 */
    int     lux   = -1;         /* 光照 lux, 无效时为 -1 */
    bool    ahtOk = false;
    bool    bhOk  = false;
    qint64  tsMs  = 0;          /* 采样时刻 (ms since epoch) */
};

/*
 * 采集工作线程对象(移入 QThread 运行)。
 * 定时采样 AHT20 + BH1726, 采样结果经 sampleReady 队列信号发给 UI;
 * CSV 日志落盘在此线程完成, 避免 UI 线程做磁盘/网络 IO。
 */
class SensorWorker : public QObject
{
    Q_OBJECT
public:
    explicit SensorWorker(QObject *parent = 0);
    ~SensorWorker();

    Sample lastSample() const { return m_last; }

public slots:
    void start();               /* 线程启动后由 QThread::started 触发 */
    void stop();                /* 退出前收尾(阻塞调用方线程) */

signals:
    void sampleReady(const Sample &s);
    void statusMsg(const QString &text);

private slots:
    void doSample();

private:
    void openCsv();
    void appendCsv(const Sample &s);
    void postStatus(const QString &text);

    Aht20Device  *m_aht;
    Bh1726Device *m_bh;
    QFile         m_csv;
    bool          m_csvOpen;
    bool          m_csvWarned;
    bool          m_running;
    Sample        m_last;
};

#endif // SENSORWORKER_H
