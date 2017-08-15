///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/ringbuffer.hpp>

namespace ork {

///////////////////////////////////////////////////////////////////////////////

//! single producer single consumer thread safe bounded queue 
template <typename T,size_t max_items=256> 
struct spsc_bounded_queue
{
	typedef SpScRingBuf<T,max_items> impl_t;
	typedef T value_type;

	spsc_bounded_queue()
		: mImpl()
	{

	}
	~spsc_bounded_queue()
	{

	}
	void push(const T& item) // blocking
	{
		mImpl.push(item);
	}
	bool try_push(const T& item) // non-blocking
	{
		return mImpl.try_push(item);
	}
	bool try_pop(T& item) // non-blocking
	{
		return mImpl.try_pop(item);
	}
    T pop() // blocking
    {
        return mImpl.pop();
    }

	impl_t mImpl;
	static const size_t kSIZE = sizeof(T);
};

///////////////////////////////////////////////////////////////////////////////

//! multiple producer multiple consumer thread safe bounded queue 
template <typename T,size_t max_items=256> 
struct mpmc_bounded_queue
{
	typedef MpMcRingBuf<T,max_items> impl_t;
	typedef T value_type;
	
	mpmc_bounded_queue()
		: mImpl()
	{

	}
	~mpmc_bounded_queue()
	{

	}
	void push(const T& item) // blocking
	{
		mImpl.push(item);
	}
	bool try_push(const T& item) // non-blocking
	{
		return mImpl.try_push(item);
	}
	bool try_pop(T& item) // non-blocking
	{
		return mImpl.try_pop(item);
	}

	impl_t mImpl;
	static const size_t kSIZE = sizeof(T);
};



}
