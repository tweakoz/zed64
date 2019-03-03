///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

class CCRC
{

	static const int CRC_INIT = 0xFFFF;

	static const uint16_t crc_table[256]; 

public:

    static void Init( uint16_t & _v ) { _v = CRC_INIT; }
    static void Add( uint16_t & _v, uint8_t _d );
    static void Add( uint16_t & _v, int32_t _d );
    static void Add( uint16_t & _v, int16_t _d );
    static uint32_t HashStringCaseInsensitive( const char * _string );
    static uint32_t HashStringCaseSensitive( const char * _string );
	static uint32_t HashMemory( const void * _address, int32_t _length );

	static bool DoesDataMatch( const void *pv0, const void *pv1, int ilen );

};

///////////////////////////////////////////////////////////////////////////////
