#ifndef __NX_TsDemux_h__
#define	__NX_TsDemux_h__

//	Stream Type
#define		TS_DATATYPE_PAT		0
#define		TS_DATATYPE_PMT		1
#define		TS_DATATYPE_VIDEO	2
#define		TS_DATATYPE_AUDIO	3
#define		TS_DATATYPE_USER	4

#define		MAX_PMT				6

typedef		int						DEMUX_RESULT;
typedef		struct	tagDEMUX_INFO	*DEMUX_HANDLE;

typedef	struct	tagPAT_INFO {
	unsigned int	numPMT;
	unsigned int	PMT_PID[MAX_PMT];
	unsigned int	pogramNum[MAX_PMT];
}PAT_INFO;

typedef	struct	tagPMT_INFO {
	unsigned int	PID;
	unsigned int	programNumber;
	unsigned int	version;
	unsigned int	PCRPid;
	unsigned int	videoPid;
	unsigned int	videoStreamType;
	unsigned int	audioPid;
	unsigned int	audioStreamType;
}PMT_INFO;

typedef	struct	tagES_INFO {
	unsigned int	PID;
	unsigned char	*pBuf;
	unsigned int	BufSize;
	unsigned int	Discontiniuty;
	long long		PTS;
}ES_INFO;

typedef struct	tagPROGRAM_INFO {
	unsigned int	NumPMT;
	PMT_INFO		Pmt[MAX_PMT];
}PROGRAM_INFO;


//	ERROR
enum {
	NX_DMXERR_PID_DUP		= -13,	//	Duplicate PID
	NX_DMXERR_PID_SPACE		= -12,	//	Not enough TS buffer to add new PID
	NX_DMXERR_PES_SIZE		= -11,	//	PES buffer size error ( too small or broken stream )
	NX_DMXERR_PES_HEADER	= -10,	//	PES header error
	NX_DMXERR_ES_BUFFER		= -9,	//	Es Buffer Error
	NX_DMXERR_SEC_CRC		= -8,	//	Section CRC Error
	NX_DMXERR_SEC_LEN		= -7,	//	Section Length Error
	NX_DMXERR_CC			= -6,	//	TS Continuity Counter Error
	NX_DMXERR_TS			= -5,	//	TS Stream Error
	NX_DMXERR_MEMORY		= -4,	//	Memory Error
	NX_DMXERR_HANDLE		= -3,	//	Invalid Parameter
	NX_DMXERR_INVAL_PARAM	= -2,	//	Invalid Parameter
	NX_DMXERR_ERR			= -1,	//	Error
	NX_DMXERR_NONE			= 0,
};

#ifdef __cplusplus
extern "C"{
#endif

DEMUX_HANDLE NX_TsDemuxOpen( void (*cbEsSend)(void *, void *, unsigned int), void *pObject );

DEMUX_RESULT NX_TsDemuxStart( DEMUX_HANDLE handle );

DEMUX_RESULT NX_TsDemuxStop( DEMUX_HANDLE handle );

DEMUX_RESULT NX_TsDemuxAddPID( DEMUX_HANDLE handle, unsigned int PID, unsigned int pesBufSize, unsigned int type );

DEMUX_RESULT NX_TsDemuxDelPID( DEMUX_HANDLE handle, unsigned int PID );

DEMUX_RESULT NX_TsDemuxResetPID( DEMUX_HANDLE handle );

DEMUX_RESULT NX_TsDemuxSetCallBack( DEMUX_HANDLE handle, void (*cbEsSend)(void *, void *, unsigned int), void *pObject );

DEMUX_RESULT NX_TsDemuxParse( DEMUX_HANDLE handle, unsigned char *pBuf );

DEMUX_RESULT NX_TsDemuxReset( DEMUX_HANDLE handle );

DEMUX_RESULT NX_TsDemuxClose( DEMUX_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif	//__NX_TsDemux_h__
