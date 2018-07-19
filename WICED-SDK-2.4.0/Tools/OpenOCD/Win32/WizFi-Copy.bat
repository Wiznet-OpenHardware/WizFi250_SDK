@ECHO OFF

::SET FILE_SRC_BTL_ELF=..\..\..\build\waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO\Binary\waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO.stripped.elf
::SET FILE_SRC_APP_ELF=..\..\..\build\wizfi_wiced-BCM9WCDUSI09\Binary\wizfi_wiced-BCM9WCDUSI09.stripped.elf
::SET FILE_SRC_DCT_ELF=..\..\..\build\wizfi_wiced-BCM9WCDUSI09\DCT.stripped.elf
::SET FILE_SRC_APP_BIN=..\..\..\build\wizfi_wiced-BCM9WCDUSI09\Binary\wizfi_wiced-BCM9WCDUSI09.bin
::SET FILE_SRC_DCT_BIN=..\..\..\build\wizfi_wiced-BCM9WCDUSI09\DCT.bin
::SET FILE_SRC_OTA_BIN=..\..\..\build\waf_ota_upgrade-BCM9WCDUSI09\Binary\waf_ota_upgrade-BCM9WCDUSI09.bin
::SET FILE_SRC_SRF_BIN=..\..\..\build\waf_sflash_write-BCM9WCDUSI09\Binary\waf_sflash_write-BCM9WCDUSI09.bin
::
::SET FILE_DES_BTL_ELF=..\FWFiles\waf_bootloader-NoOS-NoNS-BCM9WCDUSI09-SDIO.stripped.elf
::SET FILE_DES_APP_ELF=..\FWFiles\wizfi_wiced-BCM9WCDUSI09.stripped.elf
::SET FILE_DES_DCT_ELF=..\FWFiles\DCT.stripped.elf
::SET FILE_DES_APP_BIN=..\FWFiles\wizfi_wiced-BCM9WCDUSI09.bin
::SET FILE_DES_DCT_BIN=..\FWFiles\DCT.bin
::SET FILE_DES_OTA_BIN=..\FWFiles\waf_ota_upgrade-BCM9WCDUSI09.bin
::SET FILE_DES_SRF_BIN=..\FWFiles\waf_sflash_write-BCM9WCDUSI09.bin

SET FILE_SRC_BTL_ELF=..\..\..\build\waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO\Binary\waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO.stripped.elf
SET FILE_SRC_APP_ELF=..\..\..\build\wizfi_wiced-BCMUSI11\Binary\wizfi_wiced-BCMUSI11.stripped.elf
SET FILE_SRC_DCT_ELF=..\..\..\build\wizfi_wiced-BCMUSI11\DCT.stripped.elf
SET FILE_SRC_APP_BIN=..\..\..\build\wizfi_wiced-BCMUSI11\Binary\wizfi_wiced-BCMUSI11.bin
SET FILE_SRC_DCT_BIN=..\..\..\build\wizfi_wiced-BCMUSI11\DCT.bin
SET FILE_SRC_OTA_BIN=..\..\..\build\waf_ota_upgrade-BCMUSI11\Binary\waf_ota_upgrade-BCMUSI11.bin
SET FILE_SRC_SRF_BIN=..\..\..\build\waf_sflash_write-BCMUSI11\Binary\waf_sflash_write-BCMUSI11.bin

SET FILE_DES_BTL_ELF=..\FWFiles\waf_bootloader-NoOS-NoNS-BCMUSI11-SDIO.stripped.elf
SET FILE_DES_APP_ELF=..\FWFiles\wizfi_wiced-BCMUSI11.stripped.elf
SET FILE_DES_DCT_ELF=..\FWFiles\DCT.stripped.elf
SET FILE_DES_APP_BIN=..\FWFiles\wizfi_wiced-BCMUSI11.bin
SET FILE_DES_DCT_BIN=..\FWFiles\DCT.bin
SET FILE_DES_OTA_BIN=..\FWFiles\waf_ota_upgrade-BCMUSI11.bin
SET FILE_DES_SRF_BIN=..\FWFiles\waf_sflash_write-BCMUSI11.bin

SET FILE_DES2_BTL_ELF=..\FWFiles\BL.elf
SET FILE_DES2_APP_ELF=..\FWFiles\APP.elf
SET FILE_DES2_DCT_ELF=..\FWFiles\DCT.elf
SET FILE_DES2_APP_BIN=..\FWFiles\APP.bin
SET FILE_DES2_DCT_BIN=..\FWFiles\DCT.bin
SET FILE_DES2_OTA_BIN=..\FWFiles\OTA.bin


mkdir ..\FWFiles
del /Q ..\FWFiles\*.*

copy %FILE_SRC_BTL_ELF% %FILE_DES_BTL_ELF%
copy %FILE_SRC_APP_ELF% %FILE_DES_APP_ELF%
copy %FILE_SRC_DCT_ELF% %FILE_DES_DCT_ELF%
copy %FILE_SRC_APP_BIN% %FILE_DES_APP_BIN%
copy %FILE_SRC_DCT_BIN% %FILE_DES_DCT_BIN%
copy %FILE_SRC_OTA_BIN% %FILE_DES_OTA_BIN%
copy %FILE_SRC_SRF_BIN% %FILE_DES_SRF_BIN%

copy %FILE_SRC_BTL_ELF% %FILE_DES_BTL_ELF%
copy %FILE_SRC_APP_ELF% %FILE_DES_APP_ELF%
copy %FILE_SRC_DCT_ELF% %FILE_DES_DCT_ELF%
copy %FILE_SRC_APP_BIN% %FILE_DES_APP_BIN%
copy %FILE_SRC_DCT_BIN% %FILE_DES_DCT_BIN%
copy %FILE_SRC_OTA_BIN% %FILE_DES_OTA_BIN%
copy %FILE_SRC_SRF_BIN% %FILE_DES_SRF_BIN%

copy %FILE_DES_BTL_ELF% %FILE_DES2_BTL_ELF%
copy %FILE_DES_APP_ELF% %FILE_DES2_APP_ELF%
copy %FILE_DES_DCT_ELF% %FILE_DES2_DCT_ELF%
copy %FILE_DES_APP_BIN% %FILE_DES2_APP_BIN%
copy %FILE_DES_DCT_BIN% %FILE_DES2_DCT_BIN%
copy %FILE_DES_OTA_BIN% %FILE_DES2_OTA_BIN%

:END
