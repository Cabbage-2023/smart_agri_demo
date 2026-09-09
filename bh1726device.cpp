#include "bh1726device.h"
#include "hwconfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/input.h>

Bh1726Device::Bh1726Device()
    : m_fd(-1)
    , m_lux(0)
{
}

Bh1726Device::~Bh1726Device()
{
    if (m_fd >= 0)
        ::close(m_fd);
}

/* 扫描 sysfs 下挂 BH1726 的 i2c 设备目录(以 -0029 结尾且含 enable 属性) */
QString Bh1726Device::findSysfsDir()
{
    QDir base(Hw::bh1726SysfsBase());
    const QStringList names = base.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < names.size(); ++i)
    {
        if (!names.at(i).endsWith(QLatin1String("-0029")))
            continue;
        QString dir = base.filePath(names.at(i));
        if (QFileInfo::exists(dir + "/" + Hw::bh1726EnableAttr()))
            return dir;
    }
    return QString();
}

/* sysfs 设备目录下 input/inputN/eventN -> /dev/input/eventN */
QString Bh1726Device::findEventPath(const QString &sysfsDir)
{
    QDir in(sysfsDir + "/input");
    if (!in.exists())
        return QString();
    const QStringList inputs = in.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < inputs.size(); ++i)
    {
        if (!inputs.at(i).startsWith(QLatin1String("input")))
            continue;
        QDir ev(in.filePath(inputs.at(i)));
        /* sysfs 中 inputN/eventN 是"目录"(内含 dev 等属性), 须同时列目录与文件 */
        const QStringList events = ev.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (int j = 0; j < events.size(); ++j)
        {
            if (events.at(j).startsWith(QLatin1String("event")))
                return QString("/dev/input/") + events.at(j);
        }
    }
    return QString();
}

bool Bh1726Device::init()
{
    m_sysfsDir = findSysfsDir();
    if (m_sysfsDir.isEmpty())
    {
        m_err = QStringLiteral("未找到 BH1726 sysfs 节点(*-0029 + %1)，"
                               "请确认内核已加载 bh1726 input 驱动")
                .arg(Hw::bh1726EnableAttr());
        return false;
    }

    /* 使能传感器(写 1; 若已使能重复写无害) */
    QFile en(m_sysfsDir + "/" + Hw::bh1726EnableAttr());
    if (!en.open(QIODevice::WriteOnly))
    {
        m_err = QStringLiteral("打开使能属性失败: %1").arg(m_sysfsDir + "/" + Hw::bh1726EnableAttr());
        return false;
    }
    en.write("1", 1);
    en.close();

    m_eventPath = findEventPath(m_sysfsDir);
    if (m_eventPath.isEmpty())
    {
        m_err = QStringLiteral("sysfs 下未发现 input 事件节点");
        return false;
    }
    m_fd = ::open(m_eventPath.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK);
    if (m_fd < 0)
    {
        m_err = QStringLiteral("open %1 失败: %2")
                .arg(m_eventPath).arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    m_err.clear();
    return true;
}

bool Bh1726Device::readLux(int &lux)
{
    if (m_fd < 0)
        return false;

    struct input_absinfo abs;
    memset(&abs, 0, sizeof(abs));
    if (ioctl(m_fd, EVIOCGABS(ABS_MISC), &abs) < 0)
    {
        /* 个别内核版本仅以事件流上报, 退化为非阻塞读事件 */
        struct input_event ev;
        while (::read(m_fd, &ev, sizeof(ev)) == (int)sizeof(ev))
        {
            if (ev.type == EV_ABS && ev.code == ABS_MISC)
                m_lux = ev.value;
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            m_err = QStringLiteral("EVIOCGABS 失败: %1")
                    .arg(QString::fromLocal8Bit(strerror(errno)));
            return false;
        }
    }
    else
    {
        m_lux = abs.value;
    }

    if (m_lux < 0)
        m_lux = 0;
    lux = m_lux;
    return true;
}
