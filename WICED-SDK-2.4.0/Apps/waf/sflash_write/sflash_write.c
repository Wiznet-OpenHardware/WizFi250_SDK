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
 * SPI Flash Write Application
 *
 * This application is used by OpenOCD and the WICED build
 * system to write to serial flash. The app is loaded into
 * MCU RAM directly and once running, interacts with OpenOCD
 * to write consecutive chunks of data received via JTAG to
 * serial flash.
 *
 * The linker script places two of the structures
 * defined by the app at the beginning of RAM. These structure
 * variables contain all the details of the commands
 * to be executed and data to be written
 *
 */

#include <stdio.h>
#include <string.h>
#include "spi_flash.h"
#include "wwd_assert.h"
#include "watchdog.h"
#include "platform_sflash_dct.h"
#include "bootloader.h"				//MikeJ
#include "../bootloader/ioutil.h"	//MikeJ

//#define DEBUG_PRINT

/*
 * Commands to execute - bitwise OR together
 * TCL script write_sflash.tcl must match these defines
 */
#define MFG_SPI_FLASH_COMMAND_NONE                      (0x00000000)

#define MFG_SPI_FLASH_COMMAND_INITIAL_VERIFY            (0x00000001)
#define MFG_SPI_FLASH_COMMAND_ERASE_CHIP                (0x00000002)
#define MFG_SPI_FLASH_COMMAND_WRITE                     (0x00000004)
#define MFG_SPI_FLASH_COMMAND_POST_WRITE_VERIFY         (0x00000008)
#define MFG_SPI_FLASH_COMMAND_VERIFY_CHIP_ERASURE       (0x00000010)
#define MFG_SPI_FLASH_COMMAND_WRITE_DCT                 (0x00000020)
#if 1 //MikeJ
#define MFG_SPI_FLASH_COMMAND_WRITE_OTA					(0x00000040)
#define MFG_SPI_FLASH_COMMAND_READ						(0x00000080)
#define MFG_SPI_FLASH_COMMAND_BUFFER_CLEAR				(0x00000100)
#endif


/*
 * Result codes
 * TCL script write_sflash.tcl must match this enum
 */
typedef enum
{
    MFG_SPI_FLASH_RESULT_IN_PROGRESS               = 0xffffffff,
#if 0 //MikeJ
    MFG_SPI_FLASH_RESULT_OK                        = 0,
    MFG_SPI_FLASH_RESULT_ERASE_FAILED              = 1,
    MFG_SPI_FLASH_RESULT_VERIFY_AFTER_WRITE_FAILED = 2,
    MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_BUFFER       = 3,
    MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_CHIP         = 4,
    MFG_SPI_FLASH_RESULT_DCT_LOC_NOT_FOUND         = 5,
#else
    MFG_SPI_FLASH_RESULT_OK                        = 1,
    MFG_SPI_FLASH_RESULT_ERASE_FAILED              = 2,
    MFG_SPI_FLASH_RESULT_VERIFY_AFTER_WRITE_FAILED = 3,
    MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_BUFFER       = 4,
    MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_CHIP         = 5,
    MFG_SPI_FLASH_RESULT_DCT_LOC_NOT_FOUND         = 6,
#endif
    MFG_SPI_FLASH_RESULT_END                       = 0x7fffffff /* force to 32 bits */
} mfg_spi_flash_result_t;

#define WRITE_CHUNK_SIZE        (8*1024)  /* Writing in chunks is only needed to prevent reset by watchdog */

/*
 * TCL script write_sflash.tcl must match this structure
 */
typedef struct
{
	void *        entry_point;
	void *        stack_addr;
	unsigned long data_buffer_size;
} data_config_area_t;

/*
 * TCL script write_sflash.tcl must match this structure
 */

typedef struct
{
	unsigned long size;
	unsigned long dest_address;
	unsigned long command;
	unsigned long result;
	unsigned char data[__JTAG_FLASH_WRITER_DATA_BUFFER_SIZE__];
} data_transfer_area_t;

