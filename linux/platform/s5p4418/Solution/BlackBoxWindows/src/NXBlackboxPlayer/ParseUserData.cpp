#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "ParseUserData.h"

CParseUserData::CParseUserData()
	: m_cbOut(0)
	, m_PayloadSize(0)
	, m_CountinuityCounter(-1)
	, m_bDiscontinuity(true)
	, m_StartTime(-1)
{
}

CParseUserData::~CParseUserData()
{
}

bool CParseUserData::Init( void (*cbParseOut)( void *, unsigned char *, int , __int64 ), void *privateData )
{
	m_cbOut = cbParseOut;
	m_PrivateData = privateData;
	return false;
}

int CParseUserData::ParsePES()
{
	unsigned char *pData = m_PesPayload;
	int pesLength;
	//unsigned int flags=0;
	unsigned char PES_header_data_length = 0;
	__int64 timeStamp = 0, PTS, DTS;
	unsigned int PTS_DTS_flags;
	//unsigned char PES_private_data_flag = 0;
 
	//	Parse PES Header
	if( pData[0] != 0 || pData[1] != 0 || pData[2] != 1 )
		return ERR_PES_START;
	if( pData[3] != USER_STREAM_ID )
		return ERR_UNKNOWN_PES;

	pesLength = pData[4]<<8 | pData[5];
	if( pesLength+6 > m_PayloadSize ){
		return ERR_PES_LENGTH;
	}
	pData += 6;

	PTS_DTS_flags = (pData[1] >> 6) & 0x3;
	//	Start pesLength Area
	PES_header_data_length = pData[2];
	pData += 3;

	if( PTS_DTS_flags == 0x2 )
	{
		//	  pts[32..30]				| pts[29..22]		   | pts[21..15]				 | pts[14..7]		   | pts[6..0]
		PTS	= (((__int64)pData[0]&0x0e)<<29) | ((__int64)pData[1]<<22) | (((__int64)pData[2]&0xfe)<<14) | ((__int64)pData[3]<<7) | (((__int64)pData[4]&0xfe)>>1);
	}

	if( PTS_DTS_flags == 0x3 )
	{
		//	  pts[32..30]				| pts[29..22]		   | pts[21..15]				 | pts[14..7]		   | pts[6..0]
		PTS	= (((__int64)pData[0]&0x0e)<<29) | ((__int64)pData[1]<<22) | (((__int64)pData[2]&0xfe)<<14) | ((__int64)pData[3]<<7) | (((__int64)pData[4]&0xfe)>>1);
		//	  dts[32..30]			      | dts[29..22]		     | dts[21..15]				   | dts[14..7]		     | dts[6..0]
		DTS	= (((__int64)pData[5]&0x0e)<<29) | ((__int64)pData[6]<<22) | (((__int64)pData[7]&0xfe)<<14) | ((__int64)pData[8]<<7) | (((__int64)pData[9]&0xfe)>>1);
	}

	if( m_StartTime == -1 )
	{
		m_StartTime = PTS;
	}

	//	Skp PES Header
	pData += PES_header_data_length;

	//	Pure ES Payload
	pesLength -= (3 + PES_header_data_length);

	if( pesLength < 0 )
		return ERR_PES_LENGTH;

	//
	if( m_cbOut ){
		timeStamp = (PTS - m_StartTime)/90;
		m_cbOut( m_PrivateData, pData, pesLength, timeStamp );
	}

	return ERR_NONE;
}

int CParseUserData::Parse( unsigned char pData[188] )
{
	int ret = ERR_NONE;
	unsigned int tmp;
	unsigned int cc, adap, adap_len;
	unsigned int start_ind;
	unsigned int ts_payload = 188;

	////////////////////////////////////////////////
	//		Transport stream parser

	//	Parse TS
	if (pData[0] != 0x47)
		return ERR_TS_SYNC;

	//	error indicator(1), start indicator(1), pririty(1), PID(13)
	tmp = pData[1]<<8|pData[2];

	//	Check PID
	if( ( tmp& 0x1fff) != USER_DATA_PID )
		return 0;
	start_ind = (tmp>>14)&0x1;	//	Get start indicator

	//	scrambling control(2), adaptation field control(2), continuity counter(4)
	adap = (pData[3]>>4)&0x3;
	cc = pData[3] & 0xf;

	if( m_CountinuityCounter != (signed int)cc )
	{
		//	reset buffers
		m_PayloadSize = 0;
		ret = ERR_CONTINUITY_COUNTER;
		if( !start_ind )
		{
			m_CountinuityCounter = (cc+1)&0xf;
			m_bValid = false;
			return ret;
		}
		m_bValid = true;
		m_bDiscontinuity = true;	//	Discontinuity
	}

	m_CountinuityCounter = (cc+1)&0xf;

	pData += 4;
	ts_payload -= 4;

	//	skip adapation field data
	if( adap & 0x2 ){
		adap_len = *pData ++;
		pData += adap_len;
		ts_payload -= (adap_len+1);
	}

	////////////////////////////////////////////////
	//		PES Parser & ES Data Gather

	if( start_ind && m_bValid && m_PayloadSize>0  )
	{
		//	Send & Clear
		ret = ParsePES();
		m_PayloadSize = 0;
		m_bValid = true;
	}

	if( start_ind )
		m_bValid = true;

	if( m_PayloadSize > MAX_OUT_USER_DATA )
	{
		return ERR_BUFFER_OVERFLOW;
	}

	memcpy( m_PesPayload+m_PayloadSize, pData, ts_payload );
	m_PayloadSize += ts_payload;

	return ret;
}
