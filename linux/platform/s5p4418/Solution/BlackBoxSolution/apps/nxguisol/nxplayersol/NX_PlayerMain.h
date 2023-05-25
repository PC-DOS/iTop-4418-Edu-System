//------------------------------------------------------------------------------
//
//	Copyright (C) 2014 Nexell Co. All Rights Reserved
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

#ifndef __NX_PLAYERMAIN_H__
#define __NX_PLAYERMAIN_H__

int32_t PlayerStart( const char *filename );
int32_t PlayerStop( void );
int32_t PlayerPause( void );
int32_t PlayerSeek( uint32_t position );

int32_t PlayerGetPos( uint32_t *position, uint32_t *duration );

#endif	// __NX_PLAYERMAIN_H__