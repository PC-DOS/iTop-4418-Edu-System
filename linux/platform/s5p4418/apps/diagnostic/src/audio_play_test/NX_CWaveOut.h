/*
 * (C) Copyright 2010
 * SeongO Park, Nexell Co, <ray@nexell.co.kr>
 */

#ifndef __NX_CWaveOut_h__
#define	__NX_CWaveOut_h__

#include <alsa/asoundlib.h>


//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Alsa WaveOut Module
//
class NX_CWaveOut{
public:
	NX_CWaveOut();
	virtual ~NX_CWaveOut();

public:
	int		WaveOutOpen( unsigned int sampleFreq, int channeles, unsigned int numBuf, unsigned int sizeBuf, int useMixer );
	int		WaveOutPlayBuffer( unsigned char *pBuf, int size, int &error );
	int		WaveOutReset();							//	Stop Play & Reset Buffer
	void	WaveOutClose();
	void	WaveOutFlush();
	void	WaveOutPrepare();
	long	WaveOutGetAvailBufSize();

	void	WriteDummy( unsigned int time );		//	Milli-Seconds
	long long FindDuration( unsigned int Size );	//	duration
	long long GetBufferedTime();
private:
//	unsigned char			m_Dummy[1024*4];
	snd_pcm_t*				m_hPlayback;
	snd_pcm_hw_params_t*	m_pHWParam;
	snd_pcm_status_t*		m_pStatus;
	unsigned int			m_Periods;
	unsigned int			m_PeriodBufSize;
	unsigned int			m_SamplingFreq;
	float					m_TimePerSample;
	int						m_Channels;
	snd_pcm_uframes_t		m_MaxAvailBuffer;

};
//
//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__NX_CWaveOut_h__
