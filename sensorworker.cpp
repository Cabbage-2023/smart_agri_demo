#include "sensorworker.h"
#include "hwconfig.h"

#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QThread>
#include <QDebug>

SensorWorker::SensorWorker(QObject *parent)
    : QObject(parent)
    , m_aht(0)
    , m_bh(0)
    , m_csvOpen(false)
    , m_csvWarned(false)
    , m_running(false)
{
    m_last.tsMs = 0;
}

SensorWorker::~SensorWorker()
{
    delete m_aht;
    delete m_bh;
    if (m_csv.isOpen())
        m_csv.close();
}

void SensorWorker::postStatus(const QString &text)
{
    emit statusMsg(text);
}

void SensorWorker::start()
{
    m_running = true;

    m_aht = new Aht20Device;
    if (m_aht->init())
    {
        postStatus(QStringLiteral("AHT20 就绪 (通路: %1)")
                   .arg(m_aht->usingFallback() ? QStringLiteral("i2c-dev 裸通路")
                                               : QStringLiteral("/dev/aht20 驱动")));
    }
    else
    {
        postStatus(QStringLiteral("AHT20 初始化失败: %1").arg(m_aht->lastError()));
    }

    m_bh = new Bh1726Device;
    if (m_bh->init())
    {
        postStatus(QStringLiteral("BH1726 就绪 (事件节点: %1)").arg(m_bh->eventPath()));
    }
    else
    {
        postStatus(QStringLiteral("BH1726 初始化失败: %1").arg(m_bh->lastError()));
    }

    openCsv();

    /* 单发定时链: 采样耗时不计入周期, 不会重入 */
    QTimer::singleShot(Hw::sampleIntervalMs(), this, SLOT(doSample()));
}

void SensorWorker::stop()
{
    m_running = false;
    if (m_aht)
    {
        delete m_aht;
        m_aht = 0;
    }
    if (m_bh)
    {
        delete m_bh;
        m_bh = 0;
    }
    if (m_csv.isOpen())
    {
        m_csv.flush();
        m_csv.close();
    }
}

void SensorWorker::openCsv()
{
    const QString path = Hw::logFilePath();
    QDir().mkpath(Hw::logDir());

    m_csv.setFileName(path);
    if (!m_csv.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        m_csvOpen = false;
        postStatus(QStringLiteral("CSV 日志打开失败(%1): %2")
                   .arg(path, m_csv.errorString()));
        return;
    }
    m_csvOpen = true;
    if (m_csv.size() == 0)
        m_csv.write("time,tempC,humRH,lux,ahtOk,bhOk\n");
}

void SensorWorker::appendCsv(const Sample &s)
{
    if (!m_csvOpen)
        return;
    const QString ts = QDateTime::fromMSecsSinceEpoch(s.tsMs)
                       .toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QByteArray line = QStringLiteral("%1,%2,%3,%4,%5,%6\n")
                      .arg(ts)
                      .arg(s.ahtOk ? QString::number(s.tempC, 'f', 2) : QStringLiteral("NA"))
                      .arg(s.ahtOk ? QString::number(s.humRH, 'f', 2) : QStringLiteral("NA"))
                      .arg(s.bhOk ? QString::number(s.lux) : QStringLiteral("NA"))
                      .arg(s.ahtOk ? 1 : 0)
                      .arg(s.bhOk ? 1 : 0)
                      .toUtf8();
    m_csv.write(line);
    m_csv.flush();
}

void SensorWorker::doSample()
{
    if (!m_running)
        return;

    Sample s;
    s.tsMs = QDateTime::currentMSecsSinceEpoch();

    if (m_aht)
    {
        float t = -100.0f, h = -1.0f;
        if (m_aht->readSample(t, h))
        {
            /* 合理性过滤: 越界数据视为本次无效 */
            if (t >= -40.0f && t <= 125.0f && h >= 0.0f && h <= 100.0f)
            {
                s.tempC = t;
                s.humRH = h;
                s.ahtOk = true;
            }
        }
    }

    if (m_bh)
    {
        int lux = -1;
        if (m_bh->readLux(lux))
        {
            s.lux = lux;
            s.bhOk = true;
        }
    }

    m_last = s;
    appendCsv(s);
    emit sampleReady(s);

    /* 单发定时: 采样耗时后重新计时, 避免重入 */
    QTimer::singleShot(Hw::sampleIntervalMs(), this, SLOT(doSample()));
}
