#!/bin/bash

set -e

export LIBUSB_WIN32_VER=1.2.2.0
export LIBFTDI_VER=0.19

rm -rf install libftdi-$LIBFTDI_VER libftdi-build libftdi-install libusb-win32-src-$LIBUSB_WIN32_VER openocd openocd-build openocd-install
