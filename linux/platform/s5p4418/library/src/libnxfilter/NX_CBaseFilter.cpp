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
#include "NX_CBaseFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
//							CBaseOutputPin
//
NX_CBaseOutputPin::NX_CBaseOutputPin( NX_CBaseFilter *pOwnerFilter ):
	NX_CBasePin( pOwnerFilter ),
	m_Direction( PIN_DIRECTION_OUTPUT )
{
}
int32_t	NX_CBaseOutputPin::Deliver( NX_CSample *pSample )
{
	int32_t ret = 0;
	if( CTRUE==IsConnected() && IsActive() && NULL!=m_pPartnerPin ){
		ret = ((NX_CBaseInputPin*)m_pPartnerPin)->Receive( pSample );
		return ret;
	}
	return -1;
}
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//							CBaseInputPin
//
NX_CBaseInputPin::NX_CBaseInputPin( NX_CBaseFilter *pOwnerFilter ) :
	NX_CBasePin( pOwnerFilter ),
	m_Direction( PIN_DIRECTION_INPUT )
{
}

//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							Event Management Classes
//
//	Event Notifier
NX_CEventNotifier::NX_CEventNotifier() :
	m_pReceiver(NULL)
{
}
NX_CEventNotifier::~NX_CEventNotifier()
{
}

void NX_CEventNotifier::SendEvent( uint32_t EventCode, uint32_t EventValue )
{
	NX_CAutoLock Lock(m_EventLock);
	if( m_pReceiver == NULL )	return ;
	m_pReceiver->ProcessEvent( EventCode, EventValue );
}

void NX_CEventNotifier::SetEventReceiver( NX_CEventReceiver *pReceiver )
{
	m_pReceiver = pReceiver;
}
//
//////////////////////////////////////////////////////////////////////////////