/* provided by link script */
#if defined( __ICCARM__ )
extern void wiced_program_start(void);
#pragma section="CSTACK"
extern void* link_stack_end = __section_end("CSTACK");
#else /* #if defined( __ICCARM__ ) */
void* reset_handler;
void* link_stack_end;
#endif /* #if defined( __ICCARM__ ) */


/******************************************************************************
 * This structure provides configuration parameters, and communication area
 * to the TCL OpenOCD script
 * It will be located at the start of RAM by the linker script
 *****************************************************************************/
#if defined(__ICCARM__)
/* IAR specific */
#pragma section= "data_config_section"
const data_config_area_t data_config @ "data_config_section";
const data_config_area_t   data_config =
{
        .entry_point = (void*)wiced_program_start,
        .stack_addr  = &link_stack_end,
        .data_buffer_size = __JTAG_FLASH_WRITER_DATA_BUFFER_SIZE__,
};
#else /* #if defined(__ICCARM__) */
const data_config_area_t   data_config =
{
        .entry_point = &reset_handler,
        .stack_addr  = &link_stack_end,
        .data_buffer_size = __JTAG_FLASH_WRITER_DATA_BUFFER_SIZE__,
};
#endif /* #if defined(__ICCARM__) */


/******************************************************************************
 * This structure provides a transfer area for communications with the
 * TCL OpenOCD script
 * It will be located immediately after the data_config structure at the
 * start of RAM by the linker script
 *****************************************************************************/
#if defined (__ICCARM__)
/* IAR specific */
#pragma section= "data_transfer_section"
data_transfer_area_t data_transfer @ "data_transfer_section";
#else /* #if defined (__ICCARM__) */
data_transfer_area_t data_transfer;
#endif /* #if defined (__ICCARM__) */

#ifdef DEBUG_PRINT
#define DEBUG_PRINTF(x) printf x
#else
#define DEBUG_PRINTF(x)
#endif /* ifdef DEBUG_PRINT */

/* A temporary buffer used for reading data from the Serial flash when performing verification */
static uint8_t   Rx_Buffer[4096];

