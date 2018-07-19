#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_temp_control

$(NAME)_SOURCES := temp_control.c

$(NAME)_COMPONENTS := daemons/http_server \
                      protocols/xively \
                      sensors/NCP18XH103J03RB \
                      protocols/sntp \
                      daemons/gedday \
                      inputs/gpio_keypad

$(NAME)_RESOURCES  := apps/temp_control/main.html \
                      apps/temp_control/data.html \
                      images/brcmlogo.png \
                      images/brcmlogo_line.png \
                      images/favicon.ico \
                      scripts/general_ajax_script.js \
                      scripts/wpad.dat

GLOBAL_DEFINES := USE_SELF_SIGNED_TLS_CERT \
                  MAC_ADDRESS_SET_BY_HOST

APPLICATION_DCT := temp_control_dct.c
WIFI_CONFIG_DCT_H := wifi_config_dct.h