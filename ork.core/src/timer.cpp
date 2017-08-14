///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/timer.h>
#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace ork {




struct timespec ts;


#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
void my_clock_gettime(int iclk, timespec* pts)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), iclk, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    pts->tv_sec = mts.tv_sec;
    pts->tv_nsec = mts.tv_nsec;
}
#else
void my_clock_gettime(int  iclk, timespec* pts)
{
    clock_gettime(iclk, pts);
}
#endif

double get_sync_time(clockType type)
{
    static struct timespec ts1st;
    static bool b1sttime = true;
    int clock;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock = CALENDAR_CLOCK;
#else
    switch(type) {
    default:
    case clockRealTime:
        clock = CLOCK_REALTIME;
        break;
#if defined(LINUX)
    case clockProcessTime:
        clock = CLOCK_PROCESS_CPUTIME_ID;
        break;
    case clockThreadTime:
        clock = CLOCK_THREAD_CPUTIME_ID;
        break;
#endif
    }
#endif
    if( b1sttime )
    {
        my_clock_gettime(clock,&ts1st);
        b1sttime = false;
    }
    struct timespec tsnow;
    my_clock_gettime(clock,&tsnow);

    uint64_t tms_base = ((ts1st.tv_sec>>12)<<12)*1000;
    uint64_t tms_now = uint64_t(tsnow.tv_sec)*1000+uint64_t(tsnow.tv_nsec)/1000000;
    uint64_t tms_del = tms_now-tms_base;

    double sec = double(tms_del)*0.001;
    return sec;
}

void Timer::Start()
{
    mStartTime = get_sync_time();
}

///////////////////////////////////////////////////////////////////////////////

void Timer::End()
{
    mEndTime = get_sync_time(); 
}

///////////////////////////////////////////////////////////////////////////////

double Timer::InternalSecsSinceStart() const
{
    double cur_time = get_sync_time(); 
    return cur_time-mStartTime;
} 

///////////////////////////////////////////////////////////////////////////////

float Timer::SecsSinceStart() const
{
    double rval = InternalSecsSinceStart();
    return float(rval);
}

///////////////////////////////////////////////////////////////////////////////

float Timer::SpanInSecs() const
{   
    return (mEndTime-mStartTime);
}

}
