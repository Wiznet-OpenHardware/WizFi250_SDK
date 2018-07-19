/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef   CONSOLE_CUSTOM_H_
#define   CONSOLE_CUSTOM_H_

#include "iperf/iperf.h"
#include "mallinfo/mallinfo.h"
#include "thread/thread.h"
#include "trace/console_trace.h"
#include "platform/platform.h"

#define CUSTOM_COMMANDS  \
    IPERF_COMMANDS \
    MALLINFO_COMMANDS \
    THREAD_COMMANDS \
    TRACE_COMMANDS \
    PLATFORM_COMMANDS \

#endif /* ifdef CONSOLE_CUSTOM_H_ */

