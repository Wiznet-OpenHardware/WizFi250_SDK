#!/usr/bin/perl

#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#


if (! $ARGV[0] )
{
	print "Usage ./map_parse.pl [-a] [-w] <map file filename>";
	print "           -a = show all sections";
	print "           -w = show more warnings";
	exit;
}

#command line parameters
$printall = 0;
$warn = 0;
foreach (@ARGV)
{
	if ( $_ eq "-a" )
	{
		$printall = 1;
	}
	elsif ( $_ eq "-w" )
	{
		$warn = 1;
	}
	else
	{
		$filename = $_;
	}
}

#open the file
open INFILE, $filename or die "cant open " . $filename;
@file = <INFILE>;
close INFILE;

$file_cont = join('',@file);

#Get the start of FLASH and SRAM
$file_cont =~ m/.*Memory Configuration.*Attributes(.*)Linker script and memory map.*/sgi;
$mem_config = $1;
if ( $mem_config =~ m/APP_CODE\s+0x(\S*)\s+0x(\S*)/sgi )
{
    $start_flash = hex($1);
    $length_flash = hex($2);
}
elsif ( $mem_config =~ m/APP_FLASH\s+0x(\S*)\s+0x(\S*)/sgi )
{
    $start_flash = hex($1);
    $length_flash = hex($2);
}
else
{
    print "Could not find start/end addresses of Flash\n";
}

$mem_config =~ m/SRAM\s+0x(\S*)\s+0x(\S*)/sgi;
$start_sram = hex($1);
$length_sram = hex($2);


#remove stuff above the memory map
$file_cont =~ s/.*Linker script and memory map(.*)/$1/sgi;

#remove stuff below the memory map
$file_cont =~ s/(.*)Cross Reference Table.*/$1/sgi;


our $network;

if ( $file_cont =~ m/\/NetX_Duo.a/si )
{
    $network = "NetX_Duo";
}
elsif ( $file_cont =~ m/\/NetX.a/si )
{
    $network = "NetX";
}
elsif ( $file_cont =~ m/\/LwIP.a/si )
{
    $network = "LwIP";
}
else
{
    $network = "Unknown";
}


%module_totals = ( );

$total_flash = 0;
$total_ram = 0;
$total_other = 0;

$max_flash = 0;
$max_ram = 0;
$max_other = 0;

if ( $printall == 1 )
{
	print "Memory Area, Module, Name, Size, Address, Decimal Address, Filename, See bottom for collated totals\n";
}

$ram_known_diff = 0;
$flash_known_diff = 0;
$other_known_diff = 0;

while ( $file_cont =~ m/\n [\.\*]?(\S+?)\*?\s+0x(\S+)\s+0x(\S+)\s+(\S+)/gi )
{
	$section = $1;
	$hexaddress = $2;
	$decaddress = hex($2);
	$size = hex($3);
	$modulefn = $4;
	if   ( ( $section ne "debug_info" ) && 
	     ( $section ne "debug_macinfo" ) &&
	     ( $section ne "debug_str" ) &&
	     ( $section ne "debug_line" ) &&
	     ( $section ne "debug_loc" ) &&
	     ( $section ne "debug_frame" ) &&
	     ( $section ne "debug_abbrev" ) &&
	     ( $section ne "debug_pubnames" ) &&
	     ( $section ne "debug_aranges" ) &&
	     ( $section ne "ARM.attributes" ) &&
	     ( $section ne "comment" ) &&
	     ( $section ne "debug_ranges" ) &&
	     ( $section ne "debug_pubtypes" ) &&
	     ( $size != 0 ) &&
	     ( $decaddress != 0 ) )
	 {
	 	if ( ( $decaddress >= $start_sram ) && ( $decaddress < $start_sram + $length_sram ) )
	 	{
			$area = "RAM";
			$total_ram += $size;
			if ( $max_ram < $decaddress + $size )
			{
				$max_ram = $decaddress + $size;
			}
			
			if ( $warn )
			{
				if ( ( $total_ram != ($max_ram - $start_sram) ) && ( $ram_known_diff != (($max_ram - $start_sram) - $total_ram) ) )
				{
					$ram_known_diff = ($max_ram - $start_sram) - $total_ram;
					print "WARNING: RAM Max mismatch @ 0x$hexaddress. Max RAM = " . ($max_ram - $start_sram) . ". Total RAM = $total_ram. \n";
				}
			}
		}
		elsif ( $decaddress < $start_flash + $length_flash)
		{
			$area = "FLASH";
			$total_flash += $size;
			if ( $max_flash < $decaddress + $size )
			{
				$max_flash = $decaddress + $size;
			}
			
			if ( $warn && ( ( $total_flash != ($max_flash - $start_flash) ) && ( $flash_known_diff != (($max_flash - $start_flash) - $total_flash)) ) )
			{
				$flash_known_diff = ($max_flash - $start_flash) - $total_flash;
				print "WARNING: FLASH Max mismatch @ 0x$hexaddress. Max Flash = " . ($max_flash - $start_flash) . ". Total Flash = $total_flash. \n";
			}
		}
		else
		{
			$area = "OTHER";
			$total_other += $size;
			if ( $max_other < $decaddress + $size )
			{
				$max_other = $decaddress + $size;
			}
			
			if ( $warn && ( $total_other != $max_other ) && ( $other_known_diff != ( $max_other - $total_other ) ) )
			{
				$other_known_diff = $max_other - $total_other;
				print "WARNING: OTHER Max mismatch @ 0x$hexaddress. Max other = $max_other. Total other = $total_other. \n";
			}
		}
		$module = process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress );
	 }
}

