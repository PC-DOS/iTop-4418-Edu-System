#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <wchar.h>
#include <linux/fs.h>

#ifndef _DISK_PART_DOS_H
#define _DISK_PART_DOS_H

#define DOS_PART_DISKSIG_OFFSET 0x1b8
#define DOS_PART_TBL_OFFSET 0x1be
#define DOS_PART_MAGIC_OFFSET   0x1fe
#define DOS_PBR_FSTYPE_OFFSET   0x36
#define DOS_PBR32_FSTYPE_OFFSET 0x52
#define DOS_PBR_MEDIA_TYPE_OFFSET   0x15
#define DOS_MBR 0
#define DOS_PBR 1

typedef struct dos_partition {
    unsigned char boot_ind;     /* 0x80 - active            */
    unsigned char head;     /* starting head            */
    unsigned char sector;       /* starting sector          */
    unsigned char cyl;      /* starting cylinder            */
    unsigned char sys_ind;      /* What partition type          */
    unsigned char end_head;     /* end head             */
    unsigned char end_sector;   /* end sector               */
    unsigned char end_cyl;      /* end cylinder             */
    unsigned char start4[4];    /* starting sector counting from 0  */
    unsigned char size4[4];     /* nr of sectors in partition       */
} dos_partition_t;

typedef struct partmap
{
	int start;
	int length;
	int type;
	char img[20];
} partmap;

#endif  /* _DISK_PART_DOS_H */


typedef unsigned long long uint64;
#define DEBUG 0
#if (DEBUG)
#define debug(msg...) { printf(msg); }
#else
#define debug(msg...) do{} while (0)
#endif

unsigned char *buffer =NULL;

static int test_block_type(unsigned char *buffer)
{
    if((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
        (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
        return (-1);
    } /* no DOS Signature at all */
    if (strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET],"FAT",3)==0 ||
        strncmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET],"FAT32",5)==0) {
        return DOS_PBR; /* is PBR */
    }
    return DOS_MBR;     /* Is MBR */
}

static inline int le32_to_int(unsigned char *le32)
{
    return ((le32[3] << 24) +
        (le32[2] << 16) +
        (le32[1] << 8) +
         le32[0]
       );
}

static inline int is_extended(int part_type)
{
    return (part_type == 0x5 ||
        part_type == 0xf ||
        part_type == 0x85);
}

static inline int is_bootable(dos_partition_t *p)
{
    return p->boot_ind == 0x80;
}

static void print_one_part(dos_partition_t *p, int ext_part_sector,
               int part_num, unsigned int disksig,partmap *part)
{
    int lba_start = ext_part_sector + le32_to_int (p->start4);
    int lba_size  = le32_to_int (p->size4);

    debug("%3d\t%-10d\t%-10d\t%08x-%02x\t%02x%s%s\n",
        part_num, lba_start, lba_size, disksig, part_num, p->sys_ind,
        (is_extended(p->sys_ind) ? " Extd" : ""),
        (is_bootable(p) ? " Boot" : ""));
        part[part_num-1].start = lba_start;
        part[part_num-1].length = lba_size;
        part[part_num-1].type = p->sys_ind;
}


static void read_sd(char *path, int size, int start_blk, unsigned char *buffer )
{
	char p[100];
	
	int f;
	int ret;
	debug("%s dev [%s], %d %d \n  ", __func__,path,start_blk, size);
	f= open(path, O_RDONLY);
	lseek(f,start_blk * 512 ,0);
	ret =read(f,buffer,size * 512 );
	close(f);
	
}


