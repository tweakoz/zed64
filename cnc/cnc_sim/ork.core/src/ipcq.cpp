///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <sys/mman.h>
#include <ork/ipcq.h>
#include <ork/path.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <ork/fixedstring.h>
#include <errno.h>

static const int kmaxmsgsiz = sizeof(ork::NetworkMessage);

//static const uin32_t kmapaddrflags = MAP_SHARED|MAP_LOCKED;
static const uint32_t kmapaddrflags = MAP_SHARED;

//#define __MSGQ_DEBUG__

namespace ork {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IpcMsgQSender::IpcMsgQSender()
	// //mbShutdown(false)
	: mOutbox(nullptr)
	, mShmAddr(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////

IpcMsgQSender::~IpcMsgQSender()
{
	//mbShutdown = true;
	//while( true == mbShutdown )
	//{
	//	usleep(1<<16);
	//}
	if( mShmAddr )
	{
		SetSenderState(EMQEPS_TERMINATED);
		shm_unlink(mPath.c_str());
		munmap(mShmAddr,sizeof(msq_impl_t));
	}
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQSender::SetName(const std::string& nam)
{
	mName = nam;
	ork::uristring_t path;
	path.format("/dev/shm/%s",nam.c_str());
	mPath = path.c_str();
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQSender::Create( const std::string& nam )
{
	SetName(nam);
#ifdef __MSGQ_DEBUG__ 
	printf( "IpcMsgQSender<%p> creating msgQ<%s>\n", this, nam.c_str() );
#endif

	int shm_id = shm_open(mName.c_str(),O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	assert(shm_id>=0);

	size_t isize = sizeof(msq_impl_t);
	int ift = ftruncate(shm_id, isize);
	mShmAddr =  mmap(0,isize,PROT_READ|PROT_WRITE,kmapaddrflags,shm_id,0);
	close(shm_id);
	mOutbox = (msq_impl_t*) mShmAddr;
	const char* errstr = strerror(errno);
#ifdef __MSGQ_DEBUG__ 
	printf( "shmid<%d> errno<%s> size<%d>\n", shm_id, errstr, int(isize) );
#endif

	assert(mShmAddr!=(void*)0xffffffffffffffff);
	new(mOutbox) msq_impl_t;
	SendSyncStart();
	SetSenderState(ork::EMQEPS_RUNNING);
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQSender::Connect( const std::string& nam )
{
	SetName(nam);
#ifdef __MSGQ_DEBUG__ 	
	printf( "IpcMsgQSender<%p> connecting to msgQ<%s>\n", this, nam.c_str() );
#endif

	while( false==ork::Path(mPath).IsFile() )
	{
		usleep(1<<18);
	}

	size_t isize = sizeof(msq_impl_t);
	int shm_id = shm_open(mName.c_str(),O_RDWR,S_IRUSR|S_IWUSR);
	assert(shm_id>=0);
	mShmAddr =  mmap(0,isize,PROT_READ|PROT_WRITE,kmapaddrflags,shm_id,0);
	mOutbox = (msq_impl_t*) mShmAddr;
	close(shm_id);
#ifdef __MSGQ_DEBUG__ 
	printf( "IpcMsgQSender<%p> connected to msgQ<%s>\n", this, nam.c_str() );
#endif

}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQSender::SendSyncStart()
{
#ifdef __MSGQ_DEBUG__ 	
	printf( "IpcMsgQSender<%p> sending  start/sync\n", this );
#endif
    ork::NetworkMessage msg;
    msg.WriteString("start/sync");
	uint32_t msg_priority = 0;
	mOutbox->mMsgQ.push(msg);
	//mOutgoingIpcQ->send(&msg,sizeof(msg),msg_priority);
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQSender::send( const NetworkMessage& inc_msg )
{
	assert(mOutbox!=nullptr);
	assert(GetRecieverState()!=EMQEPS_TERMINATED);
	mOutbox->mMsgQ.push(inc_msg);
}

void IpcMsgQSender::send_debug( const NetworkMessage& inc_msg )
{
	assert(mOutbox!=nullptr);
	assert(GetRecieverState()!=EMQEPS_TERMINATED);
	mOutbox->mDbgQ.push(inc_msg);
}

void IpcMsgQSender::SetSenderState(msgq_ep_state est)
{
	assert(mOutbox!=nullptr);
	assert(mOutbox!=nullptr);
	mOutbox->mSenderState = (uint64_t) est;
}
msgq_ep_state IpcMsgQSender::GetRecieverState() const
{
	assert(mOutbox!=nullptr);
	return (msgq_ep_state) mOutbox->mRecieverState.load();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IpcMsgQReciever::IpcMsgQReciever()
	//: mbShutdown(false)
	: mInbox(nullptr)
	, mShmAddr(nullptr)
{
    mFutureCtr = 0;
}

///////////////////////////////////////////////////////////////////////////////

IpcMsgQReciever::~IpcMsgQReciever()
{
	//mbShutdown = true;
	//while( true == mbShutdown )
	//{
	//	usleep(1<<16);
	//}
	if( mShmAddr )
	{	
		SetRecieverState(EMQEPS_TERMINATED);
		shm_unlink(mPath.c_str());
		munmap(mShmAddr,sizeof(msq_impl_t));
	}
}

///////////////////////////////////////////////////////////////////////////////
// allocate a future
//  if one is not available in the pool, a new one will be heap-allocated
// once allocated, a msgq unique future ID will be assigned
//  and the future registered to that ID for later search
// 
// future's are attached/registered to msq recievers
//  because msq-reciever's will have to look a future
//  up when the future's associated message reply 
//  is recieved.
///////////////////////////////////////////////////////////////////////////////

ork::RpcFuture* IpcMsgQReciever::AllocFuture()
{
	ork::RpcFuture* rval = nullptr;
	bool ok = mFuturePool.try_pop(rval);
	if( ok )
	{
		assert(rval!=nullptr);
	}
	else // make a new one
		rval = new ork::RpcFuture;

	rval->Clear();
	rval->SetId(mFutureCtr++);
	mFutureMap.LockForWrite()[rval->GetId()]=rval;
	mFutureMap.Unlock();
	return rval;
}

///////////////////////////////////////////////////////////////////////////////
// find an allocated future by msgq unique future ID
///////////////////////////////////////////////////////////////////////////////

ork::RpcFuture* IpcMsgQReciever::FindFuture(int fid) const
{
	ork::RpcFuture* rval = nullptr;
	const future_map_t& fmap = mFutureMap.LockForRead();
	future_map_t::const_iterator it=fmap.find(fid);
	rval = (it==fmap.end()) ? nullptr : it->second;
	mFutureMap.Unlock();
	return rval;
}

///////////////////////////////////////////////////////////////////////////////
// return future to msq's future pool,
//  de-register it's ID
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IpcMsgQReciever::ReturnFuture( ork::RpcFuture* pf)
{
	future_map_t& fmap = mFutureMap.LockForWrite();
	future_map_t::iterator it=fmap.find(pf->GetId());
	assert(it!=fmap.end());
	fmap.erase(it);
	mFutureMap.Unlock();
	mFuturePool.push(pf);
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQReciever::SetName(const std::string& nam)
{
	mName = nam;
	ork::uristring_t path;
	path.format("/dev/shm/%s",nam.c_str());
	mPath = path.c_str();
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQReciever::Connect( const std::string& nam )
{
	SetName(nam);
#ifdef __MSGQ_DEBUG__ 
	printf( "IpcMsgQReciever<%p> connecting to msgQ<%s>\n", this, nam.c_str() );
#endif

	while( false==ork::Path(mPath).IsFile() )
	{
		usleep(1<<18);
	}
	size_t isize = sizeof(msq_impl_t);
	int shm_id = shm_open(mName.c_str(),O_RDWR,S_IRUSR|S_IWUSR);
	assert(shm_id>=0);
	mShmAddr =  mmap(0,isize,PROT_READ|PROT_WRITE,kmapaddrflags,shm_id,0);
	mInbox = (msq_impl_t*) mShmAddr;
	close(shm_id);
#ifdef __MSGQ_DEBUG__ 	
	printf( "IpcMsgQReciever<%p> detected msgQ<%s>\n", this, nam.c_str() );
#endif

	WaitSyncStart();
	SetRecieverState(EMQEPS_RUNNING);
#ifdef __MSGQ_DEBUG__ 	
	printf( "IpcMsgQReciever<%p> connected to msgQ<%s>\n", this, nam.c_str() );
#endif

}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQReciever::Create( const std::string& nam )
{
	SetName(nam);
#ifdef __MSGQ_DEBUG__ 	
	printf( "IpcMsgQReciever<%p> creating msgQ<%s>\n", this, nam.c_str() );
#endif

	size_t isize = sizeof(msq_impl_t);
	int shm_id = shm_open(mName.c_str(),O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	assert(shm_id>=0);
	int iftr = ftruncate(shm_id, isize);
	mShmAddr =  mmap(0,isize,PROT_READ|PROT_WRITE,kmapaddrflags,shm_id,0);
	new(mShmAddr) msq_impl_t;
	mInbox = (msq_impl_t*) mShmAddr;
	close(shm_id);
	SetRecieverState(ork::EMQEPS_RUNNING);
#ifdef __MSGQ_DEBUG__ 
    printf( "IpcMsgQReciever<%p> created msgQ<%s>\n", this, nam.c_str() );
#endif
}

///////////////////////////////////////////////////////////////////////////////

void IpcMsgQReciever::WaitSyncStart()
{
	SetRecieverState(EMQEPS_WAIT);
    // wait for sender to send start/sync message
    ork::NetworkMessage msg;

	bool bpopped = false;
	while(false==bpopped)
	{
		bpopped = mInbox->mMsgQ.try_pop(msg);
	}	
    ork::NetworkMessageIterator syncit(msg);
    std::string sync_content = msg.ReadString(syncit);
    assert(sync_content=="start/sync");
}

void IpcMsgQReciever::SetRecieverState(msgq_ep_state est)
{
	assert(mInbox!=nullptr);
	mInbox->mRecieverState.store((uint64_t)est);
}
msgq_ep_state IpcMsgQReciever::GetSenderState() const
{
	assert(mInbox!=nullptr);
	return (msgq_ep_state) mInbox->mSenderState.load();
}

///////////////////////////////////////////////////////////////////////////////

bool IpcMsgQReciever::try_recv( NetworkMessage& out_msg )
{
	assert(mInbox!=nullptr);
	bool bpopped = mInbox->mMsgQ.try_pop(out_msg);
	if( bpopped )
	{
		//out_msg.dump("try_recv");
	}
	return bpopped;
}

bool IpcMsgQReciever::try_recv_debug( NetworkMessage& out_msg )
{
	assert(mInbox!=nullptr);
	bool bpopped = mInbox->mDbgQ.try_pop(out_msg);
	if( bpopped )
	{
		//out_msg.dump("try_recv");
	}
	return bpopped;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

} // namespace net

