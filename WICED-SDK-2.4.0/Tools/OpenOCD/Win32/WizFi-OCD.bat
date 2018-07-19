@ECHO OFF

::SET OCD_CFG=-f BCM9WCD1EVAL1.cfg -f stm32f2x.cfg -f stm32f2x-flash-app.cfg
SET OCD_CFG=-f Olimex_ARM-USB-TINY-H.cfg -f stm32f2x.cfg -f stm32f2x-flash-app.cfg

::SET FILE_SRC_BTL_ELF=../../../build/waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO/Binary/waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO.stripped.elf
::SET FILE_SRC_APP_ELF=../../../build/wizfi_wiced-BCM9WCDUSI09/Binary/wizfi_wiced-BCM9WCDUSI09.stripped.elf
::SET FILE_SRC_DCT_ELF=../../../build/wizfi_wiced-BCM9WCDUSI09/DCT.stripped.elf
::SET FILE_SRC_APP_BIN=../../../build/wizfi_wiced-BCM9WCDUSI09/Binary/wizfi_wiced-BCM9WCDUSI09.bin
::SET FILE_SRC_DCT_BIN=../../../build/wizfi_wiced-BCM9WCDUSI09/DCT.bin
::SET FILE_SRC_OTA_BIN=../../../build/waf_ota_upgrade-BCM9WCDUSI09/Binary/waf_ota_upgrade-BCM9WCDUSI09.bin
::SET FILE_SRC_SRF_BIN=../../../build/waf_sflash_write-BCM9WCDUSI09/Binary/waf_sflash_write-BCM9WCDUSI09.bin
::
::SET FILE_DES_BTL_ELF=../FWFiles/waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO.stripped.elf
::SET FILE_DES_APP_ELF=../FWFiles/wizfi_wiced-BCM9WCDUSI09.stripped.elf
::SET FILE_DES_DCT_ELF=../FWFiles/DCT.stripped.elf
::SET FILE_DES_APP_BIN=../FWFiles/wizfi_wiced-BCM9WCDUSI09.bin
::SET FILE_DES_DCT_BIN=../FWFiles/DCT.bin
::SET FILE_DES_OTA_BIN=../FWFiles/waf_ota_upgrade-BCM9WCDUSI09.bin
::SET FILE_DES_SRF_BIN=../FWFiles/waf_sflash_write-BCM9WCDUSI09.bin

SET FILE_SRC_BTL_ELF=../../../build/waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO/Binary/waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO.stripped.elf
SET FILE_SRC_APP_ELF=../../../build/wizfi_wiced-BCMUSI11/Binary/wizfi_wiced-BCMUSI11.stripped.elf
SET FILE_SRC_DCT_ELF=../../../build/wizfi_wiced-BCMUSI11/DCT.stripped.elf
SET FILE_SRC_APP_BIN=../../../build/wizfi_wiced-BCMUSI11/Binary/wizfi_wiced-BCMUSI11.bin
SET FILE_SRC_DCT_BIN=../../../build/wizfi_wiced-BCMUSI11/DCT.bin
SET FILE_SRC_OTA_BIN=../../../build/waf_ota_upgrade-BCMUSI11/Binary/waf_ota_upgrade-BCMUSI11.bin
SET FILE_SRC_SRF_BIN=../../../build/waf_sflash_write-BCMUSI11/Binary/waf_sflash_write-BCMUSI11.bin

SET FILE_DES_BTL_ELF=../FWFiles/waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO.stripped.elf
SET FILE_DES_APP_ELF=../FWFiles/wizfi_wiced-BCMUSI11.stripped.elf
SET FILE_DES_DCT_ELF=../FWFiles/DCT.stripped.elf
SET FILE_DES_APP_BIN=../FWFiles/wizfi_wiced-BCMUSI11.bin
SET FILE_DES_DCT_BIN=../FWFiles/DCT.bin
SET FILE_DES_OTA_BIN=../FWFiles/waf_ota_upgrade-BCMUSI11.bin
SET FILE_DES_SRF_BIN=../FWFiles/waf_sflash_write-BCMUSI11.bin


@ECHO [WizFi-OpenOCD......]


IF "%1" == "all" (
	@ECHO "Verifying & Downloading WizFi250 Bootloader ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_BTL_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_BTL_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 Application ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_APP_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_APP_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 DCT ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_DCT_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_DCT_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	goto END
)

IF "%1" == "all-log"  (
	@ECHO "Verifying & Downloading WizFi250 Bootloader ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_BTL_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_BTL_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 Application ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_APP_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_APP_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 DCT ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_DCT_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_DCT_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"

	goto END
)

IF "%1" == "daemon"  (
	@ECHO Starting OpenOCD Daemon ...
	openocd-all-brcm-libftdi.exe %OCD_CFG% 
	goto END
)

