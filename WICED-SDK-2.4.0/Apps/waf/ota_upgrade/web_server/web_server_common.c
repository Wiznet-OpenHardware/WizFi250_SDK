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
 * Provides common functions for the web server.
 * Particularly for processing URLs
 *
 */

#include "web_server.h"
#include "wwd_debug.h"
#include <string.h>

/******************************************************
 *             HTTP Headers
 ******************************************************/

static const char ok_header[] =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: ";

static const char crlfcrlf[] ="\r\n\r\n";

#ifndef USE_404

static const char not_found_header[] = "HTTP/1.0 301\r\nLocation: /\r\n\r\n";

#else /* ifndef USE_404 */
static const char not_found_header[] =
    "HTTP/1.0 404 Not Found\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!doctype html>\n"
    "<html><head><title>404 - WICED Web Server</title></head><body>\n"
    "<h1>Address not found on WICED Web Server</h1>\n"
    "<p><a href=\"/\">Return to home page</a></p>\n"
    "</body>\n</html>\n";

#endif /* ifndef USE_404 */

static const char redirect_header[] =
    "HTTP/1.1 302 Found\r\n"
    "Location: http://192.168.0.1\r\n\r\n";


/**
 * Processes web server requests
 *
 * Finds the path and query parts of a request,
 * Allocates a packet for the response
 * Checks whether there is a path matching the request
 * If there is, a success HTTP header is added, and the handler function for that path is called to fill the content.
 * If there is not, the 404 HTTP header is written to the response packet
 *
 * @param server_url_list : the content list from which to serve pages
 * @param url             : character array containing the request string - NOT null terminated
 * @param url_len         : length in bytes of url character array
 * @param socket          : pointer to the socket to which the response will be sent
 * @param peer_ip_address : the IPv4 address of the connected peer (for debug printing)
 *
 * @return use return to indicate success=0, error<0 or end-server=1
 */

const url_list_elem_t * find_url_item( const url_list_elem_t * server_url_list, char * url, int url_len )
{
    int i = 0;

    /* Search the url to find the question mark if there is one */
    char * params = url;
    int params_len = url_len;
    while ( ( *params != '?' ) && ( params_len > 0 ) )
    {
        params++;
        params_len--;
    }

    while ( server_url_list[i].url != NULL )
    {
        /* Compare request to a path */
        if ( ( url_len - params_len == strlen( server_url_list[i].url ) ) &&
             ( 0 == memcmp( server_url_list[i].url, url, url_len - params_len ) ) )
        {
            return &server_url_list[i];
        }
        i++;
    }

    /* Check if page was found */
    return NULL;
}




int process_url_request( const url_list_elem_t * server_url_item, char * url, int url_len, void* socket, unsigned long peer_ip_address )
{
    int retval = -1;
    if ( server_url_item != NULL )
    {
        /* Search the url to find the question mark if there is one */
        char * params = url;
        int params_len = url_len;
        while ( ( *params != '?' ) && ( params_len > 0 ) )
        {
            params++;
            params_len--;
        }

        /* terminate the path part of the string with a null - will replace the question mark */
        *params = '\x00';

        /* increment the pointer to the parameter query part of the url to skip over the null which was just written */
        params++;

        /* Copy HTTP OK header into packet and adjust the write pointers */
        send_web_data( socket, (unsigned char*) ok_header, sizeof( ok_header ) - 1 ); /* minus one is to avoid copying terminating null */

        /* Add Mime type */
        send_web_data( socket, (unsigned char*) server_url_item->mime_type, strlen( server_url_item->mime_type ) );

        /* Add double carriage return, line feed */
        send_web_data( socket, (unsigned char*) crlfcrlf, sizeof( crlfcrlf ) - 1 ); /* minus one is to avoid copying terminating null */

        WEB_SERVER_PRINT(("Serving page %s to %u.%u.%u.%u\r\n", server_url_item->url, (unsigned char)((peer_ip_address >> 24)& 0xff),
                                                                     (unsigned char)((peer_ip_address >> 16)& 0xff),
                                                                     (unsigned char)((peer_ip_address >> 8)& 0xff),
                                                                     (unsigned char)((peer_ip_address >> 0)& 0xff) ));

        /* Call the content handler function to write the page content into the packet and adjust the write pointers */
        retval = server_url_item->processor( socket, params, params_len );

        WEB_SERVER_PRINT(("Finished page\r\n"));
    }
    else
    {
#ifdef WEB_SERVER_USE_302
        /* Copy the 302 header into packet */
        send_web_data( socket, (unsigned char*) redirect_header, sizeof( redirect_header ) - 1 ); /* minus one is to avoid copying terminating null */
#else
        /* Copy the 404 header into packet */
        send_web_data( socket, (unsigned char*) not_found_header, sizeof( not_found_header ) - 1 ); /* minus one is to avoid copying terminating null */

#endif
        WEB_SERVER_PRINT(("Not found %s from %u.%u.%u.%u\r\n", server_url_item->url, (unsigned char)((peer_ip_address >> 24)& 0xff),
                                                                    (unsigned char)((peer_ip_address >> 16)& 0xff),
                                                                    (unsigned char)((peer_ip_address >> 8)& 0xff),
                                                                    (unsigned char)((peer_ip_address >> 0)& 0xff) ));
    }
    return retval;
}

