/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	5

#define	EPI_MINOR_VERSION	90

#define	EPI_RC_NUMBER		153

#define	EPI_INCREMENTAL_NUMBER	39

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		5, 90, 153, 39

#define	EPI_VERSION_NUM		0x055a9927

#define EPI_VERSION_DEV		5.90.153

/* Driver Version String, ASCII, 32 chars max */
#ifdef BCMINTERNAL
#define	EPI_VERSION_STR		"5.90.153.39 (BCM INTERNAL)"
#else
#ifdef WLTEST
#define	EPI_VERSION_STR		"5.90.153.39 (WLTEST)"
#else
#define	EPI_VERSION_STR		"5.90.153.39"
#endif
#endif /* BCMINTERNAL */

#endif /* _epivers_h_ */
