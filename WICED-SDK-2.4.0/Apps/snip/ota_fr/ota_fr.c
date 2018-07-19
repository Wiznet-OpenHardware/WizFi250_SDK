/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 * Over-The-Air Upgrade & Factory Reset Application
 *
 * ------------------------------------------------------
 * PLEASE read the WICED Application Framework overview
 * document located in the WICED SDK Doc directory before
 * trying this application!
 * ------------------------------------------------------
 *
 * This application demonstrates how to use the WICED build system with a WICED development
 * board to demonstrate Over-the-Air (OTA) upgrade and factory reset capability.
 *
 * Features demonstrated
 *  - Serial flash programming (of Factory Reset Image)
 *  - WICED Over-the-Air Upgrade
 *  - Factory Reset process
 *
 * Component Overview
 *   The following text provides a brief description of each of the WICED Application
 *   Framework components used to enable the factory configuration of the serial flash,
 *   OTA upgrade and the factory reset process. Each of these components is depicted
 *   and discussed in the WICED Application Framework overview document.
 *   * SPI Flash Write Application
 *      - This app is used to program a factory reset image into the SPI serial flash
 *        The factory reset image includes a factory reset application, DCT and OTA application
 *   * Production Application
 *      - Main application running on the MCU
 *      - Stored in internal MCU flash
 *      - Overwritten during OTA upgrade (or factory reset)
 *      - The production application = the factory reset application (at the time of manufacture)
 *   * OTA Application
 *      - Used to upgrade the production application with the help of an external Wi-Fi client
 *      - Stored in external serial flash
 *      - Copied to MCU RAM and executed from RAM during the OTA upgrade process
 *   * Factory Reset Application
 *      - Identical to the production application at the time of manufacture
 *      - Stored in external serial flash
 *      - Copied to MCU flash during factory reset (overwrites production application)
 *   * Bootloader Application
 *      - Runs every time the MCU boots
 *      - Manages the OTA upgrade & factory reset process
 *
 * Prepare the WICED Evaluation Board for OTA Upgrade
 *   The steps below describe how to:
 *     1. Build a Factory Reset image and program the image into external serial Flash
 *     2. Build an application that can be used to wirelessly upgrade the WICED devboard
 *     3. Download the production application (same as factory reset app) that kicks
 *        off the OTA upgrade process
 *
 *   Connect the WICED evaluation board to your computer, then open a terminal to view UART prints
 *   The following steps assume you have a BCM943362WCD4 WICED module on a WICED eval board.
 *   If your platform is different, substitute BCM943362WCD4 for your platform name
 *   1. The factory reset image contains the DCT, the OTA App and the Factory Reset App
 *      (the ota_fr app, since the Factory Reset app == the Production app).
 *      Build and download the Factory Reset Image to the SPI serial flash using the
 *      following target:
 *        snip.ota_fr-BCM943362WCD4  OTA=waf.ota_upgrade  SFLASH=app-dct-ota-download
 *   2. Build the scan app (the scan app will be used to wirelessly upgrade the devboard)
 *      using the following target:
 *        snip.scan-BCM943362WCD4
 *   3. Download and run the ota_fr (production) application to start the OTA process
 *      using the following target:
 *        snip.ota_fr-BCM943362WCD4 download run
 *
 * Upgrade the WICED Devboard
 *   After carefully completing the above steps, the WICED devboard is ready for OTA upgrade.
 *   (Check the app printed 'Loading OTA upgrade app' to the terminal after it booted)
 *   Work through the following steps to upgrade the WICED devboard:
 *   - Using the Wi-Fi connection manager on your computer, search for, and connect to,
 *     the Wi-Fi AP called : WICED_OTA_XXXXXXXXXXXX
 *   - Open a web browser and enter the URL: wiced.com
 *   - After a short period, the WICED Webserver OTA Upgrade webpage appears
 *   - Click 'Choose File' and navigate to the file
 *     WICED-SDK/build/snip_scan-BCM943362WCD4/Binary/snip_scan-BCM943362WCD4.bin
 *     (this is the scan app binary that was created in Step 2 above)
 *   - Click 'Open' (the dialogue box disappears)
 *   - Click 'Upgrade' to begin the upgrade process, the webpage.
 *      - The D1 LED on the WICED devboard flashes intermittently during the upgrade
 *      - The webpage prints 'OTA Upgrade complete, device rebooting ...' when the
 *        process completes
 *   - With the upgrade complete, the scan app runs and Wi-Fi scan results are regularly
 *     printed to the terminal
 *
 * Factory Reset the WICED Devboard
 *   To return the WICED devboard to factory reset, work through the following steps.
 *   - Push and hold the SW1 button THEN momentarily press and release the Reset button.
 *     The D1 LED flashes quickly to indicate factory reset will occur *IF* SW1 is held
 *     for a further 5 seconds. Continue to hold SW1.
 *   - When the D1 LED stops flashing and remains permanently ON, release SW1. The
 *     bootloader copies the factory reset image from external flash into the MCU flash.
 *     This process takes about 8 seconds.
 *   - After the copy process is complete, the WICED devboard reboots and runs the factory
 *     reset (ota_fr) application. Look at the terminal to confirm factory reset completed
 *     successfully.
 *
 */

#include "wiced.h"
#include "wwd_debug.h"
#include "bootloader_app.h"

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
	uint32_t i;

	WPRINT_APP_INFO( ( "Hi, I'm the Production App (ota_fr).\r\nI live in MCU flash.\r\n\r\n" ) );
	WPRINT_APP_INFO( ( "Watch while I toggle some LEDs ...\r\n" ) );

	for (i=0; i<15; i++)
	{
		wiced_gpio_output_high( WICED_LED1 );
		wiced_gpio_output_low( WICED_LED2 );
		wiced_rtos_delay_milliseconds( 300 );
		wiced_gpio_output_low( WICED_LED1 );
		wiced_gpio_output_high( WICED_LED2 );
		wiced_rtos_delay_milliseconds( 300 );
	}

	WPRINT_APP_INFO( ( "Time for an upgrade. Goodbye cruel world!\r\n\r\n\n" ) );
	wiced_rtos_delay_milliseconds( 1000 );

	WPRINT_APP_INFO( ( "OTA upgrade started ...\r\n" ) );
    wiced_start_ota_upgrade();
}



