#include "fancontroller.h"
#include "hwconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <unistd.h>

FanController::FanController()
    : m_duty(0)
{
}

bool FanController::writeFile(const QString &path, const QString &content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
    {
        m_err = QStringLiteral("写 %1 失败: %2").arg(path, f.errorString());
        return false;
    }
    /* sysfs 的 store 回调失败时 write() 返回 -1, 必须检查(open 成功不代表写成功) */
    if (f.write(content.toUtf8()) < 0)
    {
        m_err = QStringLiteral("写 %1 失败: %2").arg(path, f.errorString());
        return false;
    }
    f.close();
    return true;
}

/* 板厂内核(ELF1)属性名为 period/duty_cycle(无 _ns 后缀);
   标准 pwm core 为 period_ns/duty_cycle_ns。按实际存在的文件名自动选择。 */
static QString pwmAttrPath(const QString &pwmDir, const char *attrWithNs)
{
    const QString full = pwmDir + QLatin1Char('/') + QLatin1String(attrWithNs);
    if (QFileInfo::exists(full))
        return full;
    QString oldName = QLatin1String(attrWithNs);
    oldName.chop(3);                /* period_ns -> period */
    const QString old = pwmDir + QLatin1Char('/') + oldName;
    return QFileInfo::exists(old) ? old : full;
}

/* 若 pwm0 尚未导出则写 export */
bool FanController::ensureExported()
{
    if (QFileInfo::exists(m_pwmDir))
        return true;

    QFile ex(m_chipDir + "/export");
    if (!ex.open(QIODevice::WriteOnly))
    {
        m_err = QStringLiteral("打开 export 失败: %1").arg(m_chipDir);
        return false;
    }
    ex.write(QByteArray::number(Hw::fanChannel()));
    ex.close();

    /* sysfs 导出为异步创建, 轮询等待 */
    for (int i = 0; i < 20; ++i)
    {
        if (QFileInfo::exists(m_pwmDir))
            return true;
        usleep(50000);
    }
    m_err = QStringLiteral("等待 pwm%1 节点超时").arg(Hw::fanChannel());
    return false;
}

bool FanController::init()
{
    m_chipDir = Hw::fanPwmChip();
    m_pwmDir = QString("%1/pwm%2").arg(m_chipDir).arg(Hw::fanChannel());

    if (!QFileInfo::exists(m_chipDir))
    {
        m_err = QStringLiteral("PWM 芯片目录不存在: %1 (可用 ls /sys/class/pwm 检查)")
                .arg(m_chipDir);
        return false;
    }
    if (!ensureExported())
        return false;

    /* 固定周期 20kHz (属性名自动兼容 period_ns / period) */
    if (!writeFile(pwmAttrPath(m_pwmDir, "period_ns"), QString::number(Hw::fanPeriodNs())))
        return false;

    /* 极性取反(可选, 部分风扇驱动板低电平导通) */
    if (Hw::fanInvert())
    {
        QFileInfo pol(m_pwmDir + "/polarity");
        if (pol.exists() && !writeFile(m_pwmDir + "/polarity", QStringLiteral("inversed")))
            return false;
    }

    /* 先给 0 占空再使能, 避免上电瞬间全速 */
    if (!writeFile(pwmAttrPath(m_pwmDir, "duty_cycle_ns"), QStringLiteral("0")))
        return false;
    if (!writeFile(m_pwmDir + "/enable", QStringLiteral("1")))
    {
        m_err = QStringLiteral("PWM enable 失败: %1").arg(m_err);
        return false;
    }
    m_err.clear();
    return true;
}

bool FanController::setDuty(int percent)
{
    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;
    if (percent == m_duty)
        return true;

    qint64 dutyNs = (qint64)Hw::fanPeriodNs() * percent / 100;
    if (!writeFile(pwmAttrPath(m_pwmDir, "duty_cycle_ns"), QString::number(dutyNs)))
        return false;
    m_duty = percent;
    m_err.clear();
    return true;
}

void FanController::shutdown()
{
    if (m_pwmDir.isEmpty())
        return;
    writeFile(pwmAttrPath(m_pwmDir, "duty_cycle_ns"), QStringLiteral("0"));
    writeFile(m_pwmDir + "/enable", QStringLiteral("0"));
    m_duty = 0;
}
