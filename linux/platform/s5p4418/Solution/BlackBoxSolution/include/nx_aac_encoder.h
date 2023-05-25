//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __NX_AAC_ENCODER_H__
#define __NX_AAC_ENCODER_H__

#ifdef __cplusplus 
extern "C" {
#endif

typedef void *AacEncHandle;

typedef  struct {
	int		sampleRate;			/*! audio file sample rate */
	int		bitRate;			/*! encoder bit rate in bits/sec */
	short	nChannels;			/*! number of channels on input (1,2) */
	short	adtsUsed;			/*! whether write adts header */
} AacEnc_Parm;

int	NX_AACENC_OPEN			( AacEncHandle *AacEncHan, AacEnc_Parm *aacpara, int *ReadSizeByte );
int	NX_AACENC_FRAME			( AacEncHandle AacEncHan, short *in_buf, unsigned char *out_buf, int *OutSizeByte );
int NX_AACENC_GET_HEADER	( AacEncHandle AacEncHan, unsigned char *pBuf );
int	NX_AACENC_CLOSE			( AacEncHandle AacEncHan );

#ifdef __cplusplus
};
#endif

#endif	// __NX_AAC_ENCODER_H__
