#ifndef __CAMERATEST_H__
#define __CAMERATEST_H__

#define	MAX_CAMERA_PORTS 2


//
//	Call Sequence
//	Create Class --> SetDspRect --> Start/Stop/Toggle --> Stop --> Delete Class
//


class CCameraTest
{
public:
	CCameraTest();
	~CCameraTest();

	bool Start();
	bool Stop();
	bool CameraToggle();
	bool SetDspRect( int x, int y, int w, int h );

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CCameraTest *pObj = (CCameraTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	int				m_CameraIndex;
	int				m_CameraPort;
	int				m_DspX;
	int				m_DspY;
	int				m_DspWidth;
	int				m_DspHeight;

	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
};

#endif // !__CAMERATEST_H__
