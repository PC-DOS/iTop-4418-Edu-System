#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



/**
 * this code from u-boot/common/decompress_ext4.h,c
*/
struct Ext4FileHeader {
	unsigned int magic;
	unsigned short major;
	unsigned short minor;
	unsigned short fileHeaderSize;
	unsigned short chunkHeaderSize;
	unsigned int blockSize;
	unsigned int totalBlocks;
	unsigned int totalChunks;
	unsigned int crc32;
};

struct Ext4ChunkHeader {
	unsigned short type;
	unsigned short reserved;
	unsigned int chunkSize;
	unsigned int totalSize;
};
#define EXT4_FILE_HEADER_MAGIC  0xED26FF3A
#define EXT4_FILE_HEADER_MAJOR  0x0001
#define EXT4_FILE_HEADER_MINOR  0x0000
#define EXT4_FILE_BLOCK_SIZE    0x1000

#define EXT4_FILE_HEADER_SIZE   (sizeof(struct Ext4FileHeader))
#define EXT4_CHUNK_HEADER_SIZE  (sizeof(struct Ext4ChunkHeader))

#define EXT4_CHUNK_TYPE_RAW     0xCAC1
#define EXT4_CHUNK_TYPE_FILL    0xCAC2
#define EXT4_CHUNK_TYPE_NONE    0xCAC3

#define SECTOR_BITS             9/* 512B */
#if(1)
	#define ext4_printf printf
#else	
	#define ext4_printf(msg...) do { }while(0)
#endif
int writeCompressedExt4Image(char *imgBase, size_t imgSize, FILE *file)
{
    unsigned int sectorSize;
    int totalChunks;
    struct Ext4ChunkHeader *chunkHeader;
    struct Ext4FileHeader *fileHeader;
    char *base = imgBase;
    size_t writeCount;

    fileHeader = (struct Ext4ChunkHeader *)base;
    totalChunks = fileHeader->totalChunks;

    ext4_printf("%s: total chunk = %d\n", __func__, totalChunks);
//printf("type : %x \n",chunkHeader->type);
    base += EXT4_FILE_HEADER_SIZE;
    while (totalChunks) {
         chunkHeader = (struct Ext4ChunkHeader *)base;
         //sectorSize = (chunkHeader->chunkSize * fileHeader->blockSize) >> SECTOR_BITS;
         sectorSize = (chunkHeader->chunkSize * fileHeader->blockSize);
         ext4_printf("sectorSize: %d\n", sectorSize);
         switch (chunkHeader->type) {
         case EXT4_CHUNK_TYPE_RAW:
             ext4_printf("raw chunk\n");
             writeCount = fwrite(base + EXT4_CHUNK_HEADER_SIZE, sectorSize, 1, file);
             if (writeCount != 1) {
                  printf("%s: write error, mismatching count %d/%d\n", __func__, 1, writeCount);
                  return -EIO;
             }
             break;

         case EXT4_CHUNK_TYPE_FILL:
             ext4_printf("fill chunk\n");
             fseek(file, sectorSize, SEEK_CUR);
             break;

         case EXT4_CHUNK_TYPE_NONE:
             ext4_printf("none chunk\n");
             fseek(file, sectorSize, SEEK_CUR);
             break;

         default:
             ext4_printf("unknown chunk\n");
             fseek(file, sectorSize, SEEK_CUR);
             break;
         }

         totalChunks--;
         base += chunkHeader->totalSize;
    }

    printf("%s: write done\n", __func__);
    return 0;
}

int UpdateBootImage(char *imgBase, size_t imgSize, char *blockLocation)
{
    FILE *file = fopen(blockLocation, "w");
    int ret = 0;
		printf("%s \n",blockLocation);
    if (file == NULL) {
    	
        printf("%s: Error open %s\n", __func__, blockLocation);
        return -EINVAL;
    }

    ret = writeCompressedExt4Image(imgBase, imgSize, file);
    printf("1\n");
    fsync(fileno(file));
    printf("2\n");
    fclose(file);
   
    return ret;
}
