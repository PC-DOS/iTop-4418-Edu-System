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

#ifndef __CNX_REFCLOCK_H__
#define __CNX_REFCLOCK_H__

#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>

#include <CNX_Singleton.h>

#ifdef __cplusplus

class CNX_RefClock
	: public CNX_Singleton<CNX_RefClock>
{
public:
	CNX_RefClock( void )
	{
		m_TickErrorTime = 0;
		pthread_mutex_init( &m_hLock, NULL );
	}

	virtual ~CNX_RefClock( void )
	{
		pthread_mutex_destroy( &m_hLock );
	}

	uint64_t GetSysTickCount( void )
	{
		uint64_t ret;
		struct timeval	tv;
		gettimeofday( &tv, NULL );
		ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
		return ret;
	}

	uint64_t GetCorrectTickCount( void )
	{
		uint64_t ret;
		struct timeval	tv;
		gettimeofday( &tv, NULL );
		pthread_mutex_lock( &m_hLock );
		ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000 + m_TickErrorTime;
		pthread_mutex_unlock( &m_hLock );
		return ret;
	}

	uint64_t GetCorrectTickCount( int64_t systemTime )	// nSec
	{
		uint64_t ret;
		pthread_mutex_lock( &m_hLock );
		ret = (systemTime / 1000000) + m_TickErrorTime;
		pthread_mutex_unlock( &m_hLock );
		return ret;
	}

	void SetTickErrorTime( int32_t tickErrorTime )
	{
		pthread_mutex_lock( &m_hLock );
		m_TickErrorTime += tickErrorTime;
		pthread_mutex_unlock( &m_hLock );
	}

private:
	uint64_t m_TickErrorTime;
	pthread_mutex_t m_hLock;
};


#endif // __cplusplus

#endif // __CNX_REFCLOCK_H__
