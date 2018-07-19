###############################################################################
#
# Usage:
#	sfwrite X			: X(APP Path like "snip_ping")
#	sfread X Y Z		: X(SF Addr), Y(Size), Z(Alignment)
#	sfdump X Y Z		: X(SF Addr), Y(Size), Z(File Name to Create)
#
# Usage example:
#	source mikej.tcl	: TCL Load
#	sfwrite snip_uart	: Write uart APP+DCT in Build Folder to Serial Flash
#	sfread 0x0 100 1	: Read 100 Bytes from SFlash Addr 0x0 with 1 byte align
#	sfdump 0x0 100 dump	: Dump 100 Bytes from SFlash Addr 0x0 to dump.bin file
#
# Info:
#	Serial Flash Range	: 0x0 ~ 0x100000
#
###############################################################################

# CHIP_RAM_START must be supplied by target specific TCL script 
set MemoryStart $CHIP_RAM_START
set SF_Size		(0x100000)
set SF_Loaded	(0)

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
set COMMAND_INITIAL_VERIFY		(0x01)
set COMMAND_ERASE				(0x02)
set COMMAND_WRITE				(0x04)
set COMMAND_POST_WRITE_VERIFY	(0x08)
set COMMAND_VERIFY_CHIP_ERASURE	(0x10)
set COMMAND_WRITE_DCT			(0x20)
set COMMAND_WRITE_OTA			(0x40)
set COMMAND_READ				(0x80)
set COMMAND_BUFFER_CLEAR		(0x100)

# These must match the mfg_spi_flash_result_t enum
set RESULT(0xffffffff)      "In Progress"
set RESULT(4294967295)      "In Progress"
set RESULT(0)               "Unknown State"
set RESULT(1)               "OK"
set RESULT(2)               "Erase Failed"
set RESULT(3)               "Verify after write failed"
set RESULT(4)               "Size too big for buffer"
set RESULT(5)               "Size too big for chip"
set RESULT(6)               "DCT location not found - has factory reset app been written?"

set BUILD_PATH				../../../build
#set PLATFORM				BCM9WCDUSI09
set PLATFORM				BCMUSI11
set BINARY_PATH				../FWFiles

proc memread32 {address} {
	mem2array memar 32 $address 1
	return $memar(0)
}

proc load_image_bin {fname foffset address length} {
	puts "loadimage address $address foffset $foffset $length"
	load_image $fname [expr $address - $foffset] bin $address $length
}

proc sfinit { } {
	global MemoryStart
	global SF_Loaded
	global entry_address_loc
	global stack_address_loc
	global buffer_size_loc
	global entry_address
	global stack_address
	global buffer_size
	global BUILD_PATH
	global PLATFORM
	global BINARY_PATH
	
	set PATHTO_sflash_write		$BINARY_PATH/waf_sflash_write-$PLATFORM.bin
	#set PATHTO_sflash_write		$BUILD_PATH/waf_sflash_write-$PLATFORM/Binary/waf_sflash_write-$PLATFORM.bin

	reset halt

	set sfbin_size [file size $PATHTO_sflash_write]
	puts "\r\n### Load sflash_write - name $PATHTO_sflash_write, size $sfbin_size, to $MemoryStart"
	load_image $PATHTO_sflash_write $MemoryStart

	if { $SF_Loaded == 1 } {
		puts "\r\n### Serial Flash OK"
		return
	} else {
		puts "\r\n### Serial Flash Init"
		set SF_Loaded 1
	}

	set entry_address [memread32 $entry_address_loc]
	set stack_address [memread32 $stack_address_loc]
	set buffer_size   [memread32 $buffer_size_loc]
    
	puts -nonewline "\r\n### Address Config : "
	puts [format " Entry(0x%8x), Stack(0x%8x), Size(0x%x)" $entry_address $stack_address $buffer_size]

	# Setup start address
	reg pc $entry_address
	reg sp $stack_address
    
	resume
}

proc sfoperate { dataSize destaddr cmd } {
	global data_size_loc
	global dest_address_loc
	global command_loc
	global result_loc

	# Write the details of the data
	mww $data_size_loc    $dataSize
	mww $dest_address_loc $destaddr
	mww $result_loc       0xffffffff

	# Write the command - This causes the writing to start
	mww $command_loc      $cmd
	
	set loops  0
	while { ([memread32 $result_loc]  == 0xffffffff) && ( $loops < 100 ) } {
		sleep 100
		incr loops
		#puts -nonewline "($loops)"
	}
}

