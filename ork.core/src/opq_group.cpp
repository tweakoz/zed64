///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/types.h>
#include <ork/opq.h>
#include <ork/timer.h>
#include <assert.h>
#include <ork/future.hpp>
//#include <sys/prctl.h>

namespace ork {

///////////////////////////////////////////////////////////////////////////

OpGroup::OpGroup(OpMultiQ*popq, const char* pname)
	: mpOpQ(popq)
	, mLimitMaxOpsInFlight(0)
	, mLimitMaxOpsQueued(0)
	, mGroupName(pname)
	, mEnabled(true)
{
	mOpsPendingCounter.set(0);
	mOpsInFlightCounter.set(0);
	mOpSerialIndex.set(0);
}

///////////////////////////////////////////////////////////////////////////

void OpGroup::push(const Op& the_op)
{
	if( mEnabled )
	{
		////////////////////////////////
		// throttle it (limit number of ops in queue)
		////////////////////////////////
		if( mLimitMaxOpsQueued )
		{
			int index = 0;
			while( mOpsPendingCounter.get() > mLimitMaxOpsQueued )
			{
				//printf( "mOpsPendingCounter<%d> mLimitMaxOpsQueued<%d>\n", mOpsPendingCounter.get(), mLimitMaxOpsQueued );
				dispersed_sleep(index++,30);
			}
		}
		////////////////////////////////

		mOpsPendingCounter.fetch_and_increment();
		mOpSerialIndex.fetch_and_increment();

		mOps.push(the_op);

		mpOpQ->notify_one();
	}
}

///////////////////////////////////////////////////////////////////////////

void OpGroup::drain()
{
	int index = 0;
	while(mOpsPendingCounter.get()>0)
	{
		//dispersed_sleep(index++,10);
		usleep(100);
	}
	//printf( "OpGroup::drain count<%d>\n", int(mSynchro.mOpCounter));
	//OpqDrained pred_is_drained;
	//mSynchro.WaitOnCondition(pred_is_drained);
}

void OpGroup::notify_op_complete()
{
	mOpsPendingCounter.fetch_and_decrement();
}


///////////////////////////////////////////////////////////////////////////

bool OpGroup::try_pop( Op& out_op )
{
	bool rval = mOps.try_pop(out_op);

	return rval;
}

///////////////////////////////////////////////////////////////////////////
} // namespace ork

