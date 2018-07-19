set remotetimeout 20 
shell start /B ./Tools/OpenOCD/Win32/openocd-all-brcm-libftdi.exe -f ./Tools/OpenOCD/Olimex_ARM-USB-TINY-H.cfg -f ./Tools/OpenOCD/stm32f2x.cfg -f ./Tools/OpenOCD/stm32f2x_gdb_jtag.cfg -l build/openocd_log.txt 