proc sfprogram { filename foffset dataSize destaddr cmd } {
	global data_loc
	global result_loc
	global RESULT 
    
	# Load the binary data into the RAM
	if { ($dataSize != 0) } {
		load_image_bin $filename $foffset $data_loc $dataSize
	}

	sfoperate $dataSize $destaddr $cmd

	puts "### Result: $RESULT([memread32 $result_loc])"
}

proc sferase { } {
	global COMMAND_ERASE
	global COMMAND_VERIFY_CHIP_ERASURE
	
	sfinit

	set erase_command_val [expr $COMMAND_ERASE | $COMMAND_VERIFY_CHIP_ERASURE]
	
	puts "\r\n### Erase Chip"
	sfprogram 0 0 0 0 $erase_command_val
}

proc sfwrite { app_path dct_path ota_path } {
	global COMMAND_INITIAL_VERIFY
	global COMMAND_WRITE
	global COMMAND_WRITE_DCT
	global COMMAND_WRITE_OTA
	global COMMAND_POST_WRITE_VERIFY
	global COMMAND_BUFFER_CLEAR
	global buffer_size

	set APP_Size [file size $app_path]
	set DCT_Size [file size $dct_path]
	set OTA_Size [file size $ota_path]
	set buf_clear_cmd_val [expr $COMMAND_BUFFER_CLEAR]
	set write_app_cmd_val [expr $COMMAND_INITIAL_VERIFY | $COMMAND_WRITE | $COMMAND_POST_WRITE_VERIFY ]
	set write_dct_cmd_val [expr $COMMAND_INITIAL_VERIFY | $COMMAND_WRITE | $COMMAND_WRITE_DCT | $COMMAND_POST_WRITE_VERIFY ]
	set write_ota_cmd_val [expr $COMMAND_INITIAL_VERIFY | $COMMAND_WRITE | $COMMAND_WRITE_OTA | $COMMAND_POST_WRITE_VERIFY ]

	sferase

	puts "\r\n### Write APP"
	set pos 0
	while { $pos < $APP_Size } {
		if { ($APP_Size - $pos) <  $buffer_size } {
			set writesize [expr ($APP_Size - $pos)]
		} else {
			set writesize $buffer_size
		}
		#puts "APP writing $writesize bytes at $pos"
		sfoperate 0 0 $buf_clear_cmd_val
		sfprogram $app_path $pos $writesize $pos $write_app_cmd_val
		set pos [expr $pos + $writesize]
	}

	puts "\r\n### Write DCT"
	#puts "DCT writing $buffer_size bytes"
	sfprogram $dct_path 0 $DCT_Size 0 $write_dct_cmd_val
	
	puts "\r\n### Write OTA"
	set pos 0
	while { $pos < $OTA_Size } {
		if { ($OTA_Size - $pos) <  $buffer_size } {
			set writesize [expr ($OTA_Size - $pos)]
		} else {
			set writesize $buffer_size
		}
		puts "OTA writing $writesize bytes at $pos"
		sfoperate 0 0 $buf_clear_cmd_val
		sfprogram $ota_path $pos $writesize $pos $write_ota_cmd_val
		set pos [expr $pos + $writesize]
	}

	puts "\r\n### Write Finished\r\n\r\n"
}

proc sfwrite_simple { filename } {
	global BUILD_PATH
	global PLATFORM

	set app_path $BUILD_PATH/$filename-$PLATFORM/Binary/$filename-$PLATFORM.bin
	set dct_path $BUILD_PATH/$filename-$PLATFORM/DCT.bin
	set ota_path $BUILD_PATH/waf_ota_upgrade-$PLATFORM/Binary/waf_ota_upgrade-$PLATFORM.bin
	
	sfwrite $app_path $dct_path $ota_path
}

proc sfwrite_factory {  } {
	global PLATFORM
	global BINARY_PATH

	set app_path $BINARY_PATH/wizfi_wiced-$PLATFORM.bin
	set dct_path $BINARY_PATH/DCT.bin
	set ota_path $BINARY_PATH/waf_ota_upgrade-$PLATFORM.bin
	
	sfwrite $app_path $dct_path $ota_path
}

