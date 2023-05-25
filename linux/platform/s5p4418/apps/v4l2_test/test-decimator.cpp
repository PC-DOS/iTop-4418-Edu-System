#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <ion.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/videodev2_nxp_media.h>

#include <nxp-v4l2.h>

#include "common.cpp"

// # ./test module w h
int main(int argc, char *argv[])
{
    int ion_fd = ion_open();
    int width;
    int height;
    int format = V4L2_PIX_FMT_YUV420M;
    int decimator_id = nxp_v4l2_decimator0;
    int sensor_id = nxp_v4l2_sensor0;
    int video_id = nxp_v4l2_mlc0_video;
    int module = 0;

    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return -EINVAL;
    }

    if (argc >= 3) {
        module = atoi(argv[1]);
        width = atoi(argv[2]);
        height = atoi(argv[3]);
    } else {
        printf("usage: ./test module width height\n");
        return 0;
    }
    printf("module %d, width %d, height %d\n", module, width, height);

    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));

    if (module == 0) {
        s.useDecimator0 = true;
        decimator_id = nxp_v4l2_decimator0;
        sensor_id = nxp_v4l2_sensor0;
    } else {
        s.useDecimator1 = true;
        decimator_id = nxp_v4l2_decimator1;
        sensor_id = nxp_v4l2_sensor1;
    }
    s.useMlc0Video = true;

    CHECK_COMMAND(v4l2_init(&s));
    CHECK_COMMAND(v4l2_set_format(decimator_id, width, height, format));
    CHECK_COMMAND(v4l2_set_crop(decimator_id, 0, 0, width, height));
    CHECK_COMMAND(v4l2_set_format(sensor_id, width, height, V4L2_MBUS_FMT_YUYV8_2X8));
    CHECK_COMMAND(v4l2_set_format(video_id, width, height, format));

    CHECK_COMMAND(v4l2_set_crop(video_id, 0, 0, width, height));

    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_PRIORITY, 0));
    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_COLORKEY, 0x0));
    CHECK_COMMAND(v4l2_reqbuf(decimator_id, MAX_BUFFER_COUNT));
    CHECK_COMMAND(v4l2_reqbuf(video_id, MAX_BUFFER_COUNT));

    printf("alloc video\n");
    struct nxp_vid_buffer bufs[MAX_BUFFER_COUNT];
    CHECK_COMMAND(alloc_buffers(ion_fd, MAX_BUFFER_COUNT, bufs, width, height, format));
    printf("vid_buf: %p, %p, %p, %p\n", bufs[0].virt[0], bufs[1].virt[0], bufs[2].virt[0], bufs[3].virt[0]);

    int i;
    for (i = 0; i < MAX_BUFFER_COUNT; i++) {
        struct nxp_vid_buffer *buf = &bufs[i];
        printf("buf plane num: %d\n", buf->plane_num);
        CHECK_COMMAND(v4l2_qbuf(decimator_id, buf->plane_num, i, buf, -1, NULL));
    }

    CHECK_COMMAND(v4l2_streamon(decimator_id));

    int out_index = 0;
    int out_dq_index = 0;
    int out_q_count = 0;
    bool started_out = false;
    int capture_index = 0;
    int count = 1000;
    if (argc >= 5) {
        count = atoi(argv[4]);
    }
    while (count >= 0) {
        struct nxp_vid_buffer *buf = &bufs[capture_index];
        CHECK_COMMAND(v4l2_dqbuf(decimator_id, buf->plane_num, &capture_index, NULL));
        CHECK_COMMAND(v4l2_qbuf(video_id, buf->plane_num, out_index, buf, -1, NULL));

        out_q_count++;
        out_index++;
        out_index %= MAX_BUFFER_COUNT;

        if (!started_out) {
            CHECK_COMMAND(v4l2_streamon(video_id));
            started_out = true;
        }

        if (out_q_count >= MAX_BUFFER_COUNT) {
            CHECK_COMMAND(v4l2_dqbuf(video_id, buf->plane_num, &out_dq_index, NULL));
            out_q_count--;
        }

        CHECK_COMMAND(v4l2_qbuf(decimator_id, buf->plane_num, capture_index, buf, -1, NULL));
        count--;
    }

    v4l2_streamoff(video_id);
    v4l2_streamoff(decimator_id);

    v4l2_exit();
    close(ion_fd);

    return 0;
}
