#!/bin/bash

TOP=`pwd`


if [ $# -ge 1 ]
then 
	BUILD_NAME=$1
else 
	BUILD_NAME=blackbox
fi

UBOOT_CONFIG_NAME=nxp4330q_${BUILD_NAME}
KERNEL_CONFIG_NAME=nxp4330_${BUILD_NAME}
 
UBOOT_DIR=$TOP/bootloader/u-boot-2013.x
KERNEL_DIR=$TOP/kernel/kernel-3.4.39

MODULES_DIR=$TOP/pyrope/modules
APPLICATION_DIR=$TOP/pyrope/apps
LIBRARY_DIR=$TOP/pyrope/library
BLACKBOX_SOLUTION_DIR=$TOP/pyrope/Solution/BlackBoxSolution

FILESYSTEM_DIR=$TOP/pyrope/fs
TOOLS_DIR=$TOP/pyrope/tools
RESULT_DIR=$TOP/pyrope/result

# byte
BOOT_PARTITION_SIZE=67108864

# Kbyte default:11,264, 16384, 24576, 32768, 49152, 
if [ ${BUILD_NAME} == "blackbox" ]; then
	RAMDISK_SIZE=11264
else
	RAMDISK_SIZE=56320
fi

RAMDISK_FILE=$FILESYSTEM_DIR/buildroot/out/ramdisk.gz


NX_BINGEN=$TOP/pyrope/tools/bin/nx_bingen
NSIH_FILE=$TOP/pyrope/boot/nsih/nsih_${BUILD_NAME}_spi.txt
SECONDBOOT_FILE=$TOP/pyrope/boot/2ndboot/pyrope_2ndboot_${BUILD_NAME}_spi.bin
SECONDBOOT_OUT_FILE=$RESULT_DIR/2ndboot_${BUILD_NAME}.bin


CMD_V_BUILD_NUM=

CMD_V_UBOOT=no
CMD_V_UBOOT_CLEAN=no

CMD_V_KERNEL=no
CMD_V_KERNEL_CLEAN=no
CMD_V_KERNEL_MODULE=no

CMD_V_KERNEL_PROJECT_MENUCONFIG=no
CMD_V_KERNEL_PROJECT_MENUCONFIG_COMPILE=no

CMD_V_APPLICATION=no
CMD_V_APPLICATION_CLEAN=no
CMD_V_FILESYSTEM=no

CMD_V_SDCARD_PACKAGING=no
CMD_V_SDCARD_SELECT_DEV=
CMD_V_EMMC_PACKAGING=no
CMD_V_EMMC_PACKAGING_2NDBOOT=no
CMD_V_EMMC_PACKAGING_UBOOT=no
CMD_V_EMMC_PACKAGING_BOOT=no

CMD_V_BASE_PORTING=no
CMD_V_NEW_BOARD=

CMD_V_BUILD_ERROR=no
CMD_V_BUILD_SEL=Not

TEMP_UBOOT_TEXT=
TEMP_KERNEL_TEXT=


YEAR=0000
MON=00
DAY=00
HOUR=00
MIN=00
SEC=00

function check_result()
{
	if [ $? -ne 0 ]; then
		cd $TOP
		echo "[Error]"
		exit
	fi
}

function currentTime()
{
	YEAR=`date +%Y`
	MON=`date +%m`
	DAY=`date +%d`
	HOUR=`date +%H`
	MIN=`date +%M`
	SEC=`date +%S`
}


function build_uboot_source()
{

	if [ ${CMD_V_UBOOT_CLEAN} = "yes" ]; then
		echo ''
		echo ''
		echo '#########################################################'
		echo '#########################################################'
		echo '#'
		echo '# build u-boot clean '
		echo '#'
		echo '#########################################################'

		sleep 1.5

		pushd . > /dev/null
		cd $UBOOT_DIR
		make distclean
		make ${UBOOT_CONFIG_NAME}_config
		popd > /dev/null
	fi


	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo "# build uboot "
	echo '#'
	echo '#########################################################'

	if [ -f $RESULT_DIR/build.nxp4330.uboot ]; then
		rm -f $RESULT_DIR/build.nxp4330.uboot
	fi
	echo "${UBOOT_CONFIG_NAME}_config" > $RESULT_DIR/build.nxp4330.uboot ##.build_${UBOOT_CONFIG_NAME}

	sleep 1.5
	pushd . > /dev/null

	cd $UBOOT_DIR
	make -j8 -sw
	check_result
	popd > /dev/null

	cp -v ${UBOOT_DIR}/u-boot.bin ${RESULT_DIR}
}


function build_kernel_source()
{
	if [ ${CMD_V_KERNEL_CLEAN} = "yes" ]; then
		echo ''
		echo ''
		echo '#########################################################'
		echo '#########################################################'
		echo '#'
		echo '# build kernel clean '
		echo '#'
		echo '#########################################################'

		sleep 1.5

		pushd . > /dev/null
		cd $KERNEL_DIR
		#make distclean
		make ARCH=arm clean -j8
		popd > /dev/null
	fi

	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo "# build kernel "
	echo '#'
	echo '#########################################################'

	sleep 1.5

	pushd . > /dev/null
	cd $KERNEL_DIR

	if [ -f .config ]; then
		echo ""
	else
		echo "${KERNEL_CONFIG_NAME}_defconfig" > $RESULT_DIR/build.nxp4330.kernel ##.build_${UBOOT_CONFIG_NAME}	
		make ARCH=arm ${KERNEL_CONFIG_NAME}_defconfig
	fi

	make ARCH=arm uImage -j8 -sw
	check_result
	popd > /dev/null

	cp -v ${KERNEL_DIR}/arch/arm/boot/uImage ${RESULT_DIR}

	if [ -d /home/share/tftpboot ]; then
		cp -v ${KERNEL_DIR}/arch/arm/boot/uImage /home/share/tftpboot/
		cp -v ${KERNEL_DIR}/arch/arm/boot/Image /home/share/tftpboot/
	fi
}

function build_kernel_module()
{

	echo ''
	echo '#########################################################'
	echo '# build kernel modules'
	echo '#########################################################'

	sleep 1.5

	pushd . > /dev/null
	cd $MODULES_DIR/coda960
	make ARCH=arm -j4 -sw
	check_result
	popd > /dev/null
}

function build_kernel_current_menuconfig()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo "# kernel ${KERNEL_CONFIG_NAME}_defconfig menuconfig "
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	pushd . > /dev/null
	cd $KERNEL_DIR
	make ARCH=arm menuconfig
	check_result
	popd > /dev/null
}

