#!/bin/sh
make -j8 -C ../src/libnxfilters || exit $?
make install -C ../src/libnxfilters

make -j8 -C ../src/libnxrtp || exit $?
make install -C ../src/libnxrtp

make -j8 -C ../apps/nxrtpsol || exit $?
make install -C ../apps/nxrtpsol

