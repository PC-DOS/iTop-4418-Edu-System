#define	LOG_TAG				"NX_AVCDEC"

#include <utils/Log.h>

#include <assert.h>
#include <OMX_AndroidTypes.h>
#include <system/graphics.h>

#include "NX_OMXVideoDecoder.h"
#include "NX_DecoderUtil.h"

//	From NX_AVCUtil
int avc_get_video_size(unsigned char *buf, int buf_size, int *width, int *height);

static int AVCCheckPortReconfiguration( NX_VIDDEC_VIDEO_COMP_TYPE *pDecComp, OMX_BYTE inBuf, OMX_S32 inSize )
{
	int w,h;	//	width, height, left, top, right, bottom

	if( ( inSize>4 && inBuf[0]==0 && inBuf[1]==0 && inBuf[2]==0 && inBuf[3]==1 && ((inBuf[4]&0x0F)==0x07) ) ||
		( inSize>4 && inBuf[0]==0 && inBuf[1]==0 && inBuf[2]==1 && ((inBuf[3]&0x0F)==0x07) ) )
	{
		if( avc_get_video_size( inBuf, inSize, &w, &h ) )
		{
			if( pDecComp->width != w || pDecComp->height != h )
			{
				DbgMsg("New Video Resolution = %ld x %ld --> %d x %d\n",
						pDecComp->width, pDecComp->height, w, h);

				//	Change Port Format & Resolution Information
				pDecComp->pOutputPort->stdPortDef.format.video.nFrameWidth  = pDecComp->width  = w;
				pDecComp->pOutputPort->stdPortDef.format.video.nFrameHeight = pDecComp->height = h;

				//	Native Mode
				if( pDecComp->bUseNativeBuffer )
				{
					pDecComp->pOutputPort->stdPortDef.nBufferSize = 4096;
				}
				else
				{
					pDecComp->pOutputPort->stdPortDef.nBufferSize = ((((w+15)>>4)<<4) * (((h+15)>>4)<<4))*3/2;
				}

				//	Need Port Reconfiguration
				SendEvent( pDecComp, OMX_EventPortSettingsChanged, OMX_DirOutput, 0, NULL );
				if( OMX_TRUE == pDecComp->bInitialized )
				{
					pDecComp->bInitialized = OMX_FALSE;
					InitVideoTimeStamp(pDecComp);
					closeVideoCodec(pDecComp);
					openVideoCodec(pDecComp);
				}
				pDecComp->pOutputPort->stdPortDef.bEnabled = OMX_FALSE;
			}
			else
			{
				DbgMsg("Video Resolution = %ld x %ld --> %d x %d\n", pDecComp->width, pDecComp->height, w, h);
			}
			return 1;
		}
	}
	return 0;
}

