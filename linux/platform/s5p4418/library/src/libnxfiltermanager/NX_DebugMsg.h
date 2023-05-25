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
#ifndef __NX_DebugMsg_h__
#define __NX_DebugMsg_h__

#define	PLAYER_DBG_LEVEL_DISABLE	0
#define PLAYER_DBG_LEVEL_ERROR		1
#define PLAYER_DBG_LEVEL_WARNING	2
#define PLAYER_DBG_LEVEL_INFO		3
#define PLAYER_DBG_LEVEL_TRACE		4
#define PLAYER_DBG_LEVEL_VERBOSE	5

#define DBG_MSG_ON			1
#define DBG_MSG_OFF			0


void NX_ChgDbgLvl( unsigned int TargetLevel );
void NX_DbgMsg( unsigned int level, const char *format, ... );
void NX_RelMsg( const char *format, ... );


//////////////////////////////////////////////////////////////////////////////
//
//					Player Internal Debug Message
//

#define NX_ErrMsg(A)	printf A

#define DBG_ERR			PLAYER_DBG_LEVEL_ERROR
#define DBG_WARN		PLAYER_DBG_LEVEL_WARNING
#define DBG_INFO		PLAYER_DBG_LEVEL_INFO
#define DBG_TRACE		PLAYER_DBG_LEVEL_TRACE
#define	DBG_VERBOSE		PLAYER_DBG_LEVEL_VERBOSE

#define DBG_MEMORY		DBG_TRACE

#endif	//	__NX_DebugMsg_h__
