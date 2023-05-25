//-------------------------------------------------------------------
// Copyright SAMSUNG Electronics Co., Ltd
// All right reserved.
//
// This software is the confidential and proprietary information
// of Samsung Electronics, Inc. ("Confidential Information").  You
// shall not disclose such Confidential Information and shall use
// it only in accordance with the terms of the license agreement
// you entered into with Samsung Electronics.
//-------------------------------------------------------------------
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define FALSE    0
#define TRUE     1

#define CEC_DEVICE_NAME        "/dev/hdmi-cec"
#define CEC_MAX_FRAME_SIZE    16

#ifndef _IO
#define	TYPE_SHIFT		8
#define _IO(type,nr)	((type<<TYPE_SHIFT) | nr)
#define _IOC_TYPE(nr)	((nr>>TYPE_SHIFT) & 0xFF)
#define _IOC_NR(nr)		(nr & 0xFF)
#endif

#define	IOC_NX_MAGIC	0x6e78	/* "nx" */

enum {
    IOCTL_HDMI_CEC_SETLADDR    =  _IOW(IOC_NX_MAGIC, 1, unsigned int),
    IOCTL_HDMI_CEC_GETPADDR    =  _IOR(IOC_NX_MAGIC, 2, unsigned int),
};

static int running = TRUE;

static void sighandler(int arg)
{
    (void) arg;

    printf("%s\n", __FUNCTION__);
    running = FALSE;
}

int main(void)
{
    int fd;

    signal(SIGINT, sighandler);

    setbuf(stdout, NULL);

    printf("CEC device driver test program\n");

    printf("opening %s...\n", CEC_DEVICE_NAME);
    if ((fd = open(CEC_DEVICE_NAME, O_RDWR)) < 0)
    {
        printf("open() failed!\n");
        return 1;
    }

#if 1
    unsigned int laddr = 0x04;
    if (ioctl(fd, IOCTL_HDMI_CEC_SETLADDR, &laddr))
    {
        printf("ioctl(CEC_IOC_SETLA) failed!\n");
    }
#endif

#if 1
    printf("Discovering CEC network...\n");

    int i;
    for (i=0; i<=0x0E; i++)
    {
        // header block
        unsigned char message = ((i & 0x0F) << 4) | (i);

        printf("polling \"%d\" - ", i);

        if (write(fd, &message, sizeof(message)) != sizeof(message))
        {
            printf("no responce\n");
            printf("write() failed!\n");
        }
        else
        {
            printf("in use\n");
        }
    }

#endif

#if 1
    printf("writing...\n");

    // physical address
    unsigned char addr = 4;

    // "polling message"
    // frame contains only one byte - header block
    // addresses of src and dst are the same
    unsigned char header = ((addr & 0x0F) << 4) | (addr & 0x0F);

    if (write(fd, &header, sizeof(header)) != sizeof(header))
    {
        printf("write() failed!\n");
    }
#endif

#if 1
    printf("reading...\n");
    unsigned char *buffer;
    ssize_t bytes;
    int retval;

    buffer = malloc(CEC_MAX_FRAME_SIZE);

    fd_set rfds;
    struct timeval tv;

    while (running) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100 ms

        retval = select(fd + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");
            break;
        } else if (retval) {

            bytes = read(fd, buffer, CEC_MAX_FRAME_SIZE);

            if (bytes < 0) {
                printf("read() failed!\n");
            } else {
                int index;
                printf("fsize: %d\n", bytes);
                printf("frame: ");
                for (index = 0; index < bytes; index++) {
                    printf("0x%02x ", buffer[index]);
                }
                printf("\n");
            }
        }

    }
#endif

    printf("closing...\n");
    if (close(fd) != 0)
    {
        printf("close() failed!\n");
        return 1;
    }

    return 0;
}
