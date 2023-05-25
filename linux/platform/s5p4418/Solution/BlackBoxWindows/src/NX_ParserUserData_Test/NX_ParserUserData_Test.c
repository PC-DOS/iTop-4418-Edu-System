#include <stdio.h>
#include <stdlib.h>

//parser userdata
#include "NX_ParserUserData.h"

int parser_file_read(__int64 Pos, long Len, unsigned char *pBuf);
int parser_file_len(LONGLONG *Total, LONGLONG *Avail);


static FILE *ParserFile;

int parser_file_read(__int64 Pos, long Len, unsigned char *pBuf)
{
	if (ParserFile != 0)	{
		fseek(ParserFile, (long)Pos, SEEK_SET);
	}

	fread(pBuf, 1, Len, ParserFile);

  	return 0;
}

int parser_file_len(LONGLONG *Total, LONGLONG *Avail)
{
	U32 size;
	fseek(ParserFile, 0, SEEK_END);
	size = ftell(ParserFile);
	fseek(ParserFile, 0, SEEK_SET);

	*Total = size;
	*Avail = size;
	return 0;

}

void main(int argc, char* argv[])
{
	int parser_handle =0;
	int max_buf_size_parser =64*1024; 
	unsigned long Size = 0, ret = 0;
	unsigned long long	TimeStamp = 0;
	unsigned char *p_buf = NULL;
	char * file_name = NULL;

	PARSER_INFO	ParserInfo;

	if (argc == 1) 
	{
		printf("Input Error!!!\n");
		exit(1);
	}

	memset(&ParserInfo, 0, sizeof(PARSER_INFO));
	file_name = argv[1];

	fopen_s(&ParserFile, file_name, "rb");
	parser_handle = NXParserUserDataInit(file_name, max_buf_size_parser);

	if((ret = NXParserUserDataWrapFunc(parser_handle, parser_file_read, parser_file_len)) != 0)
	{
		printf("NXParserUserDataWrapFunc ERROR !!!\n");
	}

	if((ret = NXParserUserDataGetInfo(parser_handle, &ParserInfo)) != 0)
	{
		printf("NXParserUserDataWrapFunc ERROR !!!\n");
	}
	printf("------Parser Info------\n");
	printf("Total Num : %d \n", ParserInfo.TrackNum);
	printf("Audio Num : %d \n", ParserInfo.AudioTrackNum);
	printf("Video Num : %d \n", ParserInfo.VideoTrackNum);
	printf("TEXT  Num : %d \n", ParserInfo.TextTrackNum);
	printf("-----------------------\n");

	while(1)
	{
		if((ret = NXParserGetIndex(parser_handle, &Size, &TimeStamp)) != 0)
		{
			printf("End Of Stream !!!\n");
			break;
		}

		p_buf = (unsigned char *)malloc(Size + 1);
		memset(p_buf, 0, Size + 1);

		if((ret = NXParserUserDataGetFrame(parser_handle, &p_buf, &Size, &TimeStamp)) != 0) 
			printf("NXParserUserDataWrapFunc ERROR !!!\n");

		printf("-user data = %s\n", p_buf);

		free(p_buf);
	}

	NXParserUserDataClose(parser_handle);
}