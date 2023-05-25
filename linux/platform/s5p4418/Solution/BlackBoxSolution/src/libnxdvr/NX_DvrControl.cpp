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

#include <stdio.h>

#include "CNX_DvrManager.h"
#include "NX_DvrControl.h"

//------------------------------------------------------------------------------
NX_DVR_HANDLE NX_DvrInit( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig )
{
	CNX_DvrManager *pHandle = new CNX_DvrManager();
	
	pHandle->Init( pMediaConfig, pRecordConfig, pDisplayConfig);

	return pHandle;
}

//------------------------------------------------------------------------------
void NX_DvrDeinit( NX_DVR_HANDLE hDvr)
{
	hDvr->Deinit();

	if( hDvr )
		delete hDvr;
}

//------------------------------------------------------------------------------
int32_t NX_DvrStart( NX_DVR_HANDLE hDvr, NX_DVR_ENCODE_TYPE nEncodeType )
{
	if( hDvr )
		hDvr->Start( nEncodeType );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrStop( NX_DVR_HANDLE hDvr )
{
	if( hDvr )
		hDvr->Stop();

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrEvent( NX_DVR_HANDLE hDvr )
{
	if( hDvr )
		hDvr->SetEvent();

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrChangeMode( NX_DVR_HANDLE hDvr, NX_DVR_ENCODE_TYPE nEncodeType )
{
	if( hDvr )
		hDvr->ChangeMode( nEncodeType );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrCapture( NX_DVR_HANDLE hDvr, int32_t channel )
{
	if( hDvr )
		hDvr->SetCapture( channel );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrSetPreview( NX_DVR_HANDLE hDvr, int32_t preview )
{
	if( hDvr )
		hDvr->SetPreview( preview );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrSetPreviewHdmi( NX_DVR_HANDLE hDvr, int32_t preview )
{
	if( hDvr )
		hDvr->SetPreviewHdmi( preview );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrSetDisplay( NX_DVR_HANDLE hDvr, NX_DVR_DISPLAY_CONFIG *pDisplayConfig )
{
	if( hDvr )
		hDvr->SetDisplay( pDisplayConfig );
	
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterFileNameCallback ( NX_DVR_HANDLE hDvr, int32_t (*cbNormalFileName)(uint8_t*, uint32_t), int32_t (*cbEventFileName)(uint8_t*, uint32_t), int32_t (*cbParkingFileName)(uint8_t*, uint32_t), int32_t (*cbJpegFileName)(uint8_t*, uint32_t) )
{
	if( hDvr )
		hDvr->RegisterGetFileNameCallback( cbNormalFileName, cbEventFileName, cbParkingFileName, cbJpegFileName );
	
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterUserDataCallback ( NX_DVR_HANDLE hDvr, int32_t (*cbUserData)(uint8_t*, uint32_t) )
{
	if( hDvr )
		hDvr->RegisterUserDataCallback( cbUserData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterTextOverlayCallback ( NX_DVR_HANDLE hDvr, int32_t (*cbFrontTextOverlay)(uint8_t*, uint32_t*, uint32_t*, uint32_t*), int32_t (*cbRearTextOverlay)(uint8_t*, uint32_t*, uint32_t*, uint32_t*) )
{
	if( hDvr )
		hDvr->RegisterTextOverlayCallback( cbFrontTextOverlay, cbRearTextOverlay );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterNotifyCallback ( NX_DVR_HANDLE hDvr, int32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) )
{
	if( hDvr )
		hDvr->RegisterNotifyCallback( cbNotify );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterImageEffectCallback( NX_DVR_HANDLE hDvr, int32_t (*cbFrontImageEffect)(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *), int32_t (*cbRearImageEffect)(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *) )
{
	if( hDvr )
		hDvr->RegisterImageEffectCallback( cbFrontImageEffect, cbRearImageEffect );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrRegisterMotionDetectCallback( NX_DVR_HANDLE hDvr, int32_t (*cbFrontMotionDetect)(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *), int32_t (*cbRearMotionDetect)(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *))
{
	if( hDvr )
		hDvr->RegisterMotionDetectCallback( cbFrontMotionDetect, cbRearMotionDetect );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrChgDebugLevel( NX_DVR_HANDLE hDvr, uint32_t dbgLevel )
{
	if( hDvr )
		hDvr->ChangeDebugLevel( dbgLevel );
	
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrGetAPIVersion( NX_DVR_HANDLE hDvr, int32_t *pMajor, int32_t *pMinor, int32_t *pRevision )
{
	if( hDvr )
		hDvr->GetAPIVersion( pMajor, pMinor, pRevision );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrGetStatistics( NX_DVR_HANDLE hDvr, NX_DVR_MEDIA_CONFIG *pMediaConfig )
{
	printf("Not yet..\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrVideoLayerEnable( NX_DVR_HANDLE hDvr, uint32_t nEnable )
{
	printf("Not yet..\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrGetVideoLayerPriority( NX_DVR_HANDLE hDvr, uint32_t *pPriority )
{
	printf("Not yet..\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_DvrSetVideoLayerPriority( NX_DVR_HANDLE hDvr, uint32_t nPriority )
{
	printf("Not yet..\n");
	return 0;
}

