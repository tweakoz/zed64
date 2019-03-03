///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sys/types.h>
#include <assert.h>
#include <ork/traits.h>
#include <stdlib.h>
#include <string.h>
#include <ork/fixedstring.h>
#include <ork/atomic.h>

namespace ork {

template <size_t ksize> struct MessagePacket;
typedef MessagePacket<4096> NetworkMessage;

//! data read marker/iterator for a NetworkMessage
template <size_t ksize> struct MessagePacketIterator
{   ///////////////////////////////////////////////////
    // Constructor: NetworkMessageIterator
    //
    // construct a Network Message Read Iterator
    //
    // Parameters:
    //
    // msg - Network Message that this iterator will iterate upon
    //
    MessagePacketIterator( const MessagePacket<ksize>& msg )
        : mMessage(msg)
        , miReadIndex(0)
    {

    }

    ///////////////////////////////////////////////////
    void Clear() { miReadIndex=0; }
    ////////////////////////////////////////////////////
    // Variable: mMessage
    // Network Message this iterator is iterating inside
    const MessagePacket<ksize>&     mMessage;
    ////////////////////////////////////////////////////
    // Variable: miReadIndex
    // Read Byte Index into the message
    int                             miReadIndex;
    ////////////////////////////////////////////////////
};

typedef MessagePacketIterator<4096> NetworkMessageIterator;

//! message packet (an atomic network message)
//! statically sized, storage for the message is embedded.
//! This allow explicit allocation policies, such as embedding in shared memory

struct MessagePacketBase
{
    size_t                  miSize;
    mutable uint64_t        miSerial;
    mutable uint64_t        miTimeSent;

    ///////////////////////////////////////////////////////
    void clear() { miSize = 0; }
    ///////////////////////////////////////////////////////
    virtual const void* GetData() const = 0;
    virtual void* GetData() = 0;
    virtual size_t GetMax() const = 0;
    size_t GetLength() const { return miSize; }
    ///////////////////////////////////////////////////////
};

template <size_t ksize> struct MessagePacket : public MessagePacketBase
{   
    typedef MessagePacketIterator<ksize> iter_t;

    ///////////////////////////////////////////////////////
    // constructor: MessagePacket
    //
    // construct a message packet
    //
    ///////////////////////////////////////////////////////
    MessagePacket() { clear(); }
    ///////////////////////////////////////////////////////
    // method: Write
    //
    // add data to the content of this message, increment write index by size of T
    //
    // Parameters:
    //
    // inp - the data to add
    //
    ///////////////////////////////////////////////////////
    static const int kstrbuflen = ksize/2;
    template <typename T> void Write( const T& inp )
    {
        static_assert(ork::is_trivially_copyable<T>::value,"can only write is_trivially_copyable's into a NetworkMessage!");
        size_t ilen = sizeof(T);
        WriteDataInternal((const void*) & inp, ilen);
    }
    void WriteString( const char* pstr )
    {
        size_t ilen = strlen(pstr)+1;
        assert( (miSize+ilen)<kmaxsize );
        assert( ilen<kstrbuflen );
        WriteDataInternal(&ilen,sizeof(ilen));
        WriteDataInternal(pstr,ilen);
        //for( size_t i=0; i<ilen; i ++ )
        //  Write( pstr[i] ); // slow method, oh well doesnt matter now...
    }
    void WriteData( const void* pdata, size_t ilen )
    {
        assert( (miSize+ilen)<=kmaxsize );
        Write(ilen);
        WriteDataInternal(pdata,ilen);
    }
    void WriteDataInternal( const void* pdata, size_t ilen )
    {
        assert( (miSize+ilen)<=kmaxsize );
        const char* pch = (const char*) pdata;
        char* pdest = & mBuffer[miSize];
        memcpy( pdest, (const char*) pdata, ilen );
        miSize += ilen;
    }
    ///////////////////////////////////////////////////////
    // method: Read
    //
    // read data from the content of this message at a given position specified by an iterator, then increment read index by size of T
    //
    // Parameters:
    //
    // it - the data's read iterator
    //
    ///////////////////////////////////////////////////////
    template <typename T> void Read( T& outp, iter_t& it ) const
    {
        static_assert(ork::is_trivially_copyable<T>::value,"can only read is_trivially_copyable's from a NetworkMessage!");
        int ilen = sizeof(T);
        assert( (it.miReadIndex+ilen)<=kmaxsize );
        ReadDataInternal((void*)&outp,ilen,it);
    }
    std::string ReadString( iter_t& it ) const
    {
        size_t ilen = 0;
        char buffer[kstrbuflen];
        //Read( ilen, it );
        ReadDataInternal((void*)&ilen,sizeof(ilen),it);
        assert( ilen<kstrbuflen );
        ReadDataInternal((void*)buffer,ilen,it);
        //for( size_t i=0; i<ilen; i++ )
        //  Read( buffer[i], it );
        return std::string(buffer);
    }
    void ReadData( void* pdest, size_t ilen, iter_t& it ) const
    {
        size_t rrlen = 0;
        Read( rrlen, it );
        assert(rrlen==ilen);
        ReadDataInternal(pdest,ilen,it);
    }
    void ReadDataInternal( void* pdest, size_t ilen, iter_t& it ) const
    {
        const char* psrc = & mBuffer[it.miReadIndex];
        memcpy( (char*) pdest, psrc, ilen );
        it.miReadIndex += ilen;
    }
    ///////////////////////////////////////////////////////
    NetworkMessage& operator = ( const NetworkMessage& rhs )
    {
        miSize = rhs.miSize;
        miSerial = rhs.miSerial;
        miTimeSent = rhs.miTimeSent;

        if( miSize )
            memcpy(mBuffer,rhs.mBuffer,miSize);

        return *this;
    }
    ////////////////////////////////////////////////////
    void dump(const char* label)
    {
        size_t icount = miSize;
        size_t j = 0;
        while(icount>0)
        {
            uint8_t* paddr = (uint8_t*)(mBuffer+j);
            printf( "msg<%p:%s> [%02lx : ", this,label, j );
            size_t thisc = (icount>=16) ? 16 : icount;
            for( size_t i=0; i<thisc; i++ )
            {
                size_t idx = j+i;
                printf( "%02x ", paddr[i] );
            }
            j+=thisc;
            icount -= thisc;

            printf("\n");
        }
    }
    const void* GetData() const override { return (const void*) mBuffer; }
    void* GetData() override { return (void*) mBuffer; }
    size_t GetMax() const override { return kmaxsize; }
    size_t GetLength() const { return miSize; }
    
    ////////////////////////////////////////////////////
    // Constant: kmaxsize
    // maximum size of content of a single atomic network message
    static const size_t kmaxsize = ksize;
    ////////////////////////////////////////////////////
    // Variable: mBuffer
    // statically sized buffer for holding a network message's content
    char                mBuffer[kmaxsize];
    ////////////////////////////////////////////////////
};

//! Message Stream Abstract interface 
//! allows bidirectional serialization with half the code (SerDes)

///////////////////////////////////////////////////////////////////////////////

template <typename T> struct MessageStreamTraits;

///////////////////////////////////////////////////////////////////////////////

struct MessageStreamBase
{
    template <typename T> MessageStreamBase& operator || ( T& inp );

