#!/bin/bash
# smart_agri cross-compile script (run on VM as: bash build.sh)
set -o pipefail
cd /home/elf/work/smart_agri || { echo "NO_DIR /home/elf/work/smart_agri"; exit 1; }

SDK=/opt/fsl-imx-x11/4.1.15-2.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
if [ ! -f "$SDK" ]; then echo "NO_SDK $SDK"; exit 1; fi
source "$SDK"

echo "=====TOOLCHAIN====="
which qmake; qmake --version 2>&1 | tail -1; which ${CROSS_COMPILE}gcc && ${CROSS_COMPILE}gcc --version | head -1

echo "=====QMAKE====="
qmake smart_agri.pro || { echo "QMAKE_FAILED"; exit 1; }

echo "=====MAKE====="
make -j4 > /tmp/make.log 2>&1
rc=$?
tail -60 /tmp/make.log
echo "=====MAKE_RC=$rc====="

if [ $rc -ne 0 ]; then
  echo "=====ERROR LINES====="
  grep -n -E "error:|fatal error|undefined reference|No such file" /tmp/make.log | head -40
  exit 1
fi

echo "=====RESULT====="
file smart_agri
ls -l smart_agri
