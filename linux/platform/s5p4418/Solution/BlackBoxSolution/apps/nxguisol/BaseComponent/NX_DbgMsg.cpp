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

#include "NX_DbgMsg.h"

//uint32_t gNxFilterDebugLevel = NX_DBG_DISABLE;
//uint32_t gNxFilterDebugLevel = NX_DBG_ERR;
//uint32_t gNxFilterDebugLevel = NX_DBG_WARN;
//uint32_t gNxFilterDebugLevel = NX_DBG_INFO;
//uint32_t gNxFilterDebugLevel = NX_DBG_DEBUG;
uint32_t gNxFilterDebugLevel = NX_DBG_VBS;

void NxChgFilterDebugLevel( uint32_t level )
{
	if( level <= NX_DBG_VBS ){
		NxDbgMsg( NX_DBG_VBS, (TEXT("NxChgFilterDebugLevel : Change debug level %d to %d.\n"), gNxFilterDebugLevel, level) );
		gNxFilterDebugLevel = level;
	}
}
