#pragma once
#include "platutils.hpp"

////////////////////////////////////////////////////////
//#include "lgpl3_guard.h"
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2011, Dmitry Vyukov 
// 
//   www.1024cores.net
//
/////////////////////////////////////////////////////////////////////////////////
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License version 3 as 
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
/////////////////////////////////////////////////////////////////////////////////

class spsq_priv
{

public:

    spsq_priv(  size_t item_size,
                size_t items_per_bucket,
                size_t max_bucket_count
                );

    ~spsq_priv ();

    void* BeginQueueItem();
    void EndQueueItem();
    const void* BeginDeQueueItem();
    void EndDeQueueItem();


private:

    struct bucket_t
    {
        bucket_t*   mNext;   // pointer to the next bucket in the queue
        char        mData[0];// the data
    };

    inline bool IsPointerWithinBucketBoundary(const bucket_t* bkt, const char* ptr) const
    {
        return      (ptr >= bkt->mData)
                &&  (ptr < (bkt->mData + kBucketSize));
    }
    inline bool IsPointerOutsideBucketBoundary(const bucket_t* bkt, const char* ptr) const
    {
        return      (ptr < bkt->mData)
                ||  (ptr >= (bkt->mData + kBucketSize));
    }

    void GetBucket();
    bucket_t* GetBucketImpl(); 
    bucket_t* AllocBucket ();
    spsq_priv (spsq_priv const&);
    void operator = (spsq_priv const&);

    //////////////////////////////////////
    // consumer part:
    // current position for reading
    // (points somewhere into bucket_t::data)
    //////////////////////////////////////

    char* volatile  mHeadPos;

    //////////////////////////////////////
    // padding between consumer's and producer's parts
    //////////////////////////////////////
    char            mPADD [CACHE_LINE_SIZE];

    //////////////////////////////////////
    // producer part:
    // current position for writing
    // (points somewhere into bucket_t::data)
    //////////////////////////////////////

    size_t           mTailPos;   // end of current bucket
    size_t           mTailEnd;   // helper variable
    size_t           mTailNext;  // current 'tail' bucket
    bucket_t*       mTailBucket;// bucket cache
    bucket_t*       mLastBucket;// default bucket size
    size_t          mNumBuckets;// current number of cached buckets

    const size_t    kItemSize;  
    const size_t    kItemsPerBucket; 
    const size_t    kBucketSize;
    const size_t    kMaxBuckets;

    //////////////////////////////////////
    // misc
    //////////////////////////////////////

    // used as 'empty' marker
    static size_t const         eof = 1;

};
