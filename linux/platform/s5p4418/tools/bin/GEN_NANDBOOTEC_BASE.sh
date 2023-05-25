#!/bin/bash

set -e


# arg 1 : image type <2ndboot|bootloader>
# arg 2 : page size (KiB)
# arg 3 : output file
# arg 4 : input file

# arg 5 : Generator
# arg 6 : NSIH file
function do_eccgen()
{
	local load_addr=0x40c00000
	local jump_addr=0x40c00000


	local image_type="${1}"
	local page_size="${2}"
	local eccgen_file="${3}"
	local origin_file="${4}"

	local generator="${5}"
	local nsih_file="${6}"


	if [ ! -f ${origin_file} ]; then
		echo "Error: Can't find original file."
		exit 1
	fi

	if [ ! -f ${nsih_file} ]; then
		echo "Error: Can't find NSIH file."
		exit 1
	fi

	if [ ! -f ${generator} ]; then
		echo "Error: Can't find GENERATOR"
		exit 1
	fi

#	echo "$1 $2 $3 $4"
	${generator} -d nand -t ${1} -p ${2} -o ${3} -i ${4} -n ${nsih_file} -l ${load_addr} -e ${jump_addr}

}