int main( void )
{
#if 0 //MikeJ
    uint32_t pos;
    sflash_handle_t sflash_handle;
    unsigned long chip_size;
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

    /* loop forever */
    while (1)
    {
    	DEBUG_PRINTF(( "Waiting for command\r\n" ));
    	/* wait for a command to be written. */
    	while ( data_transfer.command == MFG_SPI_FLASH_COMMAND_NONE )
    	{
    		watchdog_kick( );
    	}

		data_transfer.result = MFG_SPI_FLASH_RESULT_IN_PROGRESS;

    	DEBUG_PRINTF(( "Received command: %s%s%s%s%s%s\r\n",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_INITIAL_VERIFY     )?   "INITIAL_VERIFY "       : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_ERASE_CHIP         )?   "ERASE_CHIP "           : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE              )?   "WRITE "                : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_POST_WRITE_VERIFY  )?   "POST_WRITE_VERIFY "    : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_VERIFY_CHIP_ERASURE)?   "VERIFY_CHIP_ERASURE "  : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE_DCT          )?   "WRTIE_DCT "            : ""
					));
    	DEBUG_PRINTF(( "Destination address: %lu\r\n", data_transfer.dest_address ));
    	DEBUG_PRINTF(( "Size: %lu\r\n", data_transfer.size ));

		/* Check the data size is sane - cannot be bigger than the data storage area */
		if ( data_transfer.size > __JTAG_FLASH_WRITER_DATA_BUFFER_SIZE__ )
		{
			data_transfer.result = MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_BUFFER;
			DEBUG_PRINTF(( "Size too big to for storage area - aborting!\r\n" ));
			goto back_to_idle;
		}

	   	DEBUG_PRINTF(( "Initialising\r\n" ));

		/* Initialise the serial flash driver */
		init_sflash( &sflash_handle, 0, SFLASH_WRITE_ALLOWED );

		DEBUG_PRINTF(( "Done initialising\r\n" ));

		if ( ( data_transfer.command &MFG_SPI_FLASH_COMMAND_WRITE_DCT ) != 0 )
		{
			DEBUG_PRINTF(( "Getting DCT location\r\n" ));
			/* User requested to write the DCT
			 * Read the Factory Reset App header to find where to begin writing
			 */
			if ( WICED_SUCCESS != platform_get_sflash_dct_loc( &sflash_handle, &data_transfer.dest_address ) )
			{
				data_transfer.result = MFG_SPI_FLASH_RESULT_DCT_LOC_NOT_FOUND;
				DEBUG_PRINTF(( "DCT location not found - aborting!\r\n" ));
				goto back_to_idle;
			}
		}

		/* Check the data will fit on the sflash chip */
		sflash_get_size( &sflash_handle, &chip_size );
		if ( data_transfer.size + data_transfer.dest_address  > chip_size )
		{
			data_transfer.result = MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_CHIP;
			DEBUG_PRINTF(( "Size too big to fit on chip - aborting!\r\n" ));
			goto back_to_idle;
		}
		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_INITIAL_VERIFY ) != 0 )
		{
			DEBUG_PRINTF(( "Verifying existing data!\r\n" ));

			/* Read data from SPI FLASH memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t read_size = ( data_transfer.size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : data_transfer.size - pos;
				sflash_read( &sflash_handle, pos + data_transfer.dest_address, Rx_Buffer, read_size );
				if ( 0 != memcmp( Rx_Buffer, &data_transfer.data[pos], read_size ) )
				{
					/* Existing data different */
					DEBUG_PRINTF(( "Existing data is different - stop verification\r\n" ));
					break;
				}
				pos += read_size;
				watchdog_kick( );
			}
			if ( pos >= data_transfer.size )
			{
				/* Existing data matches */
				/* No write required */
				data_transfer.result = MFG_SPI_FLASH_RESULT_OK;
				DEBUG_PRINTF(( "Existing data matches - successfully aborting!\r\n" ));
				goto back_to_idle;
			}
		}



		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_ERASE_CHIP ) != 0 )
		{
			DEBUG_PRINTF(( "Erasing entire chip\r\n" ));

			/* Erase the serial flash chip */
			sflash_chip_erase( &sflash_handle );
			if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_VERIFY_CHIP_ERASURE ) != 0 )
			{
				DEBUG_PRINTF(( "Verifying erasure of entire chip\r\n" ));

				/* Verify Erasure */
				pos = 0;
				while ( pos < chip_size )
				{
					uint32_t read_size = ( chip_size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : chip_size - pos;
					sflash_read( &sflash_handle, pos, Rx_Buffer, read_size );
					uint8_t* cmp_ptr = Rx_Buffer;
					uint8_t* end_ptr = &Rx_Buffer[read_size];
					while ( ( cmp_ptr <  end_ptr ) && ( *cmp_ptr == 0xff ) )
					{
						cmp_ptr++;
					}
					if ( cmp_ptr < end_ptr )
					{
						/* Verify Error - Chip not erased properly */
						data_transfer.result = MFG_SPI_FLASH_RESULT_ERASE_FAILED;
						DEBUG_PRINTF(( "Chip was not erased properly - abort!\r\n" ));
						goto back_to_idle;
					}
					pos += read_size;
					watchdog_kick( );
				}
			}
		}

		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE ) != 0 )
		{
			DEBUG_PRINTF(( "Writing location\r\n" ));

			/* Write the WLAN firmware into memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t write_size = ( data_transfer.size - pos > WRITE_CHUNK_SIZE )? WRITE_CHUNK_SIZE : data_transfer.size - pos;
				sflash_write( &sflash_handle, pos + data_transfer.dest_address, &data_transfer.data[pos], write_size );
				pos += write_size;
				watchdog_kick( );
			}

		}

		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_POST_WRITE_VERIFY ) != 0 )
		{
			DEBUG_PRINTF(( "Verifying after write\r\n" ));

			/* Read data from SPI FLASH memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t read_size = ( data_transfer.size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : data_transfer.size - pos;
				sflash_read( &sflash_handle, pos + data_transfer.dest_address, Rx_Buffer, read_size );
				if ( 0 != memcmp( Rx_Buffer, &data_transfer.data[pos], read_size ) )
				{
					/* Verify Error - Read data different to written data */
					data_transfer.result = MFG_SPI_FLASH_RESULT_VERIFY_AFTER_WRITE_FAILED;
					DEBUG_PRINTF(( "Verify error - Data was not written successfully - abort!\r\n" ));
					goto back_to_idle;
				}
				pos += read_size;
				watchdog_kick( );
			}
		}


		/* OK! */
		data_transfer.result = MFG_SPI_FLASH_RESULT_OK;

