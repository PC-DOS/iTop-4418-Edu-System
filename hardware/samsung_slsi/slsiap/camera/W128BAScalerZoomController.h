#ifndef _W128BA_SCALER_ZOOM_CONTROLLER_H
#define _W128BA_SCALER_ZOOM_CONTROLLER_H

#include <utils/Thread.h>
#include <NXScaler.h>
#include "NXZoomController.h"

namespace android {

class W128BAScalerZoomController: public NXZoomController
{
public:
    W128BAScalerZoomController();
    W128BAScalerZoomController(int cropLeft, int cropTop, int cropWidth, int cropHeight, int baseWidth, int baseHeight, int dstWidth, int dstHeight);
    virtual ~W128BAScalerZoomController();

    // overriding
    virtual void setBase(int baseWidth, int baseHeight) {
        VirtualBaseWidth  = baseWidth;
        VirtualBaseHeight = baseHeight;
        ALOGV("base: %d%d", VirtualBaseWidth, VirtualBaseHeight);
    }
    virtual void getBase(int &baseWidth, int &baseHeight) {
        baseWidth = VirtualBaseWidth;
        baseHeight = VirtualBaseHeight;
    }
#if 0
    virtual void setSource(int width, int height) {
        VirtualWidth = width;
        VirtualHeight = height;
    }
#else
    virtual void setSource(int width, int height);
#endif
    virtual void SetDest(int width, int height) {
        DstWidth = width;
        DstHeight = height;
    }
    virtual void setFormat(int srcFormat, int dstFormat) {
        ZoomContext.src_code = srcFormat;
        ZoomContext.dst_code = dstFormat;
    }
    virtual void setCrop(int left, int top, int width, int height);
    virtual void useDefault();
    virtual bool handleZoom(struct nxp_vid_buffer *srcBuffer, private_handle_t const *dstHandle);
    virtual bool handleZoom(struct nxp_vid_buffer *srcBuffer, struct nxp_vid_buffer *dstBuffer);
    virtual bool handleZoom(private_handle_t const *srcHandle, struct nxp_vid_buffer *dstBuffer);
    virtual bool handleZoom(private_handle_t const *srcHandle, private_handle_t const *dstHandle);
    virtual bool isZoomAvaliable() {
        return ZoomContext.left != 0 ||
               ZoomContext.top  != 0;
    }

private:
    // function
    void calcZoomContext(void);

private:
    // variable
    int DstWidth;
    int DstHeight;

    int VirtualBaseWidth;
    int VirtualBaseHeight;
    int VirtualLeft;
    int VirtualTop;
    int VirtualWidth;
    int VirtualHeight;

    int SrcWidth;
    int SrcHeight;

    struct scale_ctx ZoomContext;
    Mutex ZoomLock;
};

}; // namespace

#endif
