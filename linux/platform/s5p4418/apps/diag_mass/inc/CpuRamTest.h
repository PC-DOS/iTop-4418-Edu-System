#ifndef __CPURAMTEST_H__
#define __CPURAMTEST_H__

#include <pthread.h>

class CCpuRamTest
{
public:
	CCpuRamTest();
	~CCpuRamTest();

	static bool GetCpuInfo( unsigned int &numCore, unsigned int &maxClockMH );
	static bool GetRamInfo( unsigned int &clockMH, unsigned int &sizeMB );

private:
	pthread_mutex_t	m_hMutex;
};

#endif // !__CPURAMTEST_H__
