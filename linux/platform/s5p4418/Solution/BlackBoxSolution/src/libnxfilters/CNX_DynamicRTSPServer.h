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

#ifndef __CNX_DYNAMICRTSPSERVER_H__
#define __CNX_DYNAMICRTSPSERVER_H__

#include <stdio.h>
#include <queue>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <OnDemandServerMediaSubsession.hh>
#include <RTSPServerSupportingHTTPStreaming.hh>

#include "CNX_BaseFilter.h"

typedef struct tagEncBuffer {
	uint8_t		*buf;
	int32_t		size;
	int32_t		isKey;
} EncBuffer;

enum {
	RTSP_VIDEO_DATA0 = 0,
	RTSP_VIDEO_DATA1,
	RTSP_VIDEO_DATA2,
	RTSP_VIDEO_DATA3,
	MAX_SESSION_NUM,
};

//------------------------------------------------------------------------------
class CNX_DynamicRTSPServer
	: public RTSPServerSupportingHTTPStreaming
{
public:
	static CNX_DynamicRTSPServer* createNew( UsageEnvironment& env, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds = 65 );

	void		SetOwnerFilter( void *pOwerFilter );
	void*		GetOwnerFilter( void );

	void		SetStreamType( int32_t streamType );
	int32_t		GetStreamType( void );

	void		SetMaxSessionNum( int32_t sessionNum );
	void 		SetSessionName( int32_t sessionID, uint8_t *pSessionName );

protected:
	CNX_DynamicRTSPServer( UsageEnvironment& env, int ourSocket, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds );
	virtual ~CNX_DynamicRTSPServer();

protected:
	virtual ServerMediaSession* lookupServerMediaSession( char const* streamName );

private:
	void			*m_pFilter;
	uint32_t		m_StreamType;
	int32_t			m_MaxSessionNum;
	uint8_t 		m_SessionName[MAX_SESSION_NUM][255];
};

//------------------------------------------------------------------------------
class CNX_H264LiveServerMediaSession
	: public OnDemandServerMediaSubsession
{
public:
	static CNX_H264LiveServerMediaSession* createNew( UsageEnvironment& env, bool reuseFirstSource );
	void 	checkForAuxSDPLine1();
	void 	afterPlayingDummy1();

public:
	void	SetOwnerFilter( void *pOwerFilter );
	void	SetStreamType( uint32_t streamType );

protected:
	CNX_H264LiveServerMediaSession( UsageEnvironment& env, bool reuseFirstSource );
	virtual ~CNX_H264LiveServerMediaSession();
	void	setDoneFlag() { fDoneFlag = ~0; }

protected:
	virtual	char const*		getAuxSDPLine( RTPSink* rtpSink, FramedSource* inputSource );
	virtual FramedSource*	createNewStreamSource( unsigned clientSessionId, unsigned &estBitrate );
	virtual RTPSink*		createNewRTPSink( Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource );

private:
	char			*fAuxSDPLine;
	char			fDoneFlag;
	RTPSink			*fDummyRTPSink;

private:
	void			*m_pFilter;
	uint32_t		m_StreamType;
};

//------------------------------------------------------------------------------
class CNX_LiveSource
	: public FramedSource
{
public:
	static CNX_LiveSource* createNew( UsageEnvironment& env, int device );
	static EventTriggerId eventTriggerId;
	static unsigned referenceCount;
	
public:
	void	SetOwnerFilter( void *pOwerFilter );
	void	SetStreamType( uint32_t streamType );

	void	PushSample( CNX_Sample *pSample );
	void	Post( void );

protected:
	CNX_LiveSource( UsageEnvironment& env, int device );
	virtual ~CNX_LiveSource( void );

private:
	virtual	void 	doGetNextFrame( void );
	static	void 	deliverFrame0( void* clientData );
			void 	deliverFrame( void );
			void 	GetEndcodedFrameData( void );

private:
	void					*m_pFilter;
	CNX_SampleQueue			*m_pSampleInQueue;
	CNX_Semaphore			*m_pSemIn;

	std::queue<EncBuffer>	m_EncQueue;
};

#endif	// __CNX_DYNAMICRTSPSERVER_H__