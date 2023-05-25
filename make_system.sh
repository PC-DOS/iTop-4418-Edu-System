#!/bin/bash

make_ext4fs -s -S out/target/product/drone2/root/file_contexts -l 685768704 -a system  ./system-only.img out/target/product/drone2/system
