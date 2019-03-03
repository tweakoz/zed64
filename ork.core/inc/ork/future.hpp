///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "svariant.h"
#include <functional>

namespace ork {

///////////////////////////////////////////////////////////////////
/// *Future* : see http://en.wikipedia.org/wiki/Futures_and_promises
///////////////////////////////////////////////////////////////////

struct Future
{
	typedef svar160_t var_t; //!< result container type, you will get an assert if the result overflows this
	typedef std::function<void(const Future& fut)> fut_blk_cb_t; //!< callback lambda typedef

	Future();
	bool IsSignaled() const { return mState>0; }
	template <typename T> void Signal( const T& result );
	void Clear();
	void WaitForSignal() const; //!< Block until signaled
	const var_t& GetResult() const; //!< Get the result of the future, block if necessary
	void SetCallback( const fut_blk_cb_t& cb );  //!< Set callback for asynch execution

	////////////////////

	ork::atomic<int> mState;
	var_t 	mResult;
	var_t 	mCallback;

};

template <typename T>
void Future::Signal( const T& result )
{
	mResult.Set<T>(result);

    mState++;

    if( mCallback.IsA<fut_blk_cb_t>() )
    {
        const fut_blk_cb_t& blk = mCallback.Get<fut_blk_cb_t>();
        blk(*this);
    }

}

//////////////////////////////////////////////////////////////////////////
// RpcFuture : future usable for IPC (has an ID for lookup)
//////////////////////////////////////////////////////////////////////////

struct RpcFuture : public Future
{
	typedef int rpc_future_id_t;

	RpcFuture() : mID(0) {}
	
	void SetId(rpc_future_id_t id) { mID=id; }
	rpc_future_id_t GetId() const { return mID; }

	rpc_future_id_t mID;

};



} // namespace ork