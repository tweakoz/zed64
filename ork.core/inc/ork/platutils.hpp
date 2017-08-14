///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>

#if ! defined(OSX)
#include <malloc.h>
#endif

////////////////////////////////////////////////////////
//#include "lgpl3_guard.h"
////////////////////////////////////////////////////////

/* 
 *  Copyright (c) 2011, Dmitry Vyukov 
 * 
 *   www.1024cores.net
 *
 */
/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 ****************************************************************************
 */
#define INLINE static __inline
#define NOINLINE
#define CACHE_LINE_SIZE 128

static const size_t cacheline_size = 64;

#define CompilerMemBarrier() __asm __volatile ("" ::: "memory"); // prevent a compiler from reordering instructions

namespace platutils {

INLINE void* aligned_malloc(size_t sz)
{
    void*               mem;
#if defined(IRIX)
    mem = memalign(CACHE_LINE_SIZE, sz);
    return mem;
#else
    if (posix_memalign(&mem, CACHE_LINE_SIZE, sz))
        return 0;

#endif
    return mem;
}

INLINE void aligned_free(void* mem)
{
    free(mem);
}

INLINE int get_proc_count()
{
#if defined(IRIX)
    return (int)sysconf(_MIPS_CS_NUM_PROCESSORS);
#else
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

INLINE void atomic_addr_store_release(void* volatile* addr, void* val)
{
    CompilerMemBarrier();
    addr[0] = val;
}
INLINE void* atomic_addr_load_acquire(void* volatile* addr)
{
    void* val = addr[0];
    CompilerMemBarrier();
    return val;
}
/////////////////////////
INLINE size_t atom_sizet_load_relaxed(size_t volatile* psrc)
{
    size_t val = psrc[0];
    CompilerMemBarrier();
    return val;
}
INLINE size_t atom_sizet_load_acquire(size_t volatile* psrc)
{
    size_t val = psrc[0];
    CompilerMemBarrier();
    return val;
}
INLINE void atom_sizet_store_release(size_t volatile* pdest, size_t val)
{
    CompilerMemBarrier();
    pdest[0] = val;
}
/////////////////////////
INLINE void core_yield()
{
    __asm __volatile ("pause" ::: "memory");
}
INLINE void thread_yield()
{
    sched_yield();
}

typedef pthread_t thread_t;

INLINE void thread_start(thread_t* th, void*(*func)(void*), void* arg)
{
    pthread_create(th, 0, func, arg);
}


INLINE void thread_join(thread_t* th)
{
    void*                       tmp;
    pthread_join(th[0], &tmp);
}
INLINE void thread_setup_prio()
{
    //struct sched_param param;
    //param.sched_priority = 30;
    //if (pthread_setschedparam(pthread_self(), SCHED_OTHER, &param))
    //    printf("failed to set thread prio\n");
}

} // namespace platutils