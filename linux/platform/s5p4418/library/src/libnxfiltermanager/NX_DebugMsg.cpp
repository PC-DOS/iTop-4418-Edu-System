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
#include <stdio.h>
#include <stdarg.h>
#include "NX_DebugMsg.h"


static unsigned int gst_TargetDebugLevel = PLAYER_DBG_LEVEL_INFO;	//	Error
void NX_ChangeDebugLevel( unsigned int TargetLevel )
{
#if	_DEBUG
	printf("[S5P4418]>>> NX_ChangeDebugLevel : %d to %d\n", gst_TargetDebugLevel, TargetLevel);
	gst_TargetDebugLevel = TargetLevel;
#else
	printf(">>> Cannot change debug level. This library build in release mode.\n");
#endif
}

void NX_DbgMsg( unsigned int level, const char *format, ... )
{
	char buffer[1024];
	if (DBG_MSG_ON == level)
	{
		va_list marker;
		va_start(marker, format);
		vsprintf(buffer, format, marker);
		printf("[S5P4418]%s", buffer);
		va_end(marker);
	}
	else
		return;
}


void NX_RelMsg( const char *format, ... )
{
	char buffer[1024];
	va_list marker;
	va_start(marker, format);
	vsprintf(buffer, format, marker);
	printf( "[S5P4418]%s", buffer );
	va_end(marker);
}
