# 智慧农业环境监测与控制系统 (ELF1 / i.MX6ULL)

基于**飞凌 ELF1**(NXP i.MX6ULL)开发板的 Qt5 课程实训项目:实时采集温湿度与光照,
PWM 控制风扇自动控温,LED 状态指示,带实时曲线和 CSV 日志。全中文深色界面,适配
800×480 触摸屏。

> **不用编译,拿现成二进制就能跑** —— `deploy/` 目录里是交叉编译好的 ARM 可执行文件。
> 全程只需:一根网线 + 一个 pscp + 一条命令,10 分钟跑起来。

## 功能一览

| 功能 | 说明 |
|---|---|
| AHT20 温湿度 | 每秒刷新,数值 + 实时曲线 |
| BH1726 光照 | 每秒刷新(传感器在 ELF1S 扩展板上,见下) |
| 风扇控制 | **手动**:拖滑块 0~100% 即时调速;**自动**:超过阈值起转,线性提速 |
| 控温策略 | 阈值 20~40℃ 可调(界面点按/滑块);t ≤ 阈值停转,t ≥ 阈值+8℃ 全速 |
| LED 指示 | led1 运行中;led2 高温/高湿告警;led3 风扇转动 |
| CSV 日志 | 程序目录下 `log/smart_agri.csv`,每秒一行,自动追加 |
| 容错设计 | 单个硬件离线只显示 `--`,程序不崩、其余功能照常 |

## 硬件需求

- **飞凌 ELF1** 开发板(i.MX6ULL)及其**出厂镜像** —— 镜像自带 AHT20/BH1726 驱动、
  Qt 5.6.2 运行库和 WenQuanYi 中文字体,不需要额外装任何东西
- **AHT20** 温湿度传感器:焊在 ELF1 主板上(IIC1)
- **BH1726** 光照传感器:焊在 **ELF1S 扩展板**上,经 P2/P3 连接器由主板直读(IIC2)
  —— **没有扩展板也能跑**:程序自动探测,测不到时光照显示 `--`,其余功能不受影响
- 触摸屏(800×480)、串口调试线、网线

## 快速开始(10 分钟,傻瓜版)

不需要配 NFS、不需要装 SDK、不需要会编译。板子出厂镜像自带 SSH(dropbear),
root 默认**空密码**,直接传文件。

### 第 1 步:拿到文件

GitHub 页面点 **Code → Download ZIP**,解压;或命令行 `git clone`。
只需要其中两个东西:

```
run.sh                 # 启动脚本
deploy/smart_agri      # 预编译好的 ARM 程序(2.6 MB)
```

### 第 2 步:传到板子上

1. **网线**一头插电脑、一头插板子;或让两者处于同一局域网
   (Windows 手动设 IP 与板子同网段,如板子 `192.168.0.232`,电脑设 `192.168.0.50`,
   子网掩码 `255.255.255.0`)
2. 板子 IP 在**串口**里 `ifconfig` 查
3. Windows 打开 cmd,进入解压后的文件夹,执行:

```bat
pscp -scp -pw "" -r run.sh deploy root@板子的IP:/home/root/
```

- 没有 pscp?飞凌资料包里 PuTTY 工具目录有现成的;或用 WinSCP(新建会话时
  **文件协议选 SCP**)
- 第一次连接会问是否信任主机,输入 `y` 回车
- 密码提示直接**回车**(空密码)
- 传完可以在 pscp 里确认,或直接进行下一步,运行出错再回头查

> ⚠️ **必须加 `-scp` 参数!** 板子镜像只装了老 SCP 协议支持,没装 SFTP,
> 不加 `-scp` 会报 `sftp-server: No such file or directory`。

### 第 3 步:板子上运行

串口终端里执行:

```sh
sh /home/root/run.sh
```

看到全屏深色界面、数值每秒跳动就成功了。界面右上角关闭即退出。

> 文件放板子上**任意目录**都行(run.sh 会自动定位自身)。`/home/root` 是 root
> 用户的家目录,出厂镜像固定存在,最省事;SCP 协议**不会自动创建远端目录**,
> 所以换成别的目录时,必须选一个已经存在的(或先 `mkdir`)。

> 需要板子先有 X 图形桌面(出厂镜像开机默认已启动 matchbox 桌面)。
> 如果 `DISPLAY` 报错,说明桌面没起,先启动桌面再运行。

## 界面操作

| 操作 | 方法 |
|---|---|
| 手动/自动模式 | 界面切换按钮 |
| 手动调速 | 拖动风扇滑块,0~100% 即时生效 |
| 自动控温阈值 | 界面调 20~40℃(默认 28℃) |
| 光照饱和 | 手电筒直射会读到 6 万+ 的饱和值,这是传感器量程到头,正常现象 |

## 从源码编译(可选)

只有想改代码的人才需要,`deploy/` 里的二进制开箱即用。编译需要**飞凌交叉编译
SDK**(与出厂镜像配套的 Poky SDK,Qt 5.6.2):

```sh
source /opt/fsl-imx-x11/4.1.15-2.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
cd smart_agri          # 源码目录(本仓库根目录即源码)
qmake smart_agri.pro
make -j4
```

产物 `smart_agri` 拷到板上,与 `run.sh` 放同一目录执行即可。

### 硬件路径环境变量(不用改代码)

所有硬件路径都有默认值且可用环境变量覆盖,比如风扇接到别的 PWM 通道:

