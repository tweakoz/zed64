///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/fixedstring.h>
#include <string.h>
#include <stdarg.h>
#include <functional>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

//template <size_t kmaxlen>
//const unsigned int fixedstring<kmaxlen>::kMAXLEN = kmaxlen;

///////////////////////////////////////////////////////////////////////////////

inline const char* strrstr(const char* s1, const char* s2)
{
	if (*s2 == '\0') return((char *)s1);
		
	const char* ps1 = s1 + strlen(s1);
	
	while(ps1 != s1)
	{
		--ps1;

		const char* psc1;
		const char* sc2;
		for( psc1 = ps1, sc2 = s2; ; )
		{
			if (*(psc1++) != *(sc2++))
			{
				break;
			}
			else if (*sc2 == '\0')
			{
				return ((char *)ps1);
			}
		}
	}
	return ((char *)NULL);
}


///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::recalclen() 
{
	mLength = length();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::SetChar( size_t index, char ch )
{
	OrkAssert( index < kmaxlen );
	buffer[ index ] = ch;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::set( const char* pstr )
{
	if( pstr )
	{
		int len = int(strlen( pstr ));
		if( len >= kmaxlen )
		{
			printf( "uhoh, <%s>\n, a string %d chars in length is being put into a fixedstring<%d>\n", pstr, len, int(kmaxlen) );
			assert( false );
		}
		strncpy( buffer, pstr, kmaxlen );
	}
	else
	{
		buffer[0] = 0;
	}
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::set( const char* pstr, size_t len )
{
	if( pstr )
	{
		if( len >= kmaxlen )
		{
			//orkprintf( "uhoh, <%s>\n, a string %d chars in length is being put into a fixedstring<%d>\n", pstr, len, int(kmaxlen) );
			assert( false );
		}
		::strncpy(buffer, pstr, kmaxlen);
	}
	else
	{
		buffer[0] = 0;
	}
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::append( const char*src, size_t ilen )
{
	size_t icurlen = strlen(buffer);
	OrkAssert( icurlen+ilen < kmaxlen );
	strncpy( & buffer[ icurlen ], src, ilen );
	buffer[ icurlen+ilen ] = 0;
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::format( const char*fmt, ... )
{
	va_list argp;
	va_start(argp, fmt);
	vsnprintf( & buffer[0], kmaxlen, fmt, argp);
	va_end(argp);
	recalclen();
	OrkAssert( mLength < kmaxlen );
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::setempty()
{
	buffer[0] = 0;
	buffer[1] = 0;
	mLength = 0;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::operator += ( const char *oth )
{
	assert((length()+strlen(oth))<(kmaxlen-1));
	fixedstring<kmaxlen> tmp = *this;
	this->format( "%s%s", tmp.c_str(), oth );
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::operator += ( const fixedstring_base& oth )
{
	assert((length()+oth.length())<(kmaxlen-1));
	fixedstring<kmaxlen> tmp = *this;
	this->format( "%s%s", tmp.c_str(), oth.c_str() );
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen> fixedstring<kmaxlen>::operator + ( const fixedstring& oth ) const
{
	assert((length()+oth.length())<(kmaxlen-1));
	fixedstring<kmaxlen> tmp;
	tmp.format( "%s%s", c_str(), oth.c_str() );
	//recalclen();
	return tmp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::operator = ( const fixedstring& oth )
{
	assert(oth.length()<(kmaxlen-1));
	strncpy( buffer, oth.buffer, kmaxlen );
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::operator == ( const fixedstring& oth ) const
{
	return (0 == strcmp( c_str(), oth.c_str() ));
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::operator == ( const char* oth ) const
{
	return (0 == strcmp( c_str(), oth ));
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::operator < ( const fixedstring& oth ) const
{
	return (0 > strcmp( c_str(), oth.c_str() ));
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::operator != ( const fixedstring& oth ) const
{
	return (0 != strcmp( c_str(), oth.c_str() ));
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::empty() const
{
	return length() == 0;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::fixedstring()
{
	setempty();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::fixedstring(const char*pstr)
{
	set( pstr );
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::fixedstring(const char* pstr, size_t len)
{
	set( pstr, len );
	recalclen();
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen> size_t fixedstring<kmaxlen>::hash_for_map() const
{
	size_t res = 0;
	const int ks = sizeof(size_t);
	char* pch = (char*) &res;
	for( int i=0; i<mLength; i++ )
	{
		int imod = i%ks;
		pch[imod] ^= buffer[i];
	}

	return res; //std::_Hash_impl::hash(buffer, mLength);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::replace( const char* src, const char* from, const char* to, ork_cstr_replace_pred pred )
{
	bool bok = cstr_replace( src, from, to, buffer, kmaxlen,pred);
	recalclen();
	return bok;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::replace( const char* src, const char from, const char to, ork_cstr_replace_pred pred )
{
	size_t ilen = strlen(src);
	OrkAssert(ilen<kmaxlen);
	for( size_t i=0; i<ilen; i++ )
	{
		char ch = src[i];
		if( ch == from ) ch = to;
		buffer[i] = ch;
	}
	recalclen();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
int fixedstring<kmaxlen>::FastStrCmp( const fixedstring& a, const fixedstring& b )
{
	return strcmp( a.c_str(), b.c_str() );
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
char fixedstring<kmaxlen>::operator[] ( const size_type& i ) const
{
	OrkAssert( i < size_type(mLength) );
	OrkAssert( i < kmaxlen );

	return buffer[i];
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
char& fixedstring<kmaxlen>::operator[] ( const size_type& i )
{
	OrkAssert( i < size_type(mLength) );
	OrkAssert( i < kmaxlen );

	return buffer[i];
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen> fixedstring<kmaxlen>::substr( size_type first, size_type length ) const
{
	fixedstring ret;
	size_type last = ( length==npos ) ? this->length() : (first+length);

	size_type count = (last-first);
	if( count > 0 )
	{
		OrkAssert( count < size_type(kmaxlen) );
		strncpy( ret.buffer, c_str()+first, count );
		ret.SetChar(count,0);
	}
	ret.recalclen();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::size_type fixedstring<kmaxlen>::cue_to_char( char cch, size_t start ) const
{	size_type slen = size();
	bool found = false;
	size_type idx = size_type(start);
	size_type rval = npos;
	for(size_type i = idx; ((i<slen)&&(found==false)); i++)
	{
		if(cch == buffer[i])
		{
			found = true;
			rval = i;
		}
	}
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::size_type fixedstring<kmaxlen>::find_first_of( const char* srch ) const
{
	size_type rval = npos;
	const char* pfound = strstr( buffer, srch );
	if( pfound )
	{
		rval = (pfound-buffer);
	}

	return rval;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::size_type fixedstring<kmaxlen>::find_last_of( const char* srch ) const
{
	size_type rval = npos;
	const char* pfound = strrstr( buffer, srch );
	if( pfound )
	{
		rval = (pfound-buffer);
	}

	return rval;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::size_type fixedstring<kmaxlen>::find( const char* srch, size_t pos ) const
{
	size_type rval = npos;
	const char* pfound = strstr( buffer+pos, srch );
	if( pfound )
	{
		rval = (pfound-buffer);
	}

	return rval;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::size_type fixedstring<kmaxlen>::find( const fixedstring& s, size_t pos ) const
{
	return find( s.c_str(), pos );
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>& fixedstring<kmaxlen>::replace( size_type pos1, size_type n1, const fixedstring& str )
{
	size_t inlen = str.length();
	size_t iolen = n1;
	intptr_t isizediff = iolen-inlen;

	OrkAssert( pos1+inlen < kmaxlen );
	memcpy( buffer+pos1, str.c_str(), inlen );

	if( isizediff > 0 )
	{
		intptr_t idst = pos1+inlen;
		intptr_t isrc = pos1+inlen+isizediff;
		intptr_t ilen = (mLength-isrc)+1;

		OrkAssert( idst+ilen < kmaxlen );
		memcpy( buffer+idst, c_str()+isrc, ilen );
	}

	recalclen();
	
	return *this;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::begin()
{
	size_t idx = (size()>0) ? 0 : npos;
	return iterator(idx,+1,this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::end()
{
	return iterator(npos,+1,this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::begin() const
{
	size_t idx = (size()>0) ? 0 : npos;
	return const_iterator(idx,+1,this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::end() const
{
	return const_iterator(npos,+1,this);
}

///////////////////////////////////////////////////////////////////////////////
// fixedstring<kmaxlen>::iterator_base
///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::iterator_base::iterator_base( size_t idx, int idir )
	: mindex( idx )
	, mdirection(idir)
{

}

///////////////////////////////////////////////////////////////////////////////
// fixedstring<kmaxlen>::iterator
///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::iterator::iterator( size_t idx, int idir, fixedstring* pfm)
	: mIteratorBase(idx,idir)
	, mpString(pfm)
{
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::iterator::iterator(const iterator& oth)
	: mIteratorBase(oth.mIteratorBase.mindex,oth.mIteratorBase.mdirection)
	, mpString(oth.mpString)
{
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::iterator::operator==(const iterator& oth ) const
{	if( oth.mpString != mpString )
		return false;
	if( oth.mIteratorBase.mdirection != mIteratorBase.mdirection  )
		return false;
	if( oth.mIteratorBase.mindex == mIteratorBase.mindex )
	{	return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::iterator::operator!=(const iterator& oth ) const
{	return ! operator == ( oth );
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator::pointer fixedstring<kmaxlen>::iterator::operator->() const
{
	OrkAssert( mpString != 0 );
	size_t isize = mpString->size();
	OrkAssert( mIteratorBase.mindex >= 0 );
	OrkAssert( mIteratorBase.mindex < isize );
	OrkAssert( mIteratorBase.mindex < kmaxlen );
	typename fixedstring<kmaxlen>::iterator::value_type* p0 = 
		(mIteratorBase.mdirection>0) ? &mpString->buffer[mIteratorBase.mindex] : &mpString->buffer[(isize-1)-mIteratorBase.mindex];
	return p0;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator::reference fixedstring<kmaxlen>::iterator::operator *() const
{
	return *(this->operator->());
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator--() // prefix
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex--;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator++() // prefix
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex++;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator--(int i) // postfix
{
	OrkAssert( mpString );
	iterator temp( *this );
	size_t isize = mpString->size();
	mIteratorBase.mindex--;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator++(int i) // postfix
{
	OrkAssert( mpString );
	iterator temp( *this );
	size_t isize = mpString->size();
	mIteratorBase.mindex++;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator+(int i) const// add
{
	OrkAssert( mpString );
	iterator temp( *this );
	size_t isize = mpString->size();
	temp.mIteratorBase.mindex+=i;
	if( temp.mIteratorBase.mindex >= isize )
	{
		temp.mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator-(int i) const// sub
{
	OrkAssert( mpString );
	iterator temp( *this );
	size_t isize = mpString->size();
	temp.mIteratorBase.mindex-=i;
	if( temp.mIteratorBase.mindex >= isize )
	{
		temp.mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator+=(int i) // add
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex+=i;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator fixedstring<kmaxlen>::iterator::operator-=(int i) // sub
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex-=i;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::iterator::operator < ( const iterator& oth ) const
{
	OrkAssert( oth.mpString == mpString );
	OrkAssert( oth.mIteratorBase.mdirection == mIteratorBase.mdirection );

	bool othNPOS = ( oth.mIteratorBase.mindex == npos );
	bool thsNPOS = ( mIteratorBase.mindex == npos );

	int index = int(othNPOS)+(int(thsNPOS)<<1);
	bool btable[4] = 
	{
		oth.mIteratorBase.mindex < mIteratorBase.mindex,// 0==neither
		true, // 1==othNPOS
		false, // 2==thsNPOS
		false // 3==BOTH
	};

	return btable[index];
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::iterator::operator = ( const iterator& oth )
{
	mpString = oth.mpString;
	mIteratorBase.mindex = oth.mIteratorBase.mindex;
	mIteratorBase.mdirection = oth.mIteratorBase.mdirection;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::iterator::difference_type fixedstring<kmaxlen>::iterator::operator - ( const iterator& oth ) const
{
	OrkAssert( mpString );
	OrkAssert( oth.mpString );
	OrkAssert( mpString==oth.mpString );
	OrkAssert( oth.mIteratorBase.mdirection == mIteratorBase.mdirection );
	OrkAssert( mIteratorBase.mindex < mpString->size() || mIteratorBase.mindex==npos );
	OrkAssert( oth.mIteratorBase.mindex < oth.mpString->size() || oth.mIteratorBase.mindex==npos );
	typename fixedstring<kmaxlen>::iterator::difference_type defval = (mIteratorBase.mindex-oth.mIteratorBase.mindex);
	size_t othsize = oth.mpString->size();
	if( mIteratorBase.mindex==npos && oth.mIteratorBase.mindex<othsize )
	{
		defval = mpString->size()-oth.mIteratorBase.mindex;
	}
	return defval;
}

///////////////////////////////////////////////////////////////////////////////
// fixedstring<kmaxlen>::const_iterator
///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::const_iterator::const_iterator( size_t idx, int idir, const fixedstring* pfm)
	: mIteratorBase(idx,idir)
	, mpString(pfm)
{
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::const_iterator::const_iterator(const iterator& oth)
	: mIteratorBase(oth.mIteratorBase.mindex,oth.mIteratorBase.mdirection)
	, mpString(oth.mpString)
{
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
fixedstring<kmaxlen>::const_iterator::const_iterator(const const_iterator& oth)
	: mIteratorBase(oth.mIteratorBase.mindex,oth.mIteratorBase.mdirection)
	, mpString(oth.mpString)
{
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::const_iterator::operator == (const const_iterator& oth ) const
{	OrkAssert( oth.mpString == mpString );
	OrkAssert( oth.mIteratorBase.mdirection == mIteratorBase.mdirection );
	if( oth.mIteratorBase.mindex == mIteratorBase.mindex )
	{	return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::const_iterator::operator != (const const_iterator& oth ) const
{	return ! operator == ( oth );
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator::const_pointer fixedstring<kmaxlen>::const_iterator::operator ->() const
{
	OrkAssert( mpString != 0 );
	size_t isize = mpString->size();
	OrkAssert( mIteratorBase.mindex >= 0 );
	OrkAssert( mIteratorBase.mindex < isize );
	const typename fixedstring<kmaxlen>::iterator::value_type* p0 = 
		(mIteratorBase.mdirection>0) ? &mpString->c_str()[mIteratorBase.mindex] : &mpString->c_str()[(isize-1)-mIteratorBase.mindex];
	return p0;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator::const_reference fixedstring<kmaxlen>::const_iterator::operator *() const
{
	return *(this->operator->());
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator++() // prefix
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex++;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::const_iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator--() // prefix
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex--;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::const_iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator++(int i) // postfix
{
	OrkAssert( mpString );
	const_iterator temp( *this );
	size_t isize = mpString->size();
	mIteratorBase.mindex++;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator--(int i) // postfix
{
	OrkAssert( mpString );
	const_iterator temp( *this );
	size_t isize = mpString->size();
	mIteratorBase.mindex--;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator+(int i) const // add
{
	OrkAssert( mpString );
	const_iterator temp( *this );
	size_t isize = temp.mpString->size();
	temp.mIteratorBase.mindex+=i;
	if( temp.mIteratorBase.mindex >= isize )
	{
		temp.mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator-(int i) const // add
{
	OrkAssert( mpString );
	const_iterator temp( *this );
	size_t isize = temp.mpString->size();
	temp.mIteratorBase.mindex-=i;
	if( temp.mIteratorBase.mindex >= isize )
	{
		temp.mIteratorBase.mindex = npos;
	}
	return temp;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator+=(int i) // add
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex+=i;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::const_iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator fixedstring<kmaxlen>::const_iterator::operator-=(int i) // sub
{
	OrkAssert( mpString );
	size_t isize = mpString->size();
	mIteratorBase.mindex-=i;
	if( mIteratorBase.mindex >= isize )
	{
		mIteratorBase.mindex = npos;
	}
	return typename fixedstring<kmaxlen>::const_iterator(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
bool fixedstring<kmaxlen>::const_iterator::operator < ( const const_iterator& oth ) const
{
	OrkAssert( oth.mpString == mpString );
	OrkAssert( oth.mIteratorBase.mdirection == mIteratorBase.mdirection );

	bool othNPOS = ( oth.mIteratorBase.mindex == npos );
	bool thsNPOS = ( mIteratorBase.mindex == npos );

	int index = int(othNPOS)+(int(thsNPOS)<<1);
	bool btable[4] = 
	{
		oth.mIteratorBase.mindex < mIteratorBase.mindex,// 0==neither
		true, // 1==othNPOS
		false, // 2==thsNPOS
		false // 3==BOTH
	};

	return btable[index];
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::const_iterator::operator = ( const const_iterator& oth )
{
	mpString = oth.mpString;
	mIteratorBase.mindex = oth.mIteratorBase.mindex;
	mIteratorBase.mdirection = oth.mIteratorBase.mdirection;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
typename fixedstring<kmaxlen>::const_iterator::difference_type fixedstring<kmaxlen>::const_iterator::operator - ( const typename fixedstring<kmaxlen>::const_iterator& oth ) const
{
	OrkAssert( mpString );
	if( 0 != oth.mpString )
	{
		OrkAssert( mpString==oth.mpString );
	}
	OrkAssert( oth.mIteratorBase.mdirection == mIteratorBase.mdirection );
	OrkAssert( mIteratorBase.mindex < mpString->size() || mIteratorBase.mindex == npos );
	OrkAssert( oth.mIteratorBase.mindex < oth.mpString->size() || oth.mIteratorBase.mindex == npos );
	typename fixedstring<kmaxlen>::const_iterator::difference_type defval = (mIteratorBase.mindex-oth.mIteratorBase.mindex);
	return defval;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t kmaxlen>
void fixedstring<kmaxlen>::resize( size_t n, char c )
{
	OrkAssert( n>=0 );
	OrkAssert( n<(kmaxlen-2) );

	if( n > mLength )
	{	for( size_t i=mLength; i<n; i++ )
		{
			buffer[i+1] = c;			
		}
		buffer[n+1] = 0;
		mLength = n;
	}
	else if( n < mLength )
	{
		buffer[n+1] = 0;
		mLength = n;
	}
}

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////
