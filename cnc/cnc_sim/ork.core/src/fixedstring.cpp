///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/fixedstring.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

size_t fixedstring_base::length() const
{
    return int(strlen( c_str() ));
}

fixedstring_base::fixedstring_base()
    : mLength(0)
{
    
}

///////////////////////////////////////////////////////////////////////////////

bool cstr_replace(  const char *src,
                    const char *from,
                    const char *to,
                    char* dest,
                    const size_t idestlen,
                    ork_cstr_replace_pred pred
                )
{
    const int nummarkers = 8;

    size_t isrclen = strlen(src);
    size_t ifromlen = strlen(from);
    size_t itolen = strlen(to);

    bool bdone = false;
    bool brval = true;
    const char* src_marker = src;
    const char* src_end = src+isrclen;
    char* dst_marker = dest;

    //src: whatupyodiggittyyoyo
    //from: yo
    //to: damn


    while( false==bdone )
    {
        const char* search = strstr( src_marker, from );

        //printf( "search<%s> src_marker<%s> from<%s> to<%s> src<%s> dest<%s>\n", search, src_marker, from, to, src, dest );
        //search<yodiggittyyoyo> src_marker<whatupyodiggittyyoyo> dest<>
        //search<yoyo> src_marker<diggittyyoyo> dest<whatupdamn>

        /////////////////////////////////////
        // copy [src_marker..search] ->output
        /////////////////////////////////////
        if( (search!=0) && (search >= src_marker) )
        {
            size_t ilen = size_t(search-src_marker);
            OrkAssert( (dst_marker+ilen) <= (dest+idestlen) );
            strncpy( dst_marker, src_marker, ilen );
            dst_marker += ilen;
            src_marker += ilen;
        }
        /////////////////////////////////////
        // copy "to" -> output, advance input by ifromlen
        /////////////////////////////////////
        if( (search!=0) )
        {
            bool doit = true;
            if( pred )
            {
                doit = pred( src, src_marker, isrclen );
            }

            if( doit )
            {
                OrkAssert( (dst_marker+itolen) <= (dest+idestlen) );
                strncpy( dst_marker, to, itolen );
                dst_marker += itolen;
                src_marker += ifromlen;
            }
            else
            {
                OrkAssert( dst_marker<(dest+idestlen) );
                dst_marker[0] = src_marker[0];
                dst_marker ++;
                src_marker ++;
            }

        }

        /////////////////////////////////////
        // copy [mkr..end] -> output
        /////////////////////////////////////
        else
        {
                size_t ilen = isrclen - (src_marker-src);
                strncpy( dst_marker, src_marker, ilen );
                dst_marker += ilen;
                src_marker += ilen;
        }

        bdone = (src_marker>=src_end);
    }

    dst_marker[0] = 0;
    dst_marker++;

    size_t inewlen = size_t(dst_marker-dest);
    OrkAssert( inewlen<idestlen);
    return brval;

}


///////////////////////////////////////////////////////////////////////////////
// template instantiations
///////////////////////////////////////////////////////////////////////////////

template class fixedstring<4096>;
template class fixedstring<2048>;
template class fixedstring<1024>;
template class fixedstring<512>;
template class fixedstring<256>;
template class fixedstring<128>;
template class fixedstring<64>;
template class fixedstring<32>;
template class fixedstring<33>;
template class fixedstring<16>;
template class fixedstring<8>;
template class fixedstring<5>;
template class fixedstring<4>;

///////////////////////////////////////////////////////////////////////////////

};

