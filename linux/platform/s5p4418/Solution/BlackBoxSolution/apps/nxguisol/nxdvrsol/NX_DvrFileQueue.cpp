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
#include <assert.h>
#include <sys/stat.h>

#include "NX_DvrFileQueue.h"

////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
QHANDLE DvrFileQueueInit( void )
{
	QHANDLE hQueue	= (QHANDLE)malloc( sizeof(QHANDLE_INFO) );

	hQueue->front	= NULL;
	hQueue->rear	= NULL;
	hQueue->nodeNum	= 0;
	
	pthread_mutex_init( &hQueue->hMutex, NULL );

	return hQueue;
}

int32_t DvrFileQueueDeinit( QHANDLE hQueue )
{
	QNODE *pNode;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );

	while( hQueue->nodeNum )
	{
		pNode = hQueue->front;
		if( hQueue->front->next == NULL ) {
			hQueue->front	= NULL;
			hQueue->rear	= NULL;
		}
		else {
			hQueue->front	= hQueue->front->next;
		}
		hQueue->nodeNum--;
		//printf("free node. (%p)\n", pNode);
		free( pNode );		
	}
	if( hQueue ) free(hQueue);
	pthread_mutex_unlock( &hQueue->hMutex );

	return 0;
}

int32_t DvrFileQueuePush( QHANDLE hQueue, char *pFileName, uint64_t fileSize )
{
	QNODE *pNode;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );
	
	pNode = (QNODE*)malloc( sizeof(QNODE) );
	sprintf(pNode->fileName, "%s", pFileName);
	pNode->fileSize = fileSize;
	pNode->next		= NULL;
	
	if( hQueue->front == NULL ) {
		hQueue->front	= pNode;
		hQueue->rear	= pNode;
	}
	else {
		hQueue->rear->next = pNode;
		hQueue->rear = pNode;
	}

	hQueue->nodeNum++;
	//printf("front( %p ), front-next( %p ), rear( %p ), rear-next( %p ), num( %d )\n", hQueue->front, hQueue->nodeNum ? hQueue->front->next : NULL, hQueue->rear, hQueue->nodeNum ? hQueue->rear->next : NULL, hQueue->nodeNum );

	pthread_mutex_unlock( &hQueue->hMutex );
	return 0;
}

int32_t	DvrFileQueuePop( QHANDLE hQueue, char *pFileName, uint64_t *pFileSize )
{
	QNODE *pNode;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );

	if( !hQueue->nodeNum ) {
		printf("%s(): Queue is empty.\n", __func__);
		pthread_mutex_unlock( &hQueue->hMutex );
		return -1;
	}

	pNode = hQueue->front;
	sprintf(pFileName, "%s", pNode->fileName);
	*pFileSize = pNode->fileSize;

	if( hQueue->front->next == NULL ) {
		hQueue->front	= NULL;
		hQueue->rear	= NULL;
	}
	else {
		hQueue->front	= hQueue->front->next;
	}
	hQueue->nodeNum--;
	//printf("front( %p ), front-next( %p ), rear( %p ), rear-next( %p ), num( %d )\n", hQueue->front, hQueue->nodeNum ? hQueue->front->next : NULL, hQueue->rear, hQueue->nodeNum ? hQueue->rear->next : NULL, hQueue->nodeNum );

	free( pNode );

	pthread_mutex_unlock( &hQueue->hMutex );
	return 0;
}

// Sorting insert
int32_t DvrFileQueueInsert( QHANDLE hQueue, char *pFileName, uint64_t fileSize, int32_t bSort )
{
	QNODE *pNode, *pCurNode;
	uint32_t i;
	int32_t ret = 0;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );
	
	pNode = (QNODE*)malloc( sizeof(QNODE) );
	sprintf(pNode->fileName, "%s", pFileName);
	pNode->fileSize = fileSize;
	pNode->next = NULL;

	if( hQueue->front == NULL ) {
		hQueue->front	= pNode;
		hQueue->rear	= pNode;
		hQueue->nodeNum++;
	}
	else {
		pCurNode = hQueue->front;	// first element

		for( i = 0; i < hQueue->nodeNum; i++ )
		{
			ret = bSort * strcmp( pNode->fileName, pCurNode->fileName );

			if( ret == 0 ) {
				printf("%s(): Fail, same file name.\n", __func__);
				break;
			} else if( ret > 0 ) {
				if( pCurNode->next == NULL ) {
					pCurNode->next = pNode;
					hQueue->rear = pNode;
					hQueue->nodeNum++;
					break;
				} else {
					ret = bSort * strcmp( pNode->fileName, pCurNode->next->fileName );
					if( ret < 0 ) {
						pNode->next = pCurNode->next;
						pCurNode->next = pNode;
						hQueue->nodeNum++;
						break;
					}
				}
			} else {
				pNode->next = pCurNode;
				hQueue->front = pNode;
				hQueue->nodeNum++;
				break;
			}

			pCurNode	= pCurNode->next;
		}
	}
	//printf("front( %p ), front-next( %p ), rear( %p ), rear-next( %p ), num( %d )\n", hQueue->front, hQueue->nodeNum ? hQueue->front->next : NULL, hQueue->rear, hQueue->nodeNum ? hQueue->rear->next : NULL, hQueue->nodeNum );
	pthread_mutex_unlock( &hQueue->hMutex );
	return 0;
}

int32_t DvrFileQueueNum( QHANDLE hQueue )
{
	int32_t num;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );

	num = hQueue->nodeNum;

	pthread_mutex_unlock( &hQueue->hMutex );
	
	return num;
}

int32_t	DvrFileQueueIsEmpty( QHANDLE hQueue )
{
	int32_t ret = 0;
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );

	if( !hQueue->nodeNum ) ret = 1;
	
	pthread_mutex_unlock( &hQueue->hMutex );
	return ret;
}

// for debugging
int32_t DvrFileQueueList( QHANDLE hQueue )
{
	QNODE *pNode;
	uint32_t i;
	
	assert( hQueue );
	pthread_mutex_lock( &hQueue->hMutex );
	
	pNode = hQueue->front; // first element
	for(i = 0; i < hQueue->nodeNum; i++)
	{
		printf("buffer( %p ) : %s ( %lld Kbytes )\n", pNode, pNode->fileName, pNode->fileSize);
		pNode = pNode->next;
	}

	pthread_mutex_unlock( &hQueue->hMutex );
	return 0;
}
