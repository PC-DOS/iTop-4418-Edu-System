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
#include "spi_test.h"

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
//static const char *device = "/dev/spidev0.1";
//static uint8_t mode;
//static uint8_t bits = 8;
//static uint32_t speed = 500000;
//static uint16_t delay;



int spi_init(int fd)
{
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return 0;
}

int spi_read(int fd,int offset, char * addr, int size)
{
	int ret,i;
	uint8_t tx[size+CMD_BUF];	
	uint8_t rx[size+CMD_BUF];

	for(i=0;i<size+CMD_BUF;i++)
	{
		rx[i]=0;
	}
	tx[0] =  READ_CMD;
	if(ADDR_BYTE == 3)
	{	
		tx[1] = (offset >> 16);
		tx[2] = (offset >> 8);
		tx[3] = (offset );
	}
	else if(ADDR_BYTE == 2)
	{
		tx[1] = (offset >> 8);
		tx[2] = (offset);
	}
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		pabort("can't send spi message");
#ifdef SHOW_READ_DATA
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);		
	}
	puts("");	
#endif
	memcpy(addr, rx+CMD_BUF,size);
	
	return 0;
}
int spi_blk_erase(int fd,int addr)
{
	int ret,i;
	uint8_t tx[4];	
	uint8_t rx[4];

	tx[0] =  0xd8;
	
	tx[1] = ((addr >> 16) & 0xff);
	tx[2] = ((addr >> 8) & 0xff);
	tx[3] = (addr & 0xff);

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		pabort("can't send spi message");
#ifdef SHOW_READ_DATA
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);		
	}
	puts("");	
#endif
	return 0;
}

int spi_write_enable(int fd)
{
	int ret,i;
	uint8_t tx[1];	
	uint8_t rx[1];

	tx[0] =  WRITE_ENABLE;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	return ret;

}

int spi_write(int fd, int dst, char * addr, int size)
{
	int ret,i,offset=0;
	uint8_t tx[PAGE_SIZE+CMD_BUF];	
	uint8_t rx[PAGE_SIZE+CMD_BUF];

	int write_size =0;

	tx[0] =  WRITE_CMD;

	do{
		spi_write_enable(fd);
		if(size > PAGE_SIZE)
		{
		write_size = PAGE_SIZE - (dst%PAGE_SIZE);
		}
		else
		{	
			if((PAGE_SIZE - (dst%PAGE_SIZE))  > size)
			write_size = size;
			else 
			write_size = (PAGE_SIZE - (dst%PAGE_SIZE));	
		}	

		if(ADDR_BYTE == 3)
		{	
			tx[1] = (dst >> 16);
			tx[2] = (dst >> 8);
			tx[3] = (dst );
		}
		else if(ADDR_BYTE == 2)
		{
			tx[1] = (dst >> 8);
			tx[2] = (dst );
		}
		
		memcpy(tx+CMD_BUF,&addr[offset],write_size);
		
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			//.len = ARRAY_SIZE(tx),
			.len = write_size+CMD_BUF,
			.delay_usecs = delay,
			.speed_hz = speed,
			.bits_per_word = bits,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		if (ret == 1)
			pabort("can't send spi message");

		size -= write_size;

		offset+=write_size;

		dst+=write_size;
		usleep(10000);
	}while(size > 0);

	return ret;

}


int main()
{
	int ret,i;
	int size = 64;
	uint8_t read_buf_org[4*1024] = {0, };
	uint8_t read_buf[4*1024] = {0, };
	uint8_t write_buf[4*1024] ;
	int fd;
	
	bits = 8;
	speed = 750000;

	fd = open(device, O_RDWR);
	
	if (fd < 0)
		pabort("can't open device");
	if( 0 !=spi_init(fd))
		pabort("SPI Initilize fail");

	for(i= 0; i <2048; i++){
		write_buf[i]=i*2;
	//printf("%.2x ",write_buf[i]);
	}

	printf("======= SPI EEPROM TEST!!!!============\n");
	
	// READ : org data 
	printf("== READ DATA(ORG)");
	spi_read(fd,0x000,read_buf_org,size);	//Read Data
	for (i =0 ; i <size ; i++)
	{
		if ((i%16) == 0)	
			printf("\n");
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");	

	
	// WRITE : new data
	printf("== READ DATA(new)");
	sleep(1);
	spi_write_enable(fd);
	spi_write(fd,0x7D000,write_buf,size);	//Write Data
	sleep(1);
	spi_read(fd,0x7D000,read_buf,size);	//Read Data
	for (i =0 ; i <size ; i++)
	{
		if ((i%16) == 0)	
			printf("\n");
		printf("0x%.2X ",read_buf[i]);
	}
	printf("\n");
	
	printf("=========================================\n");	

	
	close(fd);

	return 0;
}