function build_kernel_configcopy()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo "# kernel set ${KERNEL_CONFIG_NAME}_defconfig"
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	if [ -f $RESULT_DIR/build.nxp4330.kernel ]; then
		rm -f $RESULT_DIR/build.nxp4330.kernel
	fi

	sleep 1.5
	pushd . > /dev/null
	cd $KERNEL_DIR
	make distclean
	make ARCH=arm ${KERNEL_CONFIG_NAME}_defconfig
	check_result

	echo "${KERNEL_CONFIG_NAME}_defconfig" > $RESULT_DIR/build.nxp4330.kernel ##.build_${UBOOT_CONFIG_NAME}	
	popd > /dev/null
}

function build_partial_app()
{
	if [ -d $1 ]; then
		echo ''
		echo '#########################################################'
		echo "# $1 "
		echo '#########################################################'
		cd $1
		if [ ${CMD_V_APPLICATION_CLEAN} = "yes" ]; then
			make clean
		fi	
		make -sw
		check_result
	fi
}

function build_partial_lib()
{
	if [ -d $1 ]; then
		echo ''
		echo '#########################################################'
		echo "# $1 "
		echo '#########################################################'
		cd $1
		if [ ${CMD_V_APPLICATION_CLEAN} = "yes" ]; then
			make clean
		fi	
		make -sw
		check_result
		make install -sw
	fi
}

