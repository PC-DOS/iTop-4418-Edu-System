#ifndef __DISK_TEST_H__
#define	__DISK_TEST_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sched.h> 			/* schedule */
#include <sys/resource.h>
#include <linux/sched.h> 	/* SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH */

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>		/* stat */
#include <sys/vfs.h>		/* statfs */
#include <errno.h> 			/* error */

#include <sys/time.h> 		/* gettimeofday() */
#include <sys/times.h> 		/* struct tms */
#include <time.h> 			/* ETIMEDOUT */

#include <sys/signal.h>

typedef struct DISK_INFO
{
	long long	total;
	long long	free;
	long long	avail;
} DISK_INFO;


class StorageTestUtil
{
public:
	StorageTestUtil(const char *path)
		: m_pFilePath(NULL)
	{
		m_pFilePath = strdup(path);
	}
	~StorageTestUtil()
	{
		if( m_pFilePath )
		{
			free( m_pFilePath );
			m_pFilePath = NULL;
		}
	}
	int GetDiskInfo( DISK_INFO &info )
	{
		struct statfs64 fs;
		struct stat	st;
		if(stat(m_pFilePath,&st))
		{
			printf("Fail, Not exist( %s )\n",m_pFilePath);
			return -1;
		}
		else
		{
			if(! S_ISDIR(st.st_mode))
			{
				printf("Fail, Not a directory( %s, %s )\n",m_pFilePath,strerror(errno));
				return -1;
			}
		}

		if(0 > statfs64(m_pFilePath,&fs)) {
			printf("Fail, status fs %s, %s\n",m_pFilePath,strerror(errno));
			return 0;
		}

		info.total = fs.f_bsize * fs.f_blocks;
		info.free  = fs.f_bsize * fs.f_bfree;
		info.avail = fs.f_bsize * fs.f_bavail;
		return 0;
	}

	int ReadTest()
	{
		return 0;
	}

	//
	//	File Write Test Application
	//
	//	Step 1. Check writable data size.
	//	Step 2. Write data pattern data.
	//	Step 3. Read & validate data.
	//
	int WriteTest()
	{
		return 0;
	}

private:
	char *m_pFilePath;
};

#endif	//	__DISK_TEST_H__
