///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// TODO: rename this file to fixedstring.h (and .hpp)
///////////////////////////////////////////////////////////////////////////////

#include <ork/types.h>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

typedef bool (*ork_cstr_replace_pred)( const char* src, const char* loc, size_t isrclen );

///////////////////////////////////////////////////////////////////////////////

bool cstr_replace(	const char *src,
					const char *from,
					const char *to,
					char* dest,
					const size_t ilen,
					ork_cstr_replace_pred pred = 0 );

///////////////////////////////////////////////////////////////////////////////

class fixedstring_base
{
public:

	fixedstring_base();

	size_t length() const;
	size_t size() const { return length(); }
	virtual size_t get_maxlen() const = 0;
	virtual const char* c_str() const = 0;
	virtual void set(const char* pfrom) = 0;

protected:

	size_t mLength;
};

///////////////////////////////////////////////////////////////////////////////

//! Tweaks fixedstring.
//! Basically like a std::std::string.
//!	explicit allocation policy, so it is instantiable on the stack
//! has a format method for a sprintf like feel 
template <size_t kmaxlen>
class fixedstring : public fixedstring_base
{
public:

	typedef size_t size_type;
	static const size_type npos = 0xffffffff;

	virtual size_t get_maxlen() const { return kmaxlen; }

	/////////////////////////

	struct iterator_base
	{
		size_t mindex;
		int mdirection;

		iterator_base( size_t idx=npos, int idir=0 );
	};

	struct iterator 
	{const
		typedef std::random_access_iterator_tag iterator_category;
		typedef char		value_type;
		typedef char*		pointer;
		typedef char&       reference;
		typedef std::ptrdiff_t difference_type;

		fixedstring*	mpString;
		iterator( size_t idx=npos, int idir=0, fixedstring* pfm=0);
		iterator(const iterator& oth);
		void operator=( const iterator& oth );
		bool operator==(const iterator& oth ) const;
		bool operator!=(const iterator& oth ) const;
		pointer operator->() const;
		reference operator*() const;
		iterator operator++();	// prefix
		iterator operator--(); // prefix
		iterator operator++(int i);	// postfix
		iterator operator--(int i); // prefix
		iterator operator+(int i) const;	// add
		iterator operator-(int i) const;	// sub
		iterator operator+=(int i);	// add
		iterator operator-=(int i);	// sub
		bool operator<( const iterator& oth ) const;
		difference_type operator - ( const iterator& oth ) const;

		iterator_base mIteratorBase;
	};
	struct const_iterator 
	{
		typedef const char* const_pointer;
		typedef const char& const_reference;
		typedef std::ptrdiff_t difference_type;

		const fixedstring*	mpString;
		const_iterator( size_t idx=npos, int idir=0, const fixedstring* pfm=0);
		const_iterator(const iterator& oth);
		const_iterator(const const_iterator& oth);
		void operator=( const const_iterator& oth );
		bool operator==(const const_iterator& oth ) const;
		bool operator!=(const const_iterator& oth ) const;
		const_pointer operator->() const;
		const_reference operator*() const;
		const_iterator operator++(); // prefix
		const_iterator operator--(); // prefix
		const_iterator operator++(int i); // postfix
		const_iterator operator--(int i); // prefix
		const_iterator operator+(int i) const;	// add
		const_iterator operator-(int i) const;	// sub
		const_iterator operator+=(int i);	// add
		const_iterator operator-=(int i);	// sub
		bool operator < ( const const_iterator& oth ) const;
		difference_type operator - ( const const_iterator& oth ) const;

		iterator_base mIteratorBase;
	};	
	friend struct iterator;
	friend struct const_iterator;
	//////////////////////////////////////////////////////////
	static const unsigned int kMAXLEN = kmaxlen-1;
	//////////////////////////////////////////////////////////
    char* mutable_c_str() { return buffer; } // virtual
	const char* c_str() const { return buffer; } // virtual
	void SetChar( size_t index, char val );
	//////////////////////////////////////////////////////////
	void set(const char* pstr) override;
	void set( const char* pstr, size_t len );

