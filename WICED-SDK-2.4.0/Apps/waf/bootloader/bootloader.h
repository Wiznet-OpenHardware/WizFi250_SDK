/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_WICED_BOOTLOADER_H_
#define INCLUDED_WICED_BOOTLOADER_H_

#include <stdint.h>
#include "platform_dct.h"
#include "platform_bootloader.h"

typedef struct bootloader_app_header_struct
{
    uint32_t offset_to_vector_table;  /* (app start) */
    uint32_t size_of_app;             /* including this header */
    uint32_t address_of_wlan_firmware;
    uint32_t size_of_wlan_firmware;
    const char* app_name;
    const char* app_version;
    uint32_t reserved[26];  /* total size = 64 bytes */
} bootloader_app_header_t;

/*
typedef struct update_history_elem_struct
{

} update_history_elem_t;
*/

#endif /* ifndef INCLUDED_WICED_BOOTLOADER_H_ */
