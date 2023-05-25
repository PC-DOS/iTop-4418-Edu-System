/*
   1. 2ndboot

		* first stage

			NSIH FILE, BIN FILE  ------------>  PHASE1_FILE (16KB)  ------------->  PHASE2_FILE
			                    parsing, merge   (merge_fp)           ecc gen       (eccgen_fp)


		* second stage

			PHASE2_FILE  ---------------> RESULT
				         spacing - 0xff

		- romboot can read up to 4KB of physical page. rest space fill with 0xff.
  

   2. u-boot

		* first stage

			NSIH FILE, BIN FILE  ------------>  PHASE1_FILE   ------------->  PHASE2_FILE
			                    parsing, merge   (merge_fp)       ecc gen     (eccgen_fp)


		* second stage

			PHASE2_FILE  ---------------> RESULT

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "NSIH.h"
#include "GEN_NANDBOOTEC.h"

#define	VERSION_STR	"0.9.4"


/* PRINT MACRO */
#ifdef DEBUG
#define pr_debug(msg...)	do { printf("[DEBUG] " msg); } while (0)
#else
#define pr_debug(msg...)	do {} while (0)
#endif

#define pr_error(msg...)	do { printf("[ERROR] " msg); } while (0)


//
//
#define DEFAULT_LOADADDR	(0x0)
#define DEFAULT_LAUNCHADDR	(0x0)

#define	SECOND_BOOT_SIZE	(16*1024)


#define BUILD_NONE					0x0
#define BUILD_2NDBOOT				0x1
#define BUILD_BOOTLOADER			0x2

#define	DEVICE_NONE					0x0
#define	DEVICE_NAND					0x1
#define	DEVICE_OTHER				0x2



struct build_info {
	char *					nsih_file;
	uint32_t				loadaddr;
	uint32_t				launchaddr;
	uint32_t				dev_readaddr;
	int						image_type;
	unsigned int			page_size;
};
typedef struct build_info build_info_t;


//////////////////////////////////////////////////////////////////////////////
//
//	Uitility functions
//
unsigned int String2Hex ( const char *pstr )
{
	char ch;
	unsigned int value;

	value = 0;

	while ( *pstr != '\0' )
	{
		ch = *pstr++;

		if ( ch >= '0' && ch <= '9' )
		{
			value = value * 16 + ch - '0';
		}
		else if ( ch >= 'a' && ch <= 'f' )
		{
			value = value * 16 + ch - 'a' + 10;
		}
		else if ( ch >= 'A' && ch <= 'F' )
		{
			value = value * 16 + ch - 'A' + 10;
		}
	}

	return value;
}

unsigned int String2Dec ( const char *pstr )
{
	char ch;
	unsigned int value;

	value = 0;

	while ( *pstr != '\0' )
	{
		ch = *pstr++;

		if ( ch >= '0' && ch <= '9' )
		{
			value = value * 10 + ch - '0';
		}
	}

	return value;
}

void Hex2StringByte(unsigned char data, unsigned char *string)
{
	unsigned char hex;

	hex = data >> 4;
	if (hex >= 10)
		hex += 'A' - 10;
	else
		hex += '0';
	*string++ = hex;

	hex = data & 0xF;
	if (hex >= 10)
		hex += 'A' - 10;
	else
		hex += '0';
	*string++ = hex;

	*string = ' ';
}

static size_t get_file_size( FILE *fd )
{
	size_t fileSize;
	long curPos;

	curPos = ftell( fd );

	fseek(fd, 0, SEEK_END);
	fileSize = ftell( fd );
	fseek(fd, curPos, SEEK_SET );

	return fileSize;
}

//
//	end of utilities functions
//
//////////////////////////////////////////////////////////////////////////////


