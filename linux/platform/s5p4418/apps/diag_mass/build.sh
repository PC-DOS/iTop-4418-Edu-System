#!/bin/sh
rm -rf __install
make clean
make
make install
