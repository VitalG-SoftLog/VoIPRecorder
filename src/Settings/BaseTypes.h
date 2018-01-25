//////////////////////////////////////////////////////////////////////////
// Header: BaseTypes.h
//////////////////////////////////////////////////////////////////////////
#ifndef _BASE_TYPES_H_H__
#define _BASE_TYPES_H_H__

// Type: int8
// Signed 8 bit integer
typedef char int8;

// Type: uint8
// Unsigned 8 bit integer
typedef unsigned char uint8;

// Type: int16
// Signed 16 bit integer
typedef short int16;

// Type: uint16
// Unsigned 16 bit integer
typedef unsigned short uint16;

// Type: int32
// Signed 32 bit integer
typedef int int32;

// Type: uint32
// Unsigned 32 bit integer
typedef unsigned int uint32;

// Type: int64
// Signed 64 bit integer
typedef __int64 int64;

// Type: uint64
// Unsigned 64 bit integer
typedef unsigned __int64 uint64;

typedef int	BOOL;

#ifndef FALSE
#define	FALSE			0
#endif//FALSE

#ifndef TRUE
#define	TRUE			1
#endif//TRUE

#include <string>
#include "Defines.h"

#endif _BASE_TYPES_H_H__
