#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <AudioRecTest.h>
#include <utils.h>


CAudioRecTest::CAudioRecTest()
	: m_bRunning(false)
	, m_hAudCapture(NULL)
	, m_Channels(2)
	, m_Frequency(8000)
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CAudioRecTest::~CAudioRecTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CAudioRecTest::Start()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		return false;
	}

	m_bRunning = true;

	InitAudCapture();

	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}
	return true;
}

bool CAudioRecTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}

	CloseAudCapture();
	return true;
}


void CAudioRecTest::ThreadProc()
{
	short data[1024];	//	4K
	int captureByte;
	short *pSample;

	unsigned int leftSum, rightSum;

	while(m_bRunning)
	{
		captureByte = CaptureAudioSample( (unsigned char*)data, sizeof(data) );

		leftSum = rightSum = 0;
		if( captureByte > 3 )
		{
			CNX_AutoLock lock(&m_hMutex);
			pSample = data;
			for( int i=0 ; i < captureByte/2/m_Channels ; i++ )
			{
				leftSum += abs(*pSample++);
				rightSum += abs(*pSample++);
			}
			leftSum  /= captureByte/2/m_Channels;
			rightSum /= captureByte/2/m_Channels;
			m_LeftSum  = leftSum;
			m_RightSum = rightSum;
		}
//		printf("leftSum = %d, rightSum = %d\n", leftSum, rightSum);
    }
}

//------------------------------------------------------------------------------
#define	AUD_CAP_DEVICE_NAME		"default"

bool CAudioRecTest::InitAudCapture( )
{
	int err;
	snd_pcm_hw_params_t *hw_params = NULL;
	unsigned int channels = m_Channels;
	unsigned int frequency = m_Frequency;

	if ((err = snd_pcm_open (&m_hAudCapture, AUD_CAP_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		printf( "cannot open audio device %s (%s)\n", AUD_CAP_DEVICE_NAME, snd_strerror(err) );
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{
		printf( "cannot allocate hardware parameter structure (%s)\n", snd_strerror(err) );
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_any (m_hAudCapture, hw_params)) < 0)
	{
		printf( "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_access (m_hAudCapture, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		printf( "cannot set access type (%s)\n", snd_strerror (err) );
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_format (m_hAudCapture, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
	{
		printf( "cannot set sample format (%s)\n", snd_strerror (err));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (m_hAudCapture, hw_params, &frequency, 0)) < 0)
	{
		printf( "cannot set sample rate (%s)\n", snd_strerror (err) );
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_channels (m_hAudCapture, hw_params, channels)) < 0)
	{
		printf( "cannot set channel count (%s)\n", snd_strerror (err));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params (m_hAudCapture, hw_params)) < 0)
	{
		printf( "cannot set parameters (%s)\n", snd_strerror (err));
		goto error_exit;
	}
	snd_pcm_hw_params_free (hw_params);

	if( (err = snd_pcm_prepare( m_hAudCapture )) <0 )
	{
		printf( "cannot prepare audio interface for use (%s)\n", snd_strerror(err));

	}
	return true;

error_exit:
	if( hw_params )
		snd_pcm_hw_params_free (hw_params);
	
	if( m_hAudCapture ) {
		snd_pcm_close(m_hAudCapture);
		m_hAudCapture = NULL;
	}
	return false;
}

//------------------------------------------------------------------------------
int	CAudioRecTest::CaptureAudioSample( unsigned char *pBuf, int bufSize )
{
	int totalReadSize = 0, readSize;

	if( !m_hAudCapture )
		return -1;

	bufSize /= ( 2 * m_Channels );
	while( bufSize > 0 )
	{
		readSize = snd_pcm_readi( m_hAudCapture, pBuf, bufSize );
		if( readSize < 0 ) {
			printf( "snd_pcm_readi() Failed. ( %s )\n", snd_strerror(readSize) );
			return 0;
		}
		totalReadSize += readSize;
		bufSize -= (readSize);
		pBuf += (readSize * 2 * m_Channels);
	}
	return totalReadSize * 2 * m_Channels;
}

//------------------------------------------------------------------------------
void CAudioRecTest::CloseAudCapture( void )
{
	if( m_hAudCapture )
	{
		snd_pcm_close( m_hAudCapture );
		m_hAudCapture = NULL;
	}
}
