#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#include <linux/input.h>

#include <CpuRamTest.h>
#include <utils.h>

#define	RAM_CLOCK_PATH		"/sys/devices/platform/cpu/mem_clock"
#define	RAM_SIZE_PATH		"/sys/devices/platform/cpu/mem_size"

#define CPU_MAX_FREQ_PATH	"/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq"
#define CPU_MAX_CPU_PATH	"/sys/devices/system/cpu/kernel_max"

CCpuRamTest::CCpuRamTest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CCpuRamTest::~CCpuRamTest()
{
	pthread_mutex_destroy( &m_hMutex );
}

bool CCpuRamTest::GetCpuInfo( unsigned int &numCore, unsigned int &maxClockMH )
{
	char freq[80];
	char numCpu[80];

	if( !ReadSysFileInfo( CPU_MAX_FREQ_PATH, freq, sizeof(freq) ) )
	{
		return false;
	}

	if( !ReadSysFileInfo( CPU_MAX_CPU_PATH, numCpu, sizeof(numCpu) ) )
	{
		return false;
	}

	numCore = atoi(numCpu) + 1;
	maxClockMH = atoi(freq);

	return true;
}

bool CCpuRamTest::GetRamInfo( unsigned int &clockMH, unsigned int &sizeMB )
{
	char clock[80];
	char size[80];

	if( !ReadSysFileInfo( RAM_CLOCK_PATH, clock, sizeof(clock) ) )
	{
		return false;
	}

	if( !ReadSysFileInfo( RAM_SIZE_PATH, size, sizeof(size) ) )
	{
		return false;
	}

	clockMH = atoi(clock);
	sizeMB = atoi(size);

	return true;
}
