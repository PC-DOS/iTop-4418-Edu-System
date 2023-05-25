#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>      // errno

#include "s5p4418_boot.h"

#define DEBUG 1

#define POLY 0x04C11DB7L
#define TEXT_BASE 0x40c00000

char *o_path = "./u-boot.bin_sd";
char *i_path = "./u-boot.bin";
unsigned int get_fcs(unsigned int fcs, unsigned char data)
{
    register int i;
    fcs ^= (unsigned int)data;
   
	for(i=0; i<8; i++)
    {   
	    if(fcs & 0x01) fcs ^= POLY; fcs >>= 1;
	}   
	return fcs;
}

unsigned int atoh(char *a) 
{
	char ch = *a; 
	unsigned int cnt = 0, hex = 0;
	
	while(((ch != '\r') && (ch != '\n')) || (cnt == 0)) {
		ch  = *a; 
	
		if (! ch) 
			break;
	
		if (cnt != 8) {
			if (ch >= '0' && ch <= '9') {
				hex = (hex<<4) + (ch - '0');
				cnt++;
			} else if (ch >= 'a' && ch <= 'f') {
					hex = (hex<<4) + (ch-'a' + 0x0a);
					cnt++;
			} else if (ch >= 'A' && ch <= 'F') {
					hex = (hex<<4) + (ch-'A' + 0x0A);
					cnt++;
			}   
		}   
		a++;
	}   
	return hex;
}


void print_usage(void)
{
    printf( "usage: options\n"
			" -i input u-boot file path      default : ./u-boot.bin\n"
			" -n nsih file path              default : ./nish.txt\n"
			" -o output file path            default : ./u-boot.bin_sd\n"
			" -l load address                default : 0x40c00000\n"
			" -p boot port                   default : 0\n"
			" -h print this help text  \n"
		  );
}

int main( int argc, char **argv )
{

int opt  = 0;
int port = 0;
int load = TEXT_BASE;
int len	 = 0;

FILE *ifd  = NULL;
FILE *ofd  = NULL;
unsigned char *buf;
	while (-1 != (opt = getopt(argc, argv, "hi:o:p:l:"))) {
		switch(opt) {
			case 'i' : i_path  = strdup(optarg);		break;
			case 'o' : o_path  = strdup(optarg);		break;
			case 'l' : load 	 = atoh(optarg);			break;
			case 'p' : port 	 = atoi(optarg);			break;
			case 'h' : print_usage();							exit(0);
		}
	}

	/* step 1 : u-boot.bin open*/
	ifd = fopen(i_path, "ro");
	if (ifd == NULL) {
		printf("can't open u-boot.bin\n");
		return -1;
	}
	fseek(ifd,0,SEEK_END);
	len = ftell(ifd);
	fseek(ifd,0,SEEK_SET);
	
	buf = (unsigned char *)malloc(len);
	
	if(!buf) {
		printf("can't open u-boot.bin\n");
		goto end;
	}
	
	if(len !=(int) fread(buf,1,len,ifd)) {
		printf("read file. \n");
		goto end;
	}	
	
	/* step 2 : make Header	*/
	struct boot_dev_head head;
	struct boot_dev_head *bh = &head;
	struct boot_dev_mmc *bd = (struct boot_dev_mmc *)&bh->bdi;
	
	memset(bh,0,512);
	bh->load_addr = (unsigned int)load;
	bh->jump_addr = bh->load_addr;
	bh->load_size = (unsigned int)len;
	bh->signature = SIGNATURE_ID;
	bd->port_no   = port;

	/* step 3 : output file open */
	ofd = fopen(o_path, "wb");
	if (ofd == NULL ){
		printf("don't create output file.\n");
		goto end;
	}
	/* step 4 : output file Write */
	fwrite(bh, 1,512, ofd);
	fwrite(buf,1,len, ofd);

	sync();
end:
	if(ifd)
		fclose(ifd);
	if(ofd)
		fclose(ofd);
	if(buf)
		free(buf);
		
	return 0;
}

