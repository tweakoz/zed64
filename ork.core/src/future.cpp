///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/future.hpp>
#include <unistd.h>

namespace ork {
///////////////////////////////////////////////////////////////////////////////

Future::Future()
{
    mState = 0;
}

void Future::Clear()
{
    mResult.Set<bool>(false);
    mState=0;
}

void Future::WaitForSignal() const
{
    while(false==IsSignaled()){}
      usleep(1000);
}
const Future::var_t& Future::GetResult() const
{
    WaitForSignal();
    return mResult;
}//

void Future::SetCallback( const Future::fut_blk_cb_t& cb )
{
	mCallback.Set<Future::fut_blk_cb_t>(cb);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace ork