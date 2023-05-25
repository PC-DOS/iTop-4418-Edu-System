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
#include <stdint.h>

#include "CNX_DynamicRTSPServer.h"
#include "CNX_RTPFilter.h"

//------------------------------------------------------------------------------
//
// CNX_DynamicRTSPServer
//
#undef NX_DTAG
#define NX_DTAG "[CNX_DynamicRTSPServer] "
#include "NX_DbgMsg.h"

#define NEW_SMS(description) do {\
	char const* descStr = description\
	", streamed by the LIVE555 Media Server";\
	sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

//------------------------------------------------------------------------------
CNX_DynamicRTSPServer* CNX_DynamicRTSPServer::createNew( UsageEnvironment& env, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	int ourSocket = setUpOurSocket( env, ourPort );
	if( -1 == ourSocket ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Setup socket.\n"), __func__) );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return (-1 == ourSocket) ? NULL : new CNX_DynamicRTSPServer( env, ourSocket, ourPort, authDatabase, reclamationTestSeconds );
}

//------------------------------------------------------------------------------
CNX_DynamicRTSPServer::CNX_DynamicRTSPServer( UsageEnvironment& env, int ourSocket, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds )
	: RTSPServerSupportingHTTPStreaming( env, ourSocket, ourPort, authDatabase, reclamationTestSeconds )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_pFilter		= NULL;
	m_StreamType	= 0;
	m_MaxSessionNum = 0;

	for( int32_t i = 0; i < MAX_SESSION_NUM; i++ )
	{
		memset( m_SessionName[i], 0x00, sizeof(m_SessionName[i]) );
		snprintf( (char*)m_SessionName[i], sizeof(m_SessionName[i]), "video%d", i );
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
CNX_DynamicRTSPServer::~CNX_DynamicRTSPServer()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
static ServerMediaSession* createNewSMS( UsageEnvironment& env, char const* fileName, FILE* fid, void *arg )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	ServerMediaSession* sms = NULL;
	Boolean const reuseSource = False;

	NEW_SMS("Nexell Live Stream");
	OutPacketBuffer::maxSize = 2000000;	// 2M Bytes ( why? encoding bitrate is 10Mbps )
	
	CNX_DynamicRTSPServer *pObj = (CNX_DynamicRTSPServer*)arg;
	CNX_H264LiveServerMediaSession *pMediaSession = CNX_H264LiveServerMediaSession::createNew( env, reuseSource );
	
	if( pMediaSession ) {
		pMediaSession->SetOwnerFilter( pObj->GetOwnerFilter() );
		pMediaSession->SetStreamType( pObj->GetStreamType() );
	}
	
	sms->addSubsession( pMediaSession );
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return sms;
}

//------------------------------------------------------------------------------
ServerMediaSession* CNX_DynamicRTSPServer::lookupServerMediaSession( char const* streamName )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	// a. Check connection address
	// a-1. No stream name.
	if( !streamName ) {
		NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): Connection Address is not avaliable.\n"), __func__) );
		return NULL;		
	}

	// a-2. Not match stream name.
	int32_t bExist = false;
	for( int32_t i = 0; i < m_MaxSessionNum; i++ )
	{
		if( !strcmp( streamName, (char*)m_SessionName[i] ) ) {
			bExist = true;
			break;
		}
	}
	if( !bExist ) {
		NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): Connection Address is not avaliable.\n"), __func__) );
		return NULL;		
	}

	// b. Check existing session.
	for( int32_t i = 0; i < m_MaxSessionNum; i++ )
	{
		if( !strcmp( streamName, (char*)m_SessionName[i]) ) {
			if( i != GetStreamType() ) {
				for( int32_t j = 0; j < m_MaxSessionNum; j++ ) {
					ServerMediaSession* sms = NULL;
					sms = RTSPServer::lookupServerMediaSession( (char*)m_SessionName[j] );
					if( sms ) closeAllClientSessionsForServerMediaSession( sms );
				}
			}
			SetStreamType( i );
		}
	}

	// c. Check connection client number
	if( !((CNX_RTPFilter*)m_pFilter)->ConnectIsReady() ) {
		NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): Connection client is over.\n"), __func__) );
		return NULL;
	}

	// d. Create New Session.
	ServerMediaSession* newSms = NULL;
	if( !(newSms = RTSPServer::lookupServerMediaSession( streamName )) ) {
		newSms = createNewSMS( envir(), streamName, NULL, this );
		addServerMediaSession( newSms );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return newSms;
}