    virtual void SerDesImpl( void* pdata, size_t len ) = 0;
    virtual void SerDesStringImpl( std::string& str ) = 0;
    virtual void SerDesFixedStringImpl( fixedstring_base& str ) = 0;
    virtual bool IsOutputStream() const = 0;

    NetworkMessage          mMessage;

};
///////////////////////////////////////////////////////////////////////////////
template <typename T> struct MessageStreamTraits
{   static void SerDes(MessageStreamBase& stream_bas, T& data_ref)
    {   static_assert(is_trivially_copyable<T>::value,"can only write is_trivially_copyable's into a NetworkMessage!");
        size_t ilen = sizeof(T);
        stream_bas.SerDesImpl(&data_ref,ilen);
    }
};
///////////////////////////////////////////////////////////////////////////////
// TODO - figure out how to templatize on size of the fxstring!
template<> struct MessageStreamTraits<fxstring<64>>
{   static void SerDes(MessageStreamBase& stream_bas, fxstring<64>& data_ref)
    {   stream_bas.SerDesFixedStringImpl(data_ref);
    }
};
///////////////////////////////////////////////////////////////////////////////
template<> struct MessageStreamTraits<std::string>
{   static void SerDes(MessageStreamBase& stream_bas, std::string& data_ref)
    {   stream_bas.SerDesStringImpl(data_ref);
    }
};
///////////////////////////////////////////////////////////////////////////////

template <typename T> inline MessageStreamBase& MessageStreamBase::operator || ( T& inp )
{   
    MessageStreamTraits<T>::SerDes(*this,inp);
    //this->SerDes(inp);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

//! Message Stream outgoing stream (write into NetworkMessage )
struct MessageOutStream : public MessageStreamBase // into netmessage
{
    void SerDesFixedStringImpl( ork::fixedstring_base& str ) override
    {
        mMessage.WriteString(str.c_str());
    }
    void SerDesStringImpl( std::string& str ) override
    {
        mMessage.WriteString(str.c_str());
    }
    void SerDesImpl( void* pdata, size_t ilen ) override
    {
        mMessage.WriteDataInternal(pdata,ilen);
    }
    template <typename T> void WriteItem( T& item )
    {
        item.SerDes(*this);
    }
    bool IsOutputStream() const override { return true; }

};

//! Message Stream outgoing stream (read from NetworkMessage )
struct MessageInpStream : public MessageStreamBase // outof netmessage
{
    MessageInpStream() : mIterator(mMessage) {}

    void SerDesFixedStringImpl( ork::fixedstring_base& str ) override
    {
        // todo make me more efficient!!!
        std::string inp = mMessage.ReadString(mIterator);
        str.set(inp.c_str());
    }
    void SerDesStringImpl( std::string& str ) override
    {
        str = mMessage.ReadString(mIterator);
    }
    void SerDesImpl( void* pdata, size_t ilen ) override
    {
        mMessage.ReadDataInternal(pdata,ilen,mIterator);
    }

    template <typename T> void ReadItem( T& item )
    {
        item.SerDes(*this);
    }
    bool IsOutputStream() const override { return false; }

    NetworkMessageIterator  mIterator;


};

} // ork

