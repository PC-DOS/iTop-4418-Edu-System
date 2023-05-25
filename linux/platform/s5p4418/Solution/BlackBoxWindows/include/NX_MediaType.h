#ifndef __NX_MediaType_h__
#define __NX_MediaType_h__

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#define NXFILEHANDLE	HANDLE
#	define _PACKED_		 
#endif
#ifdef _LINUX
#	include <stdint.h>
#	define NXFILEHANDLE	int
#	define NULL	0
#	define _PACKED_		__attribute__((packed))
	typedef int64_t		__int64;
	typedef uint64_t	LONGLONG;
	typedef uint64_t	U64;
	typedef int64_t		S64;
#endif

//-----------------------------------------------------------------------------
//	Media architecture's result value
typedef int				NX_RESULT;


#ifndef _NX_TYPE_H
#define _NX_TYPE_H

//------------------------------------------------------------------------------
// basic data types
//------------------------------------------------------------------------------
/// @brief 8bit signed integer(s.7) value
typedef char    		S8;
/// @brief 16bit signed integer(s.15) value
typedef short   		S16;
/// @brief 32bit signed integer(s.31) value
typedef long   			S32;
/// @brief 64bit signed integer(s.63) value
typedef __int64			S64;

/// @brief 8bit unsigned integer value
typedef unsigned char	U8;
/// @brief 16bit unsigned integer value
typedef unsigned short	U16;
/// @brief 32bit unsigned integer value
typedef unsigned long	U32;
/// @brief 64bit unsigned integer value
typedef unsigned __int64	U64;

//------------------------------------------------------------------------------
// boolean type
//------------------------------------------------------------------------------
/// @brief boolean type
typedef int		CBOOL;
/// @brief		true (boolean type)
///	@remarks	this is not 1 but true
#define CTRUE	(0==0)
/// @brief		false (boolean type)
///	@remarks	this is not 0 but true
#define CFALSE	(0!=0)

//------------------------------------------------------------------------------
// NULL
//------------------------------------------------------------------------------
// @brief	null pointer
#	define CNULL 0
#endif  // _NX_TYPE_H

#endif	//	__NX_MediaType_h__