static int update_bootinfo (unsigned char *parsed_nsih, const build_info_t *BUILD_INFO, const unsigned int BinFileSize)
{
	struct NX_SecondBootInfo *bootinfo;

	bootinfo = (struct NX_SecondBootInfo *)parsed_nsih;
	if (BUILD_INFO->dev_readaddr)
	bootinfo->DEVICEADDR		= BUILD_INFO->dev_readaddr;
	bootinfo->LOADSIZE			= BinFileSize;
	if( BUILD_INFO->loadaddr)
	bootinfo->LOADADDR			= BUILD_INFO->loadaddr;
	if (BUILD_INFO->launchaddr)
	bootinfo->LAUNCHADDR		= BUILD_INFO->launchaddr;
	bootinfo->SIGNATURE			= HEADER_ID;

	pr_debug ("bootinfo sz: %d loadaddr: 0x%x, launchaddr 0x%x\n",
		bootinfo->LOADSIZE, bootinfo->LOADADDR, bootinfo->LAUNCHADDR);

	return 0;
}


static int merge_nsih_secondboot(const char *in_file, const char *out_file, const build_info_t *BUILD_INFO)
{
	int ret = 0;
	FILE *nish_fd = NULL;
	FILE *in_fd = NULL;
	FILE *out_fd = NULL;
	unsigned char *out_buf = NULL;

	int out_buf_size = SECOND_BOOT_SIZE;
	int in_file_size = 0;
	size_t read_size = 0;
	size_t write_size = 0;

	struct NX_SecondBootInfo *bootinfo;
	int is_2ndboot = (BUILD_INFO->image_type) == BUILD_2NDBOOT ? 1 : 0;

	nish_fd = fopen(BUILD_INFO->nsih_file, "rb");
	in_fd = fopen(in_file, "rb");
	out_fd = fopen(out_file, "wb");

	if( !nish_fd || !in_fd || !out_fd )
	{
		ret = -1;
		pr_error("Cannot open file!!!\n");
		goto ERROR_EXIT;
	}

	in_file_size = get_file_size( in_fd );
	if( !is_2ndboot )
	{
		out_buf_size = in_file_size + 512;
	}
	if( in_file_size + NSIH_BIN_SIZE > out_buf_size )
	{
		ret = -1;
		pr_error("Input file too large(%d)!!!\n", in_file_size);
		goto ERROR_EXIT;
	}

	out_buf = (unsigned char*)malloc( out_buf_size );
	if( NULL == out_buf )
	{
		ret = -1;
		pr_error("Out buffer allocation failed!!(%d)\n", out_buf_size);
		goto ERROR_EXIT;
	}
	memset( out_buf, 0xff, out_buf_size );

	//--------------------------------------------------------------------------
	// NSIH parsing
	ret = ProcessNSIH (BUILD_INFO->nsih_file, out_buf);
	if (ret != NSIH_BIN_SIZE)
	{
		pr_error ("NSIH Parsing failed.\n");
		ret = -1;
		goto ERROR_EXIT;
	}
	ret = 0;

	update_bootinfo (out_buf, BUILD_INFO, in_file_size);


	read_size = fread( out_buf + NSIH_BIN_SIZE, 1, in_file_size, in_fd );
	if( read_size != in_file_size )
	{
		pr_error ("File read error.\n");
		ret = -1;
		goto ERROR_EXIT;
	}

	write_size = fwrite( out_buf, 1, out_buf_size, out_fd );
	if( write_size != out_buf_size )
	{
		pr_error ("Out file write failed.\n");
		ret = -1;
		goto ERROR_EXIT;
	}

ERROR_EXIT:
	if( nish_fd )	fclose( nish_fd );
	if( in_fd )		fclose( in_fd );
	if( out_fd )	fclose( out_fd );
	if( out_buf )	free( out_buf );

	if( ret == 0 )
	{
		printf("Image Generating Success!!!\n\n");
	}
	else
	{
		printf("Image Generating Failed!!!\n\n");
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
//
//	-- first stage --
//
static int first_stage (FILE *eccgen_fp, const char *in_file, build_info_t *BUILD_INFO)
{
	FILE	*bin_fp = NULL, *merge_fp = NULL;
	unsigned int BinFileSize, MergeFileSize, SrcLeft, SrcReadSize, DstSize, DstTotalSize;
	static unsigned int pdwSrcData[7*NX_BCH_SECTOR_MAX/4], pdwDstData[8*NX_BCH_SECTOR_MAX/4];

	unsigned char *parsed_nsih	= NULL;
	unsigned char *copy_buffer	= NULL;
	unsigned long copy_leftlen	= 0;
	unsigned long merged_size	= 0;
	int is_2ndboot				= (BUILD_INFO->image_type) == BUILD_2NDBOOT ? 1 : 0;
	unsigned int align;

	int ret = 0;
	size_t sz;



	//--------------------------------------------------------------------------
	// NSIH parsing

	/* prepare buffer for parsed nsih */
	parsed_nsih = malloc (iSectorSize);
	if (!parsed_nsih)
	{
		pr_error ("Not enough memory. (NSIH parsing)");
		ret = -1;
		goto out;
	}
	memset (parsed_nsih, 0xff, iSectorSize);


	/* parsing nsih */
	ret = ProcessNSIH (BUILD_INFO->nsih_file, parsed_nsih);
	if (ret != NSIH_LEN)
	{
		pr_error ("NSIH Parsing failed.\n");
		ret = -1;
		goto out;
	}


	//--------------------------------------------------------------------------
	// BIN processing
	bin_fp = fopen (in_file, "rb");
	if (!bin_fp)
	{
		pr_error ("ERROR : Failed to open %s file.\n", in_file);
		ret = -1;
		goto out;
	}

	BinFileSize = get_file_size (bin_fp);
	pr_debug ("BinFileSize : %d\n", BinFileSize);

	/* update bootinfo */
	update_bootinfo (parsed_nsih, BUILD_INFO, BinFileSize);


	/* merge : ParsedNSIH + BinFile */
	merge_fp = tmpfile ();
	if (!merge_fp)
	{
		pr_error ("failed to create tmp file.\n");
		ret = -1;
		goto out;
	}

	/* write ParsedNSIH */
	if (is_2ndboot)
		sz = fwrite (parsed_nsih, 1, NSIH_LEN, merge_fp);
	else
		/* NSIH + (0xff * (iSectorSize - NSIH_LEN)) */
		sz = fwrite (parsed_nsih, 1, iSectorSize, merge_fp);

	
	copy_buffer = malloc (COPY_LEN);
	if (!copy_buffer)
	{
		pr_error ("Not enough memory. (merge)\n");
		ret = -1;
		goto out;
	}


	/* get MergeFileSize */
	align = (is_2ndboot) ? MAX_2NDBOOT : BUILD_INFO->page_size;

	MergeFileSize = ALIGN(BinFileSize + sz, align);
	pr_debug ("MergeFileSize: %u\n", MergeFileSize);

	copy_leftlen = MergeFileSize;

	while (copy_leftlen > 0) {
		unsigned long io_size = (copy_leftlen > COPY_LEN) ? COPY_LEN : copy_leftlen;
		pr_debug ("io size: %lu\n", io_size);

		memset (copy_buffer, 0xff, COPY_LEN);
		sz = fread (copy_buffer, 1, io_size, bin_fp);
		fwrite (copy_buffer, 1, COPY_LEN, merge_fp);

		copy_leftlen -= io_size;
	}

	free (copy_buffer);

	rewind (merge_fp);

	if (is_2ndboot && MergeFileSize > MAX_2NDBOOT)
	{
		printf ("WARNING : %s is too big for Second boot image, only will used first %d bytes.\n", in_file, MAX_2NDBOOT);
		MergeFileSize = MAX_2NDBOOT;
	}

	pr_debug ("iSectorSize: %d\n", iSectorSize);

	iNX_BCH_VAR_K		=	(iSectorSize * 8);

	iNX_BCH_VAR_N		=	(((1<<iNX_BCH_VAR_M)-1));
	iNX_BCH_VAR_R		=	(iNX_BCH_VAR_M * iNX_BCH_VAR_T);

	iNX_BCH_VAR_TMAX	=	(60);
	iNX_BCH_VAR_RMAX	=	(iNX_BCH_VAR_M * iNX_BCH_VAR_TMAX);

	iNX_BCH_VAR_R32		=	((iNX_BCH_VAR_R   +31)/32);
	iNX_BCH_VAR_RMAX32	=	((iNX_BCH_VAR_RMAX+31)/32);

	iNX_BCH_OFFSET		= 8;

	SrcLeft = MergeFileSize;

	//--------------------------------------------------------------------------
	// Encoding - ECC Generation
	printf ("\n");

	DstTotalSize = 0;

	// generate the Galois Field GF(2**mm)
	generate_gf ();
	// Compute the generator polynomial and lookahead matrix for BCH code
	gen_poly ();



	while (SrcLeft > 0)
	{
		size_t sz;
		SrcReadSize = (SrcLeft > (iSectorSize*7)) ? (iSectorSize*7) : SrcLeft;
		pr_debug ("SrcLeft: 0x%x, SrcReadSize: 0x%x\n", SrcLeft, SrcReadSize);
		sz = fread (pdwSrcData, 1, SrcReadSize, merge_fp);
		SrcLeft -= SrcReadSize;
		DstSize = MakeECCChunk (pdwSrcData, pdwDstData, SrcReadSize);
		fwrite (pdwDstData, 1, DstSize, eccgen_fp);
		
		DstTotalSize += DstSize;
	}

	//--------------------------------------------------------------------------
	printf ("\n");
	printf ("%d bytes(%d sector) generated.\n", DstTotalSize, (DstTotalSize+iSectorSize-1)/iSectorSize);


out:
	if (parsed_nsih)
		free (parsed_nsih);
	if (bin_fp)
		fclose (bin_fp);
	if (merge_fp)
		fclose (merge_fp);


	return ret;
}

//
//	-- end of first stage --
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//	-- second stage --
//
static int second_stage(const char *out_file, FILE *eccgen_fp, const build_info_t *BUILD_INFO)
{
	FILE *out_fp;
	unsigned char *copy_buffer		= NULL;

	unsigned int page_size			= 0;
	int payload_in_page				= 0;
	int BinFileSize					= 0;
	int copy_count					= 0;
	
	int ret = 0 , i;


	/* buffer allocation */
	page_size = payload_in_page = BUILD_INFO->page_size;
	copy_buffer = malloc (page_size);
	if (!copy_buffer)
	{
		ret = -10;
		goto err;
	}

	out_fp = fopen (out_file, "wb");
	if (!out_fp) {
		ret = -20;
		goto err_out_fp;
	}


	/* size calc. */
	rewind (eccgen_fp);
	BinFileSize = get_file_size (eccgen_fp);
	pr_debug ("eccgen BinFileSize: %d\n", BinFileSize);

	if (BUILD_INFO->image_type == BUILD_2NDBOOT)
		payload_in_page = MIN (payload_in_page, ROMBOOT_MAXPAGE);
	copy_count = DIV_ROUND_UP (BinFileSize, payload_in_page);
	pr_debug ("payload: %d, page: %d, copy_count: %d\n", payload_in_page, page_size, copy_count);


	/* file writing */
	for (i = 0; i < copy_count; i++)
	{
		size_t sz;
		memset (copy_buffer, 0xff, page_size);

		sz = fread (copy_buffer, 1, payload_in_page, eccgen_fp);
		fwrite (copy_buffer, 1, page_size, out_fp);
	}

	fclose (out_fp);
err_out_fp:
	free (copy_buffer);
err:
	return ret;
}
//
//		--- end of second stage ---
//
//////////////////////////////////////////////////////////////////////////////

static int make_image (const char *out_file, const char *in_file, build_info_t *BUILD_INFO)
{
	int ret;
	FILE *eccgen_fp;

	eccgen_fp = tmpfile();
	if (!eccgen_fp)
	{
		pr_error ("Cannot generate temporary file. Abort.\n");
		goto done;
	}

	ret = first_stage  (eccgen_fp, in_file, BUILD_INFO);
	if (ret < 0)
		goto done;
	
	ret = second_stage (out_file, eccgen_fp, BUILD_INFO);
	if (ret < 0)
		return -1;

done:
	fclose (eccgen_fp);
	return ret;
}


void print_usage(char *argv[])
{
	//                 1         2         3         4         5         6         7         8
	//        12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n==============================================================================\n");
	printf("Description : Nexell Binary Generator with ECC Generator\n");
	printf("version     : %s, %s, %s\n", VERSION_STR, __DATE__, __TIME__);
	printf("\n");
	printf("usage: options\n");
	printf(" -h help                                                       \n");
	printf(" -t image type, 2ndboot|bootloader (mandatory)                 \n");
	printf(" -d device type, nand|other        (mandatory)                 \n");
	printf(" -i input file name                (mandatory)                 \n");
	printf(" -o output file name               (mandatory)                 \n");
	printf(" -n nsih file name                 (mandatory)                 \n");
	printf(" -a device read address            (optional, default NSIH.txt)\n");
	printf(" -p page size  (KiB)               (optional, nand only, default %d )    \n", ROMBOOT_MAXPAGE);
	printf(" -l load address                   (optional, default 0x%08x )\n", DEFAULT_LOADADDR);
	printf(" -e launch address                 (optional, default 0x%08x )\n", DEFAULT_LAUNCHADDR);
	printf("\nExample: nand image\n");
	printf(" %s -t 2ndboot -d nand -o nand_2ndboot.bin -i pyrope_2ndboot_nand.bin -n NSIH.txt -p 4096 -l 0x40c00000 -e 0x40c00000 \n", argv[0]);
	printf(" %s -t bootloader -d nand -o nand_bootloader.bin -i u-boot.bin -n NSIH.txt -p 4096 -l 0x40c00000 -e 0x40c00000 \n", argv[0]);
	printf("\nExample: eeprom image\n");
	printf(" %s -t 2ndboot -d other -o 2ndboot.bin -i pyrope_2ndboot_spi.bin -n NSIH.txt -l 0x40c00000 -e 0x40c00000 \n", argv[0]);
	printf("\nExample: sd image\n");
	printf(" %s -t 2ndboot -d other -o 2ndboot.bin -i pyrope_2ndboot_sd.bin -n NSIH.txt -l 0x40c00000 -e 0x40c00000 \n", argv[0]);
	printf("==============================================================================\n");
	printf("\n");
}


//------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	build_info_t BUILD_INFO;

	uint32_t sector_size	= 0;
	uint32_t dev_readaddr	= 0;
	unsigned int page_size	= ROMBOOT_MAXPAGE;
	uint32_t loadaddr		= DEFAULT_LOADADDR;
	uint32_t launchaddr		= DEFAULT_LAUNCHADDR;

	char *bin_type = NULL;
	char *dev_type = NULL;
	char *out_file = NULL;
	char *in_file = NULL;
	char *nsih_file = NULL;
	int image_type = BUILD_NONE;
	int device_type = DEVICE_NONE;

	int opt;


	// arguments parsing 
	while (-1 != (opt = getopt(argc, argv, "ht:d:o:i:n:a:p:l:e:"))) {
		switch(opt) {
			case 't':
				bin_type = optarg; break;
			case 'd':
				dev_type = optarg; break;
			case 'o':
				out_file = optarg; break;
			case 'i':
				in_file = optarg; break;
			case 'n':
				nsih_file = optarg; break;
			case 'a':
				dev_readaddr = strtoul (optarg, NULL, 16); break;
			case 'p':
				page_size = strtoul (optarg, NULL, 10); break;
			case 'l':
				loadaddr = strtoul (optarg, NULL, 16); break;
			case 'e':
				launchaddr = strtoul (optarg, NULL, 16); break;
			case 'h':
			default:
				print_usage(argv); exit(0); break;
		}
	}


	// argument check
	if (!bin_type || !dev_type || !out_file || !in_file || !nsih_file)
	{
		print_usage(argv);
		return -1;
	}

	if (!strcmp (in_file, out_file))
	{
		pr_error ("Input file name is equal to output file name.\n");
		return -1;
	}

	if (page_size < 512 || !IS_POWER_OF_2 (page_size))
	{
		pr_error ("Page size must bigger than 512 and must power-of-2\n");
		return -1;
	}

	//	parse Board Type
	if (!strcmp(bin_type, "2ndboot"))
		image_type = BUILD_2NDBOOT;
	else if(!strcmp(bin_type, "bootloader"))
		image_type = BUILD_BOOTLOADER;
	pr_debug ("build type : %d\n", image_type);
	if (image_type == BUILD_NONE)
	{
		pr_error ("Enter 2ndboot or bootloader\n");
		return -1;
	}

	//	parse Device Type
	if (!strcmp(dev_type, "nand"))
		device_type = DEVICE_NAND;
	if (!strcmp(dev_type, "other"))
		device_type = DEVICE_OTHER;
	pr_debug ("device type : %d\n", device_type);
	if (device_type == DEVICE_NONE)
	{
		pr_error ("Enter nand or other\n");
		return -1;
	}

	// if( device_type==DEVICE_OTHER && image_type!=BUILD_2NDBOOT )
	// {
	// 	pr_error ("Other device support only second boot mode.\n");
	// 	return -1;
	// }

	if( device_type == DEVICE_NAND )
	{
		// get sector size
		sector_size = (page_size < 1024) ? 512 : 1024;
		if (sector_size == 512)
		{
			iSectorSize			=	512;
			iNX_BCH_VAR_T		=	24;
			iNX_BCH_VAR_M		=	13;
		}
		else if (sector_size == 1024)
		{
			iSectorSize			=	1024;
			iNX_BCH_VAR_T		=	60;
			iNX_BCH_VAR_M		=	14;
		}
		pr_debug("page size: %d, sector size: %d\n", page_size, sector_size);
	}

	BUILD_INFO.nsih_file = nsih_file;
	BUILD_INFO.loadaddr = loadaddr;
	BUILD_INFO.launchaddr = launchaddr;
	BUILD_INFO.image_type = image_type;
	BUILD_INFO.dev_readaddr = dev_readaddr;
	BUILD_INFO.page_size = page_size;


	//                 1         2         3         4         5         6         7         8
	//        12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n==============================================================================\n");
	printf(" Type        : %s\n", bin_type);
	printf(" Device      : %s\n", dev_type);
	printf(" Input       : %s\n", in_file);
	printf(" Output      : %s\n", out_file);
	printf(" NSIH        : %s\n", nsih_file);
	if (BUILD_INFO.dev_readaddr)
	printf(" Read Addr   : 0x%x\n", BUILD_INFO.dev_readaddr);
	printf(" Load Addr   : 0x%x\n", BUILD_INFO.loadaddr);
	printf(" Luanch Addr : 0x%x\n", BUILD_INFO.launchaddr);
	printf(" Signature   : 0x%x\n", HEADER_ID);
	printf("==============================================================================\n");

	//  make image
	if (device_type == DEVICE_NAND)
		make_image (out_file, in_file, &BUILD_INFO);
	else
		merge_nsih_secondboot (in_file, out_file, &BUILD_INFO);

	return 0;
}

