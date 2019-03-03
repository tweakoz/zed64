#pragma once

#include <ork/types.h>
#include <ork/svariant.h>
#include <functional>
#include <ork/atomic.h>


#if 0 //defined(__sgi)
#define USE_SPROC
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/sysmp.h>
#include <sys/wait.h>
#else
#define USE_PTHREAD
#include <pthread.h>
#endif

namespace ork {


struct thread
{

	thread();
	~thread();

	void start( const ork::void_lambda_t& l );
	void join();

	atomic<int> mState;
	#if defined(USE_PTHREAD)
	pthread_t mThreadHandle;
	#elif defined(USE_SPROC)
	int mSprocPid;
	#endif
	ork::void_lambda_t mLambda;

};

struct autothread
{
	autothread()
	{
		auto l = [&]()
		{
			this->run();
		};
		mThread.start(l);
	}
	virtual ~autothread()
	{
		join();
	}
	void join()
	{
		mThread.join();
	}
	virtual void run() = 0;

	thread mThread;
};

}

