/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
//#include <netdb.h>
#include <sys/types.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
#include <errno.h>

#define HTTPPORT (80)








#ifdef _WIN32
#include <winsock2.h>
//#include <cygwin/types.h>
#include <pthread.h>
#include <stdio.h>

#define O_NONBLOCK (0x4000)
#define F_SETFL (4)
#define F_GETFL (3)


#define tester_errno WSAGetLastError()
#define EINPROGRESS  WSAEWOULDBLOCK

static uint32_t str_to_ip(char* arg)
{
    uint32_t addr;
    if ( 4 != sscanf( arg, "%d.%d.%d.%d", &((char*)&addr)[0], &((char*)&addr)[1], &((char*)&addr)[2], &((char*)&addr)[3] ) )
    {
        return 0;
    }
    return addr;
}

static int fcntl(int fildes, int cmd, int val )
{
    if ( cmd == F_SETFL)
    {
        u_long nonblocking = ( ( val & O_NONBLOCK )!= 0 )? 1 : 0;
        ioctlsocket(fildes, FIONBIO, &nonblocking);
        return 0;
    }
    return -1;
}

typedef int socklen_t;

#define delay_milliseconds( num_ms ) Sleep( (DWORD)(num_ms) )

#define init_network( ) {  WSADATA  WSA_Data; \
                           if (WSAStartup (0x101, & WSA_Data)) \
                           { \
                               fprintf(stderr, "Could not start winsock\r\n"); \
                              return -2; \
                           } \
                        }

#else /* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/tcp.h>

#define init_network( )

#define delay_milliseconds( num_ms ) usleep((num_ms)*1000)

static uint32_t str_to_ip(char* arg)
{
    int tmp[4];
    uint32_t addr;
    if ( 4 != sscanf( arg, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3] ) )
    {
        return 0;
    }

    ((char*)&addr)[0] = tmp[0];
    ((char*)&addr)[1] = tmp[1];
    ((char*)&addr)[2] = tmp[2];
    ((char*)&addr)[3] = tmp[3];

    return addr;
}

#endif /* if defined( _WIN32 ) */






















void test_send_bad_request1( struct in_addr remote_addr );
void test_lots_of_connect_requests( struct in_addr remote_addr );


int main(int argc, char *argv[])
{
    struct hostent *he;
    struct in_addr remote_addr;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(1);
    }

    init_network( );

    /* Look up host name */
    if ( NULL == ( he = gethostbyname( argv[1] ) ) )
    {
        fprintf(stderr, "Error: unable to find address of host \"%s\"\n", argv[1] );
        exit(1);
    }

    remote_addr = *((struct in_addr *)he->h_addr);

    printf("Remote: %s (%u.%u.%u.%u)\n", argv[1], (unsigned char) ( ( ( remote_addr.s_addr ) >> 24 ) & 0xff ),
                                                  (unsigned char) ( ( ( remote_addr.s_addr ) >> 16 ) & 0xff ),
                                                  (unsigned char) ( ( ( remote_addr.s_addr ) >>  8 ) & 0xff ),
                                                  (unsigned char) ( ( ( remote_addr.s_addr ) >>  0 ) & 0xff ) );


    test_send_bad_request1( remote_addr );
//    test_lots_of_connect_requests( remote_addr );

    return 0;
}



