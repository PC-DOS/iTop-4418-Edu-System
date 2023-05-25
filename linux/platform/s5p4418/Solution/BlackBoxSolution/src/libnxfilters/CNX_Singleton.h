//------------------------------------------------------------------------------
//
//	Copyright (C) 2003 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE AND 
//	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
//	Module     : Singleton support
//	File       : nx_singleton.h
//	Description:
//	Author     : Gamza(nik@mesdigital.com)
//	Export     :
//	History    :
//	   2004/01/20 Gamza from http://www.gpgstudy.com/forum/viewtopic.php?t=1734
//------------------------------------------------------------------------------

#ifndef _CNX_SIGNLETON_H
#define _CNX_SIGNLETON_H

#ifdef __cplusplus


//------------------------------------------------------------------------------
/// @class	NX_Singleton
/// @brief	singleton template
///	usage\n
///	1. inherit singleton class
///		class MySingleton : public NX_Singleton<MySingleton> ...\n
///	2. create singleton instance
///		MySingleton* pMySingleton = new MySingleton;\n
///		or\n
///		MySingleton s_MySingleton;\n
///	3. access singleton instance by GetSingleton interface.\n
///		MySingleton::GetSingleton()...;\n
///	4. remove singleton instance.\n
///		delete pMySingleton;\n
//------------------------------------------------------------------------------
template <class T>
class CNX_Singleton
{
public:
	CNX_Singleton( void );
	virtual ~CNX_Singleton( void );

	/// @brief  get instance of singleton object
	/// @return instance of singleton object
	static T& GetSingleton( void );
	static T* GetSingletonPtr( void );
protected:
	long m_typecast_stamp;
private:
	static T* ms_pSingleton;
};

//------------------------------------------------------------------------------
//
//	inline body
//
//------------------------------------------------------------------------------
template <class T>
T* CNX_Singleton<T>::ms_pSingleton = NULL;
//---------------------------------------------------------------------------
template <class T>
CNX_Singleton<T>::CNX_Singleton( void )
{
#if 0
	// static_cast is not fully implemented in ADS v1.2
	uint32_t offset = (uint32_t)(&(((T*)1)->m_typecast_stamp)) -
					(uint32_t)(&(((NX_Singleton<T>*)1)->m_typecast_stamp));
	ms_pSingleton = (T*)( ((uint32_t)this) - offset );
#else
	ms_pSingleton = static_cast<T*>(this);
#endif
}
//---------------------------------------------------------------------------
template <class T>
CNX_Singleton<T>::~CNX_Singleton( void )
{
	//NX_ASSERT(ms_pSingleton==static_cast<T*>(this));
	ms_pSingleton = NULL;
}
//---------------------------------------------------------------------------
template <class T>
T& CNX_Singleton<T>::GetSingleton( void )
{
	return *ms_pSingleton;
}
//---------------------------------------------------------------------------
template <class T>
T* CNX_Singleton<T>::GetSingletonPtr( void )
{
	return ms_pSingleton;
}
//---------------------------------------------------------------------------


#endif // __cplusplus

#endif // _CNX_SIGNLETON_H
