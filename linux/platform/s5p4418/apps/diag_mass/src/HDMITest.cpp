#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <HDMITest.h>
#include <utils.h>

#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <ion.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/videodev2_nxp_media.h>


#include <nxp-v4l2-media.h>

#define DEFAULT_WIDTH  1920
#define DEFAULT_HEIGHT 1080

#define MAX_BUFFER_COUNT 4

#define COLOR_RED 0xffff0000
#define COLOR_GREEN 0xff00ff00
#define COLOR_BLUE  0xff0000ff

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0xff000000
#define COLOR_WB    0xffaaaaaa

#define COLOR_OTHER1    0xffaa00cc
#define COLOR_OTHER2    0xff00ccaa
#define COLOR_OTHER3    0xffaabb00

#define COLOR_OTHER4    0xff1980ff
#define COLOR_OTHER5    0xffff77ff
#define COLOR_OTHER6    0xff11ff22


#define CHECK_COMMAND(command) do { \
        int ret = command; \
        if (ret < 0) { \
            fprintf(stderr, "line %d error!!!\n", __LINE__); \
            return ret; \
        } \
    } while (0)


#define HDMI_STATE_FILE     "/sys/class/switch/hdmi/state"

static struct nxp_vid_buffer rgb_bufs[MAX_BUFFER_COUNT];
static V4L2_PRIVATE_HANDLE	hPrivate;

CHDMITest::CHDMITest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CHDMITest::~CHDMITest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CHDMITest::Start()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		return false;
	}

	m_bRunning = true;
	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}
	return true;
}

bool CHDMITest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}


static unsigned int get_size(int format, int num, int width, int height)
{
    int size;

    switch (format) {
    case V4L2_PIX_FMT_YUYV:
        if (num > 0) return 0;
        size = (width * height) * 2;
        break;
    case V4L2_PIX_FMT_YUV420M:
        if (num == 0) {
            size = width * height;
        } else {
            size = (width * height) >> 2;
        }
        break;
    case V4L2_PIX_FMT_YUV422P:
        if (num == 0) {
            size = width * height;
        } else {
            size = (width * height) >> 1;
        }
        break;
    case V4L2_PIX_FMT_YUV444:
        size = width * height;
        break;
    case V4L2_PIX_FMT_RGB32:
        size = width * height * 4;
        break;
    default:
        size = width * height * 2;
        break;
    }

    return size;
}

static int alloc_buffers(int ion_fd, int count, struct nxp_vid_buffer *bufs, int width, int height, int format)
{
    int ret;
    int size;
    int i, j;
    int y_size = width * height;
    int cb_size;
    int cr_size;
    struct nxp_vid_buffer *buffer;
    int plane_num;

    if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_RGB565 || format == V4L2_PIX_FMT_RGB32) {
        plane_num = 1;
    } else {
        plane_num = 3;
    }

    printf("Allocation Buffer: count(%d), plane(%d)\n", count, plane_num);
    for (i = 0; i < count; i++) {
        buffer = &bufs[i];
        printf("[Buffer %d] --->\n", i);
        for (j = 0; j < plane_num; j++) {
            buffer->sizes[j] = get_size(format, j, width, height);
            printf("====> sizes: %d\n", buffer->sizes[j]);
            ret = ion_alloc_fd(ion_fd, buffer->sizes[j], 0, ION_HEAP_NXP_CONTIG_MASK, 0, &buffer->fds[j]);
            if (ret < 0) {
                fprintf(stderr, "failed to ion_alloc_fd()\n");
                return ret;
            }
            buffer->virt[j] = (char *)mmap(NULL, buffer->sizes[j], PROT_READ | PROT_WRITE, MAP_SHARED, buffer->fds[j], 0);
            if (!buffer->virt[j]) {
                fprintf(stderr, "failed to mmap\n");
                return ret;
            }
            ret = ion_get_phys(ion_fd, buffer->fds[j], &buffer->phys[j]);
            if (ret < 0) {
                fprintf(stderr, "failed to get phys\n");
                return ret;
            }
            buffer->plane_num = plane_num;
            printf("\tplane %d: fd(%d), size(%d), phys(0x%x), virt(0x%x)\n",
                    j, buffer->fds[j], buffer->sizes[j], buffer->phys[j], buffer->virt[j]);
        }
    }

    return 0;
}

static bool is_preset_support(int id, uint32_t preset)
{

    int i = 0;
    int ret;
    int fd = v4l2_get_device_fd(hPrivate, id);
    if (fd < 0) {
        fprintf(stderr, "%s: can't get fd for %d\n", __func__, id);
        return false;
    }
    struct v4l2_dv_enum_preset enum_preset;
    do {
        memset(&enum_preset, 0, sizeof(struct v4l2_dv_enum_preset));
        enum_preset.index = i;
        ret = ioctl(fd, VIDIOC_ENUM_DV_PRESETS, &enum_preset);
        i++;
        if (ret < 0)
            break;
        if (enum_preset.preset == preset)
            return true;
    } while (1);

    return false;
}

static void draw_color_bar(uint32_t *buf, int width, int height, uint32_t color1, uint32_t color2, uint32_t color3)
{
    int i, j;
    int width_1_3 = width / 3;
    uint32_t *p = buf;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (j < width_1_3) {
                *p = color1;
            } else if (j < width_1_3 * 2) {
                *p = color2;
            } else {
                *p = color3;
            }
            p++;
        }
    }
}

int CHDMITest::HDMIOpen()
{
    int width;
    int height;

    uint32_t preset;

    int format = V4L2_PIX_FMT_YUV420M;

    ion_fd = ion_open();

    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return -EINVAL;
    }

    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));

