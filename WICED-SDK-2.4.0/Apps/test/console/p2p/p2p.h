/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#ifndef INCLUDED_P2P_H_
#define INCLUDED_P2P_H_

#ifdef CONSOLE_ENABLE_P2P

extern int start_p2p( int argc, char* argv[] );
extern int stop_p2p( int argc, char* argv[] );
extern int p2p_peer_list( int argc, char* argv[] );
extern int p2p_invite( int argc, char* argv[] );

#define P2P_COMMANDS \
    { (char*) "start_p2p",  start_p2p, 0, DELIMIT, NULL, (char*) "", (char*) "Run P2P"}, \
    { (char*) "stop_p2p",   stop_p2p, 0,  DELIMIT, NULL, (char*) "", (char*) "Stop P2P"}, \
    { (char*) "p2p_peers",   p2p_peer_list, 0,  DELIMIT, NULL, (char*) "", (char*) "Print P2P peer list"}, \
    { (char*) "p2p_invite",   p2p_invite, 1,  DELIMIT, NULL, (char*) "<device id>", (char*) "Invite P2P peer"}, \


#else /* ifdef CONSOLE_ENABLE_P2P */
#define P2P_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_P2P */

#endif /* ifndef INCLUDED_P2P_H_ */
