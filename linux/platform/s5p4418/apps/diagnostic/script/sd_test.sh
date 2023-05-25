#!/bin/sh

FIFO_NAME=/tmp/t_fifo
SD_ROOT=/mnt/mmc/
DIFF=/result


check_dtk_sd_mount()
{
	echo -e  " Check sd mount state :\n" >$FIFO_NAME
	mount | grep uba1
	if [ $? -eq 0 ]
	then
	#	SD_ROOT=`mount | grep mmcblk | awk '{print $3}'`
		echo "ok" >$FIFO_NAME
		return 0
	else
		echo "mount -t vfat /dev/mmcblk0p1 $SD_ROOT" >> $FIFO_NAME
			mount -t vfat /dev/mmcblk0p1 $SD_ROOT
		if [ $? -eq 0 ]
		then
			echo "ok" >$FIFO_NAME
			return 0
		else

			echo "Mount fail" >$FIFO_NAME
		   #echo "Mount fail" >$FIFO_NAME
			return 1
		fi
	fi
}

check_sd_access()
{
	echo "Test sd access :" >$FIFO_NAME
	cp /diagnostics/diag_items.txt $SD_ROOT
	sync
	diff /diagnostics/diag_items.txt mnt/mmc/diag_items.txt > $DIFF

	FILESIZE=$(stat -c%s "$DIFF")
	echo file_size = $FILESIZE
	rm $SD_ROOT/diag_items.txt
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
	echo "Check SD card " >$FIFO_NAME
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

umount $SD_ROOT
echo end
echo "end" > $FIFO_NAME
sync
