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

#define NX_DTAG		"[NX_DbgMsg] "
#include <NX_DbgMsg.h>

// NX_DBG_DISABLE, NX_DBG_ERR, NX_DBG_WARN, NX_DBG_INFO, NX_DBG_DEBUG, NX_DBG_VBS
uint32_t gNxFilterDebugLevel = NX_DBG_INFO;

void NxChgFilterDebugLevel( uint32_t level )
{
	if( level <= NX_DBG_VBS ){
		NxDbgMsg( NX_DBG_VBS, (TEXT("NxChgFilterDebugLevel : Change debug level %d to %d.\n"), gNxFilterDebugLevel, level) );
		gNxFilterDebugLevel = level;
	}
}