proc sfread { sf_addr length alignment } {
	global SF_Size
	global data_loc
	global COMMAND_READ
	global COMMAND_BUFFER_CLEAR
	global buffer_size

	set read_command_val [expr $COMMAND_READ]
	set buf_clear_cmd_val [expr $COMMAND_BUFFER_CLEAR]
	
	if { ($alignment != 1) && ($alignment != 2) && ($alignment != 4) } {
		puts "\r\n### Wrong Alignmnet, Only 1 or 2 or 4 are allowed"
		return
	}

	#puts "Debug Info: SFaddr($sf_addr), RDlen($length), SFsize($SF_Size)"
	if { [expr $sf_addr >= $SF_Size] } {
		puts "\r\n### SF Addr exceeded the limit"
		return "SF Addr exceeded the limit"
	} elseif { [expr ($sf_addr + $length) > $SF_Size] } {
		set length [expr ($SF_Size - $sf_addr)]
		puts "\r\n### Exceeded the limit, Length is resized to $length"
	}

	sfinit
	sfoperate 0 0 $buf_clear_cmd_val

	set pos 0
	while { $pos < $length } {
		# When using mdb, possible maximum length is 0xFFD. and if exceeded the max, program freeze.
		if { $alignment == 1 } {
			if { ($length - $pos) <  0x800 } {
				set readsize [expr ($length - $pos)]
			} else {
				set readsize 0x800
			}
		} else {
			if { ($length - $pos) <  $buffer_size } {
				set readsize [expr ($length - $pos)]
			} else {
				set readsize $buffer_size
			}
		}
		#puts "Debug Info: len($length), pos($pos), read($readsize)"

		sfoperate $readsize [expr $sf_addr + $pos] $read_command_val

		puts "\r\n### Read Serial Flash $readsize Bytes from [expr ($sf_addr + $pos)]"
		if { $alignment == 1 } {
			mdb $data_loc $readsize
		} elseif { $alignment == 2 } {
			mdh $data_loc [expr $readsize / 2]
		} elseif { $alignment == 4 } {
			mdw $data_loc [expr $readsize / 4]
		} else {
			puts "wrong alignment($alignment)"
		}

		set pos [expr $pos + $readsize]
	}
	
	puts "\r\n### Read Finished\r\n\r\n"
}

#proc sfdump { sf_addr length filename } {
proc sfdump { sf_addr length dumppath } {
	global SF_Size
	global data_loc
	global COMMAND_READ
	global COMMAND_BUFFER_CLEAR
	global buffer_size

	set read_command_val [expr $COMMAND_READ]
	set buf_clear_cmd_val [expr $COMMAND_BUFFER_CLEAR]

	puts "Debug Info: SFaddr($sf_addr), RDlen($length), SFsize($SF_Size)"
	if { [expr $sf_addr >= $SF_Size] } {
		puts "\r\n### SF Addr exceeded the limit"
		return "SF Addr exceeded the limit"
	} elseif { [expr ($sf_addr + $length) > $SF_Size] } {
		set length [expr ($SF_Size - $sf_addr)]
		puts "\r\n### Exceeded the limit, Length is resized to $length"
	}

	sfinit

	set pos 0
	set cnt 0
	while { $pos < $length } {
		if { ($length - $pos) <  $buffer_size } {
			set readsize [expr ($length - $pos)]
		} else {
			set readsize $buffer_size
			
		}
		#puts "Debug Info: len($length), pos($pos), read($readsize)"

		sfoperate $readsize [expr $sf_addr + $pos] $read_command_val

		puts "\r\n### Dump Serial Flash $readsize Bytes from [expr ($sf_addr + $pos)]"
		if { $cnt == 0 } {
			#dump_image $filename.bin $data_loc $readsize
			dump_image $dumppath/dump.bin $data_loc $readsize
		} else {
			#dump_image $filename$cnt.bin $data_loc $readsize
			dump_image $dumppath/dump$cnt.bin $data_loc $readsize
		}

		set pos [expr $pos + $readsize]
		set cnt [expr $cnt + 1]
	}

	puts "\r\n### Dump Finished\r\n\r\n"
}

proc sfhelp { } {
puts "\r\n=========================================================="
puts "Usage:"
puts " sfwrite X    : X(APP Path like "snip_ping")"
puts " sfread X Y Z : X(SF Addr), Y(Size), Z(Alignment)"
puts " sfdump X Y Z : X(SF Addr), Y(Size), Z(File Name)"
puts "\r\nUsage example:"
puts " sfwrite snip_uart"
puts "	: Write uart APP+DCT in Build Folder to Serial Flash"
puts " sfread 0x0 100 1"
puts "	: Read 100 Bytes from SFlash Addr 0x0 with 1 byte align"
puts " sfdump 0x0 100 dump"
puts "	: Dump 100 Bytes from SFlash Addr 0x0 to dump.bin file"
puts "\r\nSerial Flash Range : 0x0 ~ 0x100000"
puts "=========================================================="
puts ""
}




