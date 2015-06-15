///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////
// Hierarchical Finite State Machine
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <ork/svariant.h>
#include <ork/concurrent_queue.hpp>
#include <functional>

/////////////////////////////////////////////////////////////////////////////////////

namespace ork {

struct State;

/////////////////////////////////////////////////////////////////////////////////////

typedef std::function<bool()> bpred_lambda_t;

struct PredicatedTransition
{
	PredicatedTransition( State* pdest ) 
		: mDestination(pdest)
	{
		mPredicate = [](){return true;};
	}
	PredicatedTransition( State* pdest, bpred_lambda_t p ) 
		: mDestination(pdest)
		, mPredicate(p)
	{
	}
	bpred_lambda_t mPredicate;
	State* mDestination;
};

/////////////////////////////////////////////////////////////////////////////////////

template <typename T> const std::type_info* trans_key()
{
	return &typeid(T);
}

/////////////////////////////////////////////////////////////////////////////////////

struct State
{
    State( State* p=nullptr ) : mParent(p) {}

    typedef const std::type_info* event_key_t;
    typedef std::map<event_key_t,PredicatedTransition> trans_map_t;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate() {}

    trans_map_t mTransitions;
    State* mParent;
};

/////////////////////////////////////////////////////////////////////////////////////

struct ChangeStateEvent
{
	ChangeStateEvent() : mpNext(nullptr) {}
	State* mpNext;
};

/////////////////////////////////////////////////////////////////////////////////////

struct StateMachine
{
	StateMachine();
	~StateMachine();

    template<typename T> State* NewState(State* par);
    template<typename T> State* NewState();
    void AddState( State* pst );
    void AddTransition( State* pfr, State::event_key_t k, State* pto );
    void AddTransition( State* pfr, State::event_key_t k, const PredicatedTransition& p );
    void QueueStateChange( State* pst );
    void QueueEvent( const svar16_t& v );

    void Update();
    State* GetCurrentState() const { return mCurState; }

private:

	void PerformStateChange( State* pto );

    State* mCurState;
    std::set<State*> mStateSet;
    mpmc_bounded_queue<svar16_t,16> mPendingEvents;
};

/////////////////////////////////////////////////////////////////////////////////////

template <typename T> State* StateMachine::NewState(State* par)
{
	State* pst = new T(par);
	AddState(pst);
	return pst;
}

/////////////////////////////////////////////////////////////////////////////////////

template <typename T> State* StateMachine::NewState()
{
	State* pst = new T;
	AddState(pst);
	return pst;
}

/////////////////////////////////////////////////////////////////////////////////////

}