///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/types.h>
#include <ork/opq.h>
#include <ork/nf_assert.h>
#include <ork/timer.h>
#include <assert.h>
#include <ork/future.hpp>
#include <ork/atomic.h>



#if ! defined(OSX)
#include <sys/prctl.h>
#endif

namespace ork {

void dispersed_sleep(int idx, int iquantausec )
{
	static const int ktabsize = 16;
	static const int ktab[ktabsize] = 
	{
		0, 1, 3, 5, 
		17, 19, 21, 23,
		35, 37, 39, 41,
		53, 55, 57, 59,
	};
	usleep(ktab[idx&0xf]*iquantausec);
}

atomic_counter::atomic_counter(const atomic_counter&oth)
{
    set(oth.get());
}
atomic_counter::atomic_counter(int ival)
{
#if defined(USE_FETCHOP)
    mVar = atomic_alloc_variable(gatomres, 0);
    set(ival);
#else
    set(ival);
#endif
}

atomic_counter::~atomic_counter()
{
#if defined(USE_FETCHOP)
    atomic_free_variable(gatomres,mVar);
#endif
}


void atomic_counter::init()
{
#if defined(USE_FETCHOP)
	gatomres = atomic_alloc_reservoir(USE_DEFAULT_PM,1024, 0);
#endif
}

#if defined(USE_FETCHOP)
atomic_reservoir_t atomic_counter::gatomres;
#endif

///////////////////////////////////////////////////////////////////////////

void SetCurrentThreadName(const char* threadName)
{
#if defined(LINUX)
	static const int  kMAX_NAME_LEN = 15;
	char name[kMAX_NAME_LEN+1];
	for( int i=0; i<kMAX_NAME_LEN; i++ ) name[i]=0;
	strncpy(name,threadName,kMAX_NAME_LEN);
	name[kMAX_NAME_LEN]=0;
	prctl(PR_SET_NAME,(unsigned long)&name);
#endif
}

////////////////////////////////////////////////////////////////////////////////
Op::Op(const void_lambda_t& op,const std::string& name)
	: mName(name)
{
	SetOp(op);
}
////////////////////////////////////////////////////////////////////////////////
//Op::Op(const void_block_t& op,const std::string& name)
//	: mName(name)
//{
//	if( op )
//		SetOp(Block_copy(op));
//	else
//		SetOp(NOP());
//}
////////////////////////////////////////////////////////////////////////////////
Op::Op(const BarrierSyncReq& op,const std::string& name)
	: mName(name)
{
	SetOp(op);
}
////////////////////////////////////////////////////////////////////////////////
Op::Op(const Op& oth)
	: mName(oth.mName)
{
	if( oth.mWrapped.IsA<void_lambda_t>() )
	{
		SetOp(oth.mWrapped.Get<void_lambda_t>());
	}
	else if( oth.mWrapped.IsA<BarrierSyncReq>() )
	{
		SetOp(oth.mWrapped.Get<BarrierSyncReq>());
	}
	else if( oth.mWrapped.IsA<NOP>() )
	{
		SetOp(oth.mWrapped.Get<NOP>());
	}
	else // unhandled op type
	{
		assert(false);
	}
}
////////////////////////////////////////////////////////////////////////////////
Op::Op()
{
}
////////////////////////////////////////////////////////////////////////////////
Op::~Op()
{
}
////////////////////////////////////////////////////////////////////////////////
void Op::SetOp(const op_wrap_t& op)
{
	if( op.IsA<void_lambda_t>() )
	{
		mWrapped = op;
	}
	else if( op.IsA<BarrierSyncReq>() )
	{
		mWrapped = op;
	}
	else if( op.IsA<NOP>() )
	{
		mWrapped = op;
	}
	else // unhandled op type
	{
		assert(false);
	}
}
///////////////////////////////////////////////////////////////////////////
void Op::QueueASync(OpMultiQ&q) const
{
	q.push(*this);
}
void Op::QueueSync(OpMultiQ&q) const
{
	//AssertNotOnOpQ(q); // we dont have orkid's contextTLS template.. should we import it?
	q.push(*this);
	ork::Future the_fut;
	BarrierSyncReq R(the_fut);
	q.push(R);
	the_fut.GetResult();
}

///////////////////////////////////////////////////////////////////

struct OpqThreadData
{
	OpMultiQ* mpOpQ;
	int miThreadID;
	OpqThreadData() 
		: mpOpQ(nullptr)
		, miThreadID(0)
	{}
};

///////////////////////////////////////////////////////////////////

//struct OpqDrained : public IOpqSynchrComparison
//{
//	bool IsConditionMet(const OpqSynchro& synchro) const
//	{
//		return (int(synchro.mOpCounter)<=0);
//	}
//};

///////////////////////////////////////////////////////////////////////////

void* OpqThreadImpl( void* arg_opaq )
{
	OpqThreadData* opqthreaddata = (OpqThreadData*) arg_opaq;
	OpMultiQ* popq = opqthreaddata->mpOpQ;
	std::string opqn = popq->mName;
	SetCurrentThreadName( opqn.c_str() );


	static int icounter = 0;
	int thid = opqthreaddata->miThreadID+4;
	std::string channam = ork::FormatString("opqth%d",int(thid));

	////////////////////////////////////////////////
	// main opq loop
	////////////////////////////////////////////////

	popq->mThreadsRunning.fetch_and_increment();
	while(false==popq->mbOkToExit)
	{
		popq->BlockingIterate(thid);
	}
	popq->mThreadsRunning.fetch_and_decrement();

	////////////////////////////////////////////////

	//printf( "popq<%p> thread exiting...\n", popq );

	return (void*) 0;

}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::notify_all()
{
}
void OpMultiQ::notify_one()
{
	//lock_t lock(mOpWaitMtx);
	//mOpWaitCV.notify_one();
	mTotalOpsPendingCounter.fetch_and_increment();
}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::BlockingIterate(int thid)
{
	/////////////////////////////////////////////////
	// check "is exiting" status
	/////////////////////////////////////////////////

	if( mbOkToExit )
		return;

	static ork::atomic<int> gopctr;
	int iopctr = gopctr.fetch_and_increment();
	
	/////////////////////////////////////////////////
	// find a group with an op on it
	/////////////////////////////////////////////////

	OpGroup* exec_grp = nullptr;
	bool return_group_at_end = false;

	bool have_exclusive_ownership_of_group = false;

	Op the_op;

	ork::Timer tmr_grp_outer;
	tmr_grp_outer.Start();

	const int ksleepar[9] = {10,17,23,27,35,151,301,603,1201};
	//const int ksleepar[9] = {1,1,1,1,1,1,1,1,1};
	static const int ksh = 0;

	int iouterattempt = 0;
	while( 		(false==mbOkToExit)
			&& 	(exec_grp == nullptr) )
	{
		/////////////////////////////////////////
		// get a group to test
		/////////////////////////////////////////

		OpGroup* test_grp = nullptr;
		int iinnerattempt = 0;
		while(		(false==mbOkToExit)
				&&	(false == mOpGroupRing.try_pop(test_grp)) 
		){
			// sleep semirandom amt of time for thread dispersion
			usleep(ksleepar[(iinnerattempt>>ksh)%9]*100);
			iinnerattempt++;
		}
		mPerfCntInnerAttempts.fetch_and_add(iinnerattempt);

		/////////////////////////////////////////
		if( mbOkToExit ) return; // exiting ?
		/////////////////////////////////////////

		/////////////////////////////////////////
		// we have a group to test
		/////////////////////////////////////////

		assert(test_grp!=nullptr);
		const char* grp_name = test_grp->mGroupName.c_str();

		/////////////////////////////////////////

		const int imax = test_grp->mLimitMaxOpsInFlight;
		int inumopsalreadyinflight = test_grp->mOpsInFlightCounter.fetch_and_increment();
		bool exceeded_max_concurrent = ((imax!=0) && (inumopsalreadyinflight > imax));
		bool reached_max_concurrent = ((imax!=0) && (inumopsalreadyinflight+1) == imax);
		int igrpnumopspending = test_grp->mOpsPendingCounter.get();

		/////////////////////////////////////////

		if( exceeded_max_concurrent || (0 == igrpnumopspending) )
		{
			test_grp->mOpsInFlightCounter.fetch_and_decrement();
			mOpGroupRing.push(test_grp);
			int iter = mPerfCntNumIters.get();
			usleep(ksleepar[iter%9]*3);
			continue;
		}

		exec_grp = test_grp;

		if( reached_max_concurrent )
			return_group_at_end = true;
		else
			mOpGroupRing.push(test_grp);

		/////////////////////////////////////////
	}   // while we dont have a group (or we are exiting)

	if(mbOkToExit)
		return;

	assert(exec_grp!=nullptr);
	
	const int kmaxperquanta = 1024;

	float outer_wait_ms = 1000.0f * tmr_grp_outer.SecsSinceStart();
	mPerfCntGroupWaitMs.fetch_and_add( int(outer_wait_ms) );

	//ProcessOne(exec_grp,the_op);

	int inumthisquanta = 0;

	if( 1 ) // we exclusively own this group, milk it
	{
		mPerfCntExclusive.fetch_and_increment();
		ork::Timer quanta_timer;
		quanta_timer.Start();

		bool bkeepgoing = true;

		int inumunderrun = 0;

		while( bkeepgoing )
		{
			bool bgotone = exec_grp->try_pop(the_op);
			if( bgotone )
			{
				ProcessOne(exec_grp,the_op);
				exec_grp->notify_op_complete();
				mTotalOpsPendingCounter.fetch_and_decrement();
				inumthisquanta++;
			}
			else
			{	
				mPerfCntOutOfOps.fetch_and_increment();
				//usleep(100);
				inumunderrun++;
			}
			bool count_exceeded = (inumthisquanta>=kmaxperquanta);
			bool timeq_exceeded = (quanta_timer.SecsSinceStart()>=0.05);
			bool under_exceeded = (inumunderrun>=1);

			bool either_exceeded = count_exceeded||timeq_exceeded||under_exceeded;

			if( count_exceeded )
				mPerfCntNumOpsExceeded.fetch_and_increment();
			if( timeq_exceeded )
			{
				mPerfCntTimeExceeded.fetch_and_increment();
				usleep(10000);
			}

			bkeepgoing = (!either_exceeded);
		}

	}


	exec_grp->mOpsInFlightCounter.fetch_and_decrement();
	mPerfCntNumOpsAccum.fetch_and_add(inumthisquanta);
	mPerfCntNumIters.fetch_and_increment();

	if( return_group_at_end )
		mOpGroupRing.push(exec_grp); // release the group
}

void OpMultiQ::PerfReport()
{
	if( 0 ) //mPerfReport.get() > 1000 )
	{
		printf( "////////////////////////////////////////\n" );
		printf( "OpMultQ<%p> PerfReport mPerfCntNumIters<%d>\n", this, mPerfCntNumIters.get() );
		printf( "OpMultQ<%p> mPerfCntOuterAttempts<%d>\n", this, mPerfCntOuterAttempts.get() );
		printf( "OpMultQ<%p> mPerfCntInnerAttempts<%d>\n", this, mPerfCntInnerAttempts.get() );
		printf( "OpMultQ<%p> mPerfCntInner2Attempts<%d>\n", this, mPerfCntInner2Attempts.get() );
		printf( "OpMultQ<%p> mPerfCntGroupWaitMs<%d>\n", this, mPerfCntGroupWaitMs.get() );
		printf( "OpMultQ<%p> mPerfCntGroupWaitMsPerThread<%f>\n", this, float(mPerfCntGroupWaitMs.get())/float(mNumThreads) );
		printf( "OpMultQ<%p> mPerfCntExclusive<%d>\n", this, mPerfCntExclusive.get() );
		printf( "OpMultQ<%p> mPerfCntNumOpsExceeded<%d>\n", this, mPerfCntNumOpsExceeded.get() );
		printf( "OpMultQ<%p> mPerfCntTimeExceeded<%d>\n", this, mPerfCntTimeExceeded.get() );
		printf( "OpMultQ<%p> mPerfCntOutOfOps<%d>\n", this, mPerfCntOutOfOps.get() );
		printf( "OpMultQ<%p> mPerfCntNumOpsAccum<%d>\n", this, mPerfCntNumOpsAccum.get() );



		mPerfCntExclusive.set(0);
		mPerfCntNumOpsExceeded.set(0);
		mPerfCntTimeExceeded.set(0);
		mPerfCntOutOfOps.set(0);
		mPerfCntNumOpsAccum.set(0);
		mPerfCntNumIters.set(0);
		mPerfCntOuterAttempts.set(0);
		mPerfCntInnerAttempts.set(0);
		mPerfCntInner2Attempts.set(0);
		mPerfCntGroupWaitMs.set(0);
	}

}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::ProcessOne(OpGroup*pexecgrp,const Op& the_op)
{	/////////////////
	ork::Timer tmr;
	tmr.Start();
	/////////////////
	//int inumops = pexecgrp->NumOps();
	//printf( "  runop group<%s> OIF<%d> gctr<%d> npend<%d>\n", grp_name, int(pexecgrp->mOpsInFlightCounter), int(gctr), inumops );
	/////////////////////////////////////////////////
	// get debug names
	/////////////////////////////////////////////////
	const char* grp_name = pexecgrp->mGroupName.c_str();
	const char* op_nam = the_op.mName.length() ? the_op.mName.c_str() : "opx";
	/////////////////////////////////////////////////
	// we only support 3 op types currently
	//   lambdas / blocks / and barrier-sync ops
	/////////////////////////////////////////////////
	if( the_op.mWrapped.IsA<void_lambda_t>() )
	{	the_op.mWrapped.Get<void_lambda_t>()();
	}
	else if( the_op.mWrapped.IsA<BarrierSyncReq>() )
	{	auto& R = the_op.mWrapped.Get<BarrierSyncReq>();
		R.mFuture.Signal<bool>(true);
	}
	else if( the_op.mWrapped.IsA<NOP>() ) {}
	else
	{	printf( "unknown opq invokable type\n" );
		assert(false);
	}
	/////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::push(const Op& the_op)
{
	mDefaultGroup->push(the_op);
}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::sync()
{
	//AssertNotOnOpQ(*this);
	ork::Future the_fut;
	BarrierSyncReq R(the_fut);
	push(R);
	the_fut.GetResult();
}

///////////////////////////////////////////////////////////////////////////

void OpMultiQ::drain()
{
	int index = 0;
	while(int(mTotalOpsPendingCounter))
	{
		//printf( "draining mOpsPendingCounter2<%d>\n", int(mOpsPendingCounter2) );
		dispersed_sleep(index,100);
		index++;
	}
	//printf( "Opq::drain count<%d>\n", int(mSynchro.mOpCounter));
	//OpqDrained pred_is_drained;
	//mSynchro.WaitOnCondition(pred_is_drained);
}

///////////////////////////////////////////////////////////////////////////

OpGroup* OpMultiQ::CreateOpGroup(const char* pname)
{
	OpGroup* pgrp = new OpGroup(this,pname);
	mOpGroups.insert(pgrp);
	mGroupCounter++;

	mOpGroupRing.push(pgrp);
	return pgrp;
}

///////////////////////////////////////////////////////////////////////////

OpMultiQ::OpMultiQ(int inumthreads,const char* name)
	: mbOkToExit(false)
	, mName(name)
	, mNumThreads(inumthreads)
{
	mPerfCntExclusive.set(0);
	mPerfCntNumOpsExceeded.set(0);
	mPerfCntTimeExceeded.set(0);
	mPerfCntOutOfOps.set(0);
	mPerfCntNumOpsAccum.set(0);
	mPerfCntNumIters.set(0);
	mPerfCntOuterAttempts.set(0);
	mPerfCntInnerAttempts.set(0);
	mPerfCntInner2Attempts.set(0);
	mPerfCntGroupWaitMs.set(0);

	mTotalOpsPendingCounter = 0;
	mGroupCounter = 0;
	mThreadsRunning = 0;

	mDefaultGroup = CreateOpGroup("defconq");

	//sem_init(&mSemaphore,0,0);

	for( int i=0; i<inumthreads; i++ )
	{
		OpqThreadData* thd = new OpqThreadData;
		thd->miThreadID = i;

		thd->mpOpQ = this;

		auto l = [=](){
			OpqThreadImpl((void*)thd);
		};
		auto pthread = new ork::thread;
		pthread->start(l);
		mThreadSet.insert(pthread);
	}

}

///////////////////////////////////////////////////////////////////////////

OpMultiQ::~OpMultiQ()
{

	/////////////////////////////////
	// dont accept any more ops
	/////////////////////////////////

	for( auto& grp : mOpGroups )
	{
		grp->Disable();
	}

	/////////////////////////////////
	// drain what ops we have
	/////////////////////////////////

	drain();

	PerfReport();

	/////////////////////////////////
	// Signal to worker threads that we are ready to go down
	// Spam the worker thread semaphore until it does go down
	// Wait until all worker threads have exited
	/////////////////////////////////

	mbOkToExit = true;

	int index = 0;
	while(int(mThreadsRunning)!=0)
	{
		notify_all();
		dispersed_sleep(index++,100);
	}

	/////////////////////////////////

	for( ork::thread* thr : mThreadSet )
	{
		delete thr;
	}

	/////////////////////////////////
	// trash the groups
	/////////////////////////////////

	OpGroup* pgrp = nullptr;

	for( auto& it : mOpGroups )
	{
		delete it;
	}

	/////////////////////////////////



	/////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////

} // namespace ork