int NX_DecodeAvcFrame(NX_VIDDEC_VIDEO_COMP_TYPE *pDecComp, NX_QUEUE *pInQueue, NX_QUEUE *pOutQueue)
{
	OMX_BUFFERHEADERTYPE* pInBuf = NULL, *pOutBuf = NULL;
	int inSize = 0;
	OMX_BYTE inData;
	NX_VID_DEC_IN decIn;
	NX_VID_DEC_OUT decOut;
	int ret = 0;

	UNUSED_PARAM(pOutQueue);

	if( pDecComp->bFlush )
	{
		flushVideoCodec( pDecComp );
		pDecComp->bFlush = OMX_FALSE;
	}

	//	Get Next Queue Information
	NX_PopQueue( pInQueue, (void**)&pInBuf );
	if( pInBuf == NULL ){
		return 0;
	}

	inData = pInBuf->pBuffer;
	inSize = pInBuf->nFilledLen;
	pDecComp->inFrameCount++;

	TRACE("pInBuf->nFlags = 0x%08x, size = %ld\n", (int)pInBuf->nFlags, pInBuf->nFilledLen );


	//	Check End Of Stream
	if( pInBuf->nFlags & OMX_BUFFERFLAG_EOS )
	{
		DbgMsg("=========================> Receive Endof Stream Message (%ld)\n", pInBuf->nFilledLen);

		pDecComp->bStartEoS = OMX_TRUE;
		if( inSize <= 0)
		{
			goto Exit;
		}
	}

	//	Step 1. Found Sequence Information
	if( OMX_TRUE == pDecComp->bNeedSequenceData && pInBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG )
	{
		if( pInBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG )
		{
			pDecComp->bNeedSequenceData = OMX_FALSE;
			DbgMsg("Copy Extra Data (%d)\n", inSize );
			AVCCheckPortReconfiguration( pDecComp, inData, inSize );
			if( pDecComp->codecSpecificData )
				free( pDecComp->codecSpecificData );
			pDecComp->codecSpecificData = malloc(inSize);
			memcpy( pDecComp->codecSpecificData + pDecComp->codecSpecificDataSize, inData, inSize );
			pDecComp->codecSpecificDataSize += inSize;

			if( ( inSize>4 && inData[0]==0 && inData[1]==0 && inData[2]==0 && inData[3]==1 && ((inData[4]&0x0F)==0x07) ) ||
				( inSize>4 && inData[0]==0 && inData[1]==0 && inData[2]==1 && ((inData[3]&0x0F)==0x07) ) )
			{
				int w,h;	//	width, height, left, top, right, bottom
				if( avc_get_video_size( pDecComp->codecSpecificData, pDecComp->codecSpecificDataSize, &w, &h ) )
				{
					if( pDecComp->width != w || pDecComp->height != h )
					{
						//	Need Port Reconfiguration
						SendEvent( pDecComp, OMX_EventPortSettingsChanged, OMX_DirOutput, 0, NULL );

						// Change Port Format
						pDecComp->pOutputPort->stdPortDef.format.video.nFrameWidth = w;
						pDecComp->pOutputPort->stdPortDef.format.video.nFrameHeight = h;

						//	Native Mode
						if( pDecComp->bUseNativeBuffer )
						{
							pDecComp->pOutputPort->stdPortDef.nBufferSize = 4096;
						}
						else
						{
							pDecComp->pOutputPort->stdPortDef.nBufferSize = ((((w+15)>>4)<<4) * (((h+15)>>4)<<4))*3/2;
						}
						goto Exit;
					}
				}
			}

			goto Exit;
		}
	}

	//{
	//	OMX_U8 *buf = pInBuf->pBuffer;
	//	DbgMsg("pInBuf->nFlags(%7d)(%lld) = 0x%08x, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x\n", pInBuf->nFilledLen, pInBuf->nTimeStamp, pInBuf->nFlags,
	//		buf[ 0],buf[ 1],buf[ 2],buf[ 3],buf[ 4],buf[ 5],buf[ 6],buf[ 7],
	//		buf[ 8],buf[ 9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],
	//		buf[16],buf[17],buf[18],buf[19],buf[20],buf[21],buf[22],buf[23] );
	//}

	//	Push Input Time Stamp
	if( !(pInBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG) )
		PushVideoTimeStamp(pDecComp, pInBuf->nTimeStamp, pInBuf->nFlags );


	//	Step 2. Find First Key Frame & Do Initialize VPU
	if( OMX_FALSE == pDecComp->bInitialized )
	{
		int initBufSize;
		unsigned char *initBuf;

		if( pDecComp->codecSpecificDataSize == 0 && pDecComp->nExtraDataSize>0 )
		{
			initBufSize = inSize + pDecComp->nExtraDataSize;
			initBuf = (unsigned char *)malloc( initBufSize );
			memcpy( initBuf, pDecComp->pExtraData, pDecComp->nExtraDataSize );
			memcpy( initBuf + pDecComp->nExtraDataSize, inData, inSize );
		}
		else
		{
			initBufSize = inSize + pDecComp->codecSpecificDataSize;
			initBuf = (unsigned char *)malloc( initBufSize );
			memcpy( initBuf, pDecComp->codecSpecificData, pDecComp->codecSpecificDataSize );
			memcpy( initBuf + pDecComp->codecSpecificDataSize, inData, inSize );
		}

		if( OMX_TRUE == pDecComp->bNeedSequenceData )
		{
			if( AVCCheckPortReconfiguration( pDecComp, initBuf, initBufSize ) )
			{
				pDecComp->bNeedSequenceData = OMX_FALSE;
				if( pDecComp->codecSpecificData )
					free( pDecComp->codecSpecificData );
				pDecComp->codecSpecificData = malloc(initBufSize);
				memcpy( pDecComp->codecSpecificData, initBuf, initBufSize );
				pDecComp->codecSpecificDataSize = initBufSize;
				goto Exit;
			}
			else
			{
				goto Exit;
			}
		}

		//	Initialize VPU
		ret = InitializeCodaVpu(pDecComp, initBuf, initBufSize );
		free( initBuf );

		if( 0 > ret )
		{
			ErrMsg("VPU initialized Failed!!!!\n");
			goto Exit;
		}else if( ret > 0  )
		{
			ret = 0;
			goto Exit;
		}

		pDecComp->bNeedKey = OMX_FALSE;
		pDecComp->bInitialized = OMX_TRUE;

		decIn.strmBuf = inData;
		decIn.strmSize = 0;
		decIn.timeStamp = pInBuf->nTimeStamp;
		decIn.eos = 0;
		ret = NX_VidDecDecodeFrame( pDecComp->hVpuCodec, &decIn, &decOut );
	}
	else
	{
		decIn.strmBuf = inData;
		decIn.strmSize = inSize;
		decIn.timeStamp = pInBuf->nTimeStamp;
		decIn.eos = 0;
		ret = NX_VidDecDecodeFrame( pDecComp->hVpuCodec, &decIn, &decOut );
	}

	TRACE("decOut : readPos = %d, writePos = %d\n", decOut.strmReadPos, decOut.strmWritePos );
	TRACE("Output Buffer : ColorFormat(0x%08x), NatvieBuffer(%d), Thumbnail(%d), MetaDataInBuffer(%d)\n",
			pDecComp->outputFormat.eColorFormat, pDecComp->bUseNativeBuffer, pDecComp->bEnableThumbNailMode, pDecComp->bMetaDataInBuffers );


	if( ret==VID_ERR_NONE && decOut.outImgIdx >= 0 && ( decOut.outImgIdx < NX_OMX_MAX_BUF ) )
	{
		if( OMX_TRUE == pDecComp->bEnableThumbNailMode )
		{
			//	Thumbnail Mode
			NX_VID_MEMORY_INFO *pImg = &decOut.outImg;
			NX_PopQueue( pOutQueue, (void**)&pOutBuf );
			CopySurfaceToBufferYV12( (OMX_U8*)pImg->luVirAddr, (OMX_U8*)pImg->cbVirAddr, (OMX_U8*)pImg->crVirAddr,
				pOutBuf->pBuffer, pImg->luStride, pImg->cbStride, pDecComp->width, pDecComp->height );

			NX_VidDecClrDspFlag( pDecComp->hVpuCodec, NULL, decOut.outImgIdx );
			pOutBuf->nFilledLen = pDecComp->width * pDecComp->height * 3 / 2;
			if( 0 != PopVideoTimeStamp(pDecComp, &pOutBuf->nTimeStamp, &pOutBuf->nFlags )  )
			{
				pOutBuf->nTimeStamp = pInBuf->nTimeStamp;
				pOutBuf->nFlags     = pInBuf->nFlags;
			}
			DbgMsg("ThumbNail Mode : pOutBuf->nAllocLen = %ld, pOutBuf->nFilledLen = %ld\n", pOutBuf->nAllocLen, pOutBuf->nFilledLen );
			pDecComp->outFrameCount++;
			pDecComp->pCallbacks->FillBufferDone(pDecComp->hComp, pDecComp->hComp->pApplicationPrivate, pOutBuf);
			goto Exit;
		}
		else
		{
			if( pDecComp->isOutIdr == OMX_FALSE && decOut.picType != PIC_TYPE_I )
			{
				NX_VidDecClrDspFlag( pDecComp->hVpuCodec, NULL, decOut.outImgIdx );
				goto Exit;
			}
			pDecComp->isOutIdr = OMX_TRUE;

			//	Native Window Buffer Mode
			//	Get Output Buffer Pointer From Output Buffer Pool
			pOutBuf = pDecComp->pOutputBuffers[decOut.outImgIdx];
			// if( OMX_TRUE == pDecComp->bMetaDataInBuffers )
			// {
			// 	uint32_t *pOutBufType = pDecComp->pOutputBuffers[decOut.outImgIdx];
			// 	*pOutBufType = kMetadataBufferTypeGrallocSource;
			// 	pOutBuf = (OMX_BUFFERHEADERTYPE*)(((unsigned char*)pDecComp->pOutputBuffers[decOut.outImgIdx])+4);
			// 	DbgMsg("~~~~~~~~~~~~~~~~~~~ Outbuffer Data Type ~~~~~~~~~~~~~~~~~~~~~");
			// }

			if( pDecComp->outBufferUseFlag[decOut.outImgIdx] == 0 )
			{
				NX_VidDecClrDspFlag( pDecComp->hVpuCodec, NULL, decOut.outImgIdx );
				ErrMsg("Unexpected Buffer Handling!!!! Goto Exit\n");
				goto Exit;
			}
			pDecComp->outBufferUseFlag[decOut.outImgIdx] = 0;
			pDecComp->curOutBuffers --;

			pOutBuf->nFilledLen = sizeof(struct private_handle_t);
			if( 0 != PopVideoTimeStamp(pDecComp, &pOutBuf->nTimeStamp, &pOutBuf->nFlags )  )
			{
				pOutBuf->nTimeStamp = pInBuf->nTimeStamp;
				pOutBuf->nFlags     = pInBuf->nFlags;
			}
			TRACE("pOutBuf->nTimeStamp = %lld\n", pOutBuf->nTimeStamp/1000);
			pDecComp->outFrameCount++;
			pDecComp->pCallbacks->FillBufferDone(pDecComp->hComp, pDecComp->hComp->pApplicationPrivate, pOutBuf);
		}
	}

Exit:
	pInBuf->nFilledLen = 0;
	pDecComp->pCallbacks->EmptyBufferDone(pDecComp->hComp, pDecComp->hComp->pApplicationPrivate, pInBuf);

	return ret;
}
