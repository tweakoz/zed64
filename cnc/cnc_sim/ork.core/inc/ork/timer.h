///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <time.h>
#include <stdint.h>

namespace ork{

#ifndef GETSYNCTIME_DEFINED
#define GETSYNCTIME_DEFINED
enum clockType { clockRealTime, clockProcessTime, clockThreadTime };

double get_sync_time(clockType type = clockRealTime);
#endif

struct Timer
{
	double mStartTime;
	double mEndTime;

	void Start();
	void End();
	double InternalSecsSinceStart() const;
	float SecsSinceStart() const;
	float SpanInSecs() const;
};

}

