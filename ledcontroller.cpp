#include "ledcontroller.h"
#include "hwconfig.h"

#include <QFile>

LedController::LedController()
{
}

bool LedController::init()
{
    bool allOk = true;
    for (int i = 1; i <= 3; ++i)
    {
        /* 先解除内核触发器占用(失败无妨: 可能本来就没绑定触发器) */
        QFile tr(Hw::ledTrigger(i));
        if (tr.open(QIODevice::WriteOnly))
        {
            tr.write("none", 4);
            tr.close();
        }
        if (!setOn(i, false))
            allOk = false;
    }
    m_err = allOk ? QString() : QStringLiteral("部分 LED 节点不可用, 请检查 /sys/class/leds");
    return allOk;
}

bool LedController::setOn(int ledNo, bool on)
{
    if (ledNo < 1 || ledNo > 3)
        return false;
    QFile f(Hw::ledBrightness(ledNo));
    if (!f.open(QIODevice::WriteOnly))
    {
        m_err = QStringLiteral("open %1 失败: %2")
                .arg(Hw::ledBrightness(ledNo), f.errorString());
        return false;
    }
    f.write(on ? "1" : "0", 1);
    f.close();
    m_err.clear();
    return true;
}

void LedController::allOff()
{
    for (int i = 1; i <= 3; ++i)
        setOn(i, false);
}
