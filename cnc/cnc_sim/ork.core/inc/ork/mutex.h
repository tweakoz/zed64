///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pthread.h>
#include <functional>

namespace ork {
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class Mutex
{
public:
	//////////////////////////////////
	Mutex();
	~Mutex();
	//////////////////////////////////
	void Lock();	// recursively lock
	void Unlock();	// recursively unlock
	//////////////////////////////////	
	bool IsLocked() const { return mbLocked; }
	//////////////////////////////////
private:
	pthread_mutex_t		mMutex;
	pthread_mutexattr_t	mMutexAttr;
	bool				mbLocked;

};

///////////////////////////////////////////////////////////////////////////////

//! A locked resource is just a mutex wrapped resource which must be locked to access
//!  dont forget to unlock it when done.... (TODO: RAII based auto-unlock)

template <typename T> struct LockedResource
{
    typedef std::function<void(T& ref)> ref_lambda_t;
    typedef std::function<void(const T& ref)> const_ref_lambda_t;
	//////////////////////////////////
	LockedResource()
	{
	}
	//////////////////////////////////
	~LockedResource()
	{	
		//MyAssert( false == mResourceMutex.IsLocked() );
	}	
	//////////////////////////////////
	const T& LockForRead() const
	{	mResourceMutex.Lock();
		return mResource;
	}
	//////////////////////////////////
	T& LockForWrite()
	{	mResourceMutex.Lock();
		return mResource;
	}
	//////////////////////////////////
	void Unlock() const
	{	mResourceMutex.Unlock();
	}
	//////////////////////////////////
   void AtomicCopy( T& out_copy ) const // perhaps we can try out c++11 move semantics with this
    {
        mResourceMutex.Lock();
        out_copy = mResource;
        mResourceMutex.Unlock();
    }
	T AtomicCopy() const
	{
		mResourceMutex.Lock();
		T rval = mResource;
		mResourceMutex.Unlock();
		return rval;
	}
	//////////////////////////////////
	void AtomicOp( const ref_lambda_t& op )
	{
		mResourceMutex.Lock();
		op(mResource);
		mResourceMutex.Unlock();
	}
	//////////////////////////////////
	void AtomicOp( const const_ref_lambda_t& op ) const
	{
		mResourceMutex.Lock();
		op(mResource);
		mResourceMutex.Unlock();
	}
	//////////////////////////////////
private:
	T										mResource;
	mutable Mutex							mResourceMutex;
};

///////////////////////////////////////////////////////////////////////////////
} //namespace ork {
