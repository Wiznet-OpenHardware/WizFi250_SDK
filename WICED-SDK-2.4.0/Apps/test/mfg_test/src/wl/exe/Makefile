#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#
# GNUmakefile for wl/exe
#


UNAME = $(shell uname)

CC:=gcc

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
HOST:=WIN32
CFLAGS += -I/include
ARCH_EXE_SFX:=.exe
ARCH_DLL_SFX:=.dll
SO_LIB_FLAG = $(WLM_SO)
else # UNAME
ifeq ($(findstring MINGW,$(UNAME)),MINGW)
CFLAGS += -I/mingw/include
HOST:=WIN32
ARCH_EXE_SFX:=.exe
ARCH_DLL_SFX:=.dll
SO_LIB_FLAG = $(WLM_SO)
else # UNAME
HOST:=LINUX
ARCH_EXE_SFX:=
ARCH_DLL_SFX:=.so
SO_LIB_FLAG = -l$(WLM_SO_RAW)
endif 
endif

#CFLAGS += -DDEBUG_SERIAL

CFLAGS += -D$(HOST)
CFLAGS += -DBAND5G -DBAND2G
CFLAGS += -DBCMINTERNAL -DBCMDBG
#CFLAGS += -DBCMCCX
CFLAGS += -DBCMWAPI_WPI
CFLAGS += -DBCMWPA2
CFLAGS += -DWLCNT
CFLAGS += -DWLTEST
#CFLAGS += -DWLBTAMP
#CFLAGS += -DWLP2P
#CFLAGS += -Wextra $(CUSTOM_FLAGS)
CFLAGS += -DWLPFN -DWLPFN_AUTO_CONNECT
CFLAGS += -I../../include
CFLAGS += -I../sys
CFLAGS += -g -O0

ifeq ($(findstring WIN32,$(HOST)),WIN32)
WL_OBJS         := wlu_serial_only.o wlu_pipe_win32.o #wlu_vista.o 
else
WL_OBJS         := wlu_linux.o wlu_pipe_linux.o
endif

WL_OBJS         += wlu.o wlu_cmd.o wlu_iov.o wlu_client_shared.o  wlu_pipe.o # wlu_common.o 
WL_SHARED_OBJS  := bcmwifi.o miniopt.o bcmutils.o bcm_app_utils.o
WLM_OBJS        := wlm.o
WLM_EX_APP_OBJS := wlm_example_app.o

# Prefix obj/<type>/TARGETARCH to produced .obj files
WL_OBJS         := $(WL_OBJS:%.o=obj/wl/%.o)
WL_SHARED_OBJS  := $(WL_SHARED_OBJS:%.o=obj/wl_shared/%.o)
WLM_OBJS        := $(WLM_OBJS:%.o=obj/wlm/%.o)
WLM_EX_APP_OBJS := $(WLM_EX_APP_OBJS:%.o=obj/wlm_example_app/%.o)

# TODO: Move final built objects to respective TARGETARCH dirs as well
# Final exe names
WL_EXE          := wl$(ARCH_EXE_SFX)
WLM_SO_RAW      := wiced_wlm
WLM_SO          := $(WLM_SO_RAW)$(ARCH_DLL_SFX)
WLM_EXAMPLE_EXE := wiced_wlm_example_app$(ARCH_EXE_SFX)



all: wl wlm

wl:  $(WL_EXE)

wlm: $(WLM_SO) $(WLM_EXAMPLE_EXE)



# Compilation targets
obj/wl/%.o: %.c
	@mkdir -pv $(@D)
	$(CC) -c $(CFLAGS) -DRWL_SERIAL -o $@ $^

obj/wl_shared/%.o: ../../shared/%.c
	@mkdir -pv $(@D)
	$(CC) -c $(CFLAGS) -DRWL_SERIAL -o $@ $^


	
#-DRWL_SOCKET -DRWL_DONGLE -DRWL_WIFI 

obj/wlm/%.o: %.c
	@mkdir -pv $(@D)
	$(CC) -c $(CFLAGS) -DRWL_SERIAL -DWLMSO -o $@ $^


obj/wlm_example_app/%.o: %.c
	@mkdir -pv $(@D)
	$(CC) -c $(CFLAGS) -DRWL_SERIAL -o $@ $^

#-DRWL_DONGLE -DRWL_SOCKET -DRWL_WIFI 

# Final link targets
$(WL_EXE): $(WL_OBJS) $(WL_SHARED_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ 

$(WLM_SO): $(WLM_OBJS) $(WL_OBJS) $(WL_SHARED_OBJS)
	@mkdir -pv $(@D)
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(WLM_EXAMPLE_EXE): $(WLM_SO) $(WLM_EX_APP_OBJS)
	@mkdir -pv $(@D)
	$(CC) $(LDFLAGS) -o $@ $(SO_LIB_FLAG) $(WLM_EX_APP_OBJS)



.PHONY: all clean wlm

clean:
	 rm -fv $(WL_EXE) $(WL_OBJS) $(WL_SHARED_OBJS) $(WLM_SO) $(WLM_OBJS) $(WLM_EX_APP_OBJS) $(WLM_EXAMPLE_EXE)