void test_lots_of_connect_requests( struct in_addr remote_addr )
{
    int sockets[100];
    int sock_num;
    struct sockaddr_in connect_address;

    memset(&connect_address, 0, sizeof(connect_address));
    connect_address.sin_family = AF_INET;
    connect_address.sin_port = htons(HTTPPORT);
    connect_address.sin_addr = remote_addr;

    for( sock_num = 0; sock_num < sizeof(sockets)/sizeof(int); sock_num++ )
    {
        long arg;
        /* Create socket */
        if( ( sockets[sock_num] = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
        {
            fprintf( stderr, "Error: test_lots_of_connect_requests: Could not create socket #%d\n", sock_num );
            exit(1);
        }

        /* Make socket non-blocking */
        arg = fcntl( sockets[sock_num], F_GETFL, 0 );
        arg |= O_NONBLOCK;
        fcntl( sockets[sock_num], F_SETFL, arg );
    }

    for( sock_num = 0; sock_num < sizeof(sockets)/sizeof(int); sock_num++ )
    {
        /* Try to connect socket */
        if ( ( connect( sockets[sock_num], (struct sockaddr *)&connect_address, sizeof(connect_address) ) < 0 ) &&
             ( tester_errno != EINPROGRESS ) )
        {
            fprintf(stderr, "Error: test_lots_of_connect_requests : Error connecting socket %d: errno=%d\n", sock_num, errno );
            exit(1);
        }
    }

    delay_milliseconds(2000);

    for( sock_num = 0; sock_num < sizeof(sockets)/sizeof(int); sock_num++ )
    {
        close( sockets[sock_num] );
    }
    printf("done");
}



char bad_request1[] =
        "POST /gateway/gateway.dll?SessionID=1573979968.56642501 HTTP/1.1\r\n"
        "Host: by2msg4020817.gateway.messenger.live.com\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 482\r\n"
        "Origin: http://by2msg4020817.gateway.messenger.live.com\r\n"
        "Pragma: No-Cache\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.162 Safari/535.19\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "X-Requested-Session-Content-Type: text/html\r\n"
        "X-MSN-Auth: Use-Cookie\r\n"
        "Accept: */*\r\n"
        "Referer: http://by2msg4020817.gateway.messenger.live.com/xmlProxy.htm?vn=9.090515.0&domain=live.com\r\n"
        "Accept-Encoding: gzip,deflate,sdch\r\n"
        "Accept-Language: en-GB,en-US;q=0.8,en;q=0.6\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n"
        "Cookie: mkt0=en-AU; mktstate=S=948418761&U=en-au&B=en-gb&E=&P=; mkt1=norm=en-au; wlp=A|xQZd-t:a*MsaC._|N9TE-t:a*8FoiBg._; MUID=039C6A465817612E29EC69695B176132; wls=A|N9TE-t:a*mn|xQZd-t:a*mn; wlidperf=latency=256&throughput=12; wlxS=WebIM=1; wl_preperf=req=2&com=2&cache=2; E=P:WwjUE+f6zog=:wpe2YOpuZZ7BhsAVl14NH4PNIF9usoCVhzfuMJ9zsCg=:F; wla42=KjEsODI0QzIzRTgxOTRDMTE0MCwsLDB8MSwyOEM3OEM1OTk5RTREQzUxLCwsMA==; PPLState=1; RPSTAuth=EwDgARAnAAAUDNyoPUQbnUfY4iXyVq63PSybAlKAAK45YFEL99kwnzJFCKuDE8MdGrofFnU6H7U/AqRxpHf4XbC/fHjo/vv4O8N01n+acllEaOquZJOZ0kky1XXLOtnmAHo8Q68sxc+gMFTKzcGu8FozURhGDFp0Tu+/h1+/THnpCCIfKiRXE7H88OHA4phozofbbxL4MJCeF438cBGyA2YAAAid+pkc9ou0bDABMaTHkHNAhfZQZMqzaJ0s4W22zEEcugN+fkPVCxxojz+udpVwXLCcF8HejKcp67RL7RsnFdIFGhTunfQ1BoYnTBY1oYZz5Z39cCNP3TWTlEUpHKHVRFAvsHF7kK38y54FdUDh+rWkoIv3Ux+gNaYIQEen2vNlNgt9EL6gUd+DbQz09/Zy9HFqHUFHXGz82YNXVEPGY/S1GslEdu4MKGFNGq7PZZzLOjECwvAGmscppDQ2Hea7ImqvXnOsz2OTaxTPAN/f3J5flUOkJuzsMMgvAD/0+p8ArKvLAutCxl3bO5KNu67ocaPM8jEpWWT8F5QV5YCuXQuk/1+N4G2Q0vQUftdNztNRVWji2t4sGB3vb02D0FOFqXxWE287jW8KNQBNJRd9Z6u6T3aQncOR9zjKnF4B; MSPAuth=2WWCmX1xQFLftYqCL5h1ENCjULCVUVx4FSFZlpCZisktluy3bskgC!WVHh3WOS1*Hv*hX8tp6GWbnKg8l0l54LszIJTOQ3rOSYIncNtZCgE1kQ5bk3avHUA1X*Y*MCXqqB; MSPProf=2VrX06bcuFEEGuWTWHLhk9iCyaoRhK1cB71vbqqVrXl40vwoIoFgfu!YbqDvtx*RZkNGP!iGA7Zr8dbZnhAs6iCFazYZ5KPl5mkPydbnPxh5I91Sy7Pkj07WC*oqmXMZkXzoceoPI*wZ5i8BXbzOgy2Gq3ZunpZQSGjVRzG0O2TGGJcl7QpsldLA$$; MSNPPAuth=CobH2s2vJXOa!GKaAJaO33ktF3n7TTIRc8!eiZfVkEmAqM!PwbZmM14QciMY5dd0yN3vZN2*SDE1VKUuplJ3LUI3Xr!faoywLS20wIgtyqHs4CPGWNBgg!TLEF43dfVXpTmgB1Qy!o*3CjetYbjxpRyiY5Q7AtNKot1qTaUv61ki1gKPCQqmDQUarzkSkV!Zd19H5cytlwaTz0YfZHDQkRw99jcjxAtTJyHmFkKpN6k*Q4o6VkzCTSu!BYAOGA33OopzutdQpvG2EHogQQYPQAF5YcJ8Tsyuxr7Bbsi4akuJ5C3Uifv6aSMGEpJbsBAiSbGcCSl4m7nak1!orzjLlK4o7zIicgpSU9FKZ13vlkhlwymr9ODr!my7YXKMxutc7KIpgy5VUWLgF*gwDWQkd8!zu7nL72sulco2p1jmaod2geTwbsEsiK79doiKQsVjcX5J2WDlGZDUwudPM6PCNsqpiZUspKxBEMpYR9b6k7Q!PegkewJgIMPMfS*VHvJ1VA$$; MH=MSFT; NAP=V=1.9&E=c43&C=lijdr_I7-IGP5x9jZpkYBq1kD-yRNMByeztn856DlVsZpt86USpxiQ&W=2; ANON=A=8AAF36BC95DC113E9A50A643FFFFFFFF&E=c9d&W=2; xid=38bfc04b-a139-40f5-91b8-084e92c6d43e&yCGHQ&SNTxx6-W7&169; LN=jw9aq1336525327182%260d45%2611; BP=FR=&ST=&l=WC.Hotmail&p=0; xidseq=68; HIC=824c23e8194c1140|1|2|sn116w.snt116|4082; pres=824c23e8194c1140=1\r\n"
        "\r\n"
        "PUT 9 471\r\n"
        "Routing: 1.0\r\n"
        "To: 1:nikvh___@hotmail.com\r\n"
        "From: 1:nikvh___@hotmail.com;epid={34d7adbf-ee4e-4ae3-ac93-526f47fa9030}\r\n"
        "\r\n"
        "Reliability: 1.0\r\n"
        "\r\n"
        "Publication: 1.0\r\n"
        "Uri: /user\r\n"
        "Content-Type: application/user+xml\r\n"
        "Content-Length: 244\r\n"
        "\r\n"
        "<user><s n=\"IM\"><Status>NLN</Status></s><sep n=\"IM\"><Capabilities>2684355072:2147484936:1</Capabilities><WebAppData domain=\"mail.live.com\"><MsgIds><Id>json</Id></MsgIds><Properties></Properties></WebAppData><DMN>mail.live.com</DMN></sep></user>"
        ;


void test_send_bad_request1( struct in_addr remote_addr )
{
    int socket_hnd;
    struct sockaddr_in connect_address;

    memset(&connect_address, 0, sizeof(connect_address));
    connect_address.sin_family = AF_INET;
    connect_address.sin_port = htons(HTTPPORT);
    connect_address.sin_addr = remote_addr;

    /* Create socket */
    if( ( socket_hnd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        fprintf( stderr, "Error: test_send_bad_request1: Could not create socket\n" );
        exit(1);
    }

    /* Try to connect socket */
    if ( connect( socket_hnd, (struct sockaddr *)&connect_address, sizeof(connect_address) ) < 0 )
    {
        fprintf(stderr, "Error: test_send_bad_request1 : Error connecting socket: errno=%d\n", errno );
        exit(1);
    }

    if ( send( socket_hnd, bad_request1, sizeof(bad_request1) -1, 0 ) < 0 )
    {
        fprintf(stderr, "Error: test_send_bad_request1 : Error sending data: errno=%d\n", errno );
        exit(1);
    }


    delay_milliseconds(2000);

    close( socket_hnd );
    printf("done");
}
