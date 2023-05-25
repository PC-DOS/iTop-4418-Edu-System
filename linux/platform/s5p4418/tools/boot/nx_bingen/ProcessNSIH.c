#include <stdio.h>

int ProcessNSIH (const char *pfilename, unsigned char *pOutData)
{
	FILE *fp;
	char ch;
	int writesize, skipline, line, bytesize, i;
	unsigned int writeval;

	fp = fopen (pfilename, "rb");
	if (!fp)
	{
		printf ("ProcessNSIH : ERROR - Failed to open %s file.\n", pfilename);
		return 0;
	}

	bytesize = 0;
	writeval = 0;
	writesize = 0;
	skipline = 0;
	line = 0;

	while (0 == feof (fp))
	{
		ch = fgetc (fp);

		if (skipline == 0)
		{
			if (ch >= '0' && ch <= '9')
			{
				writeval = writeval * 16 + ch - '0';
				writesize += 4;
			}
			else if (ch >= 'a' && ch <= 'f')
			{
				writeval = writeval * 16 + ch - 'a' + 10;
				writesize += 4;
			}
			else if (ch >= 'A' && ch <= 'F')
			{
				writeval = writeval * 16 + ch - 'A' + 10;
				writesize += 4;
			}
			else
			{
				if (writesize == 8 || writesize == 16 || writesize == 32)
				{
					for (i=0 ; i<writesize/8 ; i++)
					{
						pOutData[bytesize++] = (unsigned char)(writeval & 0xFF);
						writeval >>= 8;
					}
				}
				else
				{
					if (writesize != 0)
						printf ("ProcessNSIH : Error at %d line.\n", line+1);
				}

				writesize = 0;
				skipline = 1;
			}
		}

		if (ch == '\n')
		{
			line++;
			skipline = 0;
			writeval = 0;
		}
	}

	printf ("ProcessNSIH : %d line processed.\n", line+1);
	printf ("ProcessNSIH : %d bytes generated.\n", bytesize);

	fclose (fp);

	return bytesize;
}
