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
 *
 * This application provides an example of a simple web server
 * using ThreadX / NetX.
 *
 * The application does not use a filesystem. All served pages are
 * generated by an associated content_[platform].c file
 *
 */

#include "tx_api.h"
#include "nx_api.h"
#include "netx_applications/dhcp/nxd_dhcp_client.h"
#include "wwd_wifi.h"
#include "wwd_management.h"
#include "Network/wwd_buffer_interface.h"
#include "Network/wwd_network_constants.h"
#include "Platform/wwd_platform_interface.h"
#include "web_server.h"
#include "wwd_network.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "wwd_rtos.h"

/******************************************************
 *        Constants
 ******************************************************/

#define ETHER_PAYLOAD_MTU (1500)
#define IP_HEADER_SIZE    (60)
#define TCP_HEADER_SIZE   (20)
#define MAX_TCP_PAYLOAD   ( ETHER_PAYLOAD_MTU - IP_HEADER_SIZE - TCP_HEADER_SIZE )

#define WEB_SERVER_ACCEPT_TIMEOUT_MS ( 1000 )
#define WEB_SERVER_INITIAL_DATA_TIMEOUT_MS ( 2000 )
#define WEB_SERVER_RECEIVE_QUOTED_LENGTH_DATA_MS ( 2000 )
#define WEB_SERVER_DISCONNECT_TIMEOUT ( 2000 )

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define CONTENT_LENGTH_STRING "Content-Length: "
#define CONTENT_TYPE_BOUNDARY_STRING "Content-Type: multipart/form-data; boundary="


/******************************************************
 *             Static Variables
 ******************************************************/

/******************************************************
 *             Global Function Prototypes
 ******************************************************/
typedef struct
{
  char* header;
  void* buffer;
  int buffer_size;
  wiced_bool_t integer;
  wiced_bool_t filled;
} http_header_field_t;


#ifndef __GNUC__
void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen );
#endif /* ifndef __GNUC__ */

static int parse_header( NX_PACKET** pkt_ptr, NX_IP* ip_handle, NX_TCP_SOCKET* socket, http_header_field_t* header_fields );


