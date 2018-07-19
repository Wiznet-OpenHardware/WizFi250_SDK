/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 *
 * WICED Bootloader
 *
 * The bootloader boots applications and provides a low level
 * API for apps
 *
 * Note:
 *   The WICED IDE has been pre-configured to enable debugging
 *   into the bootloader. Instructions for setting up bootloader
 *   debugging (if required) are provided in Appendix C of
 *   the WICED SDK Quickstart Guide
 *
 */
#include <string.h>
#include <stddef.h>
#include "platform_bootloader.h"
#include "wwd_rtos.h"
#include "RTOS/wwd_rtos_interface.h"
#include "spi_flash.h"
#include "bootloader.h"
#include "platform_dct.h"
#if 1 //MikeJ
#include "watchdog.h"
#include "ioutil.h"
#include "flash_if.h"
#include "ymodem.h"
#endif //MikeJ

#if 1	// kaizen 20130521 ID1071 For improving firmware update function using serial.
#include "wiced_rtos.h"
#endif

#if 1
#include "wiced_platform.h"
#include "platform_common_config.h"
#endif
#define OFFSET(type, member)                          ((uint32_t)&((type *)0)->member)

typedef struct
{
    platform_dct_data_t platform_dct;

/*  TODO: add these options:
    update history
    Features Enabled
*/
} bootloader_dct_data_t;

/* GLOBALS NOT ALLOWED in API functions - These could be called by app which has not allocated space */

/* TODO: Disable interrupts in bootloader API, to avoid reentrancy problems? - may not need to due to APIs not being able to use globals */


static int                         set_boot_status( boot_status_t status );
static void                        perform_factory_reset( void );
static void                        start_ota_upgrade( void );
static int                         init_dct_mfg_info( platform_dct_mfg_info_t* data_in );
static int                         write_app_config_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write );
static int                         write_wifi_config_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write );
static int                         write_certificate_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write );
static void*                       get_app_config_dct( void );
static platform_dct_wifi_config_t* get_wifi_config_dct( void );
static platform_dct_mfg_info_t*    get_mfg_info_dct( void );
static platform_dct_security_t*    get_security_credentials_dct( void );
static wiced_config_ap_entry_t*    get_ap_list_dct( uint8_t index );
static int                         write_ap_list_dct( uint8_t index, wiced_config_ap_entry_t* ap_details );
static wiced_config_soft_ap_t*     get_soft_ap_dct( void );
static wiced_config_soft_ap_t*     get_config_ap_dct( void );


#if defined ( __IAR_SYSTEMS_ICC__ )
/* IAR added stuff, create a special section to place bootloader api */
/* structure */
#pragma section = "bootloader_api_section"
const bootloader_api_t bootloader_api @ "bootloader_api_section";
#endif /* #if defined ( __IAR_SYSTEMS_ICC__ ) */
const bootloader_api_t bootloader_api =
{
    /* General bootloader functions */
    .set_boot_status             = set_boot_status,
    .perform_factory_reset       = perform_factory_reset,
    .start_ota_upgrade           = start_ota_upgrade,
    .init_dct_mfg_info           = init_dct_mfg_info,
    .write_app_config_dct        = write_app_config_dct,
    .write_wifi_config_dct       = write_wifi_config_dct,
    .write_certificate_dct       = write_certificate_dct,
    .get_app_config_dct          = get_app_config_dct,
    .get_wifi_config_dct         = get_wifi_config_dct,
    .get_mfg_info_dct            = get_mfg_info_dct,
    .get_security_credentials_dct= get_security_credentials_dct,
    .get_ap_list_dct             = get_ap_list_dct,
    .write_ap_list_dct           = write_ap_list_dct,
    .get_soft_ap_dct             = get_soft_ap_dct,
    .get_config_ap_dct           = get_config_ap_dct,
    /* Platform specific bootloader functions */
    .platform_kick_watchdog      = platform_kick_watchdog,
    .platform_reboot             = platform_reboot,
    .platform_erase_app          = platform_erase_app,
    .platform_write_app_chunk    = platform_write_app_chunk,
    .platform_set_app_valid_bit  = platform_set_app_valid_bit,
    .platform_read_wifi_firmware = platform_read_wifi_firmware,
    .platform_start_app          = platform_start_app
#if 1 //MikeJ
	, .platform_verification     = platform_verification
#endif
};

