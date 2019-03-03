///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/types.h>
#include <ork/concurrent_queue.hpp>
#include <ork/svariant.h>
#include <ork/atomic.h>
#include <ork/thread.h>
#include <semaphore.h>
#include <string>

#include <set>

typedef ork::svar128_t op_wrap_t;


//////////////////////////////////////////////////////////////////////
namespace ork {

struct OpMultiQ;
struct Future; 

void dispersed_sleep(int idx, int iquantausec );

//////////////////////////////////////////////////////////////////////

void SetCurrentThreadName(const char* threadName);

//////////////////////////////////////////////////////////////////////

struct BarrierSyncReq
{
	BarrierSyncReq(ork::Future&f):mFuture(f){}
	ork::Future& mFuture;
};

//////////////////////////////////////////////////////////////////////

struct NOP {};

//////////////////////////////////////////////////////////////////////

//! operation for an OpQ
struct Op
{
	op_wrap_t mWrapped;	
	std::string mName;

	Op(const Op& oth);
	Op(const BarrierSyncReq& op,const std::string& name="");
	Op(const void_lambda_t& op,const std::string& name="");
	//Op(const void_block_t& op,const std::string& name="");
	Op();
	~Op();

	void SetOp(const op_wrap_t& op);

	void QueueASync(OpMultiQ&q) const;
	void QueueSync(OpMultiQ&q) const;;
};

//////////////////////////////////////////////////////////////////////

//! collection of operations under a named group
//! also has a set of policies governing execution of said operations (such as max concurrency, etc..)
struct OpGroup
{

	OpGroup(OpMultiQ*popq, const char* pname);
	void push( const Op& the_op );
	bool try_pop( Op& out_op );
	void drain();
	void MakeSerial() { mLimitMaxOpsInFlight=1; }
	void Disable() { mEnabled=false; }
	int NumOps() const { return mOpsPendingCounter.get(); }
	void notify_op_complete();
	
	////////////////////////////////

	std::string mGroupName;
	OpMultiQ* mpOpQ;

	////////////////////////////////

	ork::mpmc_bounded_queue<Op,65536> 	mOps;
	ork::atomic_counter			 		mOpsInFlightCounter;
	ork::atomic_counter			 		mOpSerialIndex;
	ork::atomic_counter	 				mOpsPendingCounter;

	bool 								mEnabled;

	////////////////////////////////

	int 								mLimitMaxOpsInFlight;
	int 								mLimitMaxOpsQueued;
};

//////////////////////////////////////////////////////////////////////

//! collection of operation groups  and a thread pool with which to run them
struct OpMultiQ
{
	OpMultiQ(int inumthreads, const char* name = "DefOpQ");
	~OpMultiQ();

	void push(const Op& the_op);
	void push_sync(const Op& the_op);
	void sync();
	void drain();

	OpGroup* CreateOpGroup(const char* pname);

	static OpMultiQ* GlobalConQ();
	static OpMultiQ* GlobalSerQ();

	void notify_one();
	void BlockingIterate(int thid);

	OpGroup* mDefaultGroup;
	ork::atomic<int> mGroupCounter;
	std::set<OpGroup*> mOpGroups;
	bool mbOkToExit;
	ork::atomic<int> mThreadsRunning;
	std::string mName;
	int mNumThreads;

	atomic_counter mPerfCntExclusive;
	atomic_counter mPerfCntNumOpsExceeded;
	atomic_counter mPerfCntTimeExceeded;
	atomic_counter mPerfCntOutOfOps;
	atomic_counter mPerfCntNumOpsAccum;
	atomic_counter mPerfCntOuterAttempts;
	atomic_counter mPerfCntInnerAttempts;
	atomic_counter mPerfCntInner2Attempts;
	atomic_counter mPerfCntNumIters;
	atomic_counter mPerfCntGroupWaitMs;


private:

	void ProcessOne(OpGroup*pgrp,const Op& the_op);
	void notify_all();

	void PerfReport();

	ork::mpmc_bounded_queue<OpGroup*,32> 	mOpGroupRing;

	ork::atomic<int> mTotalOpsPendingCounter;
	std::set<thread*> mThreadSet;
	//std::condition_variable mOpWaitCV;
	//mtx_t mOpWaitMtx;

};	

}; // namespace ork