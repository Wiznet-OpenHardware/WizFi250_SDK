#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_mfg_test

$(NAME)_SOURCES := src/wl/exe/wlu_server_shared.c \
                   src/wl/exe/wlu_server_wiced.c \
                   src/wl/exe/wlu_pipe.c \
                   mfg_test_init.c

$(NAME)_SOURCES += $(CHIP)_mfg_test_wifi_image.c
NO_WIFI_FIRMWARE := YES

$(NAME)_INCLUDES := ./src/include
$(NAME)_DEFINES  := RWL_SERIAL TARGET_wiced

GLOBAL_DEFINES := STDIO_BUFFER_SIZE=1024 WICED_PAYLOAD_MTU=1536
#AVOID_GLOMMING_IOVAR AVOID_APSTA SET_COUNTRY_WITH_IOCTL_NOT_IOVAR
