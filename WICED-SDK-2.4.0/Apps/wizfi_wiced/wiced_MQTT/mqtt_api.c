/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */


#include "wiced.h"
#include "mqtt_api.h"
#include "mqtt_internal.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               extern Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static mqtt_session_t session;
//daniel 160630 Add MQTT Protocol
mqtt_connection_t *g_p_mqtt_connection;
/******************************************************
 *               Function Definitions
 ******************************************************/


wiced_result_t wiced_mqtt_init( wiced_mqtt_object_t mqtt_obj )
{
    wiced_result_t result = WICED_SUCCESS;

    /* Make sure the actual structure size and size defined in macro are not mismatching */
    if ( sizeof(mqtt_connection_t) > WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT )
    {
        return WICED_ERROR;
    }
    result = mqtt_core_init( mqtt_obj );
    //daniel 160630 modify original source for socket callback. => wiced_result_t wiced_tcp_register_callbacks( wiced_tcp_socket_t* socket, wiced_socket_callback_t connect_callback, wiced_socket_callback_t receive_callback, wiced_socket_callback_t disconnect_callback )
    if (result == WICED_SUCCESS)		g_p_mqtt_connection = mqtt_obj;
    else								g_p_mqtt_connection = 0;

    return result;
}

wiced_result_t wiced_mqtt_deinit( wiced_mqtt_object_t mqtt_obj )
{
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    wiced_result_t result = WICED_SUCCESS;

    result = mqtt_core_deinit( conn );
    //daniel 160630 modify original source
    g_p_mqtt_connection = mqtt_obj;
    return result;
}

wiced_result_t wiced_mqtt_connect( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, wiced_mqtt_security_t *security, wiced_mqtt_pkt_connect_t *conninfo )
{
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    wiced_result_t result = WICED_SUCCESS;
    mqtt_connect_arg_t args;

    memset( &args, 0, sizeof(mqtt_connect_arg_t) );

    result = mqtt_connection_init( address, conninfo->port_number, interface, callback, conn, security );

    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("[MQTT LIB] : error intializing the  mqtt connection setup\n"));
        return result;
    }

    args.clean_session = conninfo->clean_session;
    args.will_flag = 0;
    if ( conninfo->username != NULL )
    {
        args.username_flag = 1;
        args.username.str = (uint8_t*) conninfo->username;
        args.username.len = (uint16_t) strlen( (char*) conninfo->username );
    }
    else
    {
        args.username_flag = 0;
    }

    if ( conninfo->password != NULL )
    {
        args.password_flag = 1;
        args.password.str = (uint8_t*) conninfo->password;
        args.password.len = (uint16_t) strlen( (char*) conninfo->password );
    }
    else
    {
        args.password_flag = 0;
    }
    args.client_id.str = (uint8_t*) conninfo->client_id;
    args.client_id.len = (uint16_t) strlen( (char*) conninfo->client_id );
    args.keep_alive = conninfo->keep_alive; /*in seconds */
    args.mqtt_version = conninfo->mqtt_version;

    return mqtt_connect( conn, &args, &session );
}

wiced_result_t wiced_mqtt_disconnect( wiced_mqtt_object_t mqtt_obj )
{
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    return mqtt_disconnect( conn );
}

wiced_mqtt_msgid_t wiced_mqtt_subscribe( wiced_mqtt_object_t mqtt_obj, char *topic, uint8_t qos )
{
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    mqtt_subscribe_arg_t args;

    args.topic_filter.str = (uint8_t*) topic;
    args.topic_filter.len = (uint16_t) strlen( topic );
    args.qos = qos;
    args.packet_id = ( ++conn->packet_id );
    if ( mqtt_subscribe( conn, &args ) != WICED_SUCCESS )
    {
        return 0;
    }
    return args.packet_id;
}

wiced_mqtt_msgid_t wiced_mqtt_unsubscribe( wiced_mqtt_object_t mqtt_obj, char *topic )
{
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    mqtt_unsubscribe_arg_t args;

    args.topic_filter.str = (uint8_t*) topic;
    args.topic_filter.len = (uint16_t) ( strlen( topic ) );
    args.packet_id = ( ++conn->packet_id );
    if ( mqtt_unsubscribe( conn, &args ) != WICED_SUCCESS )
    {
        return 0;
    }
    return args.packet_id;
}

wiced_mqtt_msgid_t wiced_mqtt_publish( wiced_mqtt_object_t mqtt_obj, uint8_t *topic, uint8_t *data, uint32_t data_len, uint8_t qos )
{
    mqtt_publish_arg_t args;
    mqtt_connection_t *conn = (mqtt_connection_t*) mqtt_obj;
    args.topic.str = (uint8_t*) topic;
    args.topic.len = (uint16_t) strlen( (char*) topic );
    args.data = (uint8_t*) data;
    args.data_len = data_len;
    args.dup = 0;
    args.qos = qos;
    args.retain = 0;
    args.packet_id = ( ++conn->packet_id );
    if ( mqtt_publish( conn, &args ) != WICED_SUCCESS )
    {
        return 0;
    }
    return args.packet_id;
}
