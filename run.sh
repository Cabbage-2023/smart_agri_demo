#!/bin/sh
# 智慧农业环境监测与控制系统 - 板端启动脚本
# 用法(开发板终端, 脚本和可执行文件放在板子上任意目录均可):
#   sh run.sh
export DISPLAY=:0.0
export QT_XCB_FORCE_SOFTWARE_OPENGL=1
export QT_OPENGL=software
export QT_QPA_PLATFORM=xcb
# 出厂镜像 /etc/localtime 链接损坏导致全线 UTC, 显式固定北京时间(中国无夏令时, 对正常镜像也无害)
export TZ=CST-8
# 抑制 BH1726 驱动饱和告警在 console 刷屏 (dmesg 记录不受影响)
echo 4 > /proc/sys/kernel/printk 2>/dev/null
# 自动定位脚本所在目录, 复制到哪个目录都能直接跑
cd "$(dirname "$0")" || exit 1
# ZIP 下载 / Windows 传输会丢失可执行位, 运行前补 chmod (对已可执行的无害)
if [ -f ./deploy/smart_agri ]; then
    chmod +x ./deploy/smart_agri
    exec ./deploy/smart_agri "$@"
fi
if [ -f ./smart_agri ]; then
    chmod +x ./smart_agri
    exec ./smart_agri "$@"
fi
echo "错误: 找不到 smart_agri (应位于 deploy/ 目录或与 run.sh 同目录)" >&2
exit 1