back_to_idle:
		data_transfer.command = MFG_SPI_FLASH_COMMAND_NONE;
		data_transfer.dest_address = 0;
		data_transfer.size = 0;
		memset( data_transfer.data, 0, __JTAG_FLASH_WRITER_DATA_BUFFER_SIZE__ );
    }

    return 0;
#else

    uint32_t pos;
    sflash_handle_t sflash_handle;
    unsigned long chip_size;

	//----------------------------------- Debug Log - MikeJ ----------------------------------------
	uint8_t tmpbuf[20];
	Serial_Init();
	SerialPutString("\r\n\r\n\r\n");
	SerialPutString("=====================\r\n");
	SerialPutString("Serial Flash Handler\r\n");
	SerialPutString("=====================\r\n\r\n");
	//DEBUG_PRINTF(( "\r\n\r\n\r\n=====================\r\n" ));
	//DEBUG_PRINTF(( "Serial Flash Handler\r\n" ));
	//DEBUG_PRINTF(( "=====================\r\n\r\n" ));
	//----------------------------------------------------------------------------------------------
    /* loop forever */
    while (1)
    {
    	//DEBUG_PRINTF(( "Waiting for command\r\n" ));
    	/* wait for a command to be written. */
    	while ( data_transfer.command == MFG_SPI_FLASH_COMMAND_NONE )
    	{
    		watchdog_kick( );
    	}

		//----------------------------------- Debug Log - MikeJ ----------------------------------------
		SerialPutString("Start - cmd(0x");
		Int2Str(tmpbuf, 16, data_transfer.command); SerialPutString(tmpbuf);
		SerialPutString("), dest(0x");
		Int2Str(tmpbuf, 16, data_transfer.dest_address); SerialPutString(tmpbuf);
		SerialPutString("), size(");
		Int2Str(tmpbuf, 10, data_transfer.size); SerialPutString(tmpbuf);
		SerialPutString(")\r\n");
		//---------------------------------------------------------------------------------------------
		data_transfer.result = MFG_SPI_FLASH_RESULT_IN_PROGRESS;

    	DEBUG_PRINTF(( "Received command: %s%s%s%s%s%s\r\n",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_INITIAL_VERIFY     )?   "INITIAL_VERIFY "       : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_ERASE_CHIP         )?   "ERASE_CHIP "           : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE              )?   "WRITE "                : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_POST_WRITE_VERIFY  )?   "POST_WRITE_VERIFY "    : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_VERIFY_CHIP_ERASURE)?   "VERIFY_CHIP_ERASURE "  : "",
						( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE_DCT          )?   "WRTIE_DCT "            : ""
					));
    	DEBUG_PRINTF(( "Destination address: %lu\r\n", data_transfer.dest_address ));
    	DEBUG_PRINTF(( "Size: %lu\r\n", data_transfer.size ));

		/* Check the data size is sane - cannot be bigger than the data storage area */
		if ( data_transfer.size > DATA_BUFFER_SIZE )
		{
			SerialPutString("Too BIG Data!!!\r\n");	//MikeJ
			data_transfer.result = MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_BUFFER;
			//DEBUG_PRINTF(( "Size too big to for storage area - aborting!\r\n" ));
			//goto back_to_idle;
			goto fail_end;	//MikeJ
		}
	   	//DEBUG_PRINTF(( "Initialising\r\n" ));

		/* Initialise the serial flash driver */
		init_sflash( &sflash_handle, 0, SFLASH_WRITE_ALLOWED );
		//DEBUG_PRINTF(( "Done initialising\r\n" ));

		//----------------------------------- Buf Clear Operation - MikeJ ----------------------------------------
		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_BUFFER_CLEAR ) != 0 )
		{
			SerialPutString("Buffer Clear\r\n");
			//DEBUG_PRINTF(( "Buffer Clear\r\n" ));
			memset(data_transfer.data, 0, DATA_BUFFER_SIZE);
			goto succ_end;
		}
		//----------------------------------- Read Operation - MikeJ ----------------------------------------
		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_READ ) != 0 )
		{
			SerialPutString("Chip Read\r\n");
			//DEBUG_PRINTF(( "Chip Read\r\n" ));
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t read_size = ( data_transfer.size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : data_transfer.size - pos;
				sflash_read( &sflash_handle, pos + data_transfer.dest_address, Rx_Buffer, read_size );
				memcpy(&data_transfer.data[pos], Rx_Buffer, read_size);
				pos += read_size;
				watchdog_kick( );
			}
			goto succ_end;
		}
		//----------------------------------- OTA Addr Change - MikeJ ----------------------------------------
		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE_OTA ) != 0 )
		{
		#define APP_IMAGE_LOCATION_IN_SFLASH   ( 0 )
			bootloader_app_header_t image_header;
		    platform_dct_header_t dct_header;
			static uint32_t start_of_ota_image;

			SerialPutString("Config OTA Dst addr\r\n");
			//DEBUG_PRINTF(( "Config OTA Dst addr\r\n" ));

			if(data_transfer.dest_address == APP_IMAGE_LOCATION_IN_SFLASH) {	// 처음 한번만 OTA시작 위치를 읽음
				SerialPutString("Read Start Pointer of OTA\r\n");
				//DEBUG_PRINTF(( "Read Start Pointer of OTA\r\n" ));
			    /* Read the factory app image header */
				sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH, &image_header, sizeof( image_header ) );
				/* Read the DCT header (starts immediately after app */
				sflash_read( &sflash_handle, image_header.size_of_app, &dct_header, sizeof( platform_dct_header_t ) );
				/* Read the image header of the OTA application which starts at the end of the dct image */
				start_of_ota_image = image_header.size_of_app + dct_header.full_size;
				/*/--------------------- Debug Log - MikeJ ---------------------
				watchdog_kick( );
				SerialPutString("### Get APP size - (");
				Int2Str(tmpbuf, 16, image_header.size_of_app); SerialPutString(tmpbuf);
				SerialPutString(")\r\n");
				SerialPutString("### Get DCT size - (");
				Int2Str(tmpbuf, 16, dct_header.full_size); SerialPutString(tmpbuf);
				SerialPutString(")\r\n");
				SerialPutString("### Get OTA Addr - (");
				Int2Str(tmpbuf, 16, start_of_ota_image); SerialPutString(tmpbuf);
				SerialPutString(")\r\n");
				///----------------------------------------------------------*/
			}

			data_transfer.dest_address += start_of_ota_image;	// OTA입장에서는 0~ 에 write하면 sf에서는 제 위치에 기록

		}
		//-----------------------------------------------------------------------------------------------
		if ( ( data_transfer.command &MFG_SPI_FLASH_COMMAND_WRITE_DCT ) != 0 )
		{
			//DEBUG_PRINTF(( "Getting DCT location\r\n" ));
			SerialPutString("Config DCT Dst addr\r\n");	//MikeJ
			/* User requested to write the DCT
			 * Read the Factory Reset App header to find where to begin writing
			 */
			if ( WICED_SUCCESS != platform_get_sflash_dct_loc( &sflash_handle, &data_transfer.dest_address ) )
			{
				data_transfer.result = MFG_SPI_FLASH_RESULT_DCT_LOC_NOT_FOUND;
				//DEBUG_PRINTF(( "DCT location not found - aborting!\r\n" ));
				SerialPutString("DCT location not found - aborting!\r\n");
				//goto back_to_idle;
				goto fail_end;	//MikeJ
			}
			/*/--------------------- Debug Log - MikeJ ---------------------
			SerialPutString("DCT Start Address: ");
			Int2Str(tmpbuf, 16, data_transfer.dest_address);
			SerialPutString(tmpbuf);
			SerialPutString("\r\n");
			*///----------------------------------------------------------
		}

		/* Check the data will fit on the sflash chip */
		sflash_get_size( &sflash_handle, &chip_size );
		if ( data_transfer.size + data_transfer.dest_address  > chip_size )
		{
			SerialPutString("Chip Size Over !!!\r\n");	//MikeJ
			data_transfer.result = MFG_SPI_FLASH_RESULT_SIZE_TOO_BIG_CHIP;
			//DEBUG_PRINTF(( "Size too big to fit on chip - aborting!\r\n" ));
			//goto back_to_idle;
			goto fail_end;	//MikeJ
		}
		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_INITIAL_VERIFY ) != 0 )
		{
			//DEBUG_PRINTF(( "Verifying existing data!\r\n" ));
			SerialPutString("Init Verify\r\n");	//MikeJ

			/* Read data from SPI FLASH memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t read_size = ( data_transfer.size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : data_transfer.size - pos;
				sflash_read( &sflash_handle, pos + data_transfer.dest_address, Rx_Buffer, read_size );
				if ( 0 != memcmp( Rx_Buffer, &data_transfer.data[pos], read_size ) )
				{
					/* Existing data different */
					//DEBUG_PRINTF(( "Existing data is different - stop verification\r\n" ));
					break;
				}
				pos += read_size;
				watchdog_kick( );
			}
			if ( pos >= data_transfer.size )
			{
				/* Existing data matches */
				/* No write required */
				SerialPutString("No Write Required\r\n");	//MikeJ
				data_transfer.result = MFG_SPI_FLASH_RESULT_OK;
				//DEBUG_PRINTF(( "Existing data matches - successfully aborting!\r\n" ));
				//goto back_to_idle;
				goto succ_end;	//MikeJ
			}
		}



		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_ERASE_CHIP ) != 0 )
		{
			//DEBUG_PRINTF(( "Erasing entire chip\r\n" ));
			SerialPutString("Chip Erase\r\n");	//MikeJ

			/* Erase the serial flash chip */
			sflash_chip_erase( &sflash_handle );
			if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_VERIFY_CHIP_ERASURE ) != 0 )
			{
				//DEBUG_PRINTF(( "Verifying erasure of entire chip\r\n" ));
				SerialPutString("Erase Verify\r\n");	//MikeJ

				/* Verify Erasure */
				pos = 0;
				while ( pos < chip_size )
				{
					uint32_t read_size = ( chip_size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : chip_size - pos;
					sflash_read( &sflash_handle, pos, Rx_Buffer, read_size );
					uint8_t* cmp_ptr = Rx_Buffer;
					uint8_t* end_ptr = &Rx_Buffer[read_size];
					while ( ( cmp_ptr <  end_ptr ) && ( *cmp_ptr == 0xff ) )
					{
						cmp_ptr++;
					}
					if ( cmp_ptr < end_ptr )
					{
						/* Verify Error - Chip not erased properly */
						data_transfer.result = MFG_SPI_FLASH_RESULT_ERASE_FAILED;
						//DEBUG_PRINTF(( "Chip was not erased properly - abort!\r\n" ));
						SerialPutString("Chip was not erased properly - abort!\r\n");	//MikeJ
						//goto back_to_idle;
						goto fail_end;	//MikeJ
					}
					pos += read_size;
					watchdog_kick( );
				}
			}
		}

		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_WRITE ) != 0 )
		{
			//DEBUG_PRINTF(( "Writing location\r\n" ));
			SerialPutString("Chip Write\r\n");	//MikeJ

			/* Write the WLAN firmware into memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t write_size = ( data_transfer.size - pos > WRITE_CHUNK_SIZE )? WRITE_CHUNK_SIZE : data_transfer.size - pos;
				sflash_write( &sflash_handle, pos + data_transfer.dest_address, &data_transfer.data[pos], write_size );
				pos += write_size;
				watchdog_kick( );
			}

		}

		if ( ( data_transfer.command & MFG_SPI_FLASH_COMMAND_POST_WRITE_VERIFY ) != 0 )
		{
			//DEBUG_PRINTF(( "Verifying after write\r\n" ));
			SerialPutString("Post Verify\r\n");	//MikeJ

			/* Read data from SPI FLASH memory */
			pos = 0;
			while ( pos < data_transfer.size )
			{
				uint32_t read_size = ( data_transfer.size - pos > sizeof(Rx_Buffer) )? sizeof(Rx_Buffer) : data_transfer.size - pos;
				sflash_read( &sflash_handle, pos + data_transfer.dest_address, Rx_Buffer, read_size );
				if ( 0 != memcmp( Rx_Buffer, &data_transfer.data[pos], read_size ) )
				{
					/* Verify Error - Read data different to written data */
					data_transfer.result = MFG_SPI_FLASH_RESULT_VERIFY_AFTER_WRITE_FAILED;
					//DEBUG_PRINTF(( "Verify error - Data was not written successfully - abort!\r\n" ));
					SerialPutString("Verify error - Data was not written successfully - abort!\r\n");	//MikeJ
					//goto back_to_idle;
					goto fail_end;	//MikeJ
				}
				pos += read_size;
				watchdog_kick( );
			}
		}

