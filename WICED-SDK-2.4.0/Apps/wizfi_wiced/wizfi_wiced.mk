#
# Copyright 2011, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_wizfi_wiced

$(NAME)_SOURCES := wiced_init.c \
                   wizfi_wiced.c \
                   wizfimain/wx_commands_f.c \
                   wizfimain/wx_commands_m.c \
                   wizfimain/wx_commands_misc.c \
                   wizfimain/wx_commands_s.c \
                   wizfimain/wx_commands_w.c \
                   wizfimain/wx_debug.c \
                   wizfimain/wx_general_parse.c \
                   wizfimain/wx_s2w_function.c \
                   wizfimain/wx_s2w_process.c \
                   wizfimain/wx_platform_rtos_misc.c \
                   wizfimain/wx_platform_rtos_socket.c \
                   GMMP_lib/GMMP.c \
                   GMMP_lib/gmmp_console.c \
                   GMMP_lib/ErrorCode/StringTable.c \
                   GMMP_lib/Log/GMMP_Log.c \
                   GMMP_lib/Network/Network.c \
                   GMMP_lib/Operation/GMMP_Operation.c \
                   GMMP_lib/Operation/Control/GMMP_Control.c \
                   GMMP_lib/Operation/Delivery/GMMP_Delivery.c \
                   GMMP_lib/Operation/Encrypt/GMMP_Encrypt.c \
                   GMMP_lib/Operation/FTP/GMMP_FTP.c \
                   GMMP_lib/Operation/Heartbeat/GMMP_Heartbeat.c \
                   GMMP_lib/Operation/LOB/GMMP_LOB.c \
                   GMMP_lib/Operation/LongSentence/GMMP_LSentence.c \
                   GMMP_lib/Operation/Multimedia/GMMP_Multimedia.c \
                   GMMP_lib/Operation/Notification/GMMP_Notification.c \
                   GMMP_lib/Operation/ProfileInfo/GMMP_ProfileInfo.c \
                   GMMP_lib/Operation/Reg/GMMP_Reg.c \
                   GMMP_lib/Operation/Remote/GMMP_Remote.c \
                   GMMP_lib/Util/GMMP_Util.c \
                   wizfimain/wx_commands_mqtt.c \
                   wiced_MQTT/MQTTConsole.c  \
                   wiced_MQTT/mqtt_network.c  \
                   wiced_MQTT/mqtt_frame.c    \
                   wiced_MQTT/mqtt_connection.c \
                   wiced_MQTT/mqtt_manager.c  \
                   wiced_MQTT/mqtt_session.c \
                   wiced_MQTT/mqtt_api.c

                  
# Overwrite default watchdog timeout with application-specific timeout
# sekim 20130109 WATCHDOG timeout(APPLICATION_WATCHDOG_TIMEOUT_SECONDS)
#APPLICATION_WATCHDOG_TIMEOUT_SECONDS := 10   
#APPLICATION_WATCHDOG_TIMEOUT_SECONDS := 5

# kaizen 20130408 ID1003 
GLOBAL_DEFINES += MAC_ADDRESS_SET_BY_HOST
# kaizen 20130412 ID1042
GLOBAL_DEFINES += BUILD_WIZFI250
# kaizen 20130521 ID1043 For using DEBUG log and ERROR log
#GLOBAL_DEFINES += DEBUG_WIZFI250
#GLOBAL_DEFINES += ERROR_WIZFI250

GLOBAL_DEFINES += APPLICATION_WATCHDOG_TIMEOUT_SECONDS=20

# sekim 20130311 add REPLACE_WPRINT
GLOBAL_DEFINES += REPLACE_WPRINT=1

# kaizen XXX 20130110 APPLICATION_DCT := wizfi_dct.c
APPLICATION_DCT := wizfi_dct.c

# kaizen
GLOBAL_INCLUDES:= .
