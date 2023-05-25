#!/bin/sh
export NX_DIAG_PATH=/root/diag
export NX_DIAG_FONT=$NX_DIAG_PATH/font/DejaVuSansMono.ttf
export NX_DIAG_FONT_SIZE=18
export NX_DIAG_DEBUG_FONT=$NX_DIAG_PATH/font/DejaVuSansMono.ttf
export NX_DIAG_DEBUG_FONT_SIZE=16
export NX_DIAG_DATA=$NX_DIAG_PATH/data
export NX_DIAG_OUTPUT=$NX_DIAG_PATH/output

kill -9 $(pidof aplay)
kill -9 $(pidof wpa_supplicant)
kill -9 $(pidof nxdiag_mass)

$NX_DIAG_PATH/nxdiag_mass
