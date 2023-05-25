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
#include <unistd.h>
#include <string.h>

#include <NX_DvrConfig.h>

DvrConfig gDvrConfig;

static char DvrConfigName[][32] = {
	"[Channel]",
	"[Preview]",
	"[Quality]",
	"[Hls]",
};

int32_t DvrConfigWriteDefault( const char *path )
{
	FILE *fp = NULL;

	memset( &gDvrConfig, 0x00, sizeof(DvrConfig) );

	if( NULL == (fp = fopen(path, "w")) ) {
		printf("%s(): %s open fail\n", __func__, path);
		return -1;
	}

	for(uint32_t i = 0; i < sizeof(DvrConfigName) / sizeof(DvrConfigName[0]); i++ )
	{
		fprintf(fp, "%s\t0\n", DvrConfigName[i]);
	}
	fclose( fp );
	return 0;
}

int32_t DvrConfigWrite( const char *path )
{
	FILE *fp = NULL;

	if( NULL == (fp = fopen(path, "w")) ) {
		printf("%s(): %s open fail\n", __func__, path);
		return -1;
	}

	fprintf( fp, "%s\t%d\n", DvrConfigName[0], gDvrConfig.nChannel );
	fprintf( fp, "%s\t%d\n", DvrConfigName[1], gDvrConfig.nPreview );
	fprintf( fp, "%s\t%d\n", DvrConfigName[2], gDvrConfig.nEncQuality );
	fprintf( fp, "%s\t%d\n", DvrConfigName[3], gDvrConfig.bHls );
	fclose( fp );

	printf("%s(): ch(%d), preview(%d), quality(%d), hls(%d)\n", 
		__func__, gDvrConfig.nChannel, gDvrConfig.nPreview, gDvrConfig.nEncQuality, gDvrConfig.bHls );

	return 0;
}

int32_t DvrConfigRead( const char *path  )
{
	FILE *fp = NULL;
	char configName[32];
	int32_t configValue;

	memset( &gDvrConfig, 0x00, sizeof(DvrConfig) );

	if( access(path, F_OK) ) {
		printf("%s(): Configuration file is not exist.\n", __func__);
		return -1;
	}
	
	if( NULL == (fp = fopen(path, "r")) ) {
		printf("%s(): %s open fail\n", __func__, path);
		return -1;
	}

	for(uint32_t i = 0; i < sizeof(DvrConfigName) / sizeof(DvrConfigName[0]); i++ )
	{
		fscanf( fp, "%s%d", configName, &configValue );

		if( !strcmp(configName, DvrConfigName[0]) ) {
			gDvrConfig.nChannel = configValue;
		}
		else if( !strcmp(configName, DvrConfigName[1]) ) {
			gDvrConfig.nPreview = configValue;
		}
		else if( !strcmp(configName, DvrConfigName[2]) ) {
			gDvrConfig.nEncQuality = configValue;
		}
		else if( !strcmp(configName, DvrConfigName[3]) ) {
			gDvrConfig.bHls= configValue;
		}
		else {
			printf("%s(): Unknown Config value.\n", __func__);
			fclose( fp );
			return -1;
		}
	}

	printf("%s(): ch(%d), preview(%d), quality(%d), hls(%d)\n", 
		__func__, gDvrConfig.nChannel, gDvrConfig.nPreview, gDvrConfig.nEncQuality, gDvrConfig.bHls );

	fclose( fp );

	return 0;
}