//------------------------------------------------------------------------------
void CNX_DynamicRTSPServer::SetOwnerFilter( void *pOwnerFilter )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( NULL != pOwnerFilter );

	m_pFilter = pOwnerFilter;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void *CNX_DynamicRTSPServer::GetOwnerFilter( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( NULL != m_pFilter );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return m_pFilter;
}

//------------------------------------------------------------------------------
void CNX_DynamicRTSPServer::SetStreamType( int32_t streamType )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_StreamType = streamType;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_DynamicRTSPServer::GetStreamType( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return m_StreamType;
}

//------------------------------------------------------------------------------
void CNX_DynamicRTSPServer::SetMaxSessionNum( int32_t sessionNum )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( MAX_SESSION_NUM > sessionNum );

	m_MaxSessionNum = sessionNum;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DynamicRTSPServer::SetSessionName( int32_t sessionID, uint8_t *pSessionName )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	if( sessionID >= m_MaxSessionNum ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): session number is over.\n"), __func__) );
		goto ERROR;
	}

	memset( m_SessionName[sessionID], 0x00, sizeof(m_SessionName[sessionID]) );
	strncpy( (char*)m_SessionName[sessionID], (char*)pSessionName, sizeof(m_SessionName[sessionID]) - 1 );

ERROR:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}


//------------------------------------------------------------------------------
//
// CNX_H264LiveServerMediaSession
//
#undef NX_DTAG
#define NX_DTAG "[CNX_H264LiveServerMediaSession] "
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_H264LiveServerMediaSession* CNX_H264LiveServerMediaSession::createNew(UsageEnvironment& env, bool reuseFirstSource)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );

	return new CNX_H264LiveServerMediaSession(env, reuseFirstSource);
}

//------------------------------------------------------------------------------
CNX_H264LiveServerMediaSession::CNX_H264LiveServerMediaSession(UsageEnvironment& env, bool reuseFirstSource)
	: OnDemandServerMediaSubsession(env, reuseFirstSource), fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	m_pFilter = NULL;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
CNX_H264LiveServerMediaSession::~CNX_H264LiveServerMediaSession()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	delete[] fAuxSDPLine;
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_H264LiveServerMediaSession::afterPlayingDummy1()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	setDoneFlag();

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
static void checkForAuxSDPLine(void* clientData)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	CNX_H264LiveServerMediaSession* subsess = (CNX_H264LiveServerMediaSession*)clientData;
	subsess->checkForAuxSDPLine1();

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

void CNX_H264LiveServerMediaSession::checkForAuxSDPLine1()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	char const* dasl;

	if (fAuxSDPLine != NULL) {
		setDoneFlag();
	}
	else {
		if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
			fAuxSDPLine = strDup(dasl);
			fDummyRTPSink = NULL;

			setDoneFlag();
		}
		else {
			int uSecsToDelay = 100000;
			nextTask() = envir().taskScheduler().scheduleDelayedTask( uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this );
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
static void afterPlayingDummy(void* clientData)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_H264LiveServerMediaSession* subsess = (CNX_H264LiveServerMediaSession*)clientData;
	subsess->afterPlayingDummy1();
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

char const* CNX_H264LiveServerMediaSession::getAuxSDPLine( RTPSink* rtpSink, FramedSource* inputSource )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if (fAuxSDPLine != NULL) return fAuxSDPLine;

	if (fDummyRTPSink == NULL) {
		fDummyRTPSink = rtpSink;

		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
		checkForAuxSDPLine(this);
	}

	envir().taskScheduler().doEventLoop(&fDoneFlag);
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return fAuxSDPLine;
}

//------------------------------------------------------------------------------
FramedSource* CNX_H264LiveServerMediaSession::createNewStreamSource( unsigned clientSessionId, unsigned& estBitrate )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	estBitrate = 90000; // kbps, estimate

	CNX_LiveSource *pLiveSource = CNX_LiveSource::createNew( envir(), -1 );
	if( pLiveSource ) {
		pLiveSource->SetOwnerFilter( m_pFilter );
		pLiveSource->SetStreamType( m_StreamType );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return H264VideoStreamDiscreteFramer::createNew( envir(), pLiveSource );
}

