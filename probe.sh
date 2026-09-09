#!/bin/sh
# ELF1 hardware probe - run on the BOARD serial console:
#   sh /mnt/nfs/probe.sh
# then paste the whole output back.
echo "===== [1] kernel ====="
uname -a
echo "===== [2] char/i2c/input devices ====="
ls -l /dev/aht20 2>&1
ls /dev/i2c-* 2>&1
ls /dev/input/ 2>&1
echo "===== [3] /proc/bus/input/devices ====="
cat /proc/bus/input/devices 2>&1
echo "===== [4] i2c devices under /sys/bus/i2c ====="
ls /sys/bus/i2c/devices/ 2>&1
for d in /sys/bus/i2c/devices/*/; do
  echo "-- $d"
  echo "   name: $(cat ${d}name 2>&1)"
  echo "   driver: $(readlink -f ${d}driver 2>&1)"
  ls ${d} 2>&1 | tr '\n' ' '
  echo
  echo "   enable_als_sensor: [$(cat ${d}enable_als_sensor 2>&1)]"
done
echo "===== [5] PWM chips ====="
ls -d /sys/class/pwm/pwmchip* 2>&1
for c in /sys/class/pwm/pwmchip*/; do
  echo "-- $c"
  echo "   npwm: $(cat ${c}npwm 2>&1)"
  ls ${c} 2>&1 | tr '\n' ' '
  echo
done
echo "===== [6] LEDs ====="
for l in /sys/class/leds/led*/; do
  echo "-- $l"
  echo "   trigger: [$(cat ${l}trigger 2>&1)] brightness: [$(cat ${l}brightness 2>&1)]"
done
echo "===== [7] backlight ====="
ls /sys/class/backlight/ 2>&1
echo "===== [8] Qt libs on board ====="
ls -l /usr/lib/libQt5Core.so* 2>&1
ls -l /usr/lib/arm-linux-gnueabihf/libQt5Core.so* 2>&1
echo "===== [9] fonts ====="
fc-list 2>&1 | head -15
ls /usr/share/fonts 2>&1
find /usr/share/fonts /usr/lib/fonts /usr/local/share/fonts -name '*.tt*' 2>&1 | head -10
echo "===== [10] NFS mount ====="
mount | grep -i nfs 2>&1
df -h /mnt/nfs 2>&1 | head -3
ls /mnt/nfs 2>&1 | head -8
echo "===== [11] misc ====="
cat /proc/cmdline 2>&1
ls /dev/ 2>&1 | grep -iE 'aht|als|light|misc'
echo "===== PROBE DONE ====="
