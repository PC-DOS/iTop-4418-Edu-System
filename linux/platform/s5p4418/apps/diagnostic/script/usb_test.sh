#!/bin/sh

FIFO_NAME=/tmp/t_fifo
USB_ROOT=/mnt/usb/
DIFF=/result


check_dtk_sd_mount()
{
	echo -e  " Check usb mount state :\n" >$FIFO_NAME
	mount | grep uba1
	if [ $? -eq 0 ]
	then
		SD_ROOT=`mount | grep mmcblk | awk '{print $3}'`
		echo "ok" >$FIFO_NAME
		return 0
	else
		echo "mount -t ext4 /dev/sda1 $USB_ROOT" >> $FIFO_NAME
			mount -t  ext4 dev/sda1 $USB_ROOT
		if [ $? -eq 0 ]
		then
			echo "ok" >$FIFO_NAME
			return 0
		else

			echo "Mount fail" >$RESULT_NAME
			echo "fail" >$FIFO_NAME
			return 1
		fi
	fi
}

check_sd_access()
{
	echo "Test usb access :" >$FIFO_NAME
	cp /diagnostics/diag_items.txt $USB_ROOT
	sync
	diff /diagnostics/diag_items.txt mnt/usb/diag_items.txt > $DIFF

	FILESIZE=$(stat -c%s "$DIFF")
	echo file_size = $FILESIZE
	rm $USB_ROOT/diag_items.txt
	sync
		echo "ok" > $FIFO_NAME

 	if [ $FILESIZE -eq 0 ]
	then
		echo "ok" > $FIFO_NAME
		return 0
	else
		echo "fail" > $FIFO_NAME
		return 1
	fi
}


#
#	return value
#		succeed : 0 
#		failed  : 1
#
check_sd_mount()
{
	echo "Check USB card " >$FIFO_NAME
	check_dtk_sd_mount
	if [ $? -eq 0 ]
	then
		check_sd_access
		return 0
	else
		return 1
	fi
}

echo "start" > $FIFO_NAME

echo start
check_sd_mount

umount $USB_ROOT
echo end
echo "end" > $FIFO_NAME
sync
