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

char *partmapf = NULL;

void write_sd(char *path, int size, int start_blk, unsigned char *buffer );

#define DEBUG 0
#if (DEBUG)
#define debug(msg...) { printf(msg); }
#else
#define debug(msg...) do{} while (0)
#endif

#define MAX_PARTITION_TABLE 7
#define SECTOR_BITS             9 /* 512B */
#define SD_BLOCK_SIZE						512

#define SECONDBOOT 0x01
#define UBOOT 0x02

typedef struct partcmd
{
	char cmd[10];
	char device[10];
	char  dev_no[10];
	char f_type[15];
	char p_name[15];
	char  s_addr[15];
	char  len[15];
	char f_name[20];
} partcmd;

typedef struct partmap
{
	int start;
	int length;
	int type;
	char img[20];
} partmap;


typedef struct rawimage
{
	int start;
	int length;
	int type;
	char img[20];
} rawimage;


int file_test(char *path)
{
	int fd=0;
	fd=open(path,O_RDONLY);
	if(fd <0)
		return -1;
	else
		{
			close(fd);
			return 0;
		}	
}
extern int UpdateBootImage(char *imgBase, size_t imgSize, char *blockLocation);	

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

void usage()
{
	printf( "usage: options\n"
					" -d	device  node 	      default : none\n "
					" -p	partmap file 				default	: ./partmap.txt \n"
					" -f  format device \n"
					" -h	print this help text\n");
}

long long get_disk_size(char *path )
{
	unsigned long long size = 0;
	int fd = open( path, O_RDONLY);
	if(fd < 0)
	{
		printf("SD open fail");
		return -1;
	}	
	ioctl(fd, BLKGETSIZE64, &size);	
  close(fd);
	return size/SD_BLOCK_SIZE;
}

