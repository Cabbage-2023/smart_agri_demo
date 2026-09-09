#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <QString>

/*
 * 板载 LED 控制 (leds-gpio 子系统)。
 * led1 运行指示 / led2 温湿度告警 / led3 风扇状态
 */
class LedController
{
public:
    LedController();

    bool init();               /* 逐一关闭 trigger, 切到手动模式 */
    bool setOn(int ledNo, bool on);   /* ledNo: 1..3 */
    void allOff();

    QString lastError() const { return m_err; }

private:
    QString m_err;
};

#endif // LEDCONTROLLER_H
