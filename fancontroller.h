#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

#include <QString>

/*
 * 风扇 PWM 控制(内核 pwm-sysfs)。
 * ELF1: PWM7 -> /sys/class/pwm/pwmchip6/pwm0, 周期 50000ns (20kHz)
 * 使用流程: export 通道 -> 写周期 -> (可选 polarity=inversed) -> enable -> duty
 */
class FanController
{
public:
    FanController();

    bool init();                 /* 导出通道并配置周期, 返回是否可用 */
    bool setDuty(int percent);   /* 0~100 */
    int  duty() const { return m_duty; }
    void shutdown();             /* 停风扇并禁用通道 */

    QString lastError() const { return m_err; }

private:
    bool writeFile(const QString &path, const QString &content);
    bool ensureExported();

    QString m_chipDir;
    QString m_pwmDir;
    int     m_duty;
    QString m_err;
};

#endif // FANCONTROLLER_H
