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
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include "NX_DvrFileManager.h"

// #define FIXED_MAX_SIZE			(8 * 1024)

////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//
int32_t CheckDirectory( char *directory )
{
	int32_t ret = 0;
	struct stat stStat;

	if( !stat( directory, &stStat ) ) {
		if( !S_ISDIR( stStat.st_mode ) ) {
			printf("%s(): Fail, is not directory.\n", __func__);
			ret = -1;
		}
	} else {
		if( 0 > mkdir( directory, 0777 ) ) {
			printf("%s(): Fail, create directory.\n", __func__);
			ret = -1;
		}
	}
	
	return ret;
}

int64_t CheckFileSize( char *fileName )
{
	struct stat stStat;
	
	if( 0 > stat( fileName, &stStat ) ) {
		return -1;
	}

	return (int64_t)(stStat.st_size / 1024);	// KBytes
}

int64_t GetFileSystemSize( const char *pos ) 
{
	struct statfs stStatfs;
	int64_t size = 0;

	if( statfs( pos, &stStatfs ) ) {
		printf("%s(): Invalid system.\n", __func__);
		return -1;
	}
	size = stStatfs.f_blocks * (stStatfs.f_bsize / 1024);
	//printf("%s(): total block size of \"%s\" = %lld Kbytes\n", __func__, pos, size);
	
	return size;
}

int32_t RemoveFile( char *fileName )
{
	if( 0 > access(fileName, F_OK) ) {
		printf("%s is not exist!\n", fileName);
		return -1;
	}

	if( unlink(fileName) )
	{
		switch(errno)
		{
		case ENOENT :
			printf("is not entry. ( errno : %d )\n", ENOENT);
			break;
        case EACCES :
            printf("access deny. ( errno : %d )\n", EACCES);
            break;
        case ENOTDIR :
            printf("is not file, is directory. ( errno : %d )\n", ENOTDIR);
            break;
        case EROFS :
            printf("read only file. ( errno : %d )\n", EROFS);
            break;
        default :
			printf("unknown error. ( errno : %d )\n", errno);
        }   
    } else {
		//printf("remove file. ( %s )\n", fileName);
    }   

	sync();

	return 0;
}

int32_t CreateFileList( FILE_MANAGER_HANDLE hManager, const char *pFileExtension )
{
	DIR *dp;
	struct dirent *dirEntry = NULL;
	
	char fileName[1024];
	int64_t fileSize;

	if( (dp = opendir( hManager->topDir ) ) == NULL ){
		printf("%s(): Directory open failed (%s).\n", __func__, hManager->topDir);
		return -1;
	}

	while( ((dirEntry = readdir(dp))) != NULL )
	{
		if( (strlen(dirEntry->d_name) - strlen(pFileExtension)) == (uint8_t)(strstr(dirEntry->d_name, pFileExtension) - dirEntry->d_name)) {
			sprintf(fileName, "%s/%s", hManager->topDir, dirEntry->d_name);
			fileSize = CheckFileSize( fileName );
			DvrFileQueueInsert( hManager->hQueue, fileName, fileSize, 1 );
			
			hManager->curSize += fileSize;
		}
	}
	closedir( dp );

	//DvrFileQueueList( hManager->hQueue );

	return 0;
}


void *DvrFileManagerThread( void *arg )
{
	FILE_MANAGER_HANDLE hManager = (FILE_MANAGER_HANDLE)arg;
	
	char fileName[1024];
	uint64_t fileSize;
	
	while( hManager->bThreadRun )
	{
		pthread_mutex_lock( &hManager->hLock );
		if( hManager->maxSize < hManager->curSize ) {
			DvrFileQueuePop( hManager->hQueue, fileName, &fileSize );
			
			printf("%s(): remove old files. ( allow size : %lld Kbytes, current size : %lld Kbytes -> %lld Kbytes )\n", __func__, hManager->maxSize, hManager->curSize, hManager->curSize - fileSize); 
			hManager->curSize = hManager->curSize - fileSize;
			if( hManager->curSize < 0 ) hManager->curSize = 0;

			RemoveFile( fileName );
		}

		pthread_mutex_unlock( &hManager->hLock );

		usleep(10000);
	}

	return (void*)0xDEADDEAD;
}

////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
FILE_MANAGER_HANDLE	DvrFileManagerInit( const char *topDir, int32_t ratio, const char *pFileExtension )
{
	FILE_MANAGER_HANDLE hManager = (FILE_MANAGER_HANDLE)malloc( sizeof(FILE_MANAGER_INFO) );
	memset( hManager, 0x00, sizeof(FILE_MANAGER_INFO) );

	sprintf( hManager->topDir, "%s", topDir );
	if( 0 > CheckDirectory( hManager->topDir ) )
		goto ERROR;

	hManager->curSize		= 0;

#ifndef FIXED_MAX_SIZE
	hManager->maxSize		= (int64_t)((double)GetFileSystemSize( hManager->topDir ) * (double)((double)ratio / (double)100.));
#else
	hManager->maxSize		= FIXED_MAX_SIZE;
#endif

	if( hManager->maxSize <= 0 ) {
		printf("%s(): Fail, allow size error.( %lld )\n", __func__, hManager->maxSize);
		goto ERROR;
	}

	hManager->hQueue		= DvrFileQueueInit();

	hManager->hThread		= 0;
	hManager->bThreadRun	= 0;
	pthread_mutex_init( &hManager->hLock, NULL );

	// Create File List
	if( !hManager->hQueue )
		goto ERROR;

	CreateFileList( hManager, pFileExtension );

	printf("%s(): MaxSize = %lld, CurrnetSize = %lld\n", __func__, hManager->maxSize, hManager->curSize);

	return hManager;

ERROR :
	printf("%s(): Fail, Initialize.\n", __func__);
	pthread_mutex_destroy( &hManager->hLock );
	if( hManager->hQueue ) DvrFileQueueDeinit( hManager->hQueue );
	if( hManager ) free( hManager );

	return NULL;
}

void DvrFileManagerDeinit( FILE_MANAGER_HANDLE hManager )
{
	assert( hManager );
	pthread_mutex_destroy( &hManager->hLock );

	if( hManager->hQueue ) DvrFileQueueDeinit( hManager->hQueue );
	if( hManager ) free( hManager );
}

int32_t DvrFileManagerStart( FILE_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( hManager->bThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = true;
	if( 0 > pthread_create( &hManager->hThread, NULL, &DvrFileManagerThread, (void*)hManager) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

int32_t DvrFileManagerStop( FILE_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( !hManager->bThreadRun) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = false;
	pthread_join( hManager->hThread, NULL );
	
	return 0;
}

int32_t DvrFileManagerRegCmd( FILE_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	hManager->hCmd = hCmd;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrFileManagerPush( FILE_MANAGER_HANDLE hManager, char *pData )
{
	int64_t size = 0;
	
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	
	size = CheckFileSize(pData);

	DvrFileQueuePush(hManager->hQueue, pData, size);
	hManager->curSize += size;

	pthread_mutex_unlock( &hManager->hLock );
	return 0;
}
