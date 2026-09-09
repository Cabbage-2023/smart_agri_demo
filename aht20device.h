#ifndef AHT20DEVICE_H
#define AHT20DEVICE_H

#include <QString>

/*
 * AHT20 温湿度传感器。
 * 通路1(默认): 内核 misc 驱动字符设备 /dev/aht20 (飞凌官方 Qt 例程同款)
 *   一次 read() 返回 2 个 unsigned int: 湿度20bit raw / 温度20bit raw
 * 通路2(后备): 内核节点缺失时降级到 /dev/i2c-N 裸协议
 *   (AHT20 标准时序: 0xBE 软复位 -> 0xAC 0x33 0x00 触发 -> 80ms 后读 7 字节)
 */
class Aht20Device
{
public:
    Aht20Device();
    ~Aht20Device();

    bool init();                 /* 返回是否成功打开任一通路 */
    bool readSample(float &tempC, float &humRH);

    QString lastError() const { return m_err; }
    bool usingFallback() const { return m_i2cMode; }

private:
    bool openCharDev();
    bool openI2cBus();
    bool i2cWrite(const unsigned char *buf, int len);
    bool i2cRead(unsigned char *buf, int len);
    bool i2cTrigger();

    int     m_fd;
    bool    m_i2cMode;           /* true = 裸 I2C 通路 */
    QString m_err;
};

#endif // AHT20DEVICE_H