int main(int argc, char **argv)
{
	FILE *map = NULL;
	FILE *ptable=NULL;
	char *dev_node = NULL;
	int part_num=0;
	int raw_num=0;
	int fd=0;
	int i=0;
	int r_part_num=0;
	partmap n_part[MAX_PARTITION_TABLE];
	partmap r_part[MAX_PARTITION_TABLE];
	rawimage rimg[MAX_PARTITION_TABLE];
	partcmd ph;
	unsigned long long t_size=0;
	char buf[256];
	char cmd[256];
	int c;
	int tmp=0;
	int format = 0;
	
	while (1) {
	  c = getopt(argc, argv, "d:p:hf");
	  if (c == -1) 
	      break;
	      
	  switch (c) 
	  {   
	  case 'p':
	  	partmapf = strdup(optarg); 
	  	break;
	
	  case 'd':
	  	dev_node = strdup(optarg);
	    break;
	
		case 'f':
	  	format = 1;
	    break;
	  case 'h':
	      usage();
	      return 0;
	
	  default:
	      printf("unkown option parameter(%c)\n", c); 
	      break;
	  }   
	}
	
	if(dev_node == NULL)
	{
		printf("Insert SD Card device node -d \n");
		exit(0); 
	}
	else
	{	
		if(0 > file_test(dev_node) )
		{
			printf("fail SD card open\n");
			goto end;
		}
	}
	t_size = get_disk_size(dev_node);
	
	if(t_size <= 0)
	{
		printf("get sd Size info fail \n");
		goto end;
	}
	print_partition_extended(dev_node,0, 0, 1, 0,r_part);

#if (DEBUG)
	for(i=0;i<7;i++)
	{
		printf(" %d  start : %d size %d ,tupe %d \n",i,r_part[i].start,r_part[i].length,r_part[i].type );
	}		
#endif
	
	if( partmapf == NULL)
	{
		printf("Insert partmap.tx path -p  \n");
	}
	map = fopen(partmapf,"r");
		
	if(map==NULL)
	{
		printf("Error! partmap.txt file open fail!\n");
		return -1;
	}
	
	
	/* Parsing Partmap.txt */  
	do{
		memset(buf,0,256);
		
		if(fgets(buf,256,map)==NULL)
		{
			printf("partmap Parsing end\n");
			break;
		}
		if(buf[0] == '#')
			continue;
			//sscanf(buf,"%[^=]=%[^,],%[^:]:%[^:]:%[^:]:%[^,],%[^:];",ph.cmd,ph.device,ph.dev_no,ph.f_type,ph.p_name,ph.s_addr,ph.len);
			sscanf(buf,"%[^=]=%[^,],%[^:]:%[^:]:%[^:]:%[^,],%[^,],%[^:];",ph.cmd,ph.device,ph.dev_no,ph.f_type,ph.p_name,ph.s_addr,ph.len,ph.f_name);
			
		//if( (!strcmp(ph.device,"mmc")) && ((!strcmp(ph.p_name ,"ext4")) || (!strcmp(ph.p_name , "emmc" )) ))
		if( (!strcmp(ph.device,"mmc"))) // && ((!strcmp(ph.p_name ,"ext4")) || (!strcmp(ph.p_name , "emmc" )) ))
		{
			if( !strcmp(ph.p_name ,"ext4") || !strcmp(ph.p_name , "emmc" ))
			{
				n_part[part_num].start = String2Hex(ph.s_addr);
				n_part[part_num].length = String2Hex(ph.len);
				//printf(" start : %d len %d \n",	n_part[part_num].start,n_part[part_num].length );
		
				if(!format)
				{	
					for(i=0;i<7;i++)
					{
						if ( n_part[part_num].start/512 == r_part[i].start &&  n_part[part_num].length/512 == r_part[i].length)
							break;
						if (i==7)
							format = 1;
					}	
				}

			if(ph.f_name[0] == 0)
					sprintf(n_part[part_num].img,"%s.img",ph.f_type);
				else
					sprintf(n_part[part_num].img,"%s.img",ph.f_name);
			 		
			
				part_num++;
			}
			else if( !strcmp(ph.p_name,"2nd"))
			{
				rimg[raw_num].start = String2Hex(ph.s_addr);
				rimg[raw_num].length = String2Hex(ph.len);
				rimg[raw_num].type = SECONDBOOT;
				if(ph.f_name[0] == 0)
					sprintf(rimg[raw_num].img,"%s.bin",ph.f_type);
				else
					sprintf(rimg[raw_num].img,"%s.bin",ph.f_name);	
					
				
				debug("Raw %d : %d %d %d %s\n " ,raw_num,	rimg[raw_num].start, rimg[raw_num].length ,	rimg[raw_num].type , rimg[raw_num].img);
				raw_num++;
			}
			else if( !strcmp(ph.p_name,"raw"))
			{
				rimg[raw_num].start = String2Hex(ph.s_addr);
				rimg[raw_num].length = String2Hex(ph.len);
				rimg[raw_num].type = 0;
				sprintf(rimg[raw_num].img,"%s.img",ph.f_type);
				if(ph.f_name[0] != 0)
					sprintf(rimg[raw_num].img,"%s.bin",ph.f_name);	
				raw_num++;
			}
			else if( !strcmp(ph.p_name,"boot"))
			{
				rimg[raw_num].start = String2Hex(ph.s_addr);
				rimg[raw_num].length = String2Hex(ph.len);
				rimg[raw_num].type = UBOOT;
	
				if(ph.f_name[0] == 0)
					sprintf(rimg[raw_num].img,"%s.bin","u-boot");
				else
					sprintf(rimg[raw_num].img,"%s.bin",ph.f_name);	
								
				raw_num++;
			}
		}
	
	}while(1);
		
	if(format)
	{
		ptable = fopen("./ptable","w");
		
		if(ptable == NULL)
		{
			goto end;
		}
		sprintf(buf,"unit: sectors\n\n");
		fwrite(buf,1,strlen(buf),ptable);
		if(part_num <= 4)
		{	
		
			for(i=0;i<part_num;i++)
			{
				sprintf(buf,"%s%d : start = %8d,  size = %8d, Id=83\n",dev_node,i+1,(n_part[i].start)/SD_BLOCK_SIZE , (i+1,n_part[i].length)/SD_BLOCK_SIZE);
				fwrite(buf,1,strlen(buf) ,ptable);
			}
		}
		
		else
		{	
			int i=0;
			for(i=0;i<3;i++)
			{
				sprintf(buf,"%s%d : start=%8d,  size=%8d, Id=83\n",dev_node,i+1,(n_part[i].start)/SD_BLOCK_SIZE , (n_part[i].length)/SD_BLOCK_SIZE);
				fwrite(buf,1,strlen(buf) ,ptable);
				n_part[i].type=83;
			}
			int ext_s =(n_part[2].start+n_part[2].length)/SD_BLOCK_SIZE;
			sprintf(buf,"%s%d : start=%8d,  size=%8lld, Id= 5\n",dev_node, 4,ext_s , t_size - ext_s);
			
			fwrite(buf,1,strlen(buf) ,ptable);
			for(i=3;i<part_num;i++)
			{
				sprintf(buf,"%s%d : start= %8d,  size= %8d, Id=83\n",dev_node,i+2,(n_part[i].start)/SD_BLOCK_SIZE , n_part[i].length ? (n_part[i].length)/512 : t_size - (n_part[i].start/SD_BLOCK_SIZE) );
				n_part[i].type=5;
				fwrite(buf,1,strlen(buf) ,ptable);
			}
		}
		fclose(ptable);
		
		sprintf(buf ,"sudo sfdisk %s  < ./ptable --force -q  ",dev_node);		
		tmp = system( buf );
		tmp = system("rm ptable");
	}
	
	
	#if 1
	/* raw image write */
	FILE *w_img = NULL;
	char * source =NULL;
	char *dev = NULL;
	int ret;
	dev = malloc(20);
	
	int filesize;
	
	for(i=0;i<raw_num ;i++)
	{
		w_img = fopen(rimg[i].img,"r");
		if(w_img == NULL)
		{
			printf("no Img %s \n",rimg[i].img);
			continue;
		}
		fseek(w_img,0L, SEEK_END);
		filesize = ftell(w_img);
		fseek(w_img, 0L, SEEK_SET);
	
		source = malloc(filesize);
		ret = fread(source,1,filesize,w_img);
		if(source == NULL)
		{
			printf("alloc fail\n");
		}
		printf(" write %s \n",rimg[i].img); 
		write_sd(dev_node,filesize ,rimg[i].start/512 ,source );
		printf("write done.\n");
		free(source);
		fclose(w_img);
	}
	
	
	/* Img write for partition*/	
			
	for(i=0;i<part_num ;i++)
	{
		w_img = fopen(n_part[i].img,"r"); 
		if(w_img == NULL)
		{
			printf("no Img %s \n",n_part[i].img);
			continue;
		}
		fseek(w_img,0L, SEEK_END);
		filesize = ftell(w_img);
		fseek(w_img, 0L, SEEK_SET);
		//printf("File size %8d \n", filesize);
		
		source = malloc(filesize);
		if(source == NULL)
		{
			printf("alloc fail\n");
		}
	
		ret = fread(source,1,filesize,w_img);
			
		sprintf(dev,"%s%d",dev_node,i<4 ? i+1 : i+2)	;
		printf(" write %s \n",n_part[i].img); 
		UpdateBootImage(source,filesize,dev );
		
		free(source);
		fclose(w_img);	
	}
	#endif
end:
	fclose(map);
		
		return 0;
}