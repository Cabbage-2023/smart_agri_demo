#!/bin/sh
# 智慧农业环境监测与控制系统 - 板端启动脚本
# 用法(开发板终端, 脚本和可执行文件放在板子上任意目录均可):
#   sh run.sh
export DISPLAY=:0.0
export QT_XCB_FORCE_SOFTWARE_OPENGL=1
export QT_OPENGL=software
export QT_QPA_PLATFORM=xcb
# 抑制 BH1726 驱动饱和告警在 console 刷屏 (dmesg 记录不受影响)
echo 4 > /proc/sys/kernel/printk 2>/dev/null
# 自动定位脚本所在目录, 复制到哪个目录都能直接跑
cd "$(dirname "$0")" || exit 1
if [ -x ./deploy/smart_agri ]; then
    exec ./deploy/smart_agri "$@"
fi
exec ./smart_agri "$@"
