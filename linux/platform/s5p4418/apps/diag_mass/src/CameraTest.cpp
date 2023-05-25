#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <nx_fourcc.h>
#include <nx_vip.h>
#include <nx_dsp.h>

#include <utils.h>
#include <CameraTest.h>


#define	CAP_BUFFERS		4
#define	CAP_WIDTH		1024
#define	CAP_HEIGHT		768

#define	MAX_CAMERA_PORTS 2
static const int CameraPortList[MAX_CAMERA_PORTS] =
{
	VIP_PORT_0,
	VIP_PORT_MIPI
};

CCameraTest::CCameraTest()
	: m_CameraIndex(0)
	, m_CameraPort(CameraPortList[m_CameraIndex])
	, m_DspX(0)
	, m_DspY(0)
	, m_DspWidth(320)
	, m_DspHeight(240)
	, m_bRunning(false)
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CCameraTest::~CCameraTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

void CCameraTest::ThreadProc()
{
	//	Camera
	VIP_HANDLE hVip;
	VIP_INFO vipInfo;

	//	Display
	DISPLAY_HANDLE hDsp;
	DISPLAY_INFO dspInfo;

	NX_VID_MEMORY_HANDLE hMem[CAP_BUFFERS];

	NX_VID_MEMORY_INFO *pPrevDsp = NULL;
	NX_VID_MEMORY_INFO *pCurCapturedBuf = NULL;

	int width  = CAP_WIDTH;
	int height = CAP_HEIGHT;
	int pos = 0;
	long long timeStamp;

	//	Allocation Memory
	for( int i=0; i<CAP_BUFFERS ; i++ )
	{
		hMem[i] = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
	}

	//	Initialize Video Input Processor
	memset( &vipInfo, 0, sizeof(vipInfo) );
	vipInfo.port       = m_CameraPort;
	vipInfo.mode       = VIP_MODE_CLIPPER;
	//	Sensor Input Size
	vipInfo.width      = width;
	vipInfo.height     = height;
	vipInfo.numPlane   = 1;
	//	Clipper Setting
	vipInfo.cropX      = 0;
	vipInfo.cropY      = 0;
	vipInfo.cropWidth  = width;
	vipInfo.cropHeight = height;
	//	Fps
	vipInfo.fpsNum = 30;
	vipInfo.fpsDen = 1;
	hVip = NX_VipInit( &vipInfo );

	//	Initailize Display
	dspInfo.port              = DISPLAY_PORT_LCD;	//	Display Port( DISPLAY_PORT_LCD or DISPLAY_PORT_HDMI )
	dspInfo.module            = 0;					//	MLC Module (0 or 1)
	dspInfo.width             = width;				//	Source Width
	dspInfo.height            = height;				//	Source Height
	dspInfo.numPlane          = 1;					//	Image Plane Number
	dspInfo.dspSrcRect.left   = 0;					//	Clipper Setting
	dspInfo.dspSrcRect.top    = 0;
	dspInfo.dspSrcRect.right  = width;
	dspInfo.dspSrcRect.bottom = height;
	dspInfo.dspDstRect.left   = m_DspX;				//	MLC Scaler Setting
	dspInfo.dspDstRect.top    = m_DspY;
	dspInfo.dspDstRect.right  = m_DspX+m_DspWidth;
	dspInfo.dspDstRect.bottom = m_DspY+m_DspHeight;
	hDsp = NX_DspInit( &dspInfo );

	NX_DspSetColorKey(0, 0x090909);

	NX_VipQueueBuffer( hVip, hMem[pos] );

	while(m_bRunning)
	{
		NX_VipQueueBuffer( hVip, hMem[pos] );
		NX_VipDequeueBuffer( hVip, &pCurCapturedBuf, &timeStamp );

		NX_DspQueueBuffer( hDsp, pCurCapturedBuf );
		if( pPrevDsp )
		{
			NX_DspDequeueBuffer( hDsp );
		}
		pPrevDsp = pCurCapturedBuf;
		pos = (pos + 1)%CAP_BUFFERS;
	}

	for( int i=0; i<CAP_BUFFERS ; i++ )
	{
		NX_FreeVideoMemory(hMem[i]);
	}

	NX_DspClose( hDsp );
	NX_VipClose( hVip );
}


bool CCameraTest::Start()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		return false;
	}

	m_bRunning = true;
	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}
	return true;
}

bool CCameraTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}

bool CCameraTest::CameraToggle()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		m_bRunning = false;
		pthread_join( m_hThread, NULL );
		m_CameraIndex = (m_CameraIndex+1) % MAX_CAMERA_PORTS;
		m_CameraPort = CameraPortList[m_CameraIndex];
	}

	m_bRunning = true;
	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}

	m_bRunning = true;
	return true;
}

bool CCameraTest::SetDspRect( int x, int y, int w, int h )
{
	m_DspX = x;
	m_DspY = y;
	m_DspWidth = w;
	m_DspHeight = h;
	return true;
}
