#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
###############################################################################
#
# sflash_write.tcl
#
# This TCL OpenOCD script runs on a PC and communicates with the embedded 
# sflash_write app to allow the PC to write data into a serial flash chip
# attached to the target processor
#
# Usage example:
#
# source [find mfg_spi_flash/write_sflash.tcl]
# sflash_init "BCMUSI11-SDIO-debug"
# sflash_write_file "example.bin" 0x10 "BCMUSI11-SDIO-debug" 1
# shutdown
#
###############################################################################

# CHIP_RAM_START must be supplied by target specific TCL script 
set MemoryStart $CHIP_RAM_START

###############################################################################
#
# These variables must match the ones in Apps/waf/sflash_write/sflash_write.c
#
# They rely on the linker script placing the data_config_area_t and
# data_transfer_area_t structures at the start of RAM 
#
###############################################################################

# This must match data_config_area_t
set entry_address_loc  [expr $MemoryStart + 0x00 ]
set stack_address_loc  [expr $MemoryStart + 0x04 ]
set buffer_size_loc    [expr $MemoryStart + 0x08 ]

# This must match data_transfer_area_t
set data_size_loc      [expr $MemoryStart + 0x0C ]
set dest_address_loc   [expr $MemoryStart + 0x10 ]
set command_loc        [expr $MemoryStart + 0x14 ]
set result_loc         [expr $MemoryStart + 0x18 ]
set data_loc           [expr $MemoryStart + 0x1C ]


# These must match the MFG_SPI_FLASH_COMMAND defines
set COMMAND_INITIAL_VERIFY            (0x01)
set COMMAND_ERASE                     (0x02)
set COMMAND_WRITE                     (0x04)
set COMMAND_POST_WRITE_VERIFY         (0x08)
set COMMAND_VERIFY_CHIP_ERASURE       (0x10)
set COMMAND_WRITE_DCT                 (0x20)

# These must match the mfg_spi_flash_result_t enum
set RESULT(0xffffffff)      "In Progress"
set RESULT(4294967295)      "In Progress"
set RESULT(0)               "OK"
set RESULT(1)               "Erase Failed"
set RESULT(2)               "Verify after write failed"
set RESULT(3)               "Size too big for buffer"
set RESULT(4)               "Size too big for chip"
set RESULT(5)               "DCT location not found - has factory reset app been written?"


###############################################################################
# memread32
#
# Helper function that reads a 32 bit value from RAM and returns it
#
# address - the RAM address to read from
###############################################################################
proc memread32 {address} {
    mem2array memar 32 $address 1
    return $memar(0)
}

###############################################################################
# load_image_bin
#
# Loads part of a binary file into RAM
#
# fname   - filename of binary image file
# foffset - offset from the start of the binary file where data will be read
# address - the destination RAM address
# length  - number of bytes to transfer 
###############################################################################
proc load_image_bin {fname foffset address length } {
  # Load data from fname filename at foffset offset to
  # target at address. Load at most length bytes.
  puts "loadimage address $address foffset $foffset $length"
  load_image $fname [expr $address - $foffset] bin $address $length
}



###############################################################################
# sflash_init
#
# Prepares for writing to serial flashby loading the sflash_write app into
# memory and setting it running.
# This function assumes the following target has already been built:
#      waf_sflash_write-NoOS-NoNS-<PlatBusDebug>
#
# PlatBusDebug   - The platform, bus and debug part of the build target
###############################################################################
proc sflash_init { PlatBusDebug } {
    global entry_address_loc 
    global stack_address_loc
    global buffer_size_loc
    global entry_address
    global stack_address
    global buffer_size
    
    init
    reset halt

    halt
    load_image build/waf_sflash_write-NoOS-NoNS-$PlatBusDebug/Binary/waf_sflash_write-NoOS-NoNS-$PlatBusDebug.elf

    set entry_address [memread32 $entry_address_loc]
    set stack_address [memread32 $stack_address_loc ]
    set buffer_size   [memread32 $buffer_size_loc]

    # Setup start address
    reg pc $entry_address
    
    resume

}


###############################################################################
# program_sflash
#
# Executes a serial command by communicating to the sflash_write app
#
# fname    - filename of binary image file (if command requires it)
# foffset  - offset from the start of the binary file where data will be read  (if command requires it)
# dataSize - number of bytes to transfer (if command requires it)
# destaddr - the destination serial flash address (if command requires it)
# cmd      - The commmand to execute (see list above)
###############################################################################
proc program_sflash { filename foffset dataSize destaddr cmd } {
	global PlatBusDebug MemoryStart data_size_loc data_loc dest_address_loc command_loc result_loc entry_address RESULT 
    
    # Load the binary data into the RAM
    if { $dataSize != 0 } {
	   load_image_bin $filename $foffset $data_loc $dataSize
	}
	
	# Write the details of the data
	
	mww $data_size_loc    $dataSize
	mww $dest_address_loc $destaddr
	mww $result_loc       0xffffffff

    # Write the command - This causes the writing to start
    mww $command_loc      $cmd


	set loops  0
	while { ([memread32 $result_loc]  == 0xffffffff) && ( $loops < 1000 ) } {
	 sleep 10
	 incr loops
	}

	puts "****************** Result: $RESULT([memread32 $result_loc])"
}


###############################################################################
# sflash_write_file
#
# Writes an entire binary image file into serial flash
# This function assumes the following target has already been built:
#      waf_sflash_write-NoOS-NoNS-<PlatBusDebug>
#
# filename     - filename of binary image file
# destAddress  - the destination serial flash address
# PlatBusDebug - The platform, bus and debug part of the build target
# erasechip    - If 1, Erase the chip before writing.
###############################################################################
proc sflash_write_file { filename destAddress PlatBusDebug erasechip } {
	global COMMAND_ERASE COMMAND_INITIAL_VERIFY COMMAND_WRITE COMMAND_POST_WRITE_VERIFY buffer_size

	sflash_init $PlatBusDebug

	set binDataSize [file size $filename]
	set erase_command_val [expr $COMMAND_ERASE ]
	set write_command_val [expr $COMMAND_INITIAL_VERIFY | $COMMAND_WRITE | $COMMAND_POST_WRITE_VERIFY ]
	set pos 0

	if { $erasechip } {
        puts "Erasing Chip"
		program_sflash $filename $pos 0 $destAddress $erase_command_val
	}

	while { $pos < $binDataSize } {
		if { ($binDataSize - $pos) <  $buffer_size } {
			set writesize [expr ($binDataSize - $pos)]
		} else {
			set writesize $buffer_size
		}
		puts "writing $writesize bytes at [expr $destAddress + $pos]"
		program_sflash $filename $pos $writesize [expr $destAddress + $pos] $write_command_val
		set pos [expr $pos + $writesize]
	}
}

