#ifndef __NX_OMXVideoDecoderFFMpeg_h__
#define __NX_OMXVideoDecoderFFMpeg_h__

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <NX_OMXBasePort.h>
#include <NX_OMXBaseComponent.h>
#include <NX_OMXSemaphore.h>
#include <NX_OMXQueue.h>

#include <hardware/gralloc.h>
#include <media/hardware/MetadataBufferType.h>

#include <nx_video_api.h>

#ifdef NX_DYNAMIC_COMPONENTS
//	This Function need for dynamic registration
OMX_ERRORTYPE OMX_ComponentInit (OMX_HANDLETYPE hComponent);
#else
//	static registration
OMX_ERRORTYPE NX_VidDecComponentInit (OMX_HANDLETYPE hComponent);
#endif


#define	VIDDEC_NUM_PORT			2
#define	VIDDEC_INPORT_INDEX		0
#define	VIDDEC_OUTPORT_INDEX	1

#define	NX_OMX_MAX_BUF			128

#define	VIDENC_INPORT_MIN_BUF_CNT	6
#define	VIDENC_INPORT_MIN_BUF_SIZE	(1920*1088*4)

#define	VIDENC_OUTPORT_MIN_BUF_CNT	8
#define	VIDENC_OUTPORT_MIN_BUF_SIZE	(4*1024*1024)

#define	VIDENC_DEF_FRAMERATE		(30)
#define	VIDENC_DEF_BITRATE			(3*1024*1024)

typedef struct _NX_VINPUT_INFO NX_VINPUT_INFO;
struct _NX_VINPUT_INFO{
	uint32_t key;
	uint32_t flag;
	int64_t timestamp;
};

//	Define Video Decoder Component Type
typedef struct tNX_VIDENC_COMP_TYPE{
	NX_BASECOMPONENTTYPE		//	Nexell Base Component Type	
	/*					Buffer Thread							*/
	pthread_t				hBufThread;
	pthread_mutex_t			hBufMutex;
	NX_THREAD_CMD			eCmdBufThread;
	NX_SEMAPHORE			*hBufAllocSem;						//	Buffer allocation semaphore ( Semaphore )
	NX_SEMAPHORE			*hBufCtrlSem;						//	Buffer thread control semaphore ( Mutex )
	NX_SEMAPHORE			*hBufChangeSem;						//	Buffer status change semaphore ( Event )
	/*					Port Information						*/
	NX_BASEPORTTYPE			*pInputPort;						//	Input Port
	NX_BASEPORTTYPE			*pOutputPort;						//	Output Port
	OMX_BUFFERHEADERTYPE	*pInputBuffers[NX_OMX_MAX_BUF];		//	Input Buffers
	OMX_BUFFERHEADERTYPE	*pOutputBuffers[NX_OMX_MAX_BUF];	//	Output Buffers
	NX_QUEUE				*pInputPortQueue;
	NX_QUEUE				*pOutputPortQueue;

	//		Video Format
	OMX_VIDEO_PARAM_PORTFORMATTYPE	inputFormat;
	OMX_VIDEO_PARAM_PORTFORMATTYPE	outputFormat;

	//
	OMX_BOOL					bSendCodecSpecificInfo;
	OMX_VIDEO_PARAM_MPEG4TYPE	omxMp4EncParam;
	OMX_VIDEO_PARAM_AVCTYPE		omxAVCEncParam;

	//	Android Native Buffer Flasg
	OMX_BOOL					bUseNativeBuffer;
	OMX_BOOL					bMetaDataInBuffers;

	//	Encoder parameters
	uint32_t					encWidth;
	uint32_t					encHeight;
	uint32_t					encKeyInterval;		//	GOP
	uint32_t					encFrameRate;
	uint32_t					encBitRate;
	uint32_t					encIntraRefreshMbs;

	OMX_BUFFERHEADERTYPE		*pPrevInputBuffer;

	//	Sequence Data Buffer
	OMX_U8						pSeqBuf[1024];
	int32_t						seqBufSize;
	OMX_BOOL					bCodecSpecificInfo;

	//	Color Space Converter Destination Buffer
	NX_VID_MEMORY_HANDLE		hCSCMem;

	//	Encoder Handle
	NX_VID_ENC_HANDLE			hVpuCodec;
	OMX_S32						vpuCodecId;

	OMX_BYTE					pPictureBuf;

}NX_VIDENC_COMP_TYPE;

#endif	//	__NX_OMXVideoDecoderFFMpeg_h__