succ_end:	// OK
		SerialPutString("OK\r\n\r\n");
		//DEBUG_PRINTF(( "OK\r\n\r\n" ));
		data_transfer.result = MFG_SPI_FLASH_RESULT_OK;
		goto cmd_reset;
fail_end:	//FAIL
		SerialPutString("FAIL\r\n\r\n");
		//DEBUG_PRINTF(( "FAIL\r\n\r\n" ));
cmd_reset:
		data_transfer.command = MFG_SPI_FLASH_COMMAND_NONE;
		data_transfer.dest_address = 0;
		data_transfer.size = 0;
    }

    return 0;

#endif

}


#if defined(__ICCARM__)
#pragma section="CSTACK"
extern void iar_set_msp(void*);
__root void _wiced_iar_program_start(void)
{
    /* When the execution of the program is initiated from an external debugger */
    /* it will perform a reset of the CPU followed by halt and set a program counter to program entry function __iar_program_start leaving */
    /* SP unchanged */

    /* Consequently, the SP value will be set to the value which was read from address 0x00000000(applicable for CM3) */
    /* For apps which have an interrupt vector table placed at the start of the flash, the value of the SP will */
    /* contain correct value, however for apps which have interrupt vectors shifted to a different memory location, */
    /* the SP will contain garbage. On entry we must call this function which will set the SP to point to the end */
    /* of the CSTACK section */
    iar_set_msp(__section_end("CSTACK"));
}

#endif

