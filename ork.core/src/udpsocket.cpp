# include <stdlib.h>
# include <stdio.h>
# include <unistd.h> 
# include <stdint.h>
# include <netdb.h> 
# include <string.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <net/if.h>
# include <netinet/in.h> 
#include <ork/udpsocket.h>


#if ! defined(__sgi)
#define HAS_getnameinfo
#endif

namespace ork{

////////////////////////////////////////////////////////////////////////////////

UdpSocket::UdpSocket()
	: mSockFd(-1)
{
    mSockFd = socket(AF_INET,SOCK_DGRAM,0);
    printf( "Client mSockFd<%d>\n", (int) mSockFd );
}
UdpSocket::UdpSocket(const std::string& bind_addr, const std::string& bind_port)
	: mSockFd(-1)
{
    if(     bind_addr.length()==0 
        ||  bind_port.length()==0 )
		return;

    addrinfo addr_hints;
    memset(&addr_hints,0,sizeof(struct addrinfo));

    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_flags = INADDR_ANY;

    addrinfo* result = nullptr;

    int retval = getaddrinfo(bind_addr.c_str(),bind_port.c_str(),&addr_hints,&result);

    if ( 0 != retval )
		return;

    for(    addrinfo* result_check = result;
            result_check != NULL;
            result_check = result_check->ai_next ) 
    {
		mSockFd = socket(result_check->ai_family, result_check->ai_socktype, result_check->ai_protocol);
		printf( "Server mSockFd<%d>\n", (int) mSockFd );

		if ( mSockFd < 0 ) 
		    continue;

		retval = bind(mSockFd,result_check->ai_addr,(socklen_t)result_check->ai_addrlen);

		if( 0 != retval ) 
        {
            close(mSockFd);
	    	continue;
        }

		if( 0 == retval ) 
		    break;
	    else
    	{
			close(mSockFd);
			mSockFd = -1;
		}
    }

    if( result )
        freeaddrinfo(result);
}

////////////////////////////////////////////////////////////////////////////////

UdpSocket::~UdpSocket()
{
    if( mSockFd >= 0 )
        close(mSockFd);
}

////////////////////////////////////////////////////////////////////////////////

ssize_t UdpSocket::Send(    const MessagePacketBase& pkt,
                            const std::string& host,
                            const std::string& service,
                            int sendto_flags)
{
    OrkAssert( IsOpen() );
    if ( 0 == pkt.GetLength() )
        return 0;

    sockaddr_storage oldsock;
    addrinfo *result, *result_check, hint;
    socklen_t oldsocklen = sizeof(sockaddr_storage);
    int return_value;

    if ( getsockname(mSockFd,(sockaddr*)&oldsock,(socklen_t*)&oldsocklen) < 0 )
		return -1;

    memset(&hint,0,sizeof(struct addrinfo));

    hint.ai_family = oldsock.ss_family;
    hint.ai_socktype = SOCK_DGRAM;

    if ( 0 != (return_value = getaddrinfo(host.c_str(),service.c_str(),&hint,&result)))
		return -1;

    for (   result_check = result;
            result_check != NULL;
            result_check = result_check->ai_next ) // go through the linked list of struct addrinfo elements
    {
		if ( -1 != (return_value = sendto(mSockFd,pkt.GetData(),pkt.GetLength(),sendto_flags,result_check->ai_addr,result_check->ai_addrlen))) // connected without error
		{
	    	break; // Exit loop if send operation was successful
		}
    }

    return return_value;
}

////////////////////////////////////////////////////////////////////////////////

bool UdpSocket::Recv(MessagePacketBase&pkt, RecvContext& rcvctx)
{
    OrkAssert( IsOpen() );
    static const ssize_t kerr = -1;

    pkt.clear();

    rcvctx.mSrcHost = "";
    rcvctx.mSrcService = "";

    socklen_t stor_addrlen = sizeof(sockaddr_storage);

    sockaddr_storage client;
    ssize_t num_bytes = recvfrom(   mSockFd,
                                    pkt.GetData(),pkt.GetMax(),
                                    rcvctx.mFromFlags,
                                    (sockaddr*)&client,
                                    &stor_addrlen );
    
    if( num_bytes<0 )
		return false;

    ///////////////////////////////////////////
    # if defined(HAS_getnameinfo) 
    ///////////////////////////////////////////
        int retval = getnameinfo(   (sockaddr*)&client, sizeof(sockaddr_storage),
                                    rcvctx.mSrcHost.mutable_c_str(),rcvctx.mSrcHost.get_maxlen(),
                                    rcvctx.mSrcService.mutable_c_str(),rcvctx.mSrcService.get_maxlen(),
                                    rcvctx.mNumeric ? (NI_NUMERICHOST | NI_NUMERICSERV) : 0  );

        if ( 0 != retval ) // Write information to the provided memory
        {
            return false;
        }
    ///////////////////////////////////////////
    #else // fake it
    ///////////////////////////////////////////
        sockaddr_storage oldsockaddr;
        socklen_t oldsockaddrlen = sizeof(struct sockaddr_storage);
    	if ( getsockname(mSockFd,&oldsockaddr,&oldsockaddrlen) < 0 )
    	    return false;

    	if ( oldsockaddrlen > sizeof(sockaddr_storage) ) // If getsockname truncated the struct
    	    return false;

        void* addrptr = nullptr;
        size_t addrlen = 0;
        uint16_t sport = 0;
    	if( AF_INET == oldsockaddr.ss_family )
    	{
        	addrptr = &(((sockaddr_in*)&client)->sin_addr);
        	addrlen = sizeof(in_addr);
        	sport = ntohs(((sockaddr_in*)&client)->sin_port);
    	}

        hostent* he = gethostbyaddr(addrptr,addrlen,oldsockaddr.ss_family);
    	if( nullptr == he )
        	return false;

        rcvctx.mSrcHost = he->h_name;
        rcvctx.mSrcService.format("%u", sport );
    ///////////////////////////////////////////
	# endif
    ///////////////////////////////////////////

    rcvctx.mNumBytesRecieved = num_bytes;

    return true;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace ork{
