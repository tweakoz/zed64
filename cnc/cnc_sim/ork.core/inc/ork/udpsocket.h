#pragma once

// Headers (e.g. for flags)
# include <sys/types.h>
# include <sys/socket.h>
# include <ork/netpacket.h>
#include <ork/fixedstring.h>

// Macro definitions

//# define LIBSOCKET_READ 1
//# define LIBSOCKET_WRITE 2
//# define LIBSOCKET_NUMERIC 1

namespace ork{

struct RecvContext
{
    fxstring256 mSrcHost;
    fxstring16 mSrcService;
    int mFromFlags;
    bool mNumeric;
    ssize_t mNumBytesRecieved;
};

struct UdpSocket
{
	UdpSocket(); // client
	UdpSocket( const std::string& bind_addr, const std::string& bind_port); // server
	~UdpSocket();
	bool IsOpen() const { return mSockFd!=-1; }

	ssize_t Send(const MessagePacketBase&pkt, const std::string& host, const std::string& service, int sendto_flags);

	bool Recv(MessagePacketBase&pkt, RecvContext& rcvctx);

	int mSockFd;


};

} // namespace ork