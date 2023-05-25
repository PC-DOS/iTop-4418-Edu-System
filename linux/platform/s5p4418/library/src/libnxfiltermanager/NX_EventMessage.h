//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//	Nexell Proprietary & Confidential
//
//	Module     : 
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#ifndef __NX_EventMessage_h__
#define __NX_EventMessage_h__


//////////////////////////////////////////////////////////////////////////////
//
//								EVENT CODE
//

//	Player Management Event Code	: 0x0000 ~ 0xFFFF
//	User Private Event Code			: 0x10000 ~ 0x7FFFFFFF
//	Error Message Area				: 0x80000000 ~ 0x8000FFFF
//	User Error Message Area			: 0x80010000 ~ 0xFFFFFFFF
#define		EVENT_ENABLE_VIDEO					(0x0010)		//	Require Video Layer Control
#define		EVENT_VIDEO_ENABLED					(0x0020)		//	Video Layer Enabled : Enabled Display
#define		EVNT_CHANGE_DEFAULT_BUFFERING_TIME	(0x0030)		//	Change default buffering time
#define		EVENT_CHANGE_STREAM_INFO			(0x0040)		//	Change Stream Information

#define		EVENT_END_OF_STREAM					(0x1000)		//	End of stream event

#define		EVENT_DEMUX_ERROR					(0x10000)					
#define		EVENT_VIDEO_DECODER_ERROR			(EVENT_DEMUX_ERROR + 1)		
#define		EVENT_VIDEO_RENDER_ERROR			(EVENT_VIDEO_DECODER_ERROR + 1)		//	
#define		EVENT_AUDIO_DECODER_ERROR			(EVENT_VIDEO_RENDER_ERROR + 1)		//	
#define		EVENT_AUDIO_CONVERT_ERROR			(EVENT_AUDIO_DECODER_ERROR + 1)		//	
#define		EVENT_AUDIO_RENDER_ERROR			(EVENT_AUDIO_CONVERT_ERROR + 1)		//	





#define		EVENT_REQUSET_RESYNC	(0x80000001)	//	Requset Basetime reset

//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//								EVENT VALUE
//
#define		EVENT_ACTION_EANBLE		(1)
#define		EVENT_ACTION_DISABLE	(0)
//
//////////////////////////////////////////////////////////////////////////////

#endif	//	 __NX_EventMessage_h__
