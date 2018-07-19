/*
 * $Copyright 2012, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.$
 */

/** @file
 *
 * iperf Application
 *
 */

#include "wiced.h"

#include "wizfimain/wx_defines.h"
#include "wizfimain/wx_s2w_process.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern void app_main( void );
static wiced_result_t kick_watchdog( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_timed_event_t watchdog_kick_event;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    /* Initialise the device */
#if 1 // kaizen 130411 ID1006 - Adjust USART information
	WXS2w_LoadConfiguration();

	// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
	extern uint32_t g_custom_gpio;
	g_custom_gpio = g_wxProfile.custom_gpio;

	USART_Cmd( USART1, DISABLE );
	USART_Init( USART1, &g_wxProfile.usart_init_structure );
	USART_Cmd( USART1, ENABLE );
#endif

	// sekim 20131125 from [g_wxProfile member] to [common source]
	{
		extern uint8_t platfrom_spi_stdio;
		extern uint32_t g_socket_extx_option1;
		platfrom_spi_stdio = g_wxProfile.spi_stdio;
		g_socket_extx_option1 = g_wxProfile.socket_ext_option1;
	}

	/////////////////////////////////////////////////////////////////////////////
	// sekim 20131125 Add SPI Interface
	/*
	if ( g_wxProfile.spi_stdio==ENABLE )
	{
		wiced_spi_device_t spi = {WICED_SPI_1, WIZFI_SPI_CS, 30000000, SPI_NO_DMA, 8};
		spi.mode |= g_wxProfile.spi_mode;
		wiced_spi_init(&spi);
	}
	*/
	wiced_spi_device_t spi = {WICED_SPI_1, WIZFI_SPI_CS, 30000000, SPI_NO_DMA, 8};
	spi.mode |= g_wxProfile.spi_mode;
	wiced_spi_init(&spi);

	// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
	if(g_custom_gpio == CST_OPEN_DRAIN )
	{
		wiced_gpio_init( WIZFI_SPI_REQ, OUTPUT_OPEN_DRAIN_NO_PULL );
		wiced_gpio_output_low( WIZFI_SPI_REQ );
	}
	else
	{
		// sekim 20131125 Add SPI Interface
		wiced_gpio_init( WIZFI_SPI_REQ, OUTPUT_PUSH_PULL );
		wiced_gpio_output_low( WIZFI_SPI_REQ );
	}

	/////////////////////////////////////////////////////////////////////////////

    wiced_init( );

    /////////////////////////////////////////////////////////////////////////////
    // sekim 20130403 Add Task Monitor, WXS2w_SerialInput
    int a;
    wiced_register_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);
    for (a = 0; a < DEFAULT_NUMBER_OF_SYSTEM_MONITORS; ++a)
    {
        if (system_monitors[a] != NULL)	wizfi_task_monitor_index = a;
	}
    /////////////////////////////////////////////////////////////////////////////


    /* Assign watchdog kick event to the high priority worker thread */
    wiced_rtos_register_timed_event( &watchdog_kick_event, WICED_HARDWARE_IO_WORKER_THREAD, &kick_watchdog, 1000, 0 );

    /* Enter console main loop */
    app_main();
}

/*
 * Kicks watchdog
 */
static wiced_result_t kick_watchdog( void* arg )
{
    wiced_watchdog_kick( );
    return WICED_SUCCESS;
}

uint32_t host_get_time( void )
{
    return host_rtos_get_time();
}