function build_application()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo '# lib & application '
	echo '#'
	echo '#########################################################'
	echo '#########################################################'
	
	sleep 1.5

	pushd . > /dev/null

	build_partial_lib $LIBRARY_DIR/src/libnxtypefind
	build_partial_lib $LIBRARY_DIR/src/libcec
	build_partial_lib $LIBRARY_DIR/src/libion
	build_partial_lib $LIBRARY_DIR/src/libnxmalloc
	build_partial_lib $LIBRARY_DIR/src/libnxadc
	build_partial_lib $LIBRARY_DIR/src/libnxaudio
	build_partial_lib $LIBRARY_DIR/src/libnxgpio
	build_partial_lib $LIBRARY_DIR/src/libnxgraphictools
	build_partial_lib $LIBRARY_DIR/src/libnxmovieplayer
	build_partial_lib $LIBRARY_DIR/src/libnxnmeaparser
	build_partial_lib $LIBRARY_DIR/src/libnxscaler
	build_partial_lib $LIBRARY_DIR/src/libnxv4l2
	build_partial_lib $LIBRARY_DIR/src/libnxvpu
	build_partial_lib $LIBRARY_DIR/src/libnxuevent

	build_partial_app $APPLICATION_DIR/adc_test
	build_partial_app $APPLICATION_DIR/audio_test
	build_partial_app $APPLICATION_DIR/cec_test
	build_partial_app $APPLICATION_DIR/fb_test
	build_partial_app $APPLICATION_DIR/gpio_test
	build_partial_app $APPLICATION_DIR/typefind_app
	build_partial_app $APPLICATION_DIR/movie_player_app
	build_partial_app $APPLICATION_DIR/nmea_test
	build_partial_app $APPLICATION_DIR/spi_test
	build_partial_app $APPLICATION_DIR/transcoding_example
	build_partial_app $APPLICATION_DIR/v4l2_test
	build_partial_app $APPLICATION_DIR/vpu_test

	if [ -d $BLACKBOX_SOLUTION_DIR ]; then
		echo ''
		echo '#########################################################'
		echo '# BlackBox Solution '
		echo '#########################################################'
		cd $BLACKBOX_SOLUTION_DIR/build
		chmod 755 ./*.sh
		if [ ${CMD_V_APPLICATION_CLEAN} = "yes" ]; then
			##./clean-blackbox.sh
			make distclean -C ../src/libnxfilters
			make distclean -C ../src/libnxdvr
			make distclean -C ../apps/nxdvrsol
			make distclean -C ../apps/nxguisol
			make distclean -C ../apps/nxdvrmonitor

			./clean-hls.sh
			./clean-mp4.sh
			./clean-rtp.sh
		fi
		##./build-blackbox.sh
		make -j8 -C ../src/libnxfilters || exit $?
		make install -C ../src/libnxfilters
		check_result

		make -j8 -C ../src/libnxdvr || exit $?
		make install -C ../src/libnxdvr
		check_result

		make -j8 -C ../apps/nxdvrsol || exit $?
		make install -C ../apps/nxdvrsol
		check_result

		make -j8 -C ../apps/nxguisol || exit $?
		make install -C ../apps/nxguisol
		check_result

		make -j8 -C ../apps/nxdvrmonitor || exit $?
		make install -C ../apps/nxdvrmonitor
		check_result

		./build-hls.sh
		./build-mp4.sh
		./build-rtp.sh
	fi

	popd > /dev/null
}

function copy_app()
{
	if [ -d $1 ]; then
		echo '//////////////////////////////////////////////////////////////////////////////'
		echo "// copy $1 "
		cp -v $1/$2 $FILESYSTEM_DIR/buildroot/out/rootfs/usr/bin/
		check_result
	fi
}

function build_filesystem()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo "# filesystem(rootfs-ramdisk)"
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	sleep 1.5

	if [ -f ${RAMDISK_FILE} ]; then
		rm -f ${RAMDISK_FILE}
	fi

	if [ -f ${RESULT_DIR}/ramdisk.gz ]; then
		rm -f ${RESULT_DIR}/ramdisk.gz
	fi

	if [ -d $FILESYSTEM_DIR/buildroot/out/rootfs ]; then
		copy_app $APPLICATION_DIR/adc_test adc_test
		copy_app $APPLICATION_DIR/audio_test audio_test
		copy_app $APPLICATION_DIR/cec_test cec_test
		copy_app $APPLICATION_DIR/fb_test fb_test
		copy_app $APPLICATION_DIR/gpio_test gpio_test
		copy_app $APPLICATION_DIR/nmea_test nmea_test
		copy_app $APPLICATION_DIR/spi_test spi_test
		copy_app $APPLICATION_DIR/typefind_app typefind_app
		copy_app $APPLICATION_DIR/movie_player_app movieplayer_app
		copy_app $APPLICATION_DIR/transcoding_example trans_test2

		if [ -d $APPLICATION_DIR/v4l2_test ]; then
			echo '//////////////////////////'
			echo '// copy v4l2_test '
			cd $APPLICATION_DIR/v4l2_test/
			cp -v camera_test csi_test decimator_test hdmi_test $FILESYSTEM_DIR/buildroot/out/rootfs/usr/bin/
			check_result
		fi
		
		if [ -d $APPLICATION_DIR/vpu_test ]; then
			echo '//////////////////////////'
			echo '// copy vpu_test '
			cd $APPLICATION_DIR/vpu_test/
			cp -v dec_test enc_test jpg_test trans_test $FILESYSTEM_DIR/buildroot/out/rootfs/usr/bin/
			check_result
		fi

			echo ''
			echo '//////////////////////////////////////////////////////////////////////////////'
			echo '// copy gstreamer-0.10 '
			cp -vr $LIBRARY_DIR/lib/gstreamer-0.10 $FILESYSTEM_DIR/buildroot/out/rootfs/usr/lib/

			echo ''
			echo '//////////////////////////'
			echo '// copy lib '
			cp -v $LIBRARY_DIR/lib/*.so $FILESYSTEM_DIR/buildroot/out/rootfs/usr/lib/
			check_result

			echo ''
			echo '//////////////////////////'
			echo '// copy coda960 '
			cp -v $MODULES_DIR/coda960/nx_vpu.ko $FILESYSTEM_DIR/buildroot/out/rootfs/root/
			check_result
			echo ''

		if [ -d $BLACKBOX_SOLUTION_DIR ]; then
			echo '//////////////////////////'
			echo '// copy BlackBox Solution '
			cp -v $BLACKBOX_SOLUTION_DIR/lib/*.so $FILESYSTEM_DIR/buildroot/out/rootfs/usr/lib/
			cp -rfv $BLACKBOX_SOLUTION_DIR/bin/* $FILESYSTEM_DIR/buildroot/out/rootfs/root/
		fi

		pushd . > /dev/null
		cd $FILESYSTEM_DIR
		cp buildroot/scripts/mk_ramfs.sh buildroot/out/
		cd buildroot/out/

		if [ -d mnt ]; then
			sudo rm -rf mnt
		fi

		chmod 755 ./*.sh
		./mk_ramfs.sh -r rootfs -s ${RAMDISK_SIZE}

		popd > /dev/null

		echo ''
		echo ''
		echo '#########################################################'
		echo "# copy image"
		echo '#########################################################'
		cp -v ${UBOOT_DIR}/u-boot.bin ${RESULT_DIR}
		check_result
		cp -v ${KERNEL_DIR}/arch/arm/boot/uImage ${RESULT_DIR}
		check_result
		cp -v ${RAMDISK_FILE} ${RESULT_DIR}/ramdisk.gz
		check_result

		if [ -d /home/share/tftpboot ]; then
			cp -v ${KERNEL_DIR}/arch/arm/boot/uImage /home/share/tftpboot/
			cp -v ${KERNEL_DIR}/arch/arm/boot/Image /home/share/tftpboot/
		fi

		if [ -d /home/share/nfs/nxp4330 ]; then
			echo "cp -r ${FILESYSTEM_DIR}/buildroot/out/rootfs  ->  /home/share/nfs/nxp4330/"
			sudo cp -r ${FILESYSTEM_DIR}/buildroot/out/rootfs /home/share/nfs/nxp4330/
		fi
	else
		echo '#########################################################'
		echo '# error : No "./fs/buildroot/out" folder.'
		echo '#########################################################'
	fi
}

function build_fastboot_2ndboot()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo '# fastboot 2ndboot'
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	sleep 1.5
	pushd . > /dev/null
	$NX_BINGEN -t 2ndboot -d other -o $SECONDBOOT_OUT_FILE -i $SECONDBOOT_FILE -n $NSIH_FILE -l 0x40c00000 -e 0x40c00000
	sudo fastboot flash 2ndboot $SECONDBOOT_OUT_FILE
	popd > /dev/null
}

function build_fastboot_uboot()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo '# fastboot uboot'
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	sleep 1.5
	pushd . > /dev/null
	sudo fastboot flash bootloader $RESULT_DIR/u-boot.bin
	popd > /dev/null
}

function build_fastboot_boot()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo '# fastboot boot(Kernel)'
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	sleep 1.5
	pushd . > /dev/null
	sudo fastboot flash kernel $RESULT_DIR/uImage
	popd > /dev/null
}

function build_fastboot_system()
{
	echo ''
	echo ''
	echo '#########################################################'
	echo '#########################################################'
	echo '#'
	echo '# fastboot boot(system)'
	echo '#'
	echo '#########################################################'
	echo '#########################################################'

	sleep 1.5
	pushd . > /dev/null
	sudo fastboot flash ramdisk $RESULT_DIR/ramdisk.gz
	popd > /dev/null
}


function build_function_main()
{

	currentTime
	StartTime="${YEAR}-${MON}-${DAY} ${HOUR}:${MIN}:${SEC}"
	echo '#########################################################'
	echo "#  Build Time : "${StartTime}"                     #"
	echo '#########################################################'
	echo ""


	if [ ${CMD_V_UBOOT} = "yes" ]; then
		CMD_V_BUILD_SEL="Build u-boot"
		build_uboot_source
	fi

	if [ ${CMD_V_KERNEL} = "yes" ]; then
		CMD_V_BUILD_SEL="Build Kernel"
		build_kernel_source
	fi

	if [ ${CMD_V_KERNEL_MODULE} = "yes" ]; then
		CMD_V_BUILD_SEL="Build Kernel module"
		build_kernel_module
	fi

	if [ ${CMD_V_APPLICATION} = "yes" ]; then
		CMD_V_BUILD_SEL="Build Application"
		build_application
	fi

	if [ ${CMD_V_FILESYSTEM} = "yes" ]; then
		CMD_V_BUILD_SEL="Build Filesystem"
		build_filesystem
	fi

	if [ -d ${RESULT_DIR} ]; then
		echo ""
		echo '#########################################################'
		echo "ls -al ${RESULT_DIR}"
		ls -al ${RESULT_DIR}
	fi

	currentTime
	EndTime="${YEAR}-${MON}-${DAY} ${HOUR}:${MIN}:${SEC}"

	echo ""
	echo '#########################################################'
	echo "# Build Information				"
	echo "#     Build      : $CMD_V_BUILD_SEL	"
	echo "#"
	echo "#     Start Time : "${StartTime}"	"
	echo "#       End Time : "${EndTime}"	"
	echo '#########################################################'
}



################################################################
##
## main build start
##
################################################################

# export PATH=$TOP/prebuilts/gcc/linux-X86/arm/arm-eabi-4.6/bin/:$PATH
# arm-eabi-gcc -v 2> /dev/null
# check_result

if [ -d $RESULT_DIR ]; then
	echo ""
else
	mkdir -p $RESULT_DIR
fi

if [ -f $RESULT_DIR/build.nxp4330.uboot ]; then
	TEMP_UBOOT_TEXT=`cat $RESULT_DIR/build.nxp4330.uboot`
fi
if [ -f $RESULT_DIR/build.nxp4330.kernel ]; then
	TEMP_KERNEL_TEXT=`cat $RESULT_DIR/build.nxp4330.kernel`
fi

if [ ${BUILD_NAME} != "build_exit" ]; then
	while [ -z $CMD_V_BUILD_NUM ]
	do
		clear
		echo "******************************************************************** "
		echo "Build Menu "
		echo "  TOP DIR       : $TOP"
		echo "  Before Uboot  : ${TEMP_UBOOT_TEXT}"
		echo "  Before Kernel : ${TEMP_KERNEL_TEXT}"
		echo "  Build Name    : $BUILD_NAME"
		echo "******************************************************************** "
		echo "  1. ALL(+Compile)"
		echo "     1c. ALL(+Clean Build)"       
		echo " "
		echo "--------------------------------------------------------------------"
		echo "  4. u-boot+kernel (+Build)   4c. clean Build"
		echo "     41.  u-boot(+Build)		41c. u-boot(+Clean Build)"       
		echo "     42.  kernel(+Build)		42c. kernel(+Clean Build)"       
		echo " "
		echo "     4m.  kernel menuconfig"
		echo "     4mc. ${KERNEL_CONFIG_NAME}_defconfig -> .config"
		echo " "
		echo "--------------------------------------------------------------------"
		echo "  6. Application+Library"
		echo "     6c. App+Lib(+Clean Build)"       
		echo " "
		echo "--------------------------------------------------------------------"
		echo "  7. File System(ramdisk)(+Build)"
		echo " "
		echo "--------------------------------------------------------------------"
		echo "  9. eMMC packaging(All)"
		echo "     91. fastboot secondboot(2ndBoot)"
		echo "     92. fastboot bootloader(u-boot)"
		echo "     93. fastboot boot(kernel)"
		echo "     94. fastboot system(rootfs)"
		echo "--------------------------------------------------------------------"
		echo "  0. exit "
		echo "--------------------------------------------------------------------"

		echo -n "     Select Menu -> "
		read CMD_V_BUILD_NUM
		case $CMD_V_BUILD_NUM in
			#------------------------------------------------------------------------------------------------
			1) CMD_V_UBOOT=yes	
			    CMD_V_KERNEL=yes 
			    CMD_V_KERNEL_MODULE=yes
			    CMD_V_APPLICATION=yes
			    CMD_V_FILESYSTEM=yes					
			    ;;

				1c) CMD_V_UBOOT_CLEAN=yes
				    CMD_V_UBOOT=yes
				    CMD_V_KERNEL_CLEAN=yes
				    CMD_V_KERNEL=yes 
				    CMD_V_KERNEL_MODULE=yes
				    CMD_V_APPLICATION=yes
				    CMD_V_APPLICATION_CLEAN=yes
				    CMD_V_FILESYSTEM=yes
				    ;;

			#------------------------------------------------------------------------------------------------
			4) CMD_V_KERNEL=yes 
			    CMD_V_KERNEL_MODULE=yes
			    CMD_V_UBOOT=yes 
			    ;;
				4c) CMD_V_UBOOT=yes
					CMD_V_UBOOT_CLEAN=yes				
					CMD_V_KERNEL=yes 
					CMD_V_KERNEL_CLEAN=yes
					CMD_V_KERNEL_MODULE=yes
				       ;;
				41) CMD_V_UBOOT=yes							
				    ;;
				41c) CMD_V_UBOOT=yes
				       CMD_V_UBOOT_CLEAN=yes				
				       ;;
				42) CMD_V_KERNEL=yes 						
				     CMD_V_KERNEL_MODULE=yes
					 ;;
				42c) CMD_V_KERNEL=yes 
				       CMD_V_KERNEL_CLEAN=yes
					 CMD_V_KERNEL_MODULE=yes
				       ;;

				4m)	build_kernel_current_menuconfig	;;
				4mc)build_kernel_configcopy	;;

			#------------------------------------------------------------------------------------------------
			6)	CMD_V_APPLICATION=yes					
				;;
				6c) CMD_V_APPLICATION=yes
					CMD_V_APPLICATION_CLEAN=yes
					;;

			#------------------------------------------------------------------------------------------------
			7)	CMD_V_FILESYSTEM=yes					;;

			#------------------------------------------------------------------------------------------------
			9)	CMD_V_BUILD_NUM=
				build_fastboot_2ndboot
				build_fastboot_uboot
				build_fastboot_boot
				build_fastboot_system					;;

				91)	CMD_V_BUILD_NUM=
					build_fastboot_2ndboot				;;
				92)	CMD_V_BUILD_NUM=
					build_fastboot_uboot				;;
				93)	CMD_V_BUILD_NUM=
					build_fastboot_boot				;;
				94)	CMD_V_BUILD_NUM=
					build_fastboot_system				;;
			#------------------------------------------------------------------------------------------------
			0)	CMD_V_BUILD_NUM=0
				echo ""
				exit 0										;;

			#------------------------------------------------------------------------------------------------
			*)	CMD_V_BUILD_NUM=							;;

		esac
		echo
	done

	if [ $CMD_V_BUILD_NUM != 0 ]; then
		CMD_V_LOG_FILE=$RESULT_DIR/build.log
		rm -rf CMD_V_LOG_FILE
		build_function_main 2>&1 | tee $CMD_V_LOG_FILE
	fi
fi

echo ""


