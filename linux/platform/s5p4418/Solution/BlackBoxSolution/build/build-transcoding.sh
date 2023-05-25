#!/bin/sh
make -j8 -C ../src/libnxfilters || exit $?
make install -C ../src/libnxfilters

make -j8 -C ../src/libnxtranscoding || exit $?
make install -C ../src/libnxtranscoding

make -j8 -C ../apps/nxtranscodingsol || exit $?
make install -C ../apps/nxtranscodingsol