	void format( const char*fmt, ... );
	void append( const char* src, size_t ilen );
	void setempty();
	bool replace( const char* src, const char* from, const char* to, ork_cstr_replace_pred pred=0 );
	bool replace( const char* src, const char from, const char to, ork_cstr_replace_pred pred=0 );
	//////////////////////////////////////////////////////////
	void operator= ( const fixedstring& oth );
	bool operator == ( const fixedstring& oth ) const;
	bool operator == ( const char* oth ) const;
	bool operator < ( const fixedstring& oth ) const;
	bool operator != ( const fixedstring& oth ) const;
	fixedstring operator + ( const fixedstring& oth ) const;
	void operator += ( const char* oth );
	void operator += ( const fixedstring_base& oth );
	char operator [] ( const size_type & i ) const;
	char&operator [] ( const size_type & i );
	size_t hash_for_map() const;
	//////////////////////////////////////////////////////////
	fixedstring substr( size_type first=0, size_type length=npos ) const;
	size_type find ( const char* s, size_t pos = 0 ) const;
	size_type find ( const fixedstring& s, size_t pos = 0 ) const;
	fixedstring& replace( size_type pos1, size_type n1, const fixedstring& str );
	//////////////////////////////////////////////////////////
	bool empty() const;
	size_type capacity() const { return size_t(kMAXLEN); }
	void recalclen();
	//////////////////////////////////////////////////////////
	fixedstring();
	fixedstring(const char*pstr);
	fixedstring(const char*pstr, size_t len);
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	//////////////////////////////////////////////////////////
	static int FastStrCmp(const fixedstring& s1, const fixedstring& s2);
	size_type cue_to_char( char cch, size_t start ) const;
	size_type find_first_of( const char* srch ) const;
	size_type find_last_of( const char* srch ) const;
	void resize( size_t n, char c=0 );

	typedef char buffer_t[kmaxlen];
private:
	buffer_t buffer;

};

///////////////////////////////////////////////////////////////////////////////

class Char4 // run time low overhead std::string (max 4 characters)
{
    public: //

    void SetCString( const char *str );

    void SetU32( u32 uval )
    {
        muVal32 = uval;
    }

    const char *c_str( void ) const
    {
        static char rval[5];

        rval[0] = mCharMems[0];
        rval[1] = mCharMems[1];
        rval[2] = mCharMems[2];
        rval[3] = mCharMems[3];
        rval[4] = 0;

        return rval;
    }

    U32 GetU32( void ) const { return muVal32; }

    union
    {
        char mCharMems[4];
        U32  muVal32;
    };

	bool operator()( const Char4 & t1, const Char4 & t2) const
    {
        return bool( t1.muVal32 < t2.muVal32 );
    }

	bool operator==(const Char4 & t) const
    {
        return bool( GetU32() == t.GetU32() );
    }

	bool operator!=(const Char4 & t) const
    {
        return bool( GetU32() != t.GetU32() );
    }

	bool operator<(const Char4 & t) const
    {
        return bool( GetU32() < t.GetU32() );
    }

	Char4()
        : muVal32( 0 )
    {

    }

    Char4( U32 uval )
        : muVal32( uval )
    {

    }

    Char4( const char *str )
        : muVal32( 0 )
    {
        SetCString( str );
    }
};

///////////////////////////////////////////////////////////////////////////////

class Char8 // run time low overhead std::string (max 4 characters)
{
    public: //

    void SetCString( const char *str );

    inline void SetU64( u64 uval ) {  muVal64 = uval; }

    const char *c_str( void ) const
    {
        static char rval[9];

        rval[0] = mCharMems[0];
        rval[1] = mCharMems[1];
        rval[2] = mCharMems[2];
        rval[3] = mCharMems[3];

		rval[4] = mCharMems[4];
        rval[5] = mCharMems[5];
        rval[6] = mCharMems[6];
        rval[7] = mCharMems[7];

		rval[8] = 0;

        return rval;
    }

    inline U64 GetU64( void ) const { return muVal64; }

    union
    {
        char mCharMems[8];
        U64  muVal64;
    };

	inline bool operator()( const Char8 & t1, const Char8 & t2) const
    {   return bool( t1.muVal64 < t2.muVal64 );
    }
	inline bool operator==(const Char8 & t) const
    {   return bool( GetU64() == t.GetU64() );
    }
	inline bool operator!=(const Char8 & t) const
    {   return bool( GetU64() != t.GetU64() );
    }
	inline bool operator<(const Char8 & t) const
    {   return bool( GetU64() < t.GetU64() );
    }
	Char8()
        : muVal64( 0 )
    {
    }
    Char8( U32 uval )
        : muVal64( uval )
    {
    }
    Char8( const char *str )
        : muVal64( 0 )
    {   SetCString( str );
    }
};

///////////////////////////////////////////////////////////////////////////////

template<int tsize> using fxstring = fixedstring<tsize>;

typedef fxstring<16> fxstring16;
typedef fxstring<32> fxstring32;
typedef fxstring<64> fxstring64;
typedef fxstring<128> fxstring128;
typedef fxstring<256> fxstring256;

typedef fxstring<1024> PropTypeString;
typedef fxstring<1024> SerAnnoTypeString;
typedef fxstring<256> uristring_t;
typedef fxstring<16> extstring_t;


///////////////////////////////////////////////////////////////////////////////

//#include <ork/kernel/tempstring.hpp>

}

namespace std
{ template<size_t kount> struct hash<ork::fixedstring<kount>>
  {	inline size_t operator()(const ork::fixedstring<kount>& v) const
    {	return v.hash_for_map();
    }
  };
}
