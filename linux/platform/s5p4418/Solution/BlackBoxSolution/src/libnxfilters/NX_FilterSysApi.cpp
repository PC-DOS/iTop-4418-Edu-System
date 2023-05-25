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

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include <NX_FilterSysApi.h>

uint64_t gNxStartTime	= 0;
uint64_t gNxStopTime	= 0;

uint64_t NX_GetTickCount( void )
{
	uint64_t ret;
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
	return ret;
}

void NX_Sleep( uint32_t mSec )
{
#if 0
	struct timeval	tv;
	tv.tv_sec = 0;
	tv.tv_usec = mSec * 1000;
	select( 0, NULL, NULL, NULL, &tv );
#else
	usleep( mSec*1000 );
#endif
}
