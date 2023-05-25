//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __NX_DVRGSENSOR_H__
#define __NX_DVRGSENSOR_H__

#include <stdint.h>
#include <stdbool.h>

// BMA222 Gsensor
typedef struct _BMA222_GSENSOR_VALUE {
	int32_t		x;
	int32_t		y;
	int32_t		z;
	int32_t		resol;
	int32_t		delay;
} BMA222_GSENSOR_VALUE;

#define SENSOR_CONTROL_IOC_MAGIC            'S'

#define IOCTL_SENSOR_SET_INPUTDEVICE        _IOW(SENSOR_CONTROL_IOC_MAGIC, 0, unsigned int)
#define IOCTL_SENSOR_SET_DELAY_ACCEL        _IOW(SENSOR_CONTROL_IOC_MAGIC, 1, unsigned int)
#define IOCTL_SENSOR_SET_CALIB_ACCEL        _IOW(SENSOR_CONTROL_IOC_MAGIC, 2, BMA222_GSENSOR_VALUE)
//#define IOCTL_SENSOR_SET_EVENT_THRES      _IOW(SENSOR_CONTROL_IOC_MAGIC, 3, unsigned int)

#define IOCTL_SENSOR_GET_RESOLUTION         _IOR(SENSOR_CONTROL_IOC_MAGIC, 4, unsigned int)
#define IOCTL_SENSOR_GET_TEMP_INFO          _IOR(SENSOR_CONTROL_IOC_MAGIC, 5, unsigned int)
#define IOCTL_SENSOR_GET_DATA_ACCEL         _IOR(SENSOR_CONTROL_IOC_MAGIC, 6, BMA222_GSENSOR_VALUE)
#define IOCTL_SENSOR_GET_SENSOR_TYPE        _IOR(SENSOR_CONTROL_IOC_MAGIC, 7, int)

// The outhers driver

// Common Interface
#define GSENSOR_DATABASE_MAX		255
#define GSENSOR_DATABASE_NUM		1

typedef struct _GSENSOR_VALUE {
	int32_t		x;
	int32_t		y;
	int32_t		z;
} GSENSOR_VALUE;

typedef struct _GSENSOR_INFO {
	int32_t 	fd;
	int32_t		(*pReadFunc)(int32_t fd, GSENSOR_VALUE *pValue);
} GSENSOR_INFO;

typedef GSENSOR_INFO	*GSENSOR_HANDLE;

// Gsensor Read Common Interface
GSENSOR_HANDLE	DvrGsensorOpen( const char *pDeviceNode );
void			DvrGsensorClose( GSENSOR_HANDLE hGsensor );

int32_t			DvrGsensorValue( GSENSOR_HANDLE hGsensor, GSENSOR_VALUE *pValue );
int32_t 		DvrGsensorAverageValue( GSENSOR_HANDLE hGsensor, GSENSOR_VALUE *pValue );

#endif	// __NX_DVRGSENSOR_H__