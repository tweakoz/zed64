///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits>
#include <string>
#include <functional>

namespace ork {
	
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef float f32;
typedef double f64;
typedef float F32;
typedef double F64;

static const float KEPSILON = std::numeric_limits<float>::epsilon();

std::string FormatString( const char* formatstring, ... );

typedef std::function<void()> void_lambda_t;

}

#if defined(GCC)
    #if defined(_IOS)
    #define ThreadLocal 
    #else
    #define ThreadLocal __thread
    #endif
#elif defined(_MSVC)
    #define ThreadLocal __declspec(thread)
#endif

#define OrkAssert(x) assert(x)
#define OrkAssertNotImpl() assert(false);
