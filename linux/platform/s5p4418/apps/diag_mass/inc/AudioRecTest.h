#ifndef __AUDIORECTEST_H__
#define __AUDIORECTEST_H__

#include <pthread.h>
#include <alsa/asoundlib.h>
#include <utils.h>

class CAudioRecTest
{
public:
	CAudioRecTest();
	~CAudioRecTest();

	bool Start();
	bool Stop();

	bool GetLastSumInfo( int &leftSum, int &rightSum)
	{
		CNX_AutoLock lock(&m_hMutex);
		if( m_hAudCapture )
		{
			leftSum = m_LeftSum;
			rightSum = m_RightSum;
		}
		else
		{
			return false;
		}
		return true;
	}

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CAudioRecTest *pObj = (CAudioRecTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

	bool InitAudCapture( );
	int	CaptureAudioSample( unsigned char *pBuf, int bufSize );
	void CloseAudCapture( void );

private:
	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
	snd_pcm_t		*m_hAudCapture;

	int				m_Channels;
	int				m_Frequency;

	int				m_LeftSum;
	int				m_RightSum;

};

#endif // !__AUDIORECTEST_H__
