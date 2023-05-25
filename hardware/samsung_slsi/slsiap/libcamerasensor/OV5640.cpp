#include <linux/videodev2.h>
#include <nxp-v4l2.h>
#include "OV5640.h"

enum {
    WB_INCANDESCENT = 0,
    WB_FLUORESCENT,
    WB_DAYLIGHT,
    WB_CLOUDY,
    WB_TUNGSTEN,
    WB_AUTO,
    WB_MAX
};

enum {
    COLORFX_NONE = 0,
    COLORFX_MONO,
    COLORFX_SEPIA,
    COLORFX_NEGATIVE,
    COLORFX_MAX
};

#define MIN_EXPOSURE     -3
#define MAX_EXPOSURE     3

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

namespace android {

const int32_t ResolutionOV5640[] = {
 
    1280,960,

    640, 480,
};

// TOP/system/media/camera/include/system/camera_metadata_tags.h
static const uint8_t AvailableAfModesOV5640[] = {
    ANDROID_CONTROL_AF_MODE_OFF,
    ANDROID_CONTROL_AF_MODE_AUTO,
    ANDROID_CONTROL_AF_MODE_MACRO,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO
};

static const uint8_t AvailableAeModesOV5640[] = {
    ANDROID_CONTROL_AE_MODE_OFF,
    ANDROID_CONTROL_AE_MODE_ON,
    ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH
};

static const uint8_t SceneModeOverridesOV5640[] = {
    // ANDROID_CONTROL_SCENE_MODE_PORTRAIT
    ANDROID_CONTROL_AE_MODE_ON,
    ANDROID_CONTROL_AWB_MODE_AUTO,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
    // ANDROID_CONTROL_SCENE_MODE_LANDSCAPE
    ANDROID_CONTROL_AE_MODE_ON,
    ANDROID_CONTROL_AWB_MODE_AUTO,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
    // ANDROID_CONTROL_SCENE_MODE_SPORTS
    ANDROID_CONTROL_AE_MODE_ON,
    ANDROID_CONTROL_AWB_MODE_DAYLIGHT,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
#if 0
    // ANDROID_CONTROL_SCENE_MODE_NIGHT
    ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH,
    ANDROID_CONTROL_AWB_MODE_AUTO,
    ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE
#endif
};

static const uint8_t AvailableEffectsOV5640[] = {
    ANDROID_CONTROL_EFFECT_MODE_MONO,
    ANDROID_CONTROL_EFFECT_MODE_NEGATIVE,
    ANDROID_CONTROL_EFFECT_MODE_SEPIA,
    ANDROID_CONTROL_EFFECT_MODE_AQUA
};

static const uint8_t AvailableSceneModesOV5640[] = {
    ANDROID_CONTROL_SCENE_MODE_PORTRAIT,
    ANDROID_CONTROL_SCENE_MODE_LANDSCAPE,
    ANDROID_CONTROL_SCENE_MODE_SPORTS,
#if 0
    ANDROID_CONTROL_SCENE_MODE_NIGHT
#endif
};

static const uint32_t ExposureCompensationRangeOV5640[] = {
     MIN_EXPOSURE, MAX_EXPOSURE
};

static const uint8_t AvailableAntibandingModesOV5640[] = {
    ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF,
    ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ,
    ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ
};

static const uint8_t AvailableAwbModesOV5640[] = {
    ANDROID_CONTROL_AWB_MODE_OFF,
    ANDROID_CONTROL_AWB_MODE_AUTO,
    ANDROID_CONTROL_AWB_MODE_DAYLIGHT,
    ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT,
    ANDROID_CONTROL_AWB_MODE_FLUORESCENT,
    ANDROID_CONTROL_AWB_MODE_INCANDESCENT
};

static const int32_t AvailableFpsRangesOV5640[] = {
    8, 15
    //8, 15, 30
    //15, 30
    //8, 8, 15, 15, 24, 24, 15, 30
};

void OV5640::init()
{
    // TODO
    Width = ResolutionOV5640[0];
    Height = ResolutionOV5640[1];
    NumResolutions = ARRAY_SIZE(ResolutionOV5640)/2;
    Resolutions = ResolutionOV5640;
    NumAvailableAfModes = ARRAY_SIZE(AvailableAfModesOV5640);
    AvailableAfModes = AvailableAfModesOV5640;
    NumAvailableAeModes = ARRAY_SIZE(AvailableAeModesOV5640);
    AvailableAeModes = AvailableAeModesOV5640;
    NumSceneModeOverrides = ARRAY_SIZE(SceneModeOverridesOV5640);
    SceneModeOverrides = SceneModeOverridesOV5640;
    ExposureCompensationRange = ExposureCompensationRangeOV5640;
    AvailableAntibandingModes = AvailableAntibandingModesOV5640;
    NumAvailAntibandingModes = ARRAY_SIZE(AvailableAntibandingModesOV5640);
    AvailableAwbModes = AvailableAwbModesOV5640;
    NumAvailAwbModes = ARRAY_SIZE(AvailableAwbModesOV5640);
    AvailableEffects = AvailableEffectsOV5640;
    NumAvailEffects = ARRAY_SIZE(AvailableEffectsOV5640);
    AvailableSceneModes = AvailableSceneModesOV5640;
    NumAvailSceneModes = ARRAY_SIZE(AvailableSceneModesOV5640);
    AvailableFpsRanges = AvailableFpsRangesOV5640;
    NumAvailableFpsRanges = ARRAY_SIZE(AvailableFpsRangesOV5640);

    // TODO
    FocalLength = 3.43f;
    Aperture = 2.7f;
    MinFocusDistance = 0.1f;
    FNumber = 2.7f;

    // TODO
    //MaxFaceCount = 16;
    MaxFaceCount = 1;

    // Crop
    CropWidth = Width;
    CropHeight = Height;
    CropLeft = 0;
    CropTop = 0;
}

OV5640::OV5640()
{
    init();
}

OV5640::OV5640(uint32_t v4l2ID)
    : NXCameraBoardSensor(v4l2ID)
{
    init();
}

OV5640::~OV5640()
{
}

void OV5640::setAfMode(uint8_t afMode)
{
    AfMode = afMode;
}

void OV5640::afEnable(bool enable)
{

    uint32_t val = 0;

    if (enable) {
        switch (AfMode) {
            case ANDROID_CONTROL_AF_MODE_OFF:
                val = 0; // init
                break;
            case ANDROID_CONTROL_AF_MODE_AUTO:
                val = 1; // auto
                break;
            case ANDROID_CONTROL_AF_MODE_MACRO:
                val = 1; // zoom macro
                break;
            case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                val = 1; // zoom fixed
                break;
            case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                val = 1; // zoom infinity
                break;
            case ANDROID_CONTROL_AF_MODE_EDOF:
                val = 1; // init
                break;
        }
    } else
    {
        val = 0; // init
    }
    v4l2_set_ctrl(V4l2ID, V4L2_CID_FOCUS_AUTO, val);

    ALOGD("%s , val=%d",__func__,val);

}


void OV5640::setEffectMode(uint8_t effectMode)
{
    if (effectMode != EffectMode) {
        uint32_t val = 0;

        switch (effectMode) {
        case ANDROID_CONTROL_EFFECT_MODE_OFF:
            val = COLORFX_NONE;
            break;
        case ANDROID_CONTROL_EFFECT_MODE_MONO:
            val = COLORFX_MONO;
            break;
        case ANDROID_CONTROL_EFFECT_MODE_NEGATIVE:
            val = COLORFX_NEGATIVE;
            break;
        case ANDROID_CONTROL_EFFECT_MODE_SEPIA:
            val = COLORFX_SEPIA;
            break;
        default:
            //ALOGE("%s: unsupported effectmode 0x%x", __func__, effectMode);
            return;
        }

        v4l2_set_ctrl(V4l2ID, V4L2_CID_COLORFX, val);
        EffectMode = effectMode;
    }
}

void OV5640::setSceneMode(uint8_t sceneMode)
{
}

void OV5640::setAntibandingMode(uint8_t antibandingMode)
{
}

void OV5640::setAwbMode(uint8_t awbMode)
{
    if (awbMode != AwbMode) {
        uint32_t val = 0;

        switch (awbMode) {
        case ANDROID_CONTROL_AWB_MODE_OFF:
            v4l2_set_ctrl(V4l2ID, V4L2_CID_AUTO_WHITE_BALANCE, 0);
            return;
        case ANDROID_CONTROL_AWB_MODE_AUTO:
            v4l2_set_ctrl(V4l2ID, V4L2_CID_AUTO_WHITE_BALANCE, 1);
            return;
        case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
            val = WB_DAYLIGHT;
            break;
        case ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
            val = WB_DAYLIGHT;
            break;
        case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
            val = WB_FLUORESCENT;
            break;
        case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
            val = WB_INCANDESCENT;
            break;
        default:
            //ALOGE("%s: unsupported awb mode 0x%x", __func__, awbMode);
            return;
        }
        AwbMode = awbMode;

        v4l2_set_ctrl(V4l2ID, V4L2_CID_WHITE_BALANCE_TEMPERATURE, val);
    }
}

void OV5640::setExposure(int32_t exposure)
{
    if (exposure < MIN_EXPOSURE || exposure > MAX_EXPOSURE) {
        ALOGE("%s: invalid exposure %d", __func__, exposure);
        return;
    }

    if (exposure != Exposure) {
        Exposure = exposure;
        v4l2_set_ctrl(V4l2ID, V4L2_CID_BRIGHTNESS, exposure + 3);
    }
}

uint32_t OV5640::getZoomFactor(void)
{
    // disable zoom
    return 1;
}

status_t OV5640::setZoomCrop(uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    return NO_ERROR;
}

}; // namespace
