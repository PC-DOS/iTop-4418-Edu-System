//-----------------------------------------------------------------------------
//	Copyright (C) 2012 Nexell Co., All Rights Reserved
//	Nexell Co. Proprietary < Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		: second boot
//	File		: secondboot.h
//	Description	: This must be synchronized with NSIH.txt
//	Author		: Firmware Team
//	History		:
// 				Hans 2013-06-23 Create
//------------------------------------------------------------------------------
#ifndef __NX_SECONDBOOT_H__
#define __NX_SECONDBOOT_H__

#include <stdint.h>
typedef uint8_t			U8;
typedef uint16_t		U16;
typedef uint32_t		U32;


#define HEADER_ID		((((U32)'N')<< 0) | (((U32)'S')<< 8) | (((U32)'I')<<16) | (((U32)'H')<<24))

struct NX_NANDBootInfo
{
	U8	AddrStep;
	U8	tCOS;
	U8	tACC;
	U8	tOCH;
	U32 PageSize;
	U32	CRC32;
};

struct NX_SPIBootInfo
{
	U8	AddrStep;
	U8	_Reserved0[3];
	U32	_Reserved1;
	U32 CRC32;
};

struct NX_SDMMCBootInfo
{
	U8	PortNumber;
	U8	_Reserved0[3];
	U32	_Reserved1;
	U32	CRC32;
};

struct NX_SDFSBootInfo
{
	char BootFileName[12];		// 8.3 format ex)"NXDATA.TBL"
};

union NX_DeviceBootInfo
{
	struct NX_NANDBootInfo NANDBI;
	struct NX_SPIBootInfo SPIBI;
	struct NX_SDMMCBootInfo SDMMCBI;
	struct NX_SDFSBootInfo SDFSBI;
};

struct NX_DDRInitInfo
{
	U8	ChipNum;
	U8	ChipRow;
	U8	BusWidth;
	U8	_Reserved0;

	U16	ChipMask;
	U16	ChipBase;

	U8	CWL;
	U8	WL;
	U8	RL;
	U8	DDRRL;

	U32	PHYCON4;
	U32	PHYCON6;

	U32 TIMINGAREF;
	U32 TIMINGROW;
	U32	TIMINGDATA;
	U32	TIMINGPOWER;
};
struct NX_SecondBootInfo
{
	U32 VECTOR[8];			// 0x000 ~ 0x01C
	U32 VECTOR_Rel[8];		// 0x020 ~ 0x03C

	U32 DEVICEADDR;			// 0x040

	U32 LOADSIZE;			// 0x044
	U32 LOADADDR;			// 0x048
	U32 LAUNCHADDR;			// 0x04C
	union NX_DeviceBootInfo DBI;	// 0x050~0x058

	U32 PLL[4];				// 0x05C ~ 0x068
	U32 PLLSPREAD[2];		// 0x06C ~ 0x070
	U32 DVO[5];				// 0x074 ~ 0x084

	struct NX_DDRInitInfo DII;	// 0x088 ~ 0x0A8

	U32 AXIBottomSlot[32];	// 0x0AC ~ 0x128
	U32 AXIDisplaySlot[32];	// 0x12C ~ 0x1A8

	U32 Stub[(0x1F8-0x1A8)/4];	// 0x1AC ~ 0x1F8
	U32 SIGNATURE;			// 0x1FC	"NSIH"
};

// [0] : Use ICache
// [1] : Change PLL
// [2] : Decrypt
// [3] : Suspend Check

#ifdef __cplusplus
extern "C"
{
#endif

#define	NSIH_BIN_SIZE	512

int ProcessNSIH (const char *pfilename, unsigned char *pOutData);

#ifdef __cplusplus
}
#endif

#endif //__NX_SECONDBOOT_H__