# Process sections that are in both flash and ram due to having inital values load from flash into ram variables 
while ( $file_cont =~ m/\n.(\S+)\s+0x(\S+)\s+0x(\S+) load address 0x(\S+)/gi )
{
#.data           0x20000000      0xc10 load address 0x08054af0
    $area = "FLASH";
    $section = "RAM Initialisation - $1";
    $size = hex($3);
    $hexaddress = $4;
    $decaddress = hex($4);
    $modulefn = "";
   
    if ( $decaddress < $start_flash + $length_flash)
    {
        $total_flash += $size;
        if ( $max_flash < $decaddress + $size )
        {
            $max_flash = $decaddress + $size;
        }
        
        if ( $warn && ( ( $total_flash != ($max_flash - $start_flash) ) && ( $flash_known_diff != (($max_flash - $start_flash) - $total_flash)) ) )
        {
            $flash_known_diff = ($max_flash - $start_flash) - $total_flash;
            print "WARNING: FLASH Max mismatch @ 0x$hexaddress. Max Flash = " . ($max_flash - $start_flash) . ". Total Flash = $total_flash. \n";
        }
    }
    
    if ( $size != 0 )
    {
#        print "section $section size $size address 0x$hexaddress\n";
        $module = process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress );
    }
}


if ( $printall == 1 )
{
	print "\n";
}

# Get target name from map filename

$filename       =~ m/^(.*)\.map/;
my $target_name = $filename;
$target_name    =~ s/^(.*)\.map/$1/i;
$target_name    =~ s/.*\/(.*)$/$1/i;
print "$target_name\n";


print_module_totals( );

if ( $total_ram != ($max_ram - $start_sram) )
{
	print "WARNING: RAM Total mismatch, Total size of RAM sections: $total_ram, Max minus Base RAM address: " . ($max_ram - $start_sram) . " (max:$max_ram, start:$start_sram)\n";
}
if ( $total_flash != ($max_flash - $start_flash) )
{
    #print "WARNING: FLASH Total mismatch, Total size of FLASH sections: $total_flash, Max minus Base FLASH address: " . ($max_flash - $start_flash) . " (max:$max_flash, start:$start_flash)\n";
}
if ( $total_other != ($max_other) )
{
    print "WARNING: OTHER Total mismatch, Total size of OTHER sections: $total_other, Max OTHER address: " . ($max_other) . "\n";
}

sub print_sections
{
	print "Memory Area, Section Name, Address (Hex), Address (decimal), Size (bytes), Module filename\n";
	foreach $module (sort keys %module_totals)
	{
   		print "$module, $module_totals{$module}\n";
	}
	print "$area, $section , 0x$hexaddress , $decaddress, $size, $module, $modulefn\n";
}


