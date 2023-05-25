#ifndef __ParseUserData_h__
#define __ParseUserData_h__

#define	MAX_OUT_USER_DATA	(1024*4+188)
#define	USER_DATA_PID		(0x40)
#define	USER_STREAM_ID		(0xBE)

#define	ERR_PES_LENGTH			(-6)
#define	ERR_UNKNOWN_PES			(-5)
#define	ERR_PES_START			(-4)
#define	ERR_TS_SYNC				(-3)
#define	ERR_CONTINUITY_COUNTER	(-2)
#define	ERR_BUFFER_OVERFLOW		(-1)
#define	ERR_NONE				0

class CParseUserData
{
public:
	CParseUserData();
	virtual ~CParseUserData();

	//	Set callback function
public:
	bool Init( void (*cbParseOut)(void *privateData, unsigned char *pData, int size, __int64 timeStamp), void *privateData );	//	Set Callback
	int Parse( unsigned char pData[188] );								//	188 Aligned Data

private:
	int ParsePES();

private:
	void (*m_cbOut)( void *privateData, unsigned char *pData, int size, __int64 timeStamp );
	unsigned char	m_PesPayload[MAX_OUT_USER_DATA];
	int				m_PayloadSize;
	int				m_CountinuityCounter;
	bool			m_bDiscontinuity;
	bool			m_bValid;
	__int64			m_StartTime;
	void			*m_PrivateData;
};

#endif	//	 __ParseUserData_h__
