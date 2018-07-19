/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_MALLINFO_H_
#define INCLUDED_MALLINFO_H_

#ifdef CONSOLE_ENABLE_MALLINFO

extern int malloc_info_command( int argc, char* argv[] );

#define MALLINFO_COMMANDS \
    { "malloc_info",         malloc_info_command,       0, DELIMIT, NULL, NULL, "Print memory allocation information"},

#else /* ifdef CONSOLE_ENABLE_MALLINFO */
#define MALLINFO_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_MALLINFO */

#endif /* ifndef INCLUDED_MALLINFO_H_ */