sub process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress )
{
	$module = "Other";
	
    if ( $modulefn =~ m/\/App_\S+\.a/sgi )
    {
        $module = "App";
    }
	elsif ( $modulefn =~ m/vector_table/sgi ||
	        $modulefn =~ m/\/interrupt_handlers_GCC\.o/sgi ||
	        $modulefn =~ m/\/HardFault_handler\.o/sgi )
	{
		$module = "Interrupt Vectors";
	}
	elsif ( $modulefn =~ m/\/ThreadX\.a/sgi ||
	        $modulefn =~ m/\/ThreadX-\d.\d.\w+\.a/sgi )
	{
		$module = "ThreadX";
	}
	elsif ( $modulefn =~ m/\/SPI_Flash_Library/sgi )
	{
		$module = "SPI Flash Library";
	}
	elsif ( $modulefn =~ m/\/STM32F1xx_lib\.a/sgi ||
	        $modulefn =~ m/\/STM32F1xx_Drv\.a/sgi ||
	        $modulefn =~ m/\/STM32F2xx_?D?r?v?\.a/sgi ||
	        $modulefn =~ m/\/STM32F4xx_?D?r?v?\.a/sgi || 
	        $modulefn =~ m/\/ASF\.a/sgi || 
	        $modulefn =~ m/\/SAM4S\.a/sgi || 
	        $modulefn =~ m/\/K60_?D?r?v?\.a/sgi )
	{
		$module = "Host MCU-family library";
	}
    elsif ( $section =~ m/\.app_thread_stack/sgi )
    {
        $module = "App Stack";
    }
	elsif ( $modulefn =~ m/arm-none-eabi/sgi ||
	        $modulefn =~ m/[\\\/]libc\.a/sgi || 
	        $modulefn =~ m/[\\\/]libgcc\.a/sgi || 
            $modulefn =~ m/[\\\/]libm\.a/sgi ||
            $modulefn =~ m/[\\\/]common_GCC\.a/sgi ||
	        $modulefn =~ m/\/common\/GCC\/\S+\.o/sgi)
	{
		$module = "libc";
	}
	elsif ( $modulefn =~ m/\/Wiced_(NetX|NetX_Duo|LwIP)_Interface\.a/sgi )
	{
		$module = "Networking";
	}
	elsif ( $modulefn =~ m/\/Wiced\.a/sgi ||
	        $modulefn =~ m/\/Wiced_(ThreadX|FreeRTOS)_Interface\.a/sgi ||
	        $modulefn =~ m/\/Wiced_\w+\_Interface_\w+\.a/sgi ||
	        $modulefn =~ m/\/Wiced_Network_\w+_\w+\.a/sgi )
	{
		$module = "Wiced";
	}
	elsif ( $modulefn =~ m/\w+_Interface_(SPI|SDIO)\.a/sgi ||
	        $modulefn =~ m/\/WWD_for_(SPI|SDIO)_\w+\.a/sgi ||
	        $modulefn =~ m/\/WWD_\w+\_Interface\.a/sgi ||
	        $modulefn =~ m/\/WWD_\w+\_Interface_\w+\.a/sgi )
	{
		$module = "WWD";
	}
	elsif ( $modulefn =~ m/\/crt0_gcc\.o/sgi ||
	        $modulefn =~ m/\/Platform_\S+\.a/sgi )
	{
		$module = "Platform";
	}
	elsif ( $modulefn =~ m/\/Lib_(.+)\.a/sgi )
	{
		$module = $1;
	}
	elsif ( $modulefn =~ m/\/Supplicant_besl\.a/sgi ||
	        $modulefn =~ m/\/Supplicant_besl.\w+\.a/sgi ||
	        $modulefn =~ m/\/besl\.\w+\.\w+\.a/sgi )
	{
    	$module = "Supplicant - BESL";
	}
	elsif ( ( $section eq "rodata.wifi_firmware_image" ) ||
	        ( $section eq "rodata.wifi_firmware_image_size" ) )
	{ 
		$module = "Wi-Fi Firmware";
	}
	elsif ( $section eq  "rodata.vars" )
	{ 
		$module = "NVRam";
	}
	elsif ( $section eq  "fill" )
	{ 
		$module = "Startup Stack & Link Script fill";
	}
	elsif ( ( $section =~ m/.*tx_buffer_pool_memory/ ) ||
	     ( $section =~ m/.*rx_buffer_pool_memory/ ) || 
	     ( $section =~ m/.*memp_memory/ ) )
	{
		$module = "Packet Buffers";
	}
	elsif ( $section =~ m/.*xHeap/ )
	{
		$module = "FreeRTOS heap (inc Stacks & Semaphores)";
	}
	elsif ( $section =~ m/.*xHeap/ )
	{
		$module = "FreeRTOS heap (inc Stacks & Semaphores)";
	}
	elsif ( $section =~ m/.*lwip_stats/ )
	{
		$module = "LwIP stats";
	}
	elsif ( $section =~ m/.*ram_heap/ )
	{
		$module = "LwIP Heap";
	}
	elsif ( $modulefn =~ m/.*app_header\.o/sgi )
    {
        $module = "Bootloader";
    }
    elsif ( $modulefn =~ m/\/gedday\.\w+\.\w+\.\w+\.\w+\.a/sgi )
    {
        $module = "Gedday";
    }
    elsif ( $modulefn =~ m/\/NetX\.a/sgi || 
         $modulefn =~ m/\/NetX.\w+\.a/sgi )
    {
        $module = "NetX";
    }
    elsif ( $modulefn =~ m/\/NetX_Duo\.a/sgi ||
            $modulefn =~ m/\/NetX_Duo-\d.\d.\w+\.a/sgi )
   {
       $module = "NetX-Duo - Code";
   }
   elsif ( ($network eq "NetX_Duo" ) && ( $section =~ m/\.wiced_ip_handle/sgi ) )
   {
       $module = "NetX-Duo - Interfaces & Stacks";
   }
   elsif ( $section =~ m/RAM Initialisation/sgi )
   {
       $module = "RAM Initialisation";
   }
    elsif ( $modulefn =~ m/\/LwIP\.a/sgi )
    {
        $module = "LwIP";
    }
    elsif ( $modulefn =~ m/\/FreeRTOS\.a/sgi )
    {
        $module = "FreeRTOS";
    }
    elsif ( $modulefn =~ m/NoOS\.a/sgi )
    {
        $module = "NoOS";
    }
    #else
    #{
    #    print "++$area, $section, $size, $modulefn, $decaddress, $hexaddress\n";
    #}

	if ( $printall == 1 )
	{
		print "$area,$module, $section, $size, 0x$hexaddress, $decaddress, $modulefn\n";
	}
	
	$module_totals{"$module"}{"$area"} += $size;
	
	return $module;

	
}

