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
 * A DNS server which responds to all DNS queries with the local address of the server.
 */


#include "tx_api.h"
#include "nx_api.h"
#include <string.h>
#include <stdint.h>
#include "Network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"

/******************************************************
 *        DEFINES
 ******************************************************/

#define DNS_STACK_SIZE                     (1024)

#define DNS_FLAG_QR_POS                     (15)
#define DNS_FLAG_QR_SIZE                  (0x01)
#define DNS_FLAG_QR_QUERY                    (0)
#define DNS_FLAG_QR_RESPONSE                 (1)


/* DNS opcodes */
#define DNS_FLAG_OPCODE_POS                 (11)
#define DNS_FLAG_OPCODE_OPCODE_SIZE       (0x0f)
#define DNS_FLAG_OPCODE_SQUERY               (0) /* RFC 1035 */
#define DNS_FLAG_OPCODE_IQUERY               (1) /* RFC 1035, 3425 */
#define DNS_FLAG_OPCODE_STATUS               (2) /* RFC 1035 */
#define DNS_FLAG_OPCODE_NOTIFY               (4) /* RFC 1996 */
#define DNS_FLAG_OPCODE_UPDATE               (5) /* RFC 2136 */

#define DNS_FLAG_AUTHORITIVE_POS            (10)
#define DNS_FLAG_AUTHORITIVE_YES             (1)
#define DNS_FLAG_AUTHORITIVE_NO              (0)

#define DNS_FLAG_TRUNCATED_POS               (9)
#define DNS_FLAG_TRUNCATED_YES               (1)
#define DNS_FLAG_TRUNCATED_NO                (0)

#define DNS_FLAG_DESIRE_RECURS_POS           (8)
#define DNS_FLAG_DESIRE_RECURS_YES           (1)
#define DNS_FLAG_DESIRE_RECURS_NO            (0)

#define DNS_FLAG_RECURS_AVAIL_POS            (7)
#define DNS_FLAG_RECURS_AVAIL_YES            (1)
#define DNS_FLAG_RECURS_AVAIL_NO             (0)

#define DNS_FLAG_AUTHENTICATED_POS           (5)
#define DNS_FLAG_AUTHENTICATED_YES           (1)
#define DNS_FLAG_AUTHENTICATED_NO            (0)

#define DNS_FLAG_ACCEPT_NON_AUTHENT_POS      (4)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_YES      (1)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_NO       (0)

#define DNS_FLAG_REPLY_CODE_POS              (0)
#define DNS_FLAG_REPLY_CODE_NO_ERROR         (0)


#define DNS_UDP_PORT                        (53)


/* DNS socket timeout value in milliseconds. Modify this to make thread exiting more responsive */
#define DNS_SOCKET_TIMEOUT                 (500)

// kaizen 20130408
//#define WICED_DNS_DOMAIN_NAME                  "\x0a" "securedemo" "\x05" "wiced" "\x08" "broadcom" "\x03" "com" "\x00"
#define WICED_DNS_DOMAIN_NAME                  "\x08" "wizfi250" "\x06" "wiznet" "\x03" "com" "\x00"


#define DNS_MAX_DOMAIN_LEN                 (255)
#define DNS_QUESTION_TYPE_CLASS_SIZE         (4)
#define DNS_IPV4_ADDRESS_SIZE                (4)

#define DNS_QUERY_TABLE_ENTRY(s) {s, sizeof(s)}
#define DNS_QUERY_TABLE_SIZE     (sizeof(valid_dns_query_table)/sizeof(dns_query_table_entry_t))

#define MEMCAT(destination, source, source_length)    (void*)((uint8_t*)memcpy((destination),(source),(source_length)) + (source_length))

/******************************************************
 *        Structures
 ******************************************************/


#pragma pack(1)

/* DNS data structure */
typedef struct
{
    uint16_t  transaction_id;
    uint16_t  flags;
    uint16_t  num_questions;
    uint16_t  num_answer_records;
    uint16_t  num_authority_records;
    uint16_t  num_additional_records;

} dns_header_t;

typedef struct
{
    uint8_t  name_offset_indicator;
    uint8_t  name_offset_value;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t data_length;
    char     alternate_name[sizeof(WICED_DNS_DOMAIN_NAME)-1];
} dns_cname_record_t;

typedef struct
{
    uint8_t  name_offset_indicator;
    uint8_t  name_offset_value;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t data_length;
    /* Note: Actual IPv4 address needs to be manually inserted after this */
//    uint32_t  ipv4_address;
} dns_a_record_t;

typedef struct
{
    char*    query;
    uint8_t  length;
} dns_query_table_entry_t;

#pragma pack()



/******************************************************
 *        Static Variables
 ******************************************************/

