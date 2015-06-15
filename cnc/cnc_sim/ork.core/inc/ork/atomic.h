#pragma once

#include <stdio.h>
#include <atomic>
#include <unistd.h>

#if defined(__sgi)
extern "C" {
#include <sys/pmo.h>
#include <fetchop.h>
//#define USE_FETCHOP
};
#endif

struct MemFullFence
{
	static std::memory_order GetOrder() { return std::memory_order_seq_cst; }
};
struct MemRelaxed
{
	static std::memory_order GetOrder() { return std::memory_order_relaxed; }
};
struct MemAcquire
{
	static std::memory_order GetOrder() { return std::memory_order_acquire; }
};
struct MemRelease
{
	static std::memory_order GetOrder() { return std::memory_order_release; }
};

namespace ork {


template <typename T> struct atomic //: public std::atomic<T>
{
    template <typename A=MemRelaxed> void store( const T& source )
    {
    	mData.store(source,A::GetOrder());
    }

    template <typename A=MemRelaxed> T load() const
    {
    	return mData.load(A::GetOrder());
    }


    template <typename A=MemRelaxed> T compare_and_swap( T new_val, T ref_val )
    {
    	T old_val = ref_val;
    	bool changed = mData.compare_exchange_weak(old_val,new_val,A::GetOrder());
        return old_val;
    }

    template <typename A=MemRelaxed> T fetch_and_store( T new_val )
    {
    	return mData.exchange(new_val,A::GetOrder());
    }

    template <typename A=MemRelaxed> T fetch_and_increment()
    {
    	return mData.operator++(A::GetOrder());
    }

    template <typename A=MemRelaxed> T fetch_and_decrement()
    {
    	return mData.operator--(A::GetOrder());
    }

    operator T() const
    {
    	return (T)mData;
    }

    T operator++ (int iv)
    {
    	return mData.operator++(iv);
    }
    T operator-- (int iv)
    {
    	return mData.operator--(iv);
    }
    atomic<T>& operator = (const atomic<T>& inp)
    {
    	mData = inp.mData;
    	return *this;
    }
    atomic<T>& operator = (const T& inp)
    {
    	this->store<MemRelaxed>(inp);
    	return *this;
    }

    //operator bool() const { return bool(load<MemRelaxed>()); }

    std::atomic<T> mData;

};


struct atomic_counter
{
    atomic_counter(int ival=0);
    atomic_counter(const atomic_counter&oth);
    ~atomic_counter();

    inline int fetch_and_add(int iv)
    {
        return mVal.fetch_add(iv);
    }
    inline int fetch_and_increment()
    {
    #if defined(USE_FETCHOP)
        return atomic_fetch_and_increment(mVar);
    #else
        return mVal.fetch_add(1);
    #endif
    }
    inline int fetch_and_decrement()
    {
    #if defined(USE_FETCHOP)
        return atomic_fetch_and_decrement(mVar);
    #else
        return mVal.fetch_add(-1);
    #endif
    }
    inline int get() const
    {
    #if defined(USE_FETCHOP)
        return atomic_load(mVar);
    #else
        return mVal.load(MemRelaxed::GetOrder());
    #endif
    }
    inline void set(int ival)
    {
    #if defined(USE_FETCHOP)
        atomic_store(mVar,ival);
    #else
        mVal.store(ival,MemRelaxed::GetOrder());
    #endif
    }

    static void init();

    #if defined(USE_FETCHOP)

    static atomic_reservoir_t gatomres;
    atomic_var_t* mVar;

    #else
    std::atomic<int> mVal;
    #endif
};


}
