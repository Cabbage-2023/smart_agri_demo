#ifndef BH1726DEVICE_H
#define BH1726DEVICE_H

#include <QString>

/*
 * BH1726 光照传感器 (ROHM BH1726NUC, I2C 地址 0x29)。
 * ELF1 出厂内核以 input 子系统驱动承载:
 *   - sysfs 使能:  /sys/bus/i2c/devices/<bus>-0029/enable_als_sensor 写 1
 *   - 数据通道:    input 事件上报 ABS_MISC 数值(内核已完成 lux 换算)
 * 本类启动时自动扫描 sysfs 定位 event 节点, 读取走 EVIOCGABS(非阻塞,
 * 免去事件流阻塞问题), 不依赖 /dev/input/eventX 的固定编号。
 */
class Bh1726Device
{
public:
    Bh1726Device();
    ~Bh1726Device();

    bool init();
    /* 返回最近一次光照值(lux); 失败返回 false, lux 保持旧值 */
    bool readLux(int &lux);

    QString lastError() const { return m_err; }
    QString eventPath() const { return m_eventPath; }

private:
    QString findSysfsDir();
    QString findEventPath(const QString &sysfsDir);

    int     m_fd;
    int     m_lux;
    QString m_sysfsDir;
    QString m_eventPath;
    QString m_err;
};

#endif // BH1726DEVICE_H
