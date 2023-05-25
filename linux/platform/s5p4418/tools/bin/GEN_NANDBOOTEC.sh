#!/bin/bash

TOP=`pwd`

BOOT_DIR=${TOP}/linux/pyrope/boot/2ndboot
RESULT_DIR=${TOP}/result


generator="${TOP}/linux/pyrope/tools/bin/nx_bingen"
nsih_file="${TOP}/linux/pyrope/boot/nsih/nsih_lynx_nand.txt"

# Page Size  (KiB)
page_size=8192

secondboot_out_file=$RESULT_DIR/2ndboot.ecc
secondboot_file=${BOOT_DIR}/pyrope_2ndboot_lynx_nand.bin
# Do Not Generate 2ndboot.ecc
#secondboot_file=


bootloader_out_file=$RESULT_DIR/u-boot.ecc
bootloader_file=${TOP}/u-boot/u-boot.bin
# Do Not Generate u-boot.ecc 
#bootloader_file=


BASE_PATH="`dirname \"$0\"`"
source $BASE_PATH/GEN_NANDBOOTEC_BASE.sh




function usage()
{
	echo "usage: `basename $0`"
	echo "  -e     ecc generator                            "
	echo "  -n     NSIH file                                "
	echo "  -p     page size (KiB)  (default: 8192)         "
	echo "  -s     secondboot output                        "
	echo "  -k     secondboot input                         "
	echo "  -b     bootloader output                        "
	echo "  -l     bootloader input                         "
	echo "  -h     this message                             "
}


function parse_args()
{
    TEMP=`getopt -o "e:n:p:s:k:b:l:h" -- "$@"`
    eval set -- "$TEMP"

    while true; do
        case "$1" in
			-e ) generator=$2;            shift 2 ;;
			-n ) nsih_file=$2;            shift 2 ;;
            -p ) page_size=$2;            shift 2 ;;
            -s ) secondboot_out_file=$2;  shift 2 ;;
            -k ) secondboot_file=$2;      shift 2 ;;
            -b ) bootloader_out_file=$2;  shift 2 ;;
            -l ) bootloader_file=$2;      shift 2 ;;
            -h ) usage; exit 1 ;;
            -- ) break ;;
            *  ) echo "invalid option $1"; usage; exit 1 ;;
        esac
    done
}


## main routine
function main() {
	# check input parameter
	if [ -z "$page_size" ]; then
		usage;
		exit 1;
	fi


	if [ -f "$secondboot_file" ]; then
		echo "processing 2ndboot ..."
		do_eccgen 2ndboot $page_size $secondboot_out_file $secondboot_file $generator $nsih_file
	else
		echo "Skip 2ndboot"
	fi


	if [ -f "$bootloader_file" ]; then
		echo "processing 3rdboot ..."
		do_eccgen bootloader $page_size $bootloader_out_file $bootloader_file $generator $nsih_file
	else
		echo "Skip 3rdboot"
	fi
}


parse_args $@
main $@