static const dns_a_record_t a_record_template =
{
    .name_offset_indicator = 0xC0,       /* Indicates next value is a offset to a name, rather than the length of an immediately following segment */
    .name_offset_value     = 0x00,       /* Offset of cname name. To be calculated at runtime */
    .type                  = 0x0100,     /* Type A - host address */
    .class                 = 0x0100,     /* Internet Class */
    .ttl                   = 0x0,        /* Time to Live - 0 seconds - to avoid messing up proper internet DNS cache */
    .data_length           = 0x0400,     /* Data Length - 4 bytes for IPv4 address */
};

static const dns_query_table_entry_t valid_dns_query_table[] =
{
	  // kaizen 20130408
	  DNS_QUERY_TABLE_ENTRY("\x08""wizfi250""\x06""wiznet""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x0A""securedemo""\x05""wiced""\x08""broadcom""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x05""apple""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x05""apple""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x06""google""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x06""google""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x04""bing""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x04""bing""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x08""facebook""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x08""facebook""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x08""broadcom""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x08""broadcom""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x05""wiced""\x03""com"),
//    DNS_QUERY_TABLE_ENTRY("\x03""www""\x05""wiced""\x03""com"),
};

static volatile char    dns_quit_flag = 0;
static NX_UDP_SOCKET    dns_socket_handle;
static TX_SEMAPHORE     dns_complete_sema;
static TX_THREAD        dns_thread_handle;
static char             dns_thread_stack     [DNS_STACK_SIZE];

/******************************************************
 *        Function Prototypes
 ******************************************************/

static void dns_thread       ( ULONG thread_input );
void        start_dns_server ( NX_IP* local_addr );
void        quit_dns_server  ( void );

/******************************************************
 *        Function Definitions
 ******************************************************/
void start_dns_server( NX_IP* local_addr )
{
    tx_thread_create( &dns_thread_handle, "DNS thread", dns_thread, (ULONG)local_addr, dns_thread_stack, DNS_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START );
}

void quit_dns_server( void )
{
    dns_quit_flag = 1;
    tx_semaphore_create( &dns_complete_sema, (char*) "", 0 );
    dns_quit_flag = 1;
    nx_udp_socket_unbind( &dns_socket_handle );
/*    tx_thread_wait_abort( &dns_thread_handle ); */
    tx_semaphore_get( &dns_complete_sema, TX_WAIT_FOREVER );
    tx_semaphore_delete( &dns_complete_sema );
    tx_thread_terminate( &dns_thread_handle );
    tx_thread_delete( &dns_thread_handle );
}

/**
 *  Implements a very simple DNS server.
 *
 * @param my_addr : local IP address for binding of server port.
 */
static void dns_thread( ULONG thread_input )
{
    NX_IP*     my_addr = (NX_IP*) thread_input;
    UINT       status;
    NX_PACKET* packet_ptr;
    ULONG      source_ip;
    UINT       source_port;
    char * loc_ptr;
    static dns_header_t* dns_header_ptr = NULL;

    /* Create DNS socket */
    if ( NX_SUCCESS != nx_udp_socket_create( my_addr, &dns_socket_handle, "DNSsock", 0, NX_DONT_FRAGMENT, 255, 5 ) )
    {
        return;
    }

    /* Bind the socket to the local UDP port */
    if ( NX_SUCCESS != nx_udp_socket_bind( &dns_socket_handle, DNS_UDP_PORT, NX_WAIT_FOREVER ) )
    {
        nx_udp_socket_delete( &dns_socket_handle );
    }

    /* Loop endlessly */
    while ( dns_quit_flag == 0 )
    {
        /* Sleep until data is received from socket. */
        status = nx_udp_socket_receive( &dns_socket_handle, &packet_ptr, NX_WAIT_FOREVER );
        if ( status != NX_SUCCESS )
        {
            continue;
        }
        nx_udp_source_extract( packet_ptr, &source_ip, &source_port );
        dns_header_ptr = (dns_header_t*)packet_ptr->nx_packet_prepend_ptr;

        /* Fix endianness of header */
        dns_header_ptr->flags                  = ntohs( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = ntohs( dns_header_ptr->num_questions );

        /* Only respond to queries */
        if ( DNS_FLAG_QR_QUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_QR_POS ) & DNS_FLAG_QR_SIZE ) )
        {
            nx_packet_release( packet_ptr );
            continue;
        }

        if ( DNS_FLAG_OPCODE_SQUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_OPCODE_POS ) & DNS_FLAG_OPCODE_OPCODE_SIZE ) )
        {
            nx_packet_release( packet_ptr );
            continue;
        }

        if ( dns_header_ptr->num_questions < 1 )
        {
            nx_packet_release( packet_ptr );
            continue;
        }


        dns_header_ptr->flags = ( DNS_FLAG_QR_RESPONSE           << DNS_FLAG_QR_POS                 ) |
                                ( DNS_FLAG_OPCODE_SQUERY         << DNS_FLAG_OPCODE_POS             ) |
                                ( DNS_FLAG_AUTHORITIVE_YES       << DNS_FLAG_AUTHORITIVE_POS        ) |
                                ( DNS_FLAG_TRUNCATED_NO          << DNS_FLAG_TRUNCATED_POS          ) |
                                ( DNS_FLAG_DESIRE_RECURS_NO      << DNS_FLAG_DESIRE_RECURS_POS      ) |
                                ( DNS_FLAG_RECURS_AVAIL_NO       << DNS_FLAG_RECURS_AVAIL_POS       ) |
                                ( DNS_FLAG_AUTHENTICATED_YES     << DNS_FLAG_AUTHENTICATED_POS      ) |
                                ( DNS_FLAG_ACCEPT_NON_AUTHENT_NO << DNS_FLAG_ACCEPT_NON_AUTHENT_POS ) |
                                ( DNS_FLAG_REPLY_CODE_NO_ERROR   << DNS_FLAG_REPLY_CODE_POS         );


        dns_header_ptr->num_questions          = 1;
        dns_header_ptr->num_answer_records     = 1;
        dns_header_ptr->num_authority_records  = 0;


        /* Find end of question */
        loc_ptr = (char *) &dns_header_ptr[ 1 ];


