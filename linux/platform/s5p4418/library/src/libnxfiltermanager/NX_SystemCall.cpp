//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//	Nexell Proprietary & Confidential
//
//	Module     : 
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#include "NX_SystemCall.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>

//#define	USE_USLEEP


//////////////////////////////////////////////////////////////////////////////
//
//							Sleep & Time
//
void NX_Sleep( unsigned int MilliSeconds )
{
#ifdef USE_USLEEP 
	usleep( MilliSeconds * 1000 );
#else
	struct timeval	tv;
	tv.tv_sec = 0;
	tv.tv_usec = MilliSeconds * 1000;
	select( 0, NULL, NULL, NULL, &tv );
#endif
}

long long NX_GetTickCount()
{
	long long Ret;
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	Ret = ((long long)tv.tv_sec)*1000 + (long long)(tv.tv_usec/1000);
	return Ret;		
}
//
//
//////////////////////////////////////////////////////////////////////////////