//    s.useMlc0Video = true;
//    s.useMlc0Rgb = true;
    s.useMlc1Rgb = true;
    s.useMlc1Video = true;
    s.useHdmi = true;


    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;

    printf("width %d, height %d\n", width, height);


    hPrivate = v4l2_init(&s);
	if( NULL == (hPrivate = v4l2_init(&s)) ) 
	{
		printf("v4l2_init() failed!!!\n");
		return -1;
	}

    CHECK_COMMAND(v4l2_link(hPrivate, nxp_v4l2_mlc1, nxp_v4l2_hdmi));


    preset = V4L2_DV_1080P60;

    if (!is_preset_support(nxp_v4l2_hdmi, preset)) {
        fprintf(stderr, "preset %d is not supported\n", preset);
        return -1;
    }

    CHECK_COMMAND(v4l2_set_preset(hPrivate, nxp_v4l2_hdmi, preset));
    CHECK_COMMAND(v4l2_set_format(hPrivate, nxp_v4l2_mlc1_rgb, width, height, V4L2_PIX_FMT_RGB32));
    CHECK_COMMAND(v4l2_set_crop(hPrivate, nxp_v4l2_mlc1_rgb, 0, 0, width, height));
    CHECK_COMMAND(v4l2_reqbuf(hPrivate, nxp_v4l2_mlc1_rgb, MAX_BUFFER_COUNT));
    CHECK_COMMAND(alloc_buffers(ion_fd, MAX_BUFFER_COUNT, rgb_bufs, width, height, V4L2_PIX_FMT_RGB32));
    printf("rgb_buf: %p:%d, %p:%d, %p:%d, %p:%d\n",
            rgb_bufs[0].virt[0], rgb_bufs[0].sizes[0],
            rgb_bufs[1].virt[0], rgb_bufs[1].sizes[0],
            rgb_bufs[2].virt[0], rgb_bufs[2].sizes[0],
            rgb_bufs[3].virt[0], rgb_bufs[3].sizes[0]);
    draw_color_bar((uint32_t *)rgb_bufs[0].virt[0], width, height, COLOR_RED, COLOR_GREEN, COLOR_BLUE);
    draw_color_bar((uint32_t *)rgb_bufs[1].virt[0], width, height, COLOR_WHITE, COLOR_WB, COLOR_BLACK);
    draw_color_bar((uint32_t *)rgb_bufs[2].virt[0], width, height, COLOR_OTHER1, COLOR_OTHER2, COLOR_OTHER3);
    draw_color_bar((uint32_t *)rgb_bufs[3].virt[0], width, height, COLOR_OTHER4, COLOR_OTHER5, COLOR_OTHER6);

    started_out = false;
    out_q_count = 0;
    out_index = 0;

	return 0;
}

void CHDMITest::HDMIClose()
{
    v4l2_streamoff(hPrivate, nxp_v4l2_mlc1_rgb);
    v4l2_exit(hPrivate);
    close(ion_fd);
}


int CHDMITest::HDMIDisplay()
{
    int out_dq_index = 0;
    struct nxp_vid_buffer *rgb_buf;

    rgb_buf = &rgb_bufs[out_index];
    CHECK_COMMAND(v4l2_qbuf(hPrivate, nxp_v4l2_mlc1_rgb, 1, out_index, rgb_buf, -1, NULL));

    out_q_count++;
    out_index++;
    out_index %= MAX_BUFFER_COUNT;

    if (!started_out) {
        CHECK_COMMAND(v4l2_streamon(hPrivate, nxp_v4l2_mlc1_rgb));
        started_out = true;
    }

    if (out_q_count >= MAX_BUFFER_COUNT) {
        CHECK_COMMAND(v4l2_dqbuf(hPrivate, nxp_v4l2_mlc1_rgb, 1, &out_dq_index, NULL));
        out_q_count--;
    }
	return 0;
}


void CHDMITest::ThreadProc()
{
	int err;	
	int fd;
	struct pollfd fds[1];
	static unsigned char uevent_desc[2048];
	char val;

	uevent_init();
	fd = open(HDMI_STATE_FILE, O_RDONLY);
	if( fd > 0 )
	{
		if (read(fd, (char *)&val, 1) == 1 && val == '1')
		{
			m_bIsConnected = true;
			HDMIOpen();
		}
		else
		{
			m_bIsConnected = false;
		}
		close(fd);
	}

    fds[0].fd = uevent_get_fd();
    fds[0].events = POLLIN;
	while(m_bRunning)
	{
        err = poll(fds, 1, 500);

		if (err > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				uevent_next_event((char *)uevent_desc, sizeof(uevent_desc) - 2);
				int hdmi = !strcmp((const char *)uevent_desc, (const char *)"change@/devices/virtual/switch/hdmi");
				if (hdmi)
				{
					fd = open(HDMI_STATE_FILE, O_RDONLY);
					if (fd < 0)
					{
						printf("failed to open hdmi state fd: %s", HDMI_STATE_FILE);
					}
					if( fd > 0 )
					{
						if (read(fd, &val, 1) == 1 && val == '1') {
							m_bIsConnected = true;
							HDMIOpen();
							printf("Connected\n");
						}
						else
						{
							m_bIsConnected = false;
							HDMIClose();
							printf("Disconnected\n");
						}
						close(fd);
					}
				}
			}
		}
		else if (err == -1)
		{
			printf("error in vsync thread \n");
		}

		if (m_bIsConnected)
			HDMIDisplay();
    }
}
