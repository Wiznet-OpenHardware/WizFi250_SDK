/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 * A small DHCP server.
 * Allows multiple clients.
 *
 * Original source obtained on 8th April 2011 from : Public Domain source - Written by Richard Bronson - http://sourceforge.net/projects/sedhcp/
 * Heavily modified
 *
 */


#include "tx_api.h"
#include "nx_api.h"
#include <string.h>
#include <stdint.h>
#include "Network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"

/******************************************************
 *        DEFINES
 ******************************************************/

#define DHCP_STACK_SIZE               (1024)

/* BOOTP operations */
#define BOOTP_OP_REQUEST                (1)
#define BOOTP_OP_REPLY                  (2)

/* DHCP commands */
#define DHCPDISCOVER                    (1)
#define DHCPOFFER                       (2)
#define DHCPREQUEST                     (3)
#define DHCPDECLINE                     (4)
#define DHCPACK                         (5)
#define DHCPNAK                         (6)
#define DHCPRELEASE                     (7)
#define DHCPINFORM                      (8)

/* UDP port numbers for DHCP server and client */
#define IPPORT_DHCPS                   (67)
#define IPPORT_DHCPC                   (68)

/******************************************************
 *        Structures
 ******************************************************/

/* DHCP data structure */
typedef struct
{
    uint8_t  opcode;                     /* packet opcode type */
    uint8_t  hardware_type;              /* hardware addr type */
    uint8_t  hardware_addr_len;          /* hardware addr length */
    uint8_t  hops;                       /* gateway hops */
    uint32_t transaction_id;             /* transaction ID */
    uint16_t second_elapsed;             /* seconds since boot began */
    uint16_t flags;
    uint8_t  client_ip_addr[4];          /* client IP address */
    uint8_t  your_ip_addr[4];            /* 'your' IP address */
    uint8_t  server_ip_addr[4];          /* server IP address */
    uint8_t  gateway_ip_addr[4];         /* gateway IP address */
    uint8_t  client_hardware_addr[16];   /* client hardware address */
    uint8_t  legacy[192];
    uint8_t  magic[4];
    uint8_t  options[275];               /* options area */
    /* as of RFC2131 it is variable length */
} dhcp_header_t;


/******************************************************
 *        Static Variables
 ******************************************************/


static char           new_ip_addr[4]                = { 192, 168, 0, 0 };
#if 1	// kaizen 20130521 ID1073 - For set 192.168.0.xxx to dhcp client
static uint16_t       next_available_ip_addr        = ( 0 << 8 ) + 100;
#else
static uint16_t       next_available_ip_addr        = ( 1 << 8 ) + 100;
#endif
static char           subnet_option_buff[]          = { 1, 4, 255, 255, 0, 0 };
static char           server_ip_addr_option_buff[]  = { 54, 4, 192, 168, 0, 1 };
static char           mtu_option_buff[]             = { 26, 2, WICED_PAYLOAD_MTU>>8, WICED_PAYLOAD_MTU&0xff };
static char           dhcp_offer_option_buff[]      = { 53, 1, DHCPOFFER };
static char           dhcp_ack_option_buff[]        = { 53, 1, DHCPACK };
static char           dhcp_nak_option_buff[]        = { 53, 1, DHCPNAK };
static char           lease_time_option_buff[]      = { 51, 4, 0x00, 0x01, 0x51, 0x80 }; /* 1 day lease */
static char           dhcp_magic_cookie[]           = { 0x63, 0x82, 0x53, 0x63 };
static char           dhcp_quit_flag = 0;
static NX_UDP_SOCKET  dhcp_socket_handle;
static TX_SEMAPHORE   dhcp_complete_sema;
static TX_THREAD      dhcp_thread_handle;
static char           dhcp_thread_stack     [DHCP_STACK_SIZE];

/******************************************************
 *        Function Prototypes
 ******************************************************/

static unsigned char * find_option       ( dhcp_header_t* request, unsigned char option_num );
static void            dhcp_thread       ( ULONG thread_input );
void                   start_dhcp_server ( NX_IP* local_addr );
void                   quit_dhcp_server  ( void );



/******************************************************
 *        Function Definitions
 ******************************************************/

void start_dhcp_server( NX_IP* local_addr )
{
    tx_thread_create( &dhcp_thread_handle, "DHCP thread", dhcp_thread, (ULONG)local_addr, dhcp_thread_stack, DHCP_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START );

}




