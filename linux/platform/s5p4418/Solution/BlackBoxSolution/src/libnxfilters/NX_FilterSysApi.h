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

#ifndef __NX_FILTERSYSAPI_H__
#define __NX_FILTERSYSAPI_H__

#include <stdint.h>

#ifndef TEXT
	#define TEXT(A)			A
#endif
#ifndef _T
	#define _T(A)			A
#endif

//#ifndef NX_DTAG
//#define	NX_DTAG				"[]"
//#endif

#ifndef DEBUG_PRINT
#define	DEBUG_PRINT			printf
#endif

uint64_t	NX_GetTickCount	( void );
void		NX_Sleep		( uint32_t mSec );

extern uint64_t gNxStartTime;
extern uint64_t gNxStopTime;

#define NxDbgCheckTimeStart()		gNxStartTime = NX_GetTickCount()
#define NxDbgCheckTimeStop(A)		do {															\
										gNxStopTime = NX_GetTickCount();							\
										DEBUG_PRINT(TEXT("%s %s Process Time : %lld msec\n"),		\
										NX_DTAG, TEXT(A), (uint64_t)(gNxStopTime - gNxStartTime));	\
									} while(0)

#endif //	__NX_FILTERSYSAPI_H__