sub print_module_totals( )
{
	if ( $printall == 1 )
	{
		print "----------------------------------,--------,---------\n";
		print "                                  ,        ,  Static \n";
		print "Module                            , Flash  ,   RAM   \n";
		print "----------------------------------,--------,---------\n";
		foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
		{
	   		print sprintf("%-34.34s, %7d, %7d\n", $module, $module_totals{$module}{'FLASH'},  $module_totals{$module}{'RAM'});
		}
		print         "----------------------------------,--------,---------\n";
		print sprintf("TOTAL (bytes)                     , %7d, %7d\n", $total_flash,  $total_ram);
		print         "----------------------------------,--------,---------\n";
	}
	else
	{
		print "----------------------------------|---------|---------|\n";
		print "                                  |         |  Static |\n";
		print "              Module              |  Flash  |   RAM   |\n";
		print "----------------------------------+---------+---------|\n";
		foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
		{
	   		print sprintf("%-34.34s| %7d | %7d |\n", $module, $module_totals{$module}{'FLASH'},  $module_totals{$module}{'RAM'});
		}
		print         "----------------------------------+---------+---------|\n";
		print sprintf("TOTAL (bytes)                     | %7d | %7d |\n", $total_flash,  $total_ram);
		print         "----------------------------------|---------|---------|\n";
	}
	if ( $total_other != 0)
	{
		print "WARNING: $total_other bytes unaccounted for (neither FLASH or RAM)\n";
	}
	print "\n"
}


