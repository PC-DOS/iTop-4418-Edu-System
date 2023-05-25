#!/bin/sh
make ARCH=arm clean
make ARCH=arm -j4
cp control_3g.ko ../../../../../hardware/samsung_slsi/slsiap/prebuilt/modules/
