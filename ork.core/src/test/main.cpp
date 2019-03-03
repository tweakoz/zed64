//

#include <unittest++/UnitTest++.h>
#include <string.h>
#include <ork/atomic.h>

int main( int argc, char** argv, char** argp )
{	int rval = 0;
	ork::atomic_counter::init();
	/////////////////////////////////////////////
	// default Run All Tests
	/////////////////////////////////////////////
	if( argc < 2 ) 
	{	rval = UnitTest::RunAllTests();
	}
	/////////////////////////////////////////////
	// run a single test (higher signal/noise for debugging)
	/////////////////////////////////////////////
	else if( argc == 2 )
	{	
		bool blist_tests = (0 == strcmp( argv[1], "list" ));

		//UnitTest::TestResults Results;
		const char *testname = argv[1];
		const UnitTest::TestList & List = UnitTest::Test::GetTestList();
		const UnitTest::Test* ptest = List.GetHead();
		int itest = 0;
		if( blist_tests )
		{
			printf( "//////////////////////////////////\n" );
			printf( "Listing Tests\n" );
			printf( "//////////////////////////////////\n" );
		}

		while( ptest )
		{	const UnitTest::TestDetails & Details = ptest->m_details;
			
			if( blist_tests )
			{
				printf( "Test<%d:%s>\n", itest, Details.testName );
			}
			else if( 0 == strcmp( testname, Details.testName ) )
			{	printf( "Running Test<%s>\n", Details.testName );

				UnitTest::TestResults res;
				ptest->Run(res);
			}
			ptest = ptest->next;
			itest++;
		}
	}
	return rval;
}