//------------------------------------------------------------------------------
RTPSink* CNX_H264LiveServerMediaSession::createNewRTPSink( Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

//------------------------------------------------------------------------------
void CNX_H264LiveServerMediaSession::SetOwnerFilter( void *pOwerFilter )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( NULL != pOwerFilter );
	
	m_pFilter = pOwerFilter;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_H264LiveServerMediaSession::SetStreamType( uint32_t streamType )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	m_StreamType = streamType;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}


//------------------------------------------------------------------------------
//
// CNX_H264LiveServerMediaSession
//
#undef NX_DTAG
#define NX_DTAG "[CNX_LiveSource] "
#include "NX_DbgMsg.h"

unsigned CNX_LiveSource::referenceCount = 0;
EventTriggerId CNX_LiveSource::eventTriggerId = 0;

#if(0)
static void dumpdata( void *data, int len, const char *msg )
{
	int i=0;
	unsigned char *byte = (unsigned char *)data;
	printf("Dump Data : %s", msg);
	for( i=0 ; i<len ; i ++ )
	{
		if( i!=0 && i%32 == 0 ) printf("\n\t");
		printf("%.2x", byte[i] );
		if( i%4 == 3 ) printf(" ");
	}
	printf("\n");
}
#else
static void dumpdata( void *data, int len, const char *msg )
{
}
#endif

static void splitFrameData( uint8_t *keyFrameBuf, int32_t keyFrameSize, uint8_t *spsBuf, int32_t *spsSize, uint8_t *ppsBuf, int32_t *ppsSize, uint8_t *frameBuf, int32_t *frameSize )
{
	int32_t i = 0;
	int32_t ppsStart = 0, frameStart = 0;
	int32_t flags = 0;	// 0 : sps section, 1 : pps section, 2 : frame section

	for( i = 0; i < keyFrameSize; i++ )
	{
		if( keyFrameBuf[i + 0] == 0x00 && 
			keyFrameBuf[i + 1] == 0x00 && 
			keyFrameBuf[i + 2] == 0x00 && 
			keyFrameBuf[i + 3] == 0x01 )
		{
			if( 1 == flags ) {
				ppsStart = i;
			}
			else if( 2 == flags ) {
				frameStart = i;
				break;
			}
			flags++;
		}	
	}

	*spsSize = ppsStart;
	memcpy( spsBuf, keyFrameBuf, *spsSize );

	*ppsSize = frameStart - ppsStart;
	memcpy( ppsBuf, keyFrameBuf + ppsStart, *ppsSize );

	*frameSize = keyFrameSize - frameStart;
	memcpy( frameBuf, keyFrameBuf + frameStart, *frameSize );
}

//------------------------------------------------------------------------------
CNX_LiveSource* CNX_LiveSource::createNew( UsageEnvironment& env, int device )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	
	return new CNX_LiveSource( env, device );
}

//------------------------------------------------------------------------------
CNX_LiveSource::CNX_LiveSource( UsageEnvironment& env, int device )
	: FramedSource(env)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	if(referenceCount == 0)
	{
	}

	++referenceCount;
	NxDbgMsg( NX_DBG_VBS, (TEXT("refCount = %d\n"), referenceCount) );

	if (eventTriggerId == 0) {
		eventTriggerId = envir().taskScheduler().createEventTrigger( deliverFrame0 );
	}

	// User Specific Code.
	// a. private variable initialize.
	m_pFilter = NULL;

	// b. create & initialize queue.
	m_pSampleInQueue = new CNX_SampleQueue();
	m_pSampleInQueue->SetQueueDepth( 16 );
	m_pSampleInQueue->Reset();

	// c. create semaphore
	m_pSemIn = new CNX_Semaphore( 16, 0 );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
