#!/bin/sh
# 智慧农业环境监测与控制系统 - 板端启动脚本
# 用法(开发板串口/终端):  sh /mnt/nfs/agri/run.sh
export DISPLAY=:0.0
export QT_XCB_FORCE_SOFTWARE_OPENGL=1
export QT_OPENGL=software
export QT_QPA_PLATFORM=xcb
# 抑制 BH1726 驱动饱和告警在 console 刷屏 (dmesg 记录不受影响)
echo 4 > /proc/sys/kernel/printk 2>/dev/null
cd /mnt/nfs/agri || exit 1
exec ./smart_agri "$@"
