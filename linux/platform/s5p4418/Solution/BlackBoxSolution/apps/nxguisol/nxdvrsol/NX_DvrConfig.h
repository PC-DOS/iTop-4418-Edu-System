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

#ifndef __NX_DVRCONFIG_H__
#define __NX_DVRCONFIG_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct tagDvrConfig {
	int32_t		nChannel;
	int32_t		nPreview;
	int32_t		nEncQuality;
	int32_t		bHls;
} DvrConfig;

extern DvrConfig gDvrConfig;

int32_t DvrConfigWriteDefault( const char *path );

int32_t DvrConfigWrite( const char *path );
int32_t DvrConfigRead( const char *path );

#endif	// __NX_DVRCONFIG_H__