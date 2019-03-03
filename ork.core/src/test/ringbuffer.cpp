#include <unittest++/UnitTest++.h>
#include <cmath>
#include <limits>
#include <ork/cvector2.h>
#include <ork/math_misc.h>
#include <string.h>

#include <ork/ringbuffer.hpp>
#include <ork/svariant.h>
#include <ork/timer.h>
#include <ork/fixedstring.h>
#include <ork/thread.h>

using namespace ork;

typedef ork::thread thread_t;

struct EOTEST
{

};

static const int knummsgs = 1<<16;

template <typename queue_type>
	struct yo 
	{
		typedef typename queue_type::value_type value_type;

		queue_type mQueue;

		void RunTest()
		{
			auto l_producer = [&]( yo* pyo )
			{
				queue_type& the_queue = pyo->mQueue;
				printf( "STARTED PRODUCER knummsgs<%d>\n", knummsgs);
				for( int i=0; i<knummsgs; i++ )
				{
					//printf( " prod pushing<%d>\n", i );
					the_queue.push(i);
					extstring_t str;
					str.format("to<%d>", i);
					the_queue.push(str);
				}			
				the_queue.push(EOTEST());
			};

			auto l_consumer = [&]( yo* pyo )
			{
				printf( "STARTED CONSUMER\n");
				queue_type& the_queue = pyo->mQueue;
				value_type popped;

				bool bdone=false;
				int ictr1 = 0;
				int ictr2 = 0;
				while(false==bdone)
				{
					while( the_queue.try_pop(popped) )
					{
						//printf( " cons pulling ictr1<%d> ictr2<%d>\n", ictr1, ictr2 );

						if( popped.template IsA<int>() )
						{
							int iget = popped.template Get<int>();
							assert(iget==ictr1);
							ictr1++;
						}
						else if( popped.template IsA<extstring_t>() )
						{
							extstring_t& es = popped.template Get<extstring_t>();
							if( (ictr2%(1<<18))==0 )
								printf( "popped: %s\n", es.c_str() );
							ictr2++;
							assert(ictr2==ictr1);
						}
						else if( popped.template IsA<EOTEST>() )
							bdone=true;
						else
							assert(false);
					}
					usleep(2000);
				}
			};

			double fsynctime = ork::get_sync_time();
			
			ork::thread thr_p, thr_c;


		    thr_p.start([=](){ l_producer( this ); });
		    thr_c.start([=](){ l_consumer( this ); });
			thr_p.join();
			thr_c.join();

			double fsynctime2 = ork::get_sync_time();

			double elapsed = (fsynctime2-fsynctime);
			double mps = 2.0*double(knummsgs)/elapsed;
			double msg_size = double(sizeof(value_type));
			double bps = msg_size*mps;
			double gbps = bps/double(1<<30);
			printf( "nummsgs<%g> elapsed<%g> msgpersec<%g> GBps(%g)\n", double(knummsgs), elapsed, mps, gbps );			
		}




	};

///////////////////////////////////////////////////////////////////////////////

TEST(OrkMpMcRingBuf)
{
	printf("//////////////////////////////////////\n" );
	printf( "ORK MPMC CONQ TEST\n");
	printf("//////////////////////////////////////\n" );

	//auto the_yo = new yo<ork::mpmc_bounded_queue<svar1024_t,16<<10>>;
	auto the_yo = new yo<ork::SpScRingBuf<svar16k_t,16<<10>>;
	the_yo->RunTest();
	delete the_yo;

	printf("//////////////////////////////////////\n" );

}

///////////////////////////////////////////////////////////////////////////////
