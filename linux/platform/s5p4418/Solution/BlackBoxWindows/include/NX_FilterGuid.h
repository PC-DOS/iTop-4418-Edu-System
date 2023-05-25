//-----------------------------------------------------------------------------
//
//	File 			: NX_FilterGuid.h
//	Revision 		: 
//	Description 	: GUIDs used by filter
//	Part Number 	: 
//
//-----------------------------------------------------------------------------
// Ver   	Date        		Description
// 	  	2013/11/27
//-----------------------------------------------------------------------------

#ifndef _NXFILTER_GUID_H_
#define _NXFILTER_GUID_H_

static const GUID CLSID_NXMP4PARSER = 
{ 0xd3cbf5b3, 0x2c2e, 0x4c6a, { 0x90, 0x75, 0xb2, 0x82, 0x7, 0x23, 0xdd, 0x19 } };

//	Nexell Audio Decoder using FFMPEG
static const GUID CLSID_NXAudioDecoderFilter = 
{ 0x3B06D7A6, 0x2246, 0x4EB6, { 0xBA, 0xB9, 0xB7, 0xF4, 0x28, 0xF0, 0x9C, 0x18 } };

// {F5DB7063-A0FF-41F3-902D-8F98A86EE3AC}
static const GUID CLSID_NX_VIDEODECODER = 
{ 0xf5db7063, 0xa0ff, 0x41f3, { 0x90, 0x2d, 0x8f, 0x98, 0xa8, 0x6e, 0xe3, 0xac } };

//============================================================================
//
//							Demuxer Filter GUIDs
//

// {19F3FB6D-7CCB-44DC-A44D-716E1CEB3996}
static const GUID CLSID_NXTSDemuxer = 
{ 0x19f3fb6d, 0x7ccb, 0x44dc, { 0xa4, 0x4d, 0x71, 0x6e, 0x1c, 0xeb, 0x39, 0x96 } };

// {A1D57561-3785-4ef2-80F0-2D9F56AF046F}
static const GUID IID_INXStreamConfig = 
{ 0xa1d57561, 0x3785, 0x4ef2, { 0x80, 0xf0, 0x2d, 0x9f, 0x56, 0xaf, 0x4, 0x6f } };


// {CCB214CD-298A-4e0e-924C-143D75D68388}
static const GUID IID_INX_StreamSeek = 
{ 0xccb214cd, 0x298a, 0x4e0e, { 0x92, 0x4c, 0x14, 0x3d, 0x75, 0xd6, 0x83, 0x88 } };

#endif	//_FILTER_GUID_H_
