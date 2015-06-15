#include <unittest++/UnitTest++.h>
#include <ork/udpsocket.h>
#include <ork/thread.h>
#include <ork/netpacket.h>
#include <ork/timer.h>
#include <ork/atomic.h>
#include <ork/fixedstring.h>

TEST(Udp1)
{
    ork::atomic<int> icounter;
    icounter = 0;

    static const int knumpackets = 1<<10;
    static const int kpktsize = 4096;
    typedef ork::MessagePacket<kpktsize> pkt_t;
    typedef ork::MessagePacketIterator<kpktsize> it_t;

    bool bdone = false;

    auto do_server = [&]()
    {
        ork::UdpSocket udp("0.0.0.0","12340");
        CHECK( udp.IsOpen() );
        pkt_t pkt;
        uint32_t a = 0;
        ork::RecvContext rctx;
        while( a != 0xffffffff )
        {
            pkt.clear();
            it_t it( pkt );
            udp.Recv(pkt,rctx);
            pkt.Read(a,it);
            icounter++;
        }
        int inumdropped = (knumpackets+1)-icounter;
        printf( "num_udp_packets_recieved<%d> num_dropped<%d>\n", int(icounter), inumdropped );
        bdone = true;
    };
    auto do_client = [&]()
    {
        ork::UdpSocket udp_client;
        CHECK( udp_client.IsOpen() );
        pkt_t pkta, pktend;
        uint32_t a = 0x01234567;
        for( int i=0; i<256; i++ )
            pkta.Write(a);

        pktend.Write(uint32_t(0xffffffff));
        for( int i=0; i<knumpackets; i++ )
            udp_client.Send(pkta,"localhost","12340",0);
        while(false==bdone)
            udp_client.Send(pktend,"localhost","12340",0);
    };

    ork::Timer tmr;

    tmr.Start();

    ork::thread thr_server, thr_client;
    thr_server.start(do_server);
    usleep(1<<19);
    thr_client.start(do_client);
    thr_server.join();
    thr_client.join();

    float ft = tmr.SecsSinceStart();
    float pktpsec = float(icounter)/ft;
    printf( "Udp NumPackets<%f>\n", float(icounter) );
    printf( "Udp Elasped<%f>\n", ft );
    printf( "Udp PacketsPerSec<%f>\n", pktpsec );

}
