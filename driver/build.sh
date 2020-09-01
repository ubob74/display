#!/bin/bash

set -ex

FTPDIR=/srv/ftp

export KSRC=${SUNXI_HOME}/kernel/linux-sunxi.org/linux-sunxi
#export CONFIG_ILITEK=y
export CONFIG_ST7735R=y

make clean
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-

cp -i st.ko $FTPDIR/st
cp -i st_run_test.sh $FTPDIR/st