int main( void )
{

#if defined ( __IAR_SYSTEMS_ICC__ )
/* IAR allows init functions in __low_level_init(), but it is run before global
 * variables have been initialised, so the following init still needs to be done
 * When using GCC, this is done in crt0_GCC.c
 */
	extern void init_platform( void );
	extern void init_architecture( void );

    init_architecture( );
    init_platform( );
#endif /* #elif defined ( __IAR_SYSTEMS_ICC__ ) */

#define BOOT_BUTTON_WAIT_TIME 3000	//MikeJ
    /* Check if DCT has been set to indicate a particular function should be called */
    if ( platform_get_dct( )->dct_header.load_app_func == platform_restore_factory_app )
    {
        platform_restore_factory_app();
    }

    NoOS_setup_timing( );

    /* Check if Factory reset button is being held */

    uint32_t factory_reset_counter = 0;
    uint8_t led_state = 0;
    while ( ( 1 == platform_get_bootloader_button( ) ) &&
#if 0 //MikeJ
            ( ( factory_reset_counter += 100 ) <= 5000 ) &&
#else
            ( ( factory_reset_counter += 100 ) <= BOOT_BUTTON_WAIT_TIME ) &&
#endif
            ( WICED_SUCCESS == host_rtos_delay_milliseconds( 100 ) ) )
    {
        /* Factory reset button is being pressed. */
        /* User Must press it for 5 seconds to ensure it was not accidental */
        /* Toggle LED every 100ms */

        if ( led_state == 0 )
        {
            platform_set_bootloader_led( 1 );
            led_state = 1;
        }
        else
        {
            platform_set_bootloader_led( 0 );
            led_state = 0;
        }
    }

#if 0 //MikeJ
    if ( ( platform_get_dct( )->dct_header.app_valid != APP_VALID ) ||
         ( factory_reset_counter > 5000 ) )
    {
        perform_factory_reset( );  /* Function does not return */
#else
    if ( ( platform_get_dct( )->dct_header.app_valid != APP_VALID ) ||
         ( factory_reset_counter > BOOT_BUTTON_WAIT_TIME ) )
    {
		#define KEY_WAIT_TIME 15000
		uint8_t key = 0;

#if 1	// kaizen 20130529 ID1074 - Added restore factory image function using gpio3
		uint32_t restore_factory_img_btn_wait_time = 0;
		while( restore_factory_img_btn_wait_time < 1000 )
		{
			if( wiced_gpio_input_get(WICED_GPIO_3) == (wiced_bool_t)BOOTLOADER_BUTTON_PRESSED_STATE )
				perform_factory_reset( );  /* Function does not return */

			host_rtos_delay_milliseconds(100);
			restore_factory_img_btn_wait_time += 100;
		}
#endif

		Serial_Init();
		SerialPutString("\r\n\r\n\r\n");
		SerialPutString("=====================\r\n");
		SerialPutString("WizFi Bootloader     \r\n");
		SerialPutString("=====================\r\n");
		SerialPutString("1) Upload APP by USART\r\n");
		SerialPutString("2) Upload DCT by USART\r\n");
		SerialPutString("3) Factory Recover\r\n");
		SerialPutString("*) Exit\r\n\r\n");

#if 1	// kaizen 20130521 ID1071 For improving firmware update function using serial.
		while(!SerialKeyPressed(&key))
		{
			SerialPutString("C");
			watchdog_kick();
			host_rtos_delay_milliseconds( 100 );
		}
#else
		while(!SerialKeyPressed(&key)) watchdog_kick( );
#endif


#if 1	// kaizen 20131213 ID1155 Added Function in order to upload firmware both DCT and APP
		if(key == '0' ){
			SerialPutString("[USART DCT APP ALL Uploader]\r\n\r\n");
			SerialPutString("[USART DCT Uploader]\r\n\r\n");
			SerialDownload(ADDR_FLASH_SECTOR_2, ADDR_FLASH_SECTOR_2);

			SerialPutString("[USART APP Uploader]\r\n\r\n");
			SerialDownload(ADDR_FLASH_SECTOR_4, ADDR_FLASH_SECTOR_11);
		}
		else if(key == '1') {
			SerialPutString("[USART APP Uploader]\r\n\r\n");
			SerialDownload(ADDR_FLASH_SECTOR_4, ADDR_FLASH_SECTOR_11);
		} else if(key == '2') {
			SerialPutString("[USART DCT Uploader]\r\n\r\n");
			SerialDownload(ADDR_FLASH_SECTOR_2, ADDR_FLASH_SECTOR_2);
		} else if(key == '3') {
			SerialPutString("Factory Reset triggered ...\r\n");
			Serial_Deinit();
			perform_factory_reset( );  /* Function does not return */
		}
#else
		if(key == '1') {
			SerialPutString("[USART APP Uploader]\r\n\r\n"); // just misprint
			SerialDownload(ADDR_FLASH_SECTOR_4, ADDR_FLASH_SECTOR_11);
		} else if(key == '2') {
			SerialPutString("[USART DCT Uploader]\r\n\r\n");
			SerialDownload(ADDR_FLASH_SECTOR_2, ADDR_FLASH_SECTOR_2);
		} else if(key == '3') {
			SerialPutString("Factory Reset triggered ...\r\n");
			Serial_Deinit();
			perform_factory_reset( );  /* Function does not return */
		}
#endif
		SerialPutString("\r\n>>> Start APP >>>>>>>>>>>>>>>>\r\n\r\n");
		Serial_Deinit();
#endif
    }
    else
    {
        /* User released the button before 5 seconds were up */
        /* Start normal operation */
    	// sekim 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
        //platform_set_bootloader_led( 0 );
    }

    /* Start the app! */

    NoOS_stop_timing( );


    /* Check if DCT has been set to indicate a particular function should be called */
    dct_load_app_func_t load_app_func = platform_get_dct( )->dct_header.load_app_func;
    if ( load_app_func != NULL )
    {
        platform_write_dct( 0, NULL, 0, -1, NULL ); /* clear function call so it only happens this once */
        load_app_func();  /* Function does not return */
    }

    platform_start_app( 0 ); /* Function does not return */
    return 0;
}

static void start_ota_upgrade( void )
{
    platform_write_dct( 0, NULL, 0, -1, platform_load_ota_app );
    platform_reboot();
}

static void perform_factory_reset( void )
{
    platform_write_dct( 0, NULL, 0, -1, platform_restore_factory_app );
    platform_reboot();
}


/* TODO: Error checking */
static int  set_boot_status( boot_status_t status )
{
    /* TODO: Implement */
    return 0;
}

static int write_app_config_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write )
{
    return platform_write_dct( (sizeof(platform_dct_data_t) - sizeof(platform_dct_header_t) + data_start_offset + 3) & (~3), data_in_addr, num_bytes_to_write, -1, NULL );
}

