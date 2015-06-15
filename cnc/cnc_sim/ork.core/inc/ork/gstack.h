///////////////////////////////////////////////////////////////////////////////
// Orkid
// Copyrigh 1996-2009, Michael T. Mayers
// See License at OrkidRoot/license.html or http://www.tweakoz.com/orkid/license.html
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stack>

namespace ork { 

template<typename T> class gthreadedstack
{
	static ThreadLocal std::stack<T*>* gpItems;

	static std::stack<T*>& GetStack();

public:
	static T& top();
	static void push(T&);
	static void pop();
};

template<typename T> class gstack
{
	static std::stack<T*>* gpItems;

	static std::stack<T*>& GetStack();

public:
	static T& top();
	static void push(T&);
	static void pop();
};

template <typename T,const int ksize> struct fixed_stack
{
	T	mArray[ksize];
	int	miCounter;

	fixed_stack() : miCounter(0) {}

	void push( const T& inp ) 
	{
		OrkAssert(miCounter<(ksize-1));
		mArray[miCounter] = inp;
		miCounter++;
	}
	const T& top() const
	{
		OrkAssert(miCounter>0);
		return mArray[miCounter-1];
	}
	T& top()
	{
		OrkAssert(miCounter>0);
		return mArray[miCounter-1];
	}
	const T& item(int idx) const
	{
		OrkAssert(idx>=0);
		OrkAssert(idx<ksize);
		return mArray[idx];
	}
	T& item(int idx)
	{
		OrkAssert(idx>=0);
		OrkAssert(idx<ksize);
		return mArray[idx];
	}
	void pop()
	{
		OrkAssert(miCounter>0);
		miCounter--;
	}
	int size() const { return miCounter; }
	void reset() { miCounter=0; }
	void set_size( int idx ) 
	{
		OrkAssert(idx>=0);
		OrkAssert(idx<ksize);
		miCounter=idx;
	}
};

} // namespace ork
