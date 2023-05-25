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
#ifndef __NX_TYPE_H__
#ifndef __NX_MediaType__
#define __NX_MediaType__

//------------------------------------------------------------------------------
/// @name Basic data types
///
/// @brief Basic data type define and Data type constants are implemen \n
///	tation-dependent ranges of values allowed for integral data types. \n
/// The constants listed below give the ranges for the integral data types 
//------------------------------------------------------------------------------
/// @{
///

#include <stdint.h>
#define S8_MIN          -128				///< signed char min value				
#define S8_MAX          127					///< signed char max value				
#define S16_MIN         -32768				///< signed short min value				
#define S16_MAX         32767				///< signed short max value				
#define S32_MIN         -2147483648			///< signed integer min value
#define S32_MAX         2147483647			///< signed integer max value

#define U8_MIN          0					///< unsigned char min value   
#define U8_MAX          255					///< unsigned char max value   
#define U16_MIN         0					///< unsigned short min value  
#define U16_MAX         65535				///< unsigned short max value  
#define U32_MIN         0					///< unsigned integer min value
#define U32_MAX         4294967295			///< unsigned integer max value
/// @}

//==============================================================================
/// @name Boolean data type
///
/// C and C++ has difference boolean type. so we use signed integer type \n
/// instead of bool and you must use CTRUE or CFALSE macro for CBOOL type 
//
//==============================================================================
/// @{
typedef int32_t	CBOOL;							///< boolean type is 32bits signed integer
#define CTRUE	1							///< true value is  integer one
#define CFALSE	0							///< false value is  integer zero
/// @}

#endif	//	__NX_MediaType
#endif	//	__NX_TYPE_H__
