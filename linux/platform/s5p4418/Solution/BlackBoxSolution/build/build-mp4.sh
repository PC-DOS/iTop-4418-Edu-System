#!/bin/sh
make -j8 -C ../src/libnxfilters || exit $?
make install -C ../src/libnxfilters

make -j8 -C ../src/libnxmp4manager || exit $?
make install -C ../src/libnxmp4manager

make -j8 -C ../apps/nxmp4encsol || exit $?
make install -C ../apps/nxmp4encsol