void quit_dhcp_server( void )
{
    tx_semaphore_create( &dhcp_complete_sema, (char*) "", 0 );
    dhcp_quit_flag = 1;
    nx_udp_socket_unbind( &dhcp_socket_handle );
/*    tx_thread_wait_abort( &dhcp_thread_handle ); */
    tx_semaphore_get( &dhcp_complete_sema, TX_WAIT_FOREVER );
    tx_semaphore_delete( &dhcp_complete_sema );
    tx_thread_terminate( &dhcp_thread_handle );
    tx_thread_delete( &dhcp_thread_handle );
}

/**
 *  Implements a very simple DHCP server.
 *
 *  Server will always offer next available address to a DISCOVER command
 *  Server will NAK any REQUEST command which is not requesting the next available address
 *  Server will ACK any REQUEST command which is for the next available address, and then increment the next available address
 *
 * @param my_addr : local IP address for binding of server port.
 */
static void dhcp_thread( ULONG thread_input )
{
    NX_IP*     my_addr = (NX_IP*) thread_input;
    UINT       status;
    char*      option_ptr;
    ULONG      junk_network_mask;
    NX_PACKET* packet_ptr;
    ULONG      local_ip;
    ULONG      source_ip;
    UINT       source_port;
    dhcp_header_t* dhcp_header_ptr;

    /* Save local IP address to be sent in DHCP packets */
    nx_ip_address_get( my_addr, &local_ip, &junk_network_mask);
    NX_CHANGE_ULONG_ENDIAN( local_ip );
    *((ULONG*)&server_ip_addr_option_buff[2]) = local_ip;

    dhcp_quit_flag = 0;

    /* Create DHCP socket */
    if ( NX_SUCCESS != nx_udp_socket_create( my_addr, &dhcp_socket_handle, "DHCPsock", 0, NX_DONT_FRAGMENT, 255, 5 ) )
    {
        return;
    }

    /* Bind the socket to the local UDP port */
    if ( NX_SUCCESS != nx_udp_socket_bind( &dhcp_socket_handle, IPPORT_DHCPS, NX_WAIT_FOREVER ) )
    {
        nx_udp_socket_delete( &dhcp_socket_handle );
    }


    /* Loop endlessly */
    while ( dhcp_quit_flag == 0 )
    {
        /* Sleep until data is received from socket. */
        status = nx_udp_socket_receive( &dhcp_socket_handle, &packet_ptr, NX_WAIT_FOREVER );
        if ( status == NX_SUCCESS )
        {
            nx_udp_source_extract( packet_ptr, &source_ip, &source_port );
            dhcp_header_ptr = (dhcp_header_t*)packet_ptr->nx_packet_prepend_ptr;

            /* Check DHCP command */
            switch ( dhcp_header_ptr->options[2] )
            {
                case DHCPDISCOVER:
                    {
                        /* Discover command - send back OFFER response */
                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Clear the DHCP options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Create the IP address for the Offer */
                        new_ip_addr[2] = next_available_ip_addr >> 8;
                        new_ip_addr[3] = next_available_ip_addr & 0xff;
                        memcpy( &dhcp_header_ptr->your_ip_addr, new_ip_addr, 4 );

                        /* Copy the magic DHCP number */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        /* Add options */
                        option_ptr = (char *) &dhcp_header_ptr->options;
                        memcpy( option_ptr, dhcp_offer_option_buff, 3 );       /* DHCP message type */
                        option_ptr += 3;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* Server identifier */
                        option_ptr += 6;
                        memcpy( option_ptr, lease_time_option_buff, 6 );       /* Lease Time */
                        option_ptr += 6;
                        memcpy( option_ptr, subnet_option_buff, 6 );           /* Subnet Mask */
                        option_ptr += 6;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* Router (gateway) */
                        option_ptr[0] = 3; /* Router id */
                        option_ptr += 6;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* DNS server */
                        option_ptr[0] = 6; /* DNS server id */
                        option_ptr += 6;
                        memcpy( option_ptr, mtu_option_buff, 4 );              /* Interface MTU */
                        option_ptr += 4;
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        packet_ptr->nx_packet_append_ptr = (UCHAR*)option_ptr;
                        packet_ptr->nx_packet_length = (int)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr);

                        /* Send packet */
                        nx_udp_socket_send( &dhcp_socket_handle, packet_ptr, NX_IP_LIMITED_BROADCAST, IPPORT_DHCPC );
                    }
                    break;

                case DHCPREQUEST:
                    {
                        /* REQUEST command - send back ACK or NAK */
                        unsigned char* requested_address;
                        uint32_t*      server_id_req;
                        uint32_t*      req_addr_ptr;
                        uint32_t*      newip_ptr;

                        /* Check that the REQUEST is for this server */
                        server_id_req = (uint32_t*) find_option( dhcp_header_ptr, 54 );
                        if ( ( server_id_req != NULL ) &&
                             ( local_ip != *server_id_req ) )
                        {
                            nx_packet_release(packet_ptr);
                            break; /* Server ID does not match local IP address */
                        }

                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Locate the requested address in the options */
                        requested_address = find_option( dhcp_header_ptr, 50 );

                        /* Copy requested address */
                        memcpy( &dhcp_header_ptr->your_ip_addr, requested_address, 4 );

                        /* Blank options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Copy DHCP magic number into packet */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        option_ptr = (char *) &dhcp_header_ptr->options;

                        /* Check if Request if for next available IP address */
                        req_addr_ptr = (uint32_t*) dhcp_header_ptr->your_ip_addr;
                        newip_ptr = (uint32_t*) new_ip_addr;
                        if ( *req_addr_ptr != ( ( *newip_ptr & 0x0000ffff ) | ( ( next_available_ip_addr & 0xff ) << 24 ) | ( ( next_available_ip_addr & 0xff00 ) << 8 ) ) )
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK */
                            /* Add appropriate options */
                            memcpy( option_ptr, dhcp_nak_option_buff, 3 );  /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memset( &dhcp_header_ptr->your_ip_addr, 0, sizeof( dhcp_header_ptr->your_ip_addr ) ); /* Clear 'your address' field */
                        }
                        else
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK
                             * Add appropriate options
                             */
                            memcpy( option_ptr, dhcp_ack_option_buff, 3 );       /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memcpy( option_ptr, lease_time_option_buff, 6 );     /* Lease Time */
                            option_ptr += 6;
                            memcpy( option_ptr, subnet_option_buff, 6 );         /* Subnet Mask */
                            option_ptr += 6;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Router (gateway) */
                            option_ptr[0] = 3; /* Router id */
                            option_ptr += 6;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* DNS server */
                            option_ptr[0] = 6; /* DNS server id */
                            option_ptr += 6;
                            memcpy( option_ptr, mtu_option_buff, 4 );            /* Interface MTU */
                            option_ptr += 4;
                            /* printf("Assigned new IP address %d.%d.%d.%d\r\n", (uint8_t)new_ip_addr[0], (uint8_t)new_ip_addr[1], next_available_ip_addr>>8, next_available_ip_addr&0xff ); */

                            /* Increment IP address */
                            next_available_ip_addr++;
                            if ( ( next_available_ip_addr & 0xff ) == 0xff ) /* Handle low byte rollover */
                            {
                                next_available_ip_addr += 101;
                            }
                            if ( ( next_available_ip_addr >> 8 ) == 0xff ) /* Handle high byte rollover */
                            {
                                next_available_ip_addr += ( 2 << 8 );
                            }
                        }
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        packet_ptr->nx_packet_append_ptr = (UCHAR*)option_ptr;
                        packet_ptr->nx_packet_length = (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr);

                        /* Send packet */
                        nx_udp_socket_send( &dhcp_socket_handle, packet_ptr, NX_IP_LIMITED_BROADCAST, IPPORT_DHCPC );
                    }
                    break;

                default:
                    nx_packet_release(packet_ptr);
                    break;
            }
        }
    }

    /* Delete DHCP socket */
    status = nx_udp_socket_delete( &dhcp_socket_handle );
    tx_semaphore_put( &dhcp_complete_sema );
}


/**
 *  Finds a specified DHCP option
 *
 *  Searches the given DHCP request and returns a pointer to the
 *  specified DHCP option data, or NULL if not found
 *
 * @param request :    The DHCP request structure
 * @param option_num : Which DHCP option number to find
 *
 * @return Pointer to the DHCP option data, or NULL if not found
 */

static unsigned char * find_option( dhcp_header_t* request, unsigned char option_num )
{
    unsigned char* option_ptr = (unsigned char*) request->options;
    while ( ( option_ptr[0] != 0xff ) &&
            ( option_ptr[0] != option_num ) &&
            ( option_ptr < ( (unsigned char*) request ) + sizeof( dhcp_header_t ) ) )
    {
        option_ptr += option_ptr[1] + 2;
    }
    if ( option_ptr[0] == option_num )
    {
        return &option_ptr[2];
    }
    return NULL;

}