static int write_wifi_config_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write )
{
    return platform_write_dct( OFFSET(platform_dct_data_t, wifi_config) - sizeof(platform_dct_header_t) + data_start_offset, data_in_addr, num_bytes_to_write, -1, NULL );
}


static int write_certificate_dct( unsigned long data_start_offset, void* data_in_addr,  unsigned long num_bytes_to_write )
{
#if 1 //MikeJ 130416 ID1049 - Wrong Cert. writing operation
	return platform_write_dct( OFFSET(platform_dct_data_t, security_credentials) - sizeof(platform_dct_header_t) + data_start_offset, data_in_addr, num_bytes_to_write, -1, NULL );
#else
    return platform_write_dct( OFFSET(platform_dct_data_t, security_credentials) + OFFSET(platform_dct_security_t, certificate)- sizeof(platform_dct_header_t) + data_start_offset, data_in_addr, num_bytes_to_write, -1, NULL );
#endif
}

static void* get_app_config_dct( void )
{
    uint32_t ret = (uint32_t)platform_get_dct( ) + sizeof(bootloader_dct_data_t);
    ret = (ret + 3) & (~3);
    return (void*)ret;
}

static platform_dct_wifi_config_t* get_wifi_config_dct( void )
{
    return ( &platform_get_dct( )->wifi_config );
}

static platform_dct_security_t* get_security_credentials_dct( void )
{
    return ( &platform_get_dct( )->security_credentials );
}

static wiced_config_ap_entry_t* get_ap_list_dct( uint8_t index )
{
    return ( &platform_get_dct( )->wifi_config.stored_ap_list[index] );
}

static int write_ap_list_dct( uint8_t index, wiced_config_ap_entry_t* ap_details )
{
    return platform_write_dct( OFFSET(platform_dct_data_t, wifi_config) + OFFSET(platform_dct_wifi_config_t, stored_ap_list) - sizeof(platform_dct_header_t)  + index * sizeof(wiced_config_ap_entry_t) , ap_details, sizeof(wiced_config_ap_entry_t), -1, NULL );
}

static wiced_config_soft_ap_t* get_config_ap_dct( void )
{
    return ( &platform_get_dct( )->wifi_config.config_ap_settings );
}

static wiced_config_soft_ap_t* get_soft_ap_dct( void )
{
    return ( &platform_get_dct( )->wifi_config.soft_ap_settings );
}

static platform_dct_mfg_info_t* get_mfg_info_dct( void )
{
    return ( &platform_get_dct( )->mfg_info );
}

static int init_dct_mfg_info( platform_dct_mfg_info_t* data_in )
{
    platform_dct_header_t* dct = &platform_get_dct( )->dct_header;

    if ( dct->mfg_info_programmed == 1 )
    {
        /* Manufacture details already written. Only re-download of bootloader will clear this */
        return -1;
    }

    platform_write_flash_chunk( ((uint32_t)dct) + offsetof( bootloader_dct_data_t, platform_dct.mfg_info ), (uint8_t*) &data_in, sizeof(platform_dct_mfg_info_t) );

    return 0;
}

