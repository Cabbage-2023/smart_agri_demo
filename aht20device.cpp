#include "aht20device.h"
#include "hwconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

Aht20Device::Aht20Device()
    : m_fd(-1)
    , m_i2cMode(false)
{
}

Aht20Device::~Aht20Device()
{
    if (m_fd >= 0)
        ::close(m_fd);
}

/* 通路1: 出厂内核驱动字符设备 */
bool Aht20Device::openCharDev()
{
    const QString path = Hw::aht20DevPath();
    if (!QFileInfo::exists(path))
    {
        m_err = QStringLiteral("驱动节点 %1 不存在, 尝试 i2c-dev 裸通路").arg(path);
        return false;
    }
    m_fd = ::open(path.toLocal8Bit().constData(), O_RDWR | O_NONBLOCK);
    if (m_fd < 0)
    {
        m_err = QStringLiteral("open %1 失败: %2").arg(path).arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    return true;
}

/* 通路2: /dev/i2c-N 直接操作 */
bool Aht20Device::openI2cBus()
{
    const QString path = Hw::aht20I2cBus();
    m_fd = ::open(path.toLocal8Bit().constData(), O_RDWR);
    if (m_fd < 0)
    {
        m_err = QStringLiteral("open %1 失败: %2").arg(path).arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    if (ioctl(m_fd, I2C_SLAVE_FORCE, Hw::aht20I2cAddr()) < 0)
    {
        m_err = QStringLiteral("I2C_SLAVE_FORCE 0x%1 失败: %2")
                .arg(Hw::aht20I2cAddr(), 2, 16)
                .arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    m_i2cMode = true;

    /* 软复位 + 初始化(校准使能), 失败不致命 */
    unsigned char rst[] = { 0xBE };
    unsigned char ini[] = { 0xE1, 0x08, 0x00 };
    i2cWrite(rst, 1);
    usleep(10000);
    i2cWrite(ini, 3);
    usleep(10000);
    m_err.clear();
    return true;
}

bool Aht20Device::init()
{
    if (openCharDev())
        return true;
    if (openI2cBus())
        return true;
    return false;
}

bool Aht20Device::i2cWrite(const unsigned char *buf, int len)
{
    if (m_fd < 0)
        return false;
    int n = ::write(m_fd, buf, len);
    return n == len;
}

bool Aht20Device::i2cRead(unsigned char *buf, int len)
{
    if (m_fd < 0)
        return false;
    int n = ::read(m_fd, buf, len);
    return n == len;
}

bool Aht20Device::i2cTrigger()
{
    unsigned char cmd[] = { 0xAC, 0x33, 0x00 };
    if (!i2cWrite(cmd, 3))
    {
        m_err = QStringLiteral("AHT20 触发测量写失败: %1")
                .arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    usleep(90000);              /* 手册要求测量时间 > 80ms */
    return true;
}

/* 20bit raw 值换算 */
static float rawToTemp(unsigned int raw)
{
    return raw * 200.0f / 1048576.0f - 50.0f;   /* -50 ~ +150 C */
}

static float rawToHum(unsigned int raw)
{
    return raw * 100.0f / 1048576.0f;           /* 0 ~ 100 %RH */
}

bool Aht20Device::readSample(float &tempC, float &humRH)
{
    tempC = -100.0f;
    humRH = -1.0f;

    if (m_i2cMode)
    {
        /* 裸 I2C: 触发 -> 读 7 字节(状态+5数据+CRC, CRC 不校验) */
        if (!i2cTrigger())
            return false;
        unsigned char buf[7];
        if (!i2cRead(buf, 7))
        {
            m_err = QStringLiteral("AHT20 读数据失败: %1")
                    .arg(QString::fromLocal8Bit(strerror(errno)));
            return false;
        }
        if (buf[0] & 0x80)      /* bit7 busy, 重试一次 */
        {
            usleep(20000);
            if (!i2cRead(buf, 7))
                return false;
        }
        unsigned int rawH = ((unsigned int)buf[1] << 12) | ((unsigned int)buf[2] << 4)
                            | ((unsigned int)buf[3] >> 4);
        unsigned int rawT = (((unsigned int)buf[3] & 0x0F) << 16) | ((unsigned int)buf[4] << 8)
                            | (unsigned int)buf[5];
        humRH = rawToHum(rawH);
        tempC = rawToTemp(rawT);
        m_err.clear();
        return true;
    }

    /* 字符设备通路: 一次 read 返回 2 个 unsigned int (与官方例程一致) */
    unsigned int databuf[2] = { 0, 0 };
    int n = ::read(m_fd, databuf, sizeof(databuf));
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        m_err = QStringLiteral("AHT20 数据未就绪");
        return false;
    }
    if (n < 0)
    {
        m_err = QStringLiteral("AHT20 read 失败: %1")
                .arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    /* 官方驱动 read() 返回 0 但数据已填入; 兼容返回 8 的情况 */
    if (n == 0 || n == (int)sizeof(databuf))
    {
        humRH = rawToHum(databuf[0]);
        tempC = rawToTemp(databuf[1]);
        m_err.clear();
        return true;
    }
    m_err = QStringLiteral("AHT20 read 字节数异常(%1)").arg(n);
    return false;
}
