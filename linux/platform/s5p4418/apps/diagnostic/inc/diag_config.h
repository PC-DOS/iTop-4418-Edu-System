#ifndef __DIAG_CONFIG_H__
#define	__DIAG_CONFIG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	ENV_DIAG_PATH			"NX_DIAG_PATH"
#define	ENV_DIAG_LIB			"NX_DIAG_LIB"
#define	ENV_DIAG_FONT			"NX_DIAG_FONT"
#define	ENV_DIAG_FONT_SIZE		"NX_DIAG_FONT_SIZE"
#define	ENV_DIAG_DATA			"NX_DIAG_DATA"
#define	ENV_DIAG_OUTPUT			"NX_DIAG_OUTPUT"

class NX_DiagConfig
{
public:
	NX_DiagConfig()
		: m_DiagPath(NULL)
		, m_LibPath(NULL)
		, m_FontPath(NULL)
		, m_Font(NULL)
		, m_FontSize(-1)
		, m_OutPath(NULL)
	{
	}
	~NX_DiagConfig()
	{
		if( m_DiagPath )	free( m_DiagPath );
		if( m_LibPath )		free( m_LibPath );
		if( m_Font )		free( m_Font );
	}
	char *GetDiagPath()
	{
		if( m_DiagPath == NULL )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_PATH);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_PATH );
			else
				m_DiagPath = strdup( env_value );
		}
		return m_DiagPath;
	}
	char *GetLibPath()
	{
		if( m_LibPath == NULL )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_LIB);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_LIB );
			else
				m_LibPath = strdup( env_value );
		}
		return m_LibPath;
	}
	char *GetFont()
	{
		if( m_Font == NULL )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_FONT);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_FONT );
			else
				m_Font = strdup( env_value );
		}
		return m_Font;
	}
	int GetFontSize()
	{
		if( m_FontSize == -1 )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_FONT_SIZE);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_FONT_SIZE );
			else
				m_FontSize = atoi( env_value );
		}

		return m_FontSize;
	}
	char *GetDiagOutPath()
	{
		if( m_OutPath == NULL )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_OUTPUT);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_OUTPUT );
			else
				m_OutPath = strdup( env_value );
		}
		return m_OutPath;
	}
	char *GetDiagDataPath()
	{
		if( m_DataPath == NULL )
		{
			char *env_value;
			env_value = getenv(ENV_DIAG_DATA);
			if( NULL == env_value )
				printf("Cannot Found : %s environment parameter!!!\n", ENV_DIAG_DATA );
			else
				m_DataPath = strdup( env_value );
		}
		return m_DataPath;
	}

private:
	char	*m_DiagPath;
	char	*m_LibPath;
	char	*m_FontPath;
	char	*m_Font;
	int		m_FontSize;
	char	*m_DataPath;
	char	*m_OutPath;
};


#endif //	__DIAG_CONFIG_H__