void run_webserver( void * bind_address_in, const url_list_elem_t * server_url_list )
{
    NX_IP* ip_handle = (NX_IP*) bind_address_in;
    UINT status;
    NX_TCP_SOCKET socket;

    /* Create server socket */
    if ( NX_SUCCESS != ( status = nx_tcp_socket_create( ip_handle, &socket, "tcp_tx_socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 4*WICED_PAYLOAD_MTU, NX_NULL, NX_NULL ) ) )
    {
        WEB_SERVER_ERROR_PRINT("Failed to create server socket\r\n");
        nx_ip_delete( ip_handle );
        return;
    }

    /* Bind socket to listen on port 80 (HTTP) */
    if ( NX_SUCCESS != ( status = nx_tcp_server_socket_listen( ip_handle, 80, &socket, 5, NULL ) ) )
    {
        WEB_SERVER_ERROR_PRINT("Failed to bind server socket to port 80\r\n");
        nx_tcp_socket_delete( &socket );
        nx_ip_delete( ip_handle );
        return;
    }

    int quit = 0;

    /* Loop forever, serving pages */
    WEB_SERVER_PRINT(("Waiting for page requests\r\n"));
    while ( quit == 0 )
    {
        NX_PACKET *packet_ptr;

        /* Wait until a connection on the server socket is received */
        status = nx_tcp_server_socket_accept( &socket, WEB_SERVER_ACCEPT_TIMEOUT_MS * SYSTICK_FREQUENCY / 1000 );
        if ( status == NX_SUCCESS )
        {
            /* Receive from connected socket */
            status = nx_tcp_socket_receive( &socket, &packet_ptr, WEB_SERVER_INITIAL_DATA_TIMEOUT_MS * SYSTICK_FREQUENCY / 1000 );
            if ( status == NX_SUCCESS )
            {
                char url_copy[50];
                unsigned long url_len;
                const url_list_elem_t* url_item;
                char boundary[100];
                int content_length = 0;
                http_header_field_t header_fields[] =
                {
                    { CONTENT_LENGTH_STRING, &content_length, sizeof(content_length), WICED_TRUE, WICED_FALSE },
                    { CONTENT_TYPE_BOUNDARY_STRING, &boundary[2], sizeof(boundary)-2, WICED_FALSE, WICED_FALSE },
                    { NULL, NULL, 0, WICED_FALSE, WICED_FALSE }
                };
                boundary[0] = '-';
                boundary[1] = '-';

                // Extract the URL and it's length
                {
                    char* request_string = (char*) packet_ptr->nx_packet_prepend_ptr;
                    char* start_of_url   = request_string;
                    char* end_of_url     = request_string;

                    /* Check that this is a GET request - other methods are not supported */
                    if ( strncmp( request_string, "GET ", 4 ) == 0 )
                    {
                        request_string[3] = '\x00';
                        start_of_url = &request_string[4];
                    }
                    if ( strncmp( request_string, "POST ", 5 ) == 0 )
                    {
                        request_string[4] = '\x00';
                        start_of_url = &request_string[5];
                    }

                    /* Find the end of the request path ( space, newline, null, or end of packet ) */
                    end_of_url = start_of_url;
                    while ( ( *end_of_url != ' ' ) &&
                            ( *end_of_url != '\n' ) &&
                            ( *end_of_url != '\x00' ) &&
                            (  end_of_url++ < (char*) packet_ptr->nx_packet_append_ptr ) )
                    {
                    }

                    /* Copy the URL because the start and end pointers will become invalid on an upload condition */
                    url_len = end_of_url - start_of_url;
                    memcpy(url_copy, start_of_url, MIN(url_len, 50));

                    /* Find the matching URL item */
                    url_item = find_url_item( server_url_list, start_of_url, url_len );
                }

                /* Parse the header
                 * Note: packet_ptr may change to another packet */
                parse_header( &packet_ptr, ip_handle, &socket, header_fields );

                /* Verify we have a content length */
                if ( ( header_fields[0].filled == WICED_TRUE ) &&
                     ( content_length > 0 ) )
                {
                    NX_PACKET* temp_packet;

                    /* Verify we have a boundary string */
                    if  ( header_fields[1].filled == WICED_TRUE )
                    {
                        char* temp_boundary = boundary;
                        temp_packet         = packet_ptr;
                        packet_ptr          = NULL;
                        int boundary_size   = strlen( temp_boundary );

                        while (boundary_size != 0)
                        {
                            if ( temp_packet->nx_packet_length == 0 )
                            {
                                nx_packet_release(temp_packet);
                                if ( nx_tcp_socket_receive( &socket, &temp_packet, WEB_SERVER_INITIAL_DATA_TIMEOUT_MS * SYSTICK_FREQUENCY / 1000 ) != NX_SUCCESS )
                                {
                                    break;
                                }
                            }

                            /* Check for the boundary string */
                            uint32_t size = MIN(boundary_size, temp_packet->nx_packet_length);
                            if ( memcmp( temp_boundary, temp_packet->nx_packet_prepend_ptr, size ) == 0 )
                            {
                                temp_boundary += size;
                                boundary_size -= size;
                                temp_packet->nx_packet_prepend_ptr += size;
                                temp_packet->nx_packet_length      -= size;
                            }
                            else
                            {
                                break;
                            }
                        }

                        while ((*temp_packet->nx_packet_prepend_ptr == '\r' || *temp_packet->nx_packet_prepend_ptr == '\n') &&
                               temp_packet->nx_packet_length != 0)
                        {
                            ++temp_packet->nx_packet_prepend_ptr;
                            --temp_packet->nx_packet_length;
                        }

                        /* Check if we found all of the boundary mark */
                        if (boundary_size == 0)
                        {
                            http_header_field_t* empty_header_fields = &header_fields[2];

                            content_length -= temp_packet->nx_packet_length;

                            parse_header( &temp_packet, ip_handle, &socket, empty_header_fields );
                            if ( url_item->upload_processor != NULL )
                            {
                                url_item->upload_processor( temp_packet->nx_packet_prepend_ptr, temp_packet->nx_packet_length );
                            }
                            status = nx_packet_release( temp_packet );
                            wiced_assert( "release failed", status == NX_SUCCESS );

                            char* boundary_pos;
                            boundary_size   = strlen( boundary );
                            while ( ( content_length > 0 ) && ( NX_SUCCESS == nx_tcp_socket_receive( &socket, &temp_packet, WEB_SERVER_RECEIVE_QUOTED_LENGTH_DATA_MS * SYSTICK_FREQUENCY / 1000 ) ) )
                            {
                                if ( url_item->upload_processor != NULL )
                                {
                                    if ( ( boundary_pos = (char*) memmem( temp_packet->nx_packet_prepend_ptr, temp_packet->nx_packet_length, boundary, boundary_size ) ) != NULL )
                                    {
                                        url_item->upload_processor( temp_packet->nx_packet_prepend_ptr, boundary_pos - (char*) temp_packet->nx_packet_prepend_ptr );
                                        url_item->upload_processor( NULL, 0 );
                                    }
                                    else
                                    {
                                        url_item->upload_processor( temp_packet->nx_packet_prepend_ptr, temp_packet->nx_packet_length );
                                    }
                                }
                                content_length -= temp_packet->nx_packet_length;
                                status = nx_packet_release( temp_packet );
                                wiced_assert( "release failed", status == NX_SUCCESS );
                            }
                        }
                    }
                    else
                    {
                        /* Not multipart data */
                        /* receive posted data */
                        while ( NX_SUCCESS == nx_tcp_socket_receive( &socket, &temp_packet, ( content_length ) ? WEB_SERVER_RECEIVE_QUOTED_LENGTH_DATA_MS * SYSTICK_FREQUENCY / 1000  : TX_NO_WAIT ) )
                        {
                            content_length -= temp_packet->nx_packet_length;
                            status = nx_packet_release( temp_packet );
                            wiced_assert( "release failed", status == NX_SUCCESS );
                        }
                    }
                }

                ULONG peer_ip_address, peer_port;
                nx_tcp_socket_peer_info_get( &socket, &peer_ip_address, &peer_port );

                /* farm out processing of the request to a sub process */
                if ( 1 == process_url_request( url_item, url_copy, url_len, &socket, peer_ip_address ) )
                {
                    quit = 1;
                }

                /* Release request packet */
                if (packet_ptr != NULL)
                {
                    nx_packet_release( packet_ptr );
                }
            }

            /* Ensure there is a transmit buffer free so that FIN-ACK is correctly sent */
            status = nx_packet_allocate( socket.nx_tcp_socket_ip_ptr->nx_ip_default_packet_pool, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER );
            if ( status )
            {
                WEB_SERVER_ERROR_PRINT("Failed to allocate a packet for FIN-ACK\r\n");
                return;
            }
            nx_packet_release( packet_ptr );
        }
        else if ( status != NX_NOT_CONNECTED )
        {
            WEB_SERVER_ERROR_PRINT("Error accepting socket connection\r\n");
            return;
        }

        /* Disconnect the server socket. */
        nx_tcp_socket_disconnect( &socket, WEB_SERVER_DISCONNECT_TIMEOUT * SYSTICK_FREQUENCY / 1000 );
        nx_tcp_server_socket_unaccept( &socket );
        nx_tcp_server_socket_relisten( ip_handle, 80, &socket );
    }
    nx_tcp_server_socket_unlisten( ip_handle, 80 );
    nx_tcp_socket_delete( &socket );
}


static int find_newline( NX_PACKET* pkt_ptr, int start_pos )
{
    int pos = (start_pos==0)?1:start_pos;

    char* ptr = (char*) &pkt_ptr->nx_packet_prepend_ptr[pos];
    while ( ( pos < pkt_ptr->nx_packet_length ) &&
            ( ! ( ( *(ptr-1) == '\r' ) &&
                  ( *(ptr)   == '\n' ) ) ) )
    {
        pos++;
        ptr++;
    }
    if ( pos < pkt_ptr->nx_packet_length )
    {
        return pos-1;
    }

    return -1;
}



static int parse_header( NX_PACKET** pkt_ptr, NX_IP* ip_handle, NX_TCP_SOCKET* socket, http_header_field_t* header_fields )
{
    int line_start = 0;
    int line_end;

    while ( 1 )
    {
        line_end = find_newline( *pkt_ptr, line_start );
        if ( line_end == -1 )
        {
            UINT status;
            NX_PACKET* tmp_pkt;

            status = nx_tcp_socket_receive( socket, &tmp_pkt, WEB_SERVER_INITIAL_DATA_TIMEOUT_MS * SYSTICK_FREQUENCY / 1000 );
            if  ( status != NX_SUCCESS )
            {
                return -status;
            }

            nx_packet_release(*pkt_ptr);
            *pkt_ptr = tmp_pkt;

            line_start = 0;
            continue;
        }
        else if ( line_end == line_start )
        {
            /* End of header */
            line_end+=2;
            (*pkt_ptr)->nx_packet_prepend_ptr += line_end;
            (*pkt_ptr)->nx_packet_length -= line_end;
            return 0;
        }

        http_header_field_t* curr_field = header_fields;
        while ( curr_field->header != NULL )
        {
            int header_len = strlen(curr_field->header);
            int field_start_pos = line_start + header_len;
            char* field_start = (char*) &(*pkt_ptr)->nx_packet_prepend_ptr[field_start_pos];

            if ( ( curr_field->buffer != NULL) &&
                 ( 0 == memcmp( &(*pkt_ptr)->nx_packet_prepend_ptr[line_start], curr_field->header, header_len ) ) )
            {
                if ( curr_field->integer == WICED_TRUE )
                {
                    wiced_assert( "wrong size", curr_field->buffer_size == 4 );
                    *((int*)curr_field->buffer) = atoi( field_start );
                }
                else
                {
                    int field_size = MIN( curr_field->buffer_size, line_end - field_start_pos );
                    memcpy( curr_field->buffer, field_start, field_size );
                    ((char*)curr_field->buffer)[field_size] = 0;
                }
                curr_field->filled = WICED_TRUE;
            }

            curr_field++;
        }

        line_start = line_end + 2;
    }
}



void send_web_data( void * socket, unsigned char * data, unsigned long length )
{
    while ( length > 0 )
    {
        NX_PACKET *packet_ptr;
        NX_TCP_SOCKET* nxsocket = (NX_TCP_SOCKET*) socket;

        uint16_t packet_size = ( length > MAX_TCP_PAYLOAD ) ? MAX_TCP_PAYLOAD : length;

        /* Allocate a packet for the response */
        UINT status = nx_packet_allocate( nxsocket->nx_tcp_socket_ip_ptr->nx_ip_default_packet_pool, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER );
        if ( status )
        {
            WEB_SERVER_ERROR_PRINT("Failed to allocate a response packet\r\n");
            return;
        }

        memcpy( packet_ptr->nx_packet_append_ptr, data, packet_size );
        packet_ptr -> nx_packet_length = packet_size;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + packet_size;

        /* Send the packet out! */
        status = nx_tcp_socket_send( nxsocket, packet_ptr, 1000 );
        if ( status )
        {
            WEB_SERVER_ERROR_PRINT( "Error sending packet \r\n" );
            nx_packet_release( packet_ptr );
            return;
        }
        data += packet_size;
        length -= packet_size;

    }
}



#ifndef __GNUC__
void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen )
{
    unsigned char* needle_char = (unsigned char*) needle;
    unsigned char* haystack_char = (unsigned char*) haystack;
    int needle_pos = 0;

    if ( ( haystack == NULL ) ||
         ( needle == NULL ) )
    {
        return NULL;
    }

    while ( ( haystacklen > 0 ) &&
            ( needle_pos < needlelen ) )
    {
        if ( *haystack_char == needle_char[needle_pos] )
        {
            needle_pos++;
        }
        else if ( needle_pos != 0 )
        {
            /* go back to start of section */
            haystacklen += needle_pos;
            haystack_char -= needle_pos;
            needle_pos = 0;
        }
        haystack_char++;
        haystacklen--;
    }

    if ( needle_pos == needlelen )
    {
       return ((unsigned char*)haystack_char) - needlelen;
    }

    return NULL;
}

#endif /* ifndef __GNUC__ */