/*
* fbrandrect2.c : Frame buffer draw random rectangular example unsing mmap
*
* Copyright(C) 2002 holelee
*
*/

#include <stdio.h>
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for open/close .. */
#include <fcntl.h> /* for O_RDONLY */
#include <sys/ioctl.h> /* for ioctl */
#include <sys/mman.h>
#include <linux/fb.h> /* for fb_var_screeninfo, FBIOGET_VSCREENINFO */

//--------------------------------------------------------------------------------------------------------------------
typedef unsigned char 	u8;
typedef unsigned short  u16;
typedef unsigned int  	u32;

//--------------------------------------------------------------------------------------------------------------------
#define	RGB888TO565(col) 	((((col>>16)&0xFF)&0xF8)<<8) | ((((col>>8)&0xFF)&0xFC)<<3) | ((((col>>0 )&0xFF)&0xF8)>>3)

void
putpixel888To565(
		u32  base,
		int  xpos,
		int  ypos,
		int  width,
		int  height,
		u32  color
		)
{
	*(unsigned short*)(base + (ypos * width + xpos) * 2) = (unsigned short)RGB888TO565(color);
}

void
putpixel888To888(
		u32  base,
		int  xpos,
		int  ypos,
		int  width,
		int  height,
		u32  color
		)
{
	*(u8*)(base + (ypos * width + xpos) * 3 + 0) = ((color>> 0)&0xFF);	// B
	*(u8*)(base + (ypos * width + xpos) * 3 + 1) = ((color>> 8)&0xFF);	// G
	*(u8*)(base + (ypos * width + xpos) * 3 + 2) = ((color>>16)&0xFF);	// R
}

void
putpixel888To8888(
		u32  base,
		int  xpos,
		int  ypos,
		int  width,
		int  height,
		u32  color
		)
{
	*(u32*)(base + (ypos * width + xpos) * 4) = (u32)(color | (0xFF<<24));
}

void (*PUTPIXELTABLE[])(u32, int, int, int, int, u32) =
{
	putpixel888To565,
	putpixel888To888,
	putpixel888To8888,
};
//--------------------------------------------------------------------------------------------------------------------

#define FBNODE 	"/dev/fb0"

//--------------------------------------------------------------------------------------------------------------------
#define	R	0x00
#define	G	0xFF
#define	B	0x00

//--------------------------------------------------------------------------------------------------------------------
void print_usage(void)
{
    printf( "usage: options\n"
    		"-d open device ()	\n"
    		"-h help	\n"
            "-l	left 	\n"
            "-t	top 	\n"
            "-x	width	\n"
            "-y	height	\n"
            );
}

int main(int argc, char **argv)
{
	int fd, ret, opt;
	int x, y, x1, y1, x2, y2;
	int length = 0, pixelbyte;
	u32  color;
	u32 *fbbase = NULL;
	int  sx = 0, sy = 0, wx = 0, hy = 0;
 	char fbpath[64] = FBNODE;

	void (*putpixel)(u32, int, int, int, int, u32) = 0;

	struct fb_var_screeninfo fbvar;


	//---------------------------------------------------------------------
    while (-1 != (opt = getopt(argc, argv, "d:hl:t:x:y:"))) {
		switch (opt) {
		case 'd':	strcpy(fbpath, optarg);		break;
        case 'h':   print_usage();  exit(0);   	break;
        case 'l':   sx = atoi(optarg); 			break;
        case 't':   sy = atoi(optarg); 			break;
        case 'x':   wx = atoi(optarg); 			break;
        case 'y':   hy = atoi(optarg); 			break;
        default:
        	break;
         }
	}

	fd = open(fbpath, O_RDWR);
	if (fd < 0) {
		perror("fbdev open");
		exit(1);
	}

	ret = ioctl(fd, FBIOGET_VSCREENINFO, &fbvar);
	if (ret < 0) {
		perror("fbdev ioctl");
		exit(1);
	}
	printf("%s resol: %d * %d, bpp %d\n",
		fbpath, fbvar.xres, fbvar.yres, fbvar.bits_per_pixel);

	if (! wx)
		wx = fbvar.xres - sx;

	if (! hy)
		hy = fbvar.yres - sy;

	if (sx > (int)fbvar.xres || sy > (int)fbvar.yres ||
		wx > (int)fbvar.xres || hy > (int)fbvar.yres ) {
		printf("over sx %d, sy %d, w %d, h %d (xres %d, yres %d) \n",
			sx, sy, wx, hy, fbvar.xres, fbvar.yres);
		goto __exit;
	}

		printf("over sx %d, sy %d, w %d, h %d (xres %d, yres %d) \n",
			sx, sy, wx, hy, fbvar.xres, fbvar.yres);

	//---------------------------------------------------------------------
	pixelbyte = fbvar.bits_per_pixel/8;
	length	  = fbvar.xres * fbvar.yres * pixelbyte;
	color 	  = (R<<16) | (G << 8) | (B);

	switch (pixelbyte) {
	case 2:	putpixel = PUTPIXELTABLE[pixelbyte-2];	break;
	case 3: putpixel = PUTPIXELTABLE[pixelbyte-2];	break;
	case 4: putpixel = PUTPIXELTABLE[pixelbyte-2];	break;
	default:
		printf("not support bytepixel (%d) ...\r\n", pixelbyte);
		exit(1);
	}
	//---------------------------------------------------------------------

	fbbase = (u32 *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	printf("fbbase address : 0x%08x \n", fbbase);
	
	if ((unsigned)fbbase == (unsigned)-1) {
		perror("fbdev mmap");
		exit(1);
	}
	//---------------------------------------------------------------------

	/* seed for rand */
	srand(1);

//	while (0 < count--)
	while (1) {
		/* random number between 0 and xres */
		x1 = (int)(rand() % wx);	// left
		x2 = (int)(rand() % wx);	// width

		/* random number between 0 and yres */
		y1 = (int)(rand() % hy);	// top
		y2 = (int)(rand() % hy);	// height

		/* swap */
		if (x1 > x2) {
			x1 = x1 + x2;
			x2 = x1 - x2;
			x1 = x1 - x2;
		}

		/* swap */
		if (y1 > y2) {
			y1 = y1 + y2;
			y2 = y1 - y2;
			y1 = y1 - y2;
		}

		x1 += sx;
		y1 += sy;
		x2 += sx;
		y2 += sy;

#if 1
		/* random number between 0 and 65535(2^16-1) */
		color = (int)rand();
#else
		color = (R<<16 | G<<8 | B);
#endif
		// usleep(1);

		for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			putpixel((u32)fbbase, x, y, fbvar.xres, fbvar.yres, color);
	}

	//---------------------------------------------------------------------
__exit:
	munmap(fbbase, length);

	close(fd);

	return 0;
}
