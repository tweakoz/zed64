#include <ork/timer.h>
#include <ork/opq.h>
#include <string.h>
#include <ork/fsm.h>
#include <unittest++/UnitTest++.h>

using namespace ork;

static void logstate(const char* pstr)
{

}

struct S1 : public State 
{	S1(State*p):State(p){}
	void OnEnter() { logstate( "s1.enter\n"); }
	void OnExit() { logstate( "s1.exit\n"); }
	void OnUpdate() { logstate( "s1.update\n"); }
};
struct S2 : public State 
{	S2(State*p):State(p){}
	void OnEnter() { logstate( "s2.enter\n"); }
	void OnExit() { logstate( "s2.exit\n"); }
	void OnUpdate() { logstate( "s2.update\n"); }
};
struct S3 : public State 
{	S3(State*p):State(p){}
	void OnEnter() { logstate( "s3.enter\n"); }
	void OnExit() { logstate( "s3.exit\n"); }
	void OnUpdate() { logstate( "s3.update\n"); }
};
struct SA : public State 
{	SA(State*p):State(p){}
	void OnEnter() { logstate( "sa.enter\n"); }
	void OnExit() { logstate( "sa.exit\n"); }
	void OnUpdate() { logstate( "sa.update\n"); }
};
struct SB : public State 
{	SB(State*p):State(p){}
	void OnEnter() { logstate( "sb.enter\n"); }
	void OnExit() { logstate( "sb.exit\n"); }
	void OnUpdate() { printf( "sb.update\n"); }
};
struct ROOT : public State 
{	void OnEnter() { logstate( "ROOT.enter\n"); }
	void OnExit() { logstate( "ROOT.exit\n"); }
	void OnUpdate() { logstate( "ROOT.update\n"); }
};

struct e1to2{};
struct e2to3{};

///////////////////////////////////////////////////////////////////////
// deterministic fsm unit test
///////////////////////////////////////////////////////////////////////

TEST(hfsm_1)
{
	for( int i=0; i<3; i++ )
	{
		logstate( "//hfsm_1/////////////////////////\n");
		StateMachine the_SM;

		auto the_root = the_SM.NewState<ROOT>();
		auto the_sa = the_SM.NewState<SA>(the_root);
		auto the_sb = the_SM.NewState<SB>(the_root);

		auto the_s1 = the_SM.NewState<S1>(the_sa);
		auto the_s2 = the_SM.NewState<S2>(the_sa);
		auto the_s3 = the_SM.NewState<S3>(the_sb);

		the_SM.AddTransition(the_s1,trans_key<e1to2>(),the_s2);
		the_SM.AddTransition(the_s2,trans_key<e2to3>(),the_s3);

		the_SM.QueueStateChange(the_s1);
		the_SM.QueueEvent(e1to2());
		the_SM.QueueEvent(e2to3());

		while(the_SM.GetCurrentState()!=the_s3)
		{
			the_SM.Update();
		}
	}
}

///////////////////////////////////////////////////////////////////////
// probalistic fsm unit test
///////////////////////////////////////////////////////////////////////

TEST(hfsm_probalistic_1)
{
	for( int i=0; i<10; i++ )
	{
		logstate( "//hfsm_probalistic_1/////////////////////////\n");
		StateMachine the_SM;

		auto the_root = the_SM.NewState<ROOT>();
		auto the_sa = the_SM.NewState<SA>(the_root);
		auto the_sb = the_SM.NewState<SB>(the_root);

		auto the_s1 = the_SM.NewState<S1>(the_sa);
		auto the_s2 = the_SM.NewState<S2>(the_sa);
		auto the_s3 = the_SM.NewState<S3>(the_sb);

		auto probability_lambda = []() -> bool
		{
			int i = rand()&0xff;
			bool bprob = i<0x7f;
			//printf( "bprob<%d>\n", int(bprob) );
			return bprob;
		};

		PredicatedTransition trans_2(the_s2,probability_lambda);
		PredicatedTransition trans_3(the_s3,probability_lambda);

		the_SM.AddTransition(the_s1,trans_key<e1to2>(),trans_2);
		the_SM.AddTransition(the_s2,trans_key<e2to3>(),trans_3);

		the_SM.QueueStateChange(the_s1);

		for( int i=0; i<3; i++ )
		{
			the_SM.QueueEvent(e1to2());
			the_SM.Update();
		}
		for( int i=0; i<3; i++ )
		{
			the_SM.QueueEvent(e2to3());
			the_SM.Update();
		}

		//////////////////////////////////////
		// usually the StateMachine destructor will do this
		//  but we want to test it explicitly right now 
		//  RAII compliance will be in a separate test
		//////////////////////////////////////

		the_SM.QueueStateChange(nullptr);
		the_SM.Update();
		assert(the_SM.GetCurrentState()==nullptr);
	}
}