IF "%1" == "-c"  (
	openocd-all-brcm-libftdi.exe %OCD_CFG% %1 %2 %3 %4 %5 %6 %7 %8 %9
	goto END
)



IF "%1" == "jt-if-unlock"  (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "mww 0x40023c08 0x08192A3B" -c "mww 0x40023c08 0x4C5D6E7F" -c "mww 0x40023c14 0x0fffaaee" -c "mww 0x40023c14 0x0fffaaed" -c "reset init" -c "mdb 0x08000000 100" -c "mdb 0x08004000 100" -c "mdb 0x0800C000 100" -c shutdown
	goto END
)

IF "%1" == "jt-if-lock"  (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "mww 0x40023c08 0x08192A3B" -c "mww 0x40023c08 0x4C5D6E7F" -c "mww 0x40023c14 0x0fff55ee" -c "mww 0x40023c14 0x0fff55ed" -c "reset init" -c "mdb 0x08000000 100" -c "mdb 0x08004000 100" -c "mdb 0x0800C000 100" -c shutdown
	goto END
)

IF "%1" == "jt-if-erase"  (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash erase_sector 0 0 last" -c "mdb 0x08000000 100" -c "mdb 0x08004000 100" -c "mdb 0x0800C000 100" -c shutdown
	goto END
)

IF "%1" == "jt-if-view"  (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "mdb 0x08000000 100" -c "mdb 0x08008000 100" -c "mdb 0x08010000 100" -c shutdown
	goto END
)

IF "%1" == "jt-if-write-bl" (
	@ECHO "Verifying & Downloading WizFi250 Bootloader ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_BTL_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_BTL_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
REM openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_BTL_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_BTL_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	goto END
)

IF "%1" == "jt-if-write-dct" (
	@ECHO "Verifying & Downloading WizFi250 DCT ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_DCT_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_DCT_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	goto END
)

IF "%1" == "jt-if-write-app" (
	@ECHO "Verifying & Downloading WizFi250 Application ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_APP_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_APP_ELF%" -c shutdown >> OpenOCD.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	goto END
)

IF "%1" == "jt-if-write-all"  (
	@ECHO "Verifying & Downloading WizFi250 Bootloader ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_BTL_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_BTL_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 Application ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_APP_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_APP_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	@ECHO "Verifying & Downloading WizFi250 DCT ..."
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "verify_image_checksum %FILE_DES_DCT_ELF%" -c shutdown 2>&1 && echo No changes detected || openocd-all-brcm-libftdi.exe %OCD_CFG% -c "flash write_image erase %FILE_DES_DCT_ELF%" -c shutdown >> openocd_log.txt 2>&1 && echo Download complete || echo "**** OpenOCD failed****"
	goto END
)

IF "%1" == "jt-sf-erase" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sferase" -c shutdown 2>&1
	goto END
)

IF "%1" == "jt-sf-read" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sfread %2 %3 %4" -c shutdown 2>&1
	goto END
)

IF "%1" == "jt-sf-dump" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sfdump %2 %3 %4" -c shutdown 2>&1
	goto END
)

IF "%1" == "jt-sf-write-path" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sfwrite %2 %3 %4" -c shutdown 2>&1
	goto END
)

IF "%1" == "jt-sf-write-dev" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sfwrite_simple %2" -c shutdown 2>&1
	goto END
)

IF "%1" == "jt-sf-write-all" (
	openocd-all-brcm-libftdi.exe %OCD_CFG% -c "source wizfi.tcl" -c "sfwrite %FILE_DES_APP_BIN% %FILE_DES_DCT_BIN% %FILE_DES_OTA_BIN%" -c shutdown 2>&1
	goto END
)


@ECHO (Examples)
@ECHO  WizFi-OCD jt-if-unlock
@ECHO  WizFi-OCD jt-if-lock
@ECHO  WizFi-OCD jt-if-erase
@ECHO  WizFi-OCD jt-if-view
@ECHO  WizFi-OCD jt-if-write-bl
@ECHO  WizFi-OCD jt-if-write-dct
@ECHO  WizFi-OCD jt-if-write-app
@ECHO  WizFi-OCD jt-if-write-all
@ECHO  WizFi-OCD jt-sf-erase
@ECHO  WizFi-OCD jt-sf-read (SF Addr) (Length) (Alignment[1/2/4])
@ECHO  WizFi-OCD jt-sf-dump (SF Addr) (Length) (Dump Path)
@ECHO  WizFi-OCD jt-sf-write-path (APP Path) (DCT Path) (OTA Path)
@ECHO  WizFi-OCD jt-sf-write-dev (APP Name)
@ECHO  WizFi-OCD jt-sf-write-all
@ECHO  WizFi-OCD ua-if-write-dct
@ECHO  WizFi-OCD ua-if-write-app
@ECHO  WizFi-OCD daemon
@ECHO  WizFi-OCD -c "mdb 0x8000000 100" -c shutdown

:END
