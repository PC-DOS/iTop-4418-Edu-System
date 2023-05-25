#!/bin/bash

cp  ./linux/kernel/kernel-3.4.39/arch/arm/boot/uImage   ./out/target/product/drone2/boot/

make_ext4fs -s  -l 67108864 -a boot /media/work/s5p4418/kitkat-s5p4418drone/android/boot-only.img  /media/work/s5p4418/kitkat-s5p4418drone/android/out/target/product/drone2/boot

