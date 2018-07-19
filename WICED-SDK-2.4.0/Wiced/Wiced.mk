#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = Wiced


#CERTIFICATE := ../Resources/config/device.cer
#PRIVATE_KEY := ../Resources/config/id_rsa

# kaizen 20130520 ID1029 - Added WIZnet Certificate
CERTIFICATE := ../Resources/config/wiznet_ssl_server.crt
PRIVATE_KEY := ../Resources/config/wiznet_ssl_server.key

# kaizen 20140905 Added Coway Certificate
#CERTIFICATE := ../Resources/config/coway_ssl_server.crt
#PRIVATE_KEY := ../Resources/config/coway_ssl_server.key


$(NAME)_SOURCES := internal/malloc_debug.c

# Check if the WICED API is being used
ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))

$(NAME)_SOURCES += internal/wifi.c \
                   internal/config.c \
                   internal/config_http_content.c \
                   internal/time.c \
                   internal/wiced_lib.c \
                   internal/management.c \
                   internal/system_monitor.c \
                   internal/wiced_cooee.c \
                   internal/wiced_easy_setup.c

$(NAME)_RESOURCES := images/favicon.ico \
                     config/device_settings.html \
                     images/scan_icon.png \
                     images/wps_icon.png \
                     images/wiznet_logo.png \
                     images/s2web_logo.png \
                     images/encored_logo.png \
                     images/dummy_logo.png \
                     images/over_the_air_logo.png \
                     images/gpio_icon.png \
                     images/serial_setting_icon.png \
                     images/user_information_icon.png \
                     scripts/general_ajax_script.js \
                     scripts/wpad.dat \
                     config/join.html \
                     config/scan_page_outer.html \
                     config/scan_results.html \
                     config/redirect.html \
                     styles/buttons.css \
                     s2web/wps_pbc.html \
                     s2web/wps_pin.html \
                     s2web/wizfi_ota.html \
                     s2web/demo.html \
                     s2web/table.html \
                     s2web/conn_setting.html \
                     s2web/conn_setting_wps.html \
                     s2web/conn_setting_response.html \
                     s2web/conn_setting_apmode.html \
                     s2web/s2w_main.html \
                     s2web/gpio_control_main.html \
                     s2web/gpio_control_set.html \
                     s2web/gpio_control_get.html \
                     s2web/serial_info_setting.html \
                     s2web/set_wifi_key.html \
                     s2web/change_user_info.html \
                     s2web/index.html \
                     s2web/conn_setting_station.html \
					 customWeb/ShinHeung/image/ShinHeungLogo.jpg \
					 customWeb/ShinHeung/conn_setting_apmode_SH.html \
					 customWeb/ShinHeung/conn_setting_response_SH.html \
					 customWeb/ShinHeung/conn_setting_SH.html \
					 customWeb/ShinHeung/conn_setting_station_SH.html \
					 customWeb/ShinHeung/main_SH.html \
					 customWeb/ShinHeung/ota_SH.html \
					 customWeb/ShinHeung/scan_page_outer_SH.html \
					 customWeb/ShinHeung/scan_results_SH.html \
					 customWeb/ShinHeung/user_info_SH.html \
					 customWeb/ShinHeung/set_wifi_key_SH.html \
                     styles/border_radius.htc

$(NAME)_INCLUDES := Security/besl/tlv \
                    Security/besl/crypto \
                    Security/besl/include

ifeq (NetX,$(NETWORK))
$(NAME)_COMPONENTS += Wiced/Security/besl
$(NAME)_COMPONENTS += daemons/http_server
$(NAME)_COMPONENTS += daemons/dns_redirect
$(NAME)_COMPONENTS += protocols/dns
GLOBAL_DEFINES += ADD_NETX_EAPOL_SUPPORT USE_MICRORNG
endif

ifeq (NetX_Duo,$(NETWORK))
$(NAME)_COMPONENTS += Wiced/Security/besl
$(NAME)_COMPONENTS += daemons/http_server
$(NAME)_COMPONENTS += daemons/dns_redirect
$(NAME)_COMPONENTS += protocols/dns
GLOBAL_DEFINES += ADD_NETX_EAPOL_SUPPORT USE_MICRORNG
endif

ifeq (LwIP,$(NETWORK))
$(NAME)_COMPONENTS += Wiced/Security/besl
$(NAME)_COMPONENTS += daemons/http_server
$(NAME)_COMPONENTS += daemons/dns_redirect
$(NAME)_COMPONENTS += protocols/dns
endif

endif

# Add standard WICED 1.x components
$(NAME)_COMPONENTS += Wiced/WWD

# Define the default ThreadX and FreeRTOS starting stack sizes
FreeRTOS_START_STACK := 800
ThreadX_START_STACK  := 800

GLOBAL_DEFINES += WICED_STARTUP_DELAY=10 \
                  BOOTLOADER_MAGIC_NUMBER=0x4d435242

GLOBAL_INCLUDES := .
