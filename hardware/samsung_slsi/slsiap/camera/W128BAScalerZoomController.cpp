#define LOG_TAG "W128BAScalerZoomController"

#include <sys/types.h>
#include <sys/stat.h>

#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>

#include <utils/Log.h>

#include <ion/ion.h>
#include <ion-private.h>

#include "W128BAScalerZoomController.h"

namespace android {

W128BAScalerZoomController::W128BAScalerZoomController()
    :NXZoomController(),
     DstWidth(0),
     DstHeight(0),
     VirtualBaseWidth(0),
     VirtualBaseHeight(0),
     VirtualLeft(0),
     VirtualTop(0),
     VirtualWidth(0),
     VirtualHeight(0)
{
    memset(&ZoomContext, 0, sizeof(struct scale_ctx));
}

W128BAScalerZoomController::W128BAScalerZoomController(
        int cropLeft,
        int cropTop,
        int cropWidth,
        int cropHeight,
        int baseWidth,
        int baseHeight,
        int dstWidth,
        int dstHeight
        )
    :NXZoomController(),
     DstWidth(dstWidth),
     DstHeight(dstHeight),
     VirtualBaseWidth(baseWidth),
     VirtualBaseHeight(baseHeight),
     VirtualLeft(0),
     VirtualTop(0),
     VirtualWidth(0),
     VirtualHeight(0)
{
    memset(&ZoomContext, 0, sizeof(struct scale_ctx));
    setCrop(cropLeft, cropTop, cropWidth, cropHeight);
}

W128BAScalerZoomController::~W128BAScalerZoomController()
{
}

void W128BAScalerZoomController::useDefault()
{
    Mutex::Autolock l(ZoomLock);
    ZoomContext.left = VirtualLeft;
    ZoomContext.top  = VirtualTop;
    ZoomContext.src_width  = VirtualBaseWidth;
    ZoomContext.src_height = VirtualBaseHeight;
    ZoomContext.dst_width  = DstWidth;
    ZoomContext.dst_height = DstHeight;
}

bool W128BAScalerZoomController::handleZoom(struct nxp_vid_buffer *srcBuffer, private_handle_t const *dstHandle)
{
    Mutex::Autolock l(ZoomLock);
    if (nxScalerRun(srcBuffer, dstHandle, &ZoomContext)) {
        ALOGE("failed to nxScalerRun()");
        return false;
    }
    return true;
}

bool W128BAScalerZoomController::handleZoom(struct nxp_vid_buffer *srcBuffer, struct nxp_vid_buffer *dstBuffer)
{
    Mutex::Autolock l(ZoomLock);
    if (nxScalerRun(srcBuffer, dstBuffer, &ZoomContext)) {
        ALOGE("failed to nxScalerRun()");
        return false;
    }
    return true;
}

bool W128BAScalerZoomController::handleZoom(private_handle_t const *srcHandle, struct nxp_vid_buffer *dstBuffer)
{
    Mutex::Autolock l(ZoomLock);
    if (nxScalerRun(srcHandle, dstBuffer, &ZoomContext)) {
        ALOGE("failed to nxScalerRun()");
        return false;
    }
    return true;
}

bool W128BAScalerZoomController::handleZoom(private_handle_t const *srcHandle, private_handle_t const *dstHandle)
{
    Mutex::Autolock l(ZoomLock);
    if (nxScalerRun(srcHandle, dstHandle, &ZoomContext)) {
        ALOGE("failed to nxScalerRun()");
        return false;
    }
    return true;
}

void W128BAScalerZoomController::calcZoomContext(void)
{
    if (!SrcWidth || !SrcHeight || !VirtualBaseWidth || !VirtualBaseHeight) {
        ALOGE("invalid wxh(%dx%d), base wxh(%dx%d)", SrcWidth, SrcHeight, VirtualBaseWidth, VirtualBaseHeight);
        return;
    }

    int calcLeft, calcTop, calcWidth, calcHeight;

    if (VirtualBaseWidth == VirtualWidth) {
        calcLeft = 0;
        calcWidth = SrcWidth;
    } else {
        calcLeft  = VirtualLeft * SrcWidth / VirtualBaseWidth;
        calcWidth = VirtualWidth * SrcWidth / VirtualBaseWidth;
    }

    if (VirtualBaseHeight == VirtualHeight) {
        calcTop = 0;
        calcHeight = SrcHeight;
    } else {
        calcTop = VirtualTop * SrcHeight / VirtualBaseHeight;
        calcHeight = VirtualHeight * SrcHeight / VirtualBaseHeight;
    }

    // align 32pixel for cb,cr 16pixel align
    calcLeft = (calcLeft + 31) & (~31);

    // check!
    if ((calcLeft + calcWidth) > SrcWidth)
        calcWidth = SrcWidth - calcLeft;
    if ((calcTop + calcHeight) > SrcHeight)
        calcHeight = SrcHeight - calcTop;

    //calcLeft >>= 1;
    //calcLeft <<= 1;
    //calcWidth >>= 1;
    //calcWidth <<= 1;
    //calcHeight >>= 1;
    //calcHeight <<= 1;

    Mutex::Autolock l(ZoomLock);
    ZoomContext.left = calcLeft;
    ZoomContext.top  = calcTop;
    ZoomContext.src_width  = calcWidth;
    ZoomContext.src_height = calcHeight;
    ZoomContext.dst_width  = DstWidth;
    ZoomContext.dst_height = DstHeight;

    ALOGV("Width %d, DstHeight %d, VirtualBaseWidth %d, VirtualBaseHeight %d, left %d, top %d, src_width %d, src_height %d, dst_width %d, dst_height %d, src_code 0x%x, dst_code 0x%x",
            DstWidth, DstHeight, VirtualBaseWidth, VirtualBaseHeight, calcLeft, calcTop, calcWidth, calcHeight, DstWidth, DstHeight, ZoomContext.src_code, ZoomContext.dst_code);
}

void W128BAScalerZoomController::setSource(int srcWidth, int srcHeight)
{
    if (srcWidth != SrcWidth || srcHeight != SrcHeight) {
        SrcWidth = srcWidth;
        SrcHeight = srcHeight;
        calcZoomContext();
    }
}

void W128BAScalerZoomController::setCrop(int left, int top, int width, int height)
{
    if (VirtualLeft != left || VirtualTop != top || VirtualWidth != width || VirtualHeight != height) {
        //ALOGD("%s: src(%d:%d-%d:%d), arg(%d:%d-%d:%d)", __func__, VirtualLeft, VirtualTop, VirtualWidth, VirtualHeight, left, top, width, height);
        VirtualLeft = left;
        VirtualTop = top;
        VirtualWidth = width;
        VirtualHeight = height;
        calcZoomContext();
    }
}

}; // namespace
