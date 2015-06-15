///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////
// Hierarchical Finite State Machine
///////////////////////////////////////////////////////////////////////////////

#include <ork/fsm.h>
#include <ork/fixedvector.hpp>

namespace ork {

///////////////////////////////////////////////////////////////////////////////

StateMachine::StateMachine() 
	: mCurState(nullptr)
{

}

///////////////////////////////////////////////////////////////////////////////

StateMachine::~StateMachine()
{
	QueueStateChange(nullptr);
	Update();

	for( auto s : mStateSet )
		delete s;
}

///////////////////////////////////////////////////////////////////////////////

void StateMachine::QueueStateChange( State* pst )
{
	ChangeStateEvent cse;
	cse.mpNext = pst;
	mPendingEvents.push(cse);
}

void StateMachine::QueueEvent( const svar16_t& v )
{
	mPendingEvents.push(v);
}

///////////////////////////////////////////////////////////////////////////////

void StateMachine::AddState( State* pst )
{
	mStateSet.insert(pst);
}

///////////////////////////////////////////////////////////////////////////////

void StateMachine::AddTransition( State* pfr, State::event_key_t k, State* pto )
{
	assert(mStateSet.find(pfr)!=mStateSet.end());
	assert(mStateSet.find(pto)!=mStateSet.end());

	auto it = pfr->mTransitions.find(k);
	assert(it==pfr->mTransitions.end());
	pfr->mTransitions.insert(std::make_pair(k,pto));
}

void StateMachine::AddTransition( State* pfr, State::event_key_t k, const PredicatedTransition& p )
{
	State* pto = p.mDestination;
	assert(mStateSet.find(pfr)!=mStateSet.end());
	assert(mStateSet.find(pto)!=mStateSet.end());

	auto it = pfr->mTransitions.find(k);
	assert(it==pfr->mTransitions.end());
	pfr->mTransitions.insert(std::make_pair(k,p));
}

///////////////////////////////////////////////////////////////////////////////

void StateMachine::PerformStateChange( State* pto )
{
	assert((pto==nullptr)||(mStateSet.find(pto)!=mStateSet.end()));

	//////////////////////////////////////////////////
	// collect exit handlers from leaf to root	
	///////////////////

	ork::fixedvector<State*,8> exit_vect;
	ork::fixedvector<State*,8> enter_vect;

	auto exit_collect = [&]()
	{	State* pwalk = mCurState;
		while(pwalk!=nullptr)
		{
			exit_vect.push_back(pwalk);
			pwalk = pwalk->mParent;
		}
	};

	//////////////////////////////////////////////////
	// collect enter handlers from leaf to root
	///////////////////

	auto enter_collect = [&]()
	{	State* pwalk = pto;
		while(pwalk!=nullptr)
		{
			enter_vect.push_back(pwalk);
			pwalk = pwalk->mParent;
		}
	};

	//////////////////////////////////////////////////

	if( pto == mCurState )
	{
		// do nothing
	}
	else if( nullptr == mCurState )
	{
		// nothing to exit
		enter_collect();
	}
	else if( nullptr == pto )
	{
		// nothing to enter
		exit_collect();
	}
	/*else if( mCurState->mParent == pto->mParent )
	{
		// no hierarchy change
		mCurState->OnExit();
		pto->OnEnter();
	}*/
	else 
	{
		// full hierarchical exit/enter
		exit_collect();
		enter_collect();
	}

	//////////////////////////////////////////////////
	// run collected exit handlers
	//////////////////////////////////////////////////

	for( State* pex : exit_vect )
	{
		// only run if not present in enter_vect
		bool brun = true;
		for( auto iten : enter_vect )
			if( iten == pex )
				brun = false;
		if( brun )
			pex->OnExit();
	}

	//////////////////////////////////////////////////
	// run collected enter handlers
	//////////////////////////////////////////////////

	for( auto it=enter_vect.rbegin(); it!=enter_vect.rend(); it++ )
	{
		State* pen = (*it);

		// only run if not present in enter_vect
		bool brun = true;
		for( auto itex : exit_vect )
			if( itex == pen )
				brun = false;
		if( brun )
			pen->OnEnter();
	}


	//////////////////////////////////////////////////
	
	mCurState = pto;
}

///////////////////////////////////////////////////////////////////////////////

void StateMachine::Update()
{
	svar16_t ev;
	while( mPendingEvents.try_pop(ev) )
	{
		if( ev.IsA<ChangeStateEvent>() )
		{
			const auto& cse = ev.Get<ChangeStateEvent>();
			PerformStateChange(cse.mpNext);
		}
		else if( mCurState )
		{
			auto k = ev.GetTypeInfo();
			auto it = mCurState->mTransitions.find(k);
			if( it != mCurState->mTransitions.end() )
			{
				const PredicatedTransition& trans = it->second;
				if( trans.mPredicate() )
				{
					State* next = trans.mDestination;
					PerformStateChange(next);
				}
			}
		}
	}
	if( mCurState )
		mCurState->OnUpdate();
}

///////////////////////////////////////////////////////////////////////////////

}
