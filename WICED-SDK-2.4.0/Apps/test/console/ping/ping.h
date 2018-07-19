/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#ifndef INCLUDED_PING_H_
#define INCLUDED_PING_H_

extern int ping( int argc, char* argv[] );

#define PING_COMMANDS \
    { (char*) "ping",         ping,       0, DELIMIT, NULL, (char*) "<destination> [-i <interval in ms>] [-n <number>] [-l <length>]", (char*) "Pings the specified IP or Host."},


#endif /* ifndef INCLUDED_PING_H_ */
