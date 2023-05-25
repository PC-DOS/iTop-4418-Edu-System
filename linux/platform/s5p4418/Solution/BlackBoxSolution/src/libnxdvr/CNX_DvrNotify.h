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

#include <CNX_BaseFilter.h>

#define MAX_EVENT_QUEUE_DEPTH		128

typedef struct tag_DVR_EVENT_MESSAGE {
	uint32_t	eventType;
	uint8_t		*eventData;
	uint32_t	dataLength;
} DVR_EVENT_MESSAGE;

#ifndef __CNX_DVRNOTIFY_H__
#define __CNX_DVRNOTIFY_H__

class CNX_DvrNotify
	: public INX_EventNotify
{
public:
	CNX_DvrNotify();
	~CNX_DvrNotify();

public:
	bool				Run();
	bool				Stop();

	virtual int32_t		RegisterNotifyCallback( int32_t (*cbNotifyCallback)(uint32_t, uint8_t*, uint32_t) );
	int32_t				(*NotifyCallback)( uint32_t eventCode, uint8_t *pEventData, uint32_t dataLength );

protected:
	virtual void		EventNotify( uint32_t eventCode, void *pEventData, uint32_t dataLength );

private:
	void				ThreadLoop ( void );
	static void*		ThreadMain ( void *arg );

	virtual void		Push( DVR_EVENT_MESSAGE *pSample );
	virtual	void		Pop( DVR_EVENT_MESSAGE **ppSample );

private:
	bool				m_bRun;
	bool				m_bThreadExit;
	pthread_t			m_hThread;

	CNX_Semaphore		*m_pSem;

	DVR_EVENT_MESSAGE	*m_pEventMsg[MAX_EVENT_QUEUE_DEPTH];
	int32_t				m_iHeadIndex, m_iTailIndex, m_nMsgCount, m_nQueueDepth;
	pthread_mutex_t		m_hLock;
};

#endif	// __CNX_DVRNOTIFY_H__