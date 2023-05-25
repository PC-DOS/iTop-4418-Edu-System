
#define	READ_CMD		0x03
#define	WRITE_CMD		0x02
#define	WRITE_ENABLE	0x06

#define	ADDR_BYTE	3
#define	CMD_LEN		1

#define	CMD_BUF ADDR_BYTE + CMD_LEN

#define PAGE_SIZE 256

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//#define SHOW_READ_DATA
static void pabort(const char *s);

static uint8_t mode;
static uint8_t bits;
static uint32_t speed;
static uint16_t delay;

int spi_init(int fd);

int spi_read(int fd, int offset, char * addr, int size);	//offset : 24bit addr read start addr
int spi_blk_erase(int fd,int addr);						//erase Block 64K
int spi_write_enable(int fd);							
int spi_write(int fd ,int dst, char * addr, int size);