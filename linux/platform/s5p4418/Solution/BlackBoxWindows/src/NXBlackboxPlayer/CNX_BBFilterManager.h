#pragma once

#include <streams.h>

#define	MAX_VIDEO_TRACKS	5

#define	EVT_CODE_EOS		EC_COMPLETE

class CNX_BBFilterManager
{
public:
	//
	CNX_BBFilterManager( int FileType = FILE_TYPE_MP4 );
	virtual ~CNX_BBFilterManager();

	//	Init & Close
	int Init( const char *fileName, OAHWND hEventOwner, unsigned long eventID, OAHWND hFirstOwner, OAHWND hSecondOwner, OAHWND hThirdOwner );
	int Close( void );

	//	Play Control
	int Play( void );
	int Stop( void );
	int Pause( void );
	int FrameStep( int nStep );

	//	Stream Control & Seeking
	int Seek( unsigned long time );					//	1/1000 second
	int GetDuration( unsigned long *time );			//	1/1000 second
	int GetCurTime( unsigned long *time );			//	i/1000 second

	//	Volume Control
	int GetVolume( long *volume );
	int SetVolume( long volume );

	//	Event Proc
	int FilterEventProc( unsigned int *eventCode );
	void Move( OAHWND hWnd, int x, int y );

	//	Set Window Pos
	//	channel == 0 (Front), channel==1 (Rear)
	int SetWindowPosition( int channel, int x, int y, int width, int height );
	int SetFullScreen( int channel, BOOL bEnable );
	int SetAspectRatio( int channel, BOOL bEnable );

	enum { FILE_TYPE_MP4, FILE_TYPE_MP2TS };

private:
	HRESULT LoadFilter( IBaseFilter **pFilter, REFCLSID clsid, LPCWSTR pName );
	HRESULT RenderFilter( IBaseFilter *pFilter );
	HRESULT	FindVideoWindowInterface ( IBaseFilter *pFilter, IVideoWindow **ppIVW );

	IGraphBuilder	*m_pGB;		//	Filter Graph
	IMediaControl	*m_pMC;		//	Basic Media Control
	IMediaEventEx	*m_pME;		//	Media Event
	IMediaSeeking	*m_pMS;
	IVideoWindow	*m_pVW;		//	Video Window Interface for First Window
	IVideoWindow	*m_pVW2;	//	Video WIndow Interface for Second Window
	IVideoWindow	*m_pVW3;	//	Video WIndow Interface for Third Window
	IBasicAudio		*m_pBA;		//	Basic Audio Interface   (Volume Control)

	//	Async Reader & MP4 Demuxer Filter
	IBaseFilter			*m_pASyncReader;
	IBaseFilter			*m_pDemuxFilter;

	//	Video Decode Filter
	IBaseFilter			*m_pVideoDecFilter[MAX_VIDEO_TRACKS];
	IBaseFilter			*m_pAudioDecFilter;

	int					m_NumVideoTrack;
	bool				m_bPaused;
	int					m_FileType;
};
