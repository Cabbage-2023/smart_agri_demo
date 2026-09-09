#ifndef HWCONFIG_H
#define HWCONFIG_H

/*
 * 硬件路径配置集中地。
 * 所有默认值按 ELF1 出厂镜像 + 飞凌官方 Qt 例程设定;
 * 实际板端以 probe.sh 输出为准, 可用环境变量覆盖而无需重新编译:
 *   SMARTAGRI_AHT_DEV     AHT20 字符设备 (默认 /dev/aht20)
 *   SMARTAGRI_AHT_I2C     AHT20 备用裸 I2C 总线 (默认 /dev/i2c-0)
 *   SMARTAGRI_PWM_CHIP    风扇 PWM 芯片目录 (默认 /sys/class/pwm/pwmchip6)
 *   SMARTAGRI_PWM_INVERT=1  极性取反 (部分风扇板低电平导通)
 *   SMARTAGRI_LOG_DIR     CSV 日志目录 (默认 /mnt/nfs/agri/log)
 *   SMARTAGRI_INTERVAL_MS 采样周期 (默认 1000)
 */

#include <QByteArray>
#include <QString>

namespace Hw {

inline QString env(const char *name, const QString &def)
{
    QByteArray v = qgetenv(name);
    return v.isEmpty() ? def : QString::fromUtf8(v);
}

/* ---------- 采样 ---------- */
inline int sampleIntervalMs()
{
    bool ok = false;
    int v = env("SMARTAGRI_INTERVAL_MS", "1000").toInt(&ok);
    return (ok && v >= 200 && v <= 60000) ? v : 1000;
}

/* ---------- AHT20 温湿度 ---------- */
inline QString aht20DevPath()     { return env("SMARTAGRI_AHT_DEV", "/dev/aht20"); }
inline QString aht20I2cBus()      { return env("SMARTAGRI_AHT_I2C", "/dev/i2c-0"); }
inline int     aht20I2cAddr()     { return 0x38; }

/* ---------- BH1726 光照 (input 子系统驱动) ---------- */
inline QString bh1726SysfsBase()  { return env("SMARTAGRI_BH_SYSFS", "/sys/bus/i2c/devices"); }
inline QString bh1726EnableAttr() { return QStringLiteral("enable_als_sensor"); }

/* ---------- 风扇 PWM (周期 50000ns = 20kHz) ---------- */
inline QString fanPwmChip()       { return env("SMARTAGRI_PWM_CHIP", "/sys/class/pwm/pwmchip6"); }
inline int     fanChannel()       { return 0; }
inline int     fanPeriodNs()      { return 50000; }   /* 周期 ns, 对应 20kHz */
inline bool    fanInvert()        { return qgetenv("SMARTAGRI_PWM_INVERT") == "1"; }

/* ---------- LED ---------- */
inline QString ledBrightness(int n) { return QString("/sys/class/leds/led%1/brightness").arg(n); }
inline QString ledTrigger(int n)    { return QString("/sys/class/leds/led%1/trigger").arg(n); }

/* ---------- CSV 日志 ---------- */
inline QString logDir()      { return env("SMARTAGRI_LOG_DIR", "/mnt/nfs/agri/log"); }
inline QString logFilePath() { return logDir() + "/smart_agri.csv"; }

} // namespace Hw

#endif // HWCONFIG_H
