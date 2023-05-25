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

#ifndef __NX_DBGMSG_H__
#define __NX_DBGMSG_H__

#include <stdio.h>
#include <stdint.h>

#ifndef TEXT
	#define TEXT(A)			A
#endif
#ifndef _T
	#define _T(A)			A
#endif

//	Debug Tag
#ifndef NX_DTAG
#define	NX_DTAG				"[] "
#endif

#define	DEBUG

//	Debug Level
#define NX_DBG_DISABLE		0
#define	NX_DBG_ERR			1
#define	NX_DBG_WARN			2
#define	NX_DBG_INFO			3
#define NX_DBG_DEBUG		4
#define NX_DBG_VBS			5

#define	DEBUG_PRINT			printf

#ifdef NX_TRACE
#undef NX_TRACE
#endif
#ifdef NX_ASSERT
#undef NX_ASSERT
#endif
#ifdef DEBUG
#define NxDbgMsg(A, B)		do{									\
								if( gNxFilterDebugLevel>=A )	\
								{								\
									DEBUG_PRINT(NX_DTAG);		\
									DEBUG_PRINT B;				\
								}								\
							}while(0)

#define NX_TRACE(A)			DEBUG_PRINT A

#define NX_ASSERT(expr)		do{																						\
								if( !(expr) )																		\
								{																					\
									DEBUG_PRINT(TEXT("%s%s(%d) : %s (%s)\n"),										\
												NX_DTAG, TEXT(__FILE__), __LINE__, TEXT("ASSERT"), TEXT(#expr));	\
									while(1);																		\
								}																					\
							}while(0)
#else
#define NxDbgMsg(A, B)		do{}while(0)
#define NX_TRACE(A)			do{}while(0)
#define NX_ASSERT(expr)		do{}while(0)
#endif	//	DEBUG

#define NxRelMsg(A, B)		do{									\
								if( gNxFilterDebugLevel>=A )	\
								{								\
									DEBUG_PRINT(NX_DTAG);		\
									DEBUG_PRINT B;				\
								}								\
							}while(0)

#define NxErrMsg(A)			do{																		\
								DEBUG_PRINT(TEXT("%s%s(%d) : %s"),									\
											NX_DTAG, TEXT(__FILE__), __LINE__, TEXT("Error: ") );	\
								DEBUG_PRINT A;														\
							}while(0)

extern uint32_t gNxFilterDebugLevel;

void			NxChgFilterDebugLevel( uint32_t level );

#endif //	__NX_DBGMSG_H__

