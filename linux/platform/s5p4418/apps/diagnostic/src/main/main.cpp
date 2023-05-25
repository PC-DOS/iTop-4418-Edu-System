#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <SDL/SDL.h>

#include <fcntl.h>
#include <unistd.h>

#include <time.h>
#include <sys/mount.h>

#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#include <nx_diag_type.h>
#include <base_app_window.h>

#include "top_window.h"


#ifndef MAX_PATH
#define	MAX_PATH 2048
#endif
#define LOG_FILE "/mnt/mmc/result.log"

#define FBNODE "/dev/fb0"

NXDiagAppType *st_AppList = NULL;

#define	MAX_PLUGINS			64
static int g_NumPlugins = 0;
NXDiagPluginInfo *g_PluginInfo[MAX_PLUGINS];


int find_diag_plugin( const char *path )
{
	char popenCmd[1024];
	char lineBuf[1024];
	char *surfix;
	int len;
	FILE *pipeFd;
	NXDiagPluginInfo*	(*getInfo)(void) = NULL;

	sprintf( popenCmd, "find %s", path );
	pipeFd = popen( popenCmd, "r" );

	if( pipeFd )
	{
		while( fgets(lineBuf, sizeof(lineBuf), pipeFd ) != NULL )
		{
			len = strlen( lineBuf );
			if( lineBuf[len-1] == '\n' )
			{
				lineBuf[len-1] = '\0';
				len -= 1;
			}
			if( len < 4 )
				continue;

			surfix = lineBuf + len - 3;
			if( 0 == strncmp( surfix, ".so", 3 )  )
			{
				void *soHandle;
				printf("soHandle = %s\n", lineBuf);
				soHandle = dlopen( lineBuf, RTLD_LAZY );
				if( soHandle )
				{
					getInfo = (GetPluginInfo)dlsym( soHandle, PLUGIN_FUNC_SYMBOL );
					printf("soHandle = %p, getInfo = %p, %s\n", soHandle, getInfo, PLUGIN_FUNC_SYMBOL);
					if( getInfo != NULL )
					{
						NXDiagPluginInfo *pInfo = getInfo();
						if( pInfo )
						{
							printf("Found Diagnostic Plugin :\n");
							printf("  file : %s\n", lineBuf);
							printf("  name : %s\n", pInfo->name);
							if( pInfo->testDiag )
							{
								//	TODO : Registration Plugin Info
								//		   Registration Items : name, description, fileName
								g_PluginInfo[g_NumPlugins] = (NXDiagPluginInfo *)malloc(sizeof(NXDiagPluginInfo));
								memcpy( g_PluginInfo[g_NumPlugins], pInfo, sizeof(NXDiagPluginInfo) );
								strcpy( g_PluginInfo[g_NumPlugins]->fileName, lineBuf );
								g_NumPlugins ++;
							}

						}
					}
					dlclose( soHandle );
				}
			}
		}
		fclose( pipeFd );
	}
	return 0;
}


int main( int argc, char *argv[] )
{
	if( argc != 2 ){
		printf("Error : Usage : %s [plugin bin dir]\n", argv[0]);
		return -1;
	}

	find_diag_plugin( argv[1] );

	TopWindow *topWind = new TopWindow( g_NumPlugins, g_PluginInfo );
	topWind->EventLoop();
	delete topWind;
	
	return 0;
}