void write_sd(char *path, int size, int start_blk, unsigned char *buffer )
{
	char p[100];	
	int f;
	int ret;
	
	debug("%s dev [%s], %d %d \n  ", __func__,path,start_blk, size);
	f= open(path, O_RDWR);
	lseek(f,start_blk * 512 ,0);
	ret = write(f,buffer,size );
	close(f);
	
}
void print_partition_extended(char *path, int ext_part_sector, int relative,
                     int part_num, unsigned int disksig, partmap *part)
{
    
    dos_partition_t *pt;
    int i;
		unsigned char *buffer = malloc(512);
  
    read_sd(path,1,ext_part_sector,buffer);
    
    i=test_block_type(buffer);
    if (i != DOS_MBR) {
        printf ("bad MBR sector signature 0x%02x%02x\n",
            buffer[DOS_PART_MAGIC_OFFSET],
            buffer[DOS_PART_MAGIC_OFFSET + 1]);
        return;
    }
		
    if (!ext_part_sector)
        disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);

    /* Print all primary/logical partitions */
    pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
    for (i = 0; i < 4; i++, pt++) {
        /*
         * fdisk does not show the extended partitions that
         * are not in the MBR
         */

        if ((pt->sys_ind != 0) &&
            (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
            print_one_part(pt, ext_part_sector, part_num, disksig,part);
        }

        /* Reverse engr the fdisk part# assignment rule! */
        if ((ext_part_sector == 0) ||
            (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
            part_num++;
        }
    }

    /* Follows the extended partitions */
    pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
    for (i = 0; i < 4; i++, pt++) {
        if (is_extended (pt->sys_ind)) {
            int lba_start = le32_to_int (pt->start4) + relative;

            print_partition_extended( path,lba_start,
                ext_part_sector == 0  ? lba_start : relative,
                part_num, disksig,part);
        }
    }

    return;
}


#if 0
int main(void)
{
	int ext_part_sector = 0;
	int relative = 0;
  int part_num = 1;
  unsigned int disksig = 0;
	dos_partition_t *pt = NULL;
	partmap readpart[7];
	int i;

		
		print_partition_extended("/dev/sdf",0, 0, 1, 0,readpart);


		for(i=0;i<7;i++)
		{
			printf(" %d  start : %d size %d ,tupe %d \n",i,readpart[i].start,readpart[i].length,readpart[i].type );
		}
		
#ifdef TEST
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	
    for (i = 0; i < 4; i++, pt++) {
        /*  
         * fdisk does not show the extended partitions that
         * are not in the MBR
         */

				printf("XXX %x %d %d  %p\n ",pt->sys_ind ,ext_part_sector ,is_extended(pt->sys_ind),pt);

        if ((pt->sys_ind != 0) && (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
            	
            print_one_part(pt, ext_part_sector, part_num, disksig);
        }   

        /* Reverse engr the fdisk part# assignment rule! */
        if ((ext_part_sector == 0) ||
            (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) { 
            part_num++;
        }   

    }   
    
    
	map = fopen("ext.txt","r");
	fread(buffer,1,512,map);
	fclose(map);

	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	
    for (i = 0; i < 4; i++, pt++) {
        /*  
         * fdisk does not show the extended partitions that
         * are not in the MBR
         */

				printf("XXX %x %d %d  %p\n ",pt->sys_ind ,ext_part_sector ,is_extended(pt->sys_ind),pt);

        if ((pt->sys_ind != 0) && (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
            	
            print_one_part(pt, ext_part_sector, part_num, disksig);
        }   

        /* Reverse engr the fdisk part# assignment rule! */
        if ((ext_part_sector == 0) ||
            (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) { 
            part_num++;
        }   

    }   
    
    map = fopen("ext1.txt","r");
		fread(buffer,1,512,map);
	fclose(map);

	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	
    for (i = 0; i < 4; i++, pt++) {
        /*  
         * fdisk does not show the extended partitions that
         * are not in the MBR
         */

				printf("XXX %x %d %d  %p\n ",pt->sys_ind ,ext_part_sector ,is_extended(pt->sys_ind),pt);

        if ((pt->sys_ind != 0) && (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
            	
            print_one_part(pt, ext_part_sector, part_num, disksig);
        }   

        /* Reverse engr the fdisk part# assignment rule! */
        if ((ext_part_sector == 0) ||
            (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) { 
            part_num++;
        }   

    } 

#endif
	
	
	free(buffer);
	return 0;	
}
#endif