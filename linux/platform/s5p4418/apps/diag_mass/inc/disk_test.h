#ifndef __DISK_TEST_H__
#define	__DISK_TEST_H__

#include <stdlib.h>
#include <string.h>

#include <utils.h>

typedef enum {
	NAND_STORAGE,
	SD_STORAGE,
	USB_STORAGE,
} STORAGE_TYPE;

static const char *st_StorageNameStr[] = {
	"mtdblk",
	"mmcblk",
	"sda",
};

class StorageTestUtil
{
public:
	StorageTestUtil()
	{
	}
	~StorageTestUtil()
	{
	}

	static bool GetDeviceSize( STORAGE_TYPE type, int index, long long &size )
	{
		char sizePath[128];
		char blockSizePath[128];
		char sizeString[64];
		long long numBlocks;
		int blockSize;

		if( type == USB_STORAGE )
		{
			snprintf( sizePath, sizeof(sizePath), "/sys/block/%s/size", st_StorageNameStr[type] );
			snprintf( blockSizePath, sizeof(blockSizePath), "/sys/block/%s/queue/physical_block_size", st_StorageNameStr[type] );
		}
		else
		{
			snprintf( sizePath, sizeof(sizePath), "/sys/block/%s%d/size", st_StorageNameStr[type], index );
			snprintf( blockSizePath, sizeof(blockSizePath), "/sys/block/%s%d/queue/physical_block_size", st_StorageNameStr[type], index );
		}

		//printf("size Path      : %s\n", sizePath);
		//printf("blockSize Path : %s\n", blockSizePath);

		if( !ReadSysFileInfo( sizePath, sizeString, sizeof(sizeString) ) )
		{
			return false;
		}
		numBlocks = atoll(sizeString);
		if( !ReadSysFileInfo( blockSizePath, sizeString, sizeof(sizeString) ) )
		{
			return false;
		}
		blockSize = atoi(sizeString);
		size = numBlocks * (long long)blockSize;

		//printf("Size(%lld), numBlocks(%lld), blockSize(%d)\n", size, numBlocks, blockSize);
		return true;
	}

private:
	char *m_pFilePath;
};

#endif	//	__DISK_TEST_H__