CNX_LiveSource::~CNX_LiveSource(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	--referenceCount;
	NxDbgMsg( NX_DBG_VBS, (TEXT("refCount = %d\n"), referenceCount) );

	if (referenceCount == 0) {
		envir().taskScheduler().deleteEventTrigger( eventTriggerId );
		eventTriggerId = 0;
	}

	((CNX_RTPFilter*)m_pFilter)->ClearSourceInstance( this );

	// User Specific Code.
	// a. release sample queue.
	while( m_pSampleInQueue->IsReady() )
	{
		CNX_MuxerSample *pReleaseSample;
		m_pSampleInQueue->PopSample( (CNX_Sample**)&pReleaseSample );
		pReleaseSample->Unlock();
	}

	// b. release internal queue.
	while( !m_EncQueue.empty() ) {
		EncBuffer encBuf = m_EncQueue.front();
		m_EncQueue.pop();
		if( encBuf.buf ) free( encBuf.buf );
	}

	// c. delete queue & semaphore
	delete m_pSampleInQueue;
	delete m_pSemIn;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_LiveSource::doGetNextFrame()
{
	if( m_EncQueue.empty() == true ) {
		GetEndcodedFrameData();
		gettimeofday( &fPresentationTime, NULL );
		deliverFrame();
	}
	else {
		deliverFrame();
	}
}

//-----------------------------------------------------------------------------
#define MAX_DSI_BUF_SIZE	1024

void CNX_LiveSource::GetEndcodedFrameData( void )
{
	CNX_MuxerSample *pInSample;
	EncBuffer 		inBuf;

	if( !m_pSampleInQueue->IsReady() ) {
		m_pSemIn->Pend();
	}

	m_pSampleInQueue->PopSample( (CNX_Sample**)&pInSample );
	m_pSemIn->Pend();
	pInSample->GetBuffer( &inBuf.buf, &inBuf.size );
	inBuf.isKey = pInSample->GetSyncPoint();

	if( inBuf.isKey ) {
		EncBuffer spsBuf, ppsBuf, frameBuf;

		spsBuf.buf = (uint8_t*)malloc( MAX_DSI_BUF_SIZE );
		ppsBuf.buf = (uint8_t*)malloc( MAX_DSI_BUF_SIZE );
		frameBuf.buf = (uint8_t*)malloc( inBuf.size );

		splitFrameData( inBuf.buf, inBuf.size, spsBuf.buf, &spsBuf.size, ppsBuf.buf, &ppsBuf.size, frameBuf.buf, &frameBuf.size );
		
		m_EncQueue.push( spsBuf );
		m_EncQueue.push( ppsBuf );
		m_EncQueue.push( frameBuf );

		dumpdata( spsBuf.buf, 16, "SPS\n\t" );
		dumpdata( ppsBuf.buf, 16, "PPS\n\t" );
		dumpdata( frameBuf.buf, 16, "I-Frame\n\t" );
	}
	else {
		EncBuffer frameBuf;

		frameBuf.buf = (uint8_t*)malloc(inBuf.size);
		frameBuf.size = inBuf.size;

		memcpy( frameBuf.buf, inBuf.buf, frameBuf.size);

		m_EncQueue.push( frameBuf );
		dumpdata( frameBuf.buf, 16, "P-Frame\n\t" );
	}
	pInSample->Unlock();
}

//------------------------------------------------------------------------------
void CNX_LiveSource::deliverFrame0(void* clientData)
{
	((CNX_LiveSource*)clientData)->deliverFrame();
}

//------------------------------------------------------------------------------
void CNX_LiveSource::deliverFrame()
{
	if( !isCurrentlyAwaitingData() ) return;

	EncBuffer outBuf = m_EncQueue.front();
	m_EncQueue.pop();

	if( (uint32_t)outBuf.size - 4 > fMaxSize ) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = outBuf.size - 4 - fMaxSize;
	}
	else {
		fFrameSize = outBuf.size - 4;
	}

	memmove(fTo, outBuf.buf + 4, fFrameSize);
	FramedSource::afterGetting(this);

	if( outBuf.buf ) free( outBuf.buf );
}

//------------------------------------------------------------------------------
void CNX_LiveSource::SetOwnerFilter( void *pOwerFilter )
{
	NX_ASSERT( NULL != pOwerFilter );
	m_pFilter = pOwerFilter;
}

//------------------------------------------------------------------------------
void CNX_LiveSource::SetStreamType( uint32_t streamType )
{
	NX_ASSERT( NULL != m_pFilter );
	((CNX_RTPFilter*)m_pFilter)->SetSourceInstance( this, streamType );
}

//------------------------------------------------------------------------------
void CNX_LiveSource::PushSample( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != m_pSampleInQueue );
	pSample->Lock();
	m_pSampleInQueue->PushSample( pSample );
}

//------------------------------------------------------------------------------
void CNX_LiveSource::Post( void )
{
	NX_ASSERT( NULL != m_pSemIn );
	m_pSemIn->Post();
}
