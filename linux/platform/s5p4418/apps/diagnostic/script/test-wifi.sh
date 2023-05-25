#!/bin/sh

cd /modules
insmod wlan.ko
ifconfig wlan0 up
#insmod rtutil5370sta.ko
#insmod rt5370sta.ko
#insmod rtnet5370sta.ko
#ifconfig ra0 up
#iwlist ra0 scan 
#iwlist ra0 scan
#iwconfig ra0 essid CAT key restricted 1234567890
#udhcpc -b -i ra0 -s /etc/udhcpc.conf
#route