| 变量 | 默认 | 说明 |
|---|---|---|
| `SMARTAGRI_PWM_CHIP` | `/sys/class/pwm/pwmchip6` | 风扇 PWM 芯片目录 |
| `SMARTAGRI_PWM_INVERT=1` | — | 部分风扇板低电平导通,置 1 反转极性 |
| `SMARTAGRI_AHT_DEV` | `/dev/aht20` | AHT20 字符设备 |
| `SMARTAGRI_AHT_I2C` | `/dev/i2c-0` | AHT20 裸 I2C 备用总线 |
| `SMARTAGRI_LOG_DIR` | `./log` | CSV 日志目录(相对程序所在目录) |
| `SMARTAGRI_INTERVAL_MS` | `1000` | 采样周期(200~60000) |

## 目录结构

```
├── README.md            本说明
├── *.cpp *.h *.pro      Qt5.6 源码(19 个文件)
├── run.sh               板端启动脚本(自动定位目录,放哪都能跑)
├── build.sh             一键编译脚本(SDK 环境 source 后执行)
├── probe.sh             板端体检脚本:一键查看传感器/PWM/LED/字体是否就绪
└── deploy/
    └── smart_agri       预编译 ARM 可执行文件(直接部署用)
```

## 工作原理(简)

- **线程**:UI 主线程 + 采集工作线程(每秒一次,`QTimer::singleShot` 链防重入),
  队列信号跨线程传 `Sample{温度,湿度,光照,时间戳,各硬件OK标志}`
- **AHT20**:读 `/dev/aht20`(出厂 misc 驱动,20bit raw 换算);打不开时降级走
  `/dev/i2c-0` 裸 I2C 协议重试
- **BH1726**:扫描 sysfs 找 `*-0029` 设备 → 写 `enable_als_sensor` 使能 →
  打开其 input 事件节点,`EVIOCGABS(ABS_MISC)` 读光照(lux,内核驱动已换算)
- **风扇**:PWM7 = `/sys/class/pwm/pwmchip6/pwm0`,20 kHz(周期 50000 ns),
  占空比 0~100% 线性映射 duty;代码自动兼容 `period/duty_cycle` 与
  `period_ns/duty_cycle_ns` 两种内核属性命名
- **LED**:写 `/sys/class/leds/ledN/brightness`,先 `echo none > trigger` 释放心跳闪烁
- **CSV**:每秒追加一行 `时间,温度,湿度,光照,风扇占空比,各传感器状态`

## 常见问题 FAQ

| # | 问题 | 原因 / 解决 |
|---|---|---|
| 1 | `pscp` 报 `sftp-server: No such file or directory` | 没加 `-scp` 参数,见快速开始第 2 步 |
| 2 | 风扇不动,界面提示 PWM 异常 | 看风扇是否接在 PWM7;换通道用 `SMARTAGRI_PWM_CHIP` 覆盖(见环境变量表) |
| 3 | 光照显示 6 万+ 的数值 | 强光/手电直射导致**饱和**(量程到头),移开光源几秒自动回落,传感器没坏 |
| 4 | 光照一直显示 `--` | 多半是 ELF1S 扩展板没插好/没插(主板 IIC2 探测不到 BH1726);无扩展板属正常 |
| 5 | 运行报 DISPLAY 相关错误 | 板子 X 桌面没启动,先 `matchbox-wm &` 或重启进桌面再运行 |
| 6 | 串口控制台大量刷 `ALS/PS` 日志 | BH1726 驱动饱和告警,不影响运行,`run.sh` 已自动压低打印级别(dmesg 仍有完整记录) |
| 7 | CSV 日志在哪 | 程序所在目录的 `log/` 下,`smart_agri.csv`,每次运行自动追加 |
| 8 | 板子 SSH 连不上 / root 有密码 | 镜像被改过,改用**附录 A** 的 NFS 方式,或在板子上自行启动 dropbear |
| 9 | 屏幕显示中文变方块 | 出厂镜像自带 WenQuanYi 字体;精简过字体的镜像需自行补中文字体 |

## 附录 A:虚拟机 + NFS 部署(有 Ubuntu 虚拟机的同学)

有虚拟机的同学也可以走 NFS(虚拟机的 `/home/你的用户/nfs_rootfs` 往往已配好共享,
直接把文件丢进去即可,无需以下步骤)。需要从头配 NFS 时:

```sh
# 虚拟机 (Ubuntu) 上执行:
sudo apt-get install -y nfs-kernel-server
echo "/home/elf/nfs_rootfs *(rw,sync,no_root_squash,no_subtree_check)" | sudo tee -a /etc/exports
sudo service nfs-kernel-server restart

# 把文件放进共享目录:
mkdir -p /home/elf/nfs_rootfs/agri
cp run.sh deploy/smart_agri /home/elf/nfs_rootfs/agri/
```

```sh
# 板子串口上执行(挂载后直接跑):
mount -t nfs -o nolock,tcp 虚拟机IP:/home/elf/nfs_rootfs /mnt/nfs
sh /mnt/nfs/agri/run.sh
```

## 兼容性与验证声明

- 2026-09 在 ELF1 出厂镜像(内核 4.1.15,Qt 5.6.2)上实测通过:
  温湿度/光照实时刷新、风扇手动调速与自动控温、LED、中文界面、曲线、CSV 全部正常
- PWM 属性名、I2C 设备号等随镜像版本可能不同,代码已做自动兼容与降级,仍不通时
  用环境变量覆盖(见上表)

## License

[MIT](LICENSE)

---

*课程实训项目,基于飞凌官方出厂镜像与驱动编写,不含飞凌/ROHM 私有代码。*