#ifdef DNS_REDIRECT_ALL_QUERIES
        /* Skip over domain name - made up of sections with one byte leading size values */
        while ( ( *loc_ptr != 0 ) &&
                ( *loc_ptr <= 64 ) )
        {
            loc_ptr += *loc_ptr + 1;
        }
        loc_ptr++; /* skip terminating null */
#else
        uint8_t valid_query;
        for (valid_query = 0; valid_query < DNS_QUERY_TABLE_SIZE; ++valid_query)
        {
            if ( memcmp( loc_ptr, valid_dns_query_table[valid_query].query, valid_dns_query_table[valid_query].length ) == 0)
            {
                loc_ptr += valid_dns_query_table[valid_query].length;
                break;
            }
        }
        if ( valid_query >= DNS_QUERY_TABLE_SIZE )
        {
            nx_packet_release( packet_ptr );
            continue;
        }
#endif

        /* Check that domain name was not too large for packet - probably from an attack */
        if ( (loc_ptr - (char*)dns_header_ptr) > packet_ptr->nx_packet_length )
        {
            nx_packet_release( packet_ptr );
            continue;
        }

        /* Move ptr to end of question */
        loc_ptr += DNS_QUESTION_TYPE_CLASS_SIZE;

        /* check if the query name does not match this devices domain name */
//        if ( memcmp(&dns_header_ptr[ 1 ], WICED_DNS_DOMAIN_NAME, sizeof(WICED_DNS_DOMAIN_NAME)) != 0)
//        {
//            /* This must be a query for other domain */
//            /* - reply with alias (CNAME) */
//            dns_header_ptr->num_additional_records = 1;
//
//            /* Add CNAME record to redirect to our domain */
//            dns_cname_record_t* cname_record = (dns_cname_record_t*)loc_ptr;
//            loc_ptr = MEMCAT(loc_ptr, &cname_record_template, sizeof(cname_record_template));
//
//            /* Copy the A record in */
//            dns_a_record_t* a_record = (dns_a_record_t*)loc_ptr;
//            loc_ptr = MEMCAT(loc_ptr, &a_record_template, sizeof(a_record_template) );
//            a_record->name_offset_value = ((uint32_t)cname_record->alternate_name - (uint32_t)dns_header_ptr);
//        }
//        else
        {
            /* Query for this devices domain - reply with only local IP */
            dns_header_ptr->num_additional_records = 0;

            /* Copy the A record in */
            dns_a_record_t* a_record = (dns_a_record_t*)loc_ptr;
            loc_ptr = MEMCAT(loc_ptr, &a_record_template, sizeof(a_record_template) );
            a_record->name_offset_value = 0x0C;
        }


        /* Add our IP address */
        *((ULONG*)loc_ptr) = htonl( my_addr->nx_ip_address );
        loc_ptr += DNS_IPV4_ADDRESS_SIZE;

        dns_header_ptr->flags                  = htons( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = htons( dns_header_ptr->num_questions );
        dns_header_ptr->num_answer_records     = htons( dns_header_ptr->num_answer_records );
        dns_header_ptr->num_authority_records  = htons( dns_header_ptr->num_authority_records );
        dns_header_ptr->num_additional_records = htons( dns_header_ptr->num_additional_records );


        /* Send packet */
        packet_ptr->nx_packet_append_ptr = (UCHAR*)loc_ptr;
        packet_ptr->nx_packet_length = (int)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr);
        nx_udp_socket_send( &dns_socket_handle, packet_ptr, source_ip, source_port );

    }

    /* Delete DNS socket */
    status = nx_udp_socket_delete( &dns_socket_handle );
    tx_semaphore_put( &dns_complete_sema );
}


