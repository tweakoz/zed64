#pragma once

#include <ork/gstack.h>

namespace ork { 

template<typename T>
std::stack<T*>& gstack<T>::GetStack()
{
	if( 0 == gpItems )
	{
		gpItems = new std::stack<T*>();
	}
	return *gpItems;
}

template<typename T>
T& gstack<T>::top()
{
	return *GetStack().top();
}

template<typename T> void gstack<T>::push( T& item )
{
	GetStack().push( & item );
}

template<typename T> void gstack<T>::pop()
{
	GetStack().pop();
}

template<typename T> std::stack<T*>* gstack<T>::gpItems = 0;

///////////////////////////////////////////////////////////////////////

template<typename T>
std::stack<T*>& gthreadedstack<T>::GetStack()
{
	if( 0 == gpItems )
	{
		gpItems = new std::stack<T*>();
	}
	return *gpItems;
}

template<typename T>
T& gthreadedstack<T>::top()
{
	return *GetStack().top();
}

template<typename T> void gthreadedstack<T>::push( T& item )
{
	GetStack().push( & item );
}

template<typename T> void gthreadedstack<T>::pop()
{
	GetStack().pop();
}

template<typename T> ThreadLocal std::stack<T*>* gthreadedstack<T>::gpItems = 0;

} // namespace ork
