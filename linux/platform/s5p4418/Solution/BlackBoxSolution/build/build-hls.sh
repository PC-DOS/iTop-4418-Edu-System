#!/bin/sh
make -j8 -C ../src/libnxfilters || exit $?
make install -C ../src/libnxfilters

make -j8 -C ../src/libnxhls || exit $?
make install -C ../src/libnxhls

make -j8 -C ../apps/nxhlssol || exit $?
make install -C ../apps/nxhlssol
