
GNU ARM Toolchain  v20120313
=============
This file describes how to build the GNU ARM toolchain that is shipped with the WICED-SDK.

This Toolchain can be built for both Linux and Windows.

First you will need to download the following source code files : 

expat-2.0.1         http://sourceforge.net/projects/expat/files/expat/2.0.1/expat-2.0.1.tar.gz
gmp-5.0.4           ftp://ftp.gnu.org/gnu/gmp/gmp-5.0.4.tar.bz2
mpfr-3.1.0          http://www.mpfr.org/mpfr-3.1.0/mpfr-3.1.0.tar.bz2
binutils 2.22       ftp://sources.redhat.com/pub/binutils/releases/binutils-2.22.tar.bz2
gcc-core 4.6.3      ftp://sources.redhat.com/pub/gcc/releases/gcc-4.6.3/gcc-core-4.6.3.tar.bz2
gcc-g++ 4.6.3       ftp://sources.redhat.com/pub/gcc/releases/gcc-4.6.3/gcc-g++-4.6.3.tar.bz2
newlib 1.20.0       ftp://sources.redhat.com/pub/newlib/newlib-1.20.0.tar.gz
gdb 7.4             ftp://sources.redhat.com/pub/gdb/releases/gdb-7.4.tar.bz2
mpc 0.9             http://www.multiprecision.org/mpc/download/mpc-0.9.tar.gz


Unzip the build scripts file containing this readme file.
Place these files in the downloads directory under the directory containing this readme file.

==============================

Building on MINGW

Obtain MinGW installer from: http://sourceforge.net/projects/mingw/files/Automated%20MinGW%20Installer/mingw-get-inst/
In the install program, select the following items:
*  C++ Compiler
*  MSYS Basic System
*  MinGW Developer Toolkit
*  MSYS System Builder

Open a MinGW shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh


The built toolchain will be located in the ./install directory.


==============================

Building on Linux

To ensure the toolchain is widely compatible, it is recommended that the toolchain is build on an old version
of Linux. This prevents the final toolchain being dependent on recent versions of glibc.

Tested on Ubuntu 11.04 (Natty Narwhal) and 11.10 (Oneric Ocelot) and on Redhat Enterprise Linux 5

Ensure the following packages are installed: 
    * libncurses5-dev 
    * texinfo
    * m4
    * zlib1g-dev

Open a bash shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh

The built toolchain will be located in the ./install directory.

There may be issues that need resolving with the built toolchain:

* copy libgmp.so, libgmp.so.10, and libgmp.so.10.0.4 from ./addontools/lib  to ./install/lib

* rename ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1 to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_run

* create an executable script with the following content called ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1
	#!/bin/bash
	export LD_LIBRARY_PATH=`$0_dirname $0`/../../../../lib:$LD_LIBRARY_PATH
	${0}_run "$@"

* Copy linux "dirname" executable from /usr/bin/dirname to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_dirname

==============================

Building on OS X

Tested on OSX 10.7.4

Open a bash shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh

The built toolchain will be located in the ./install directory.

There may be issues that need resolving with the built toolchain:

* copy libgmp.so, libgmp.so.10, and libgmp.so.10.0.4 from ./addontools/lib  to ./install/lib

* rename ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1 to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_run

* create an executable script with the following content called ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1
	#!/bin/bash
	export LD_LIBRARY_PATH=`$0_dirname $0`/../../../../lib:$LD_LIBRARY_PATH
	${0}_run "$@"

* Copy linux "dirname" executable from /usr/bin/dirname to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_dirname

==============================

Patch Descriptions :

binutils-2.22.patch - From previous Yagarto toolchain
gcc-4.6.3.patch       - From previous Yagarto toolchain
gdb-7.3.3.patch       - fix for bug causing symbol lookup to not be offered - http://sourceware.org/bugzilla/show_bug.cgi?id=12648
newlib-1.20.0.patch   - libgloss does not work with ARM v6m devices (Cortex-M0 etc) - disable building it in this mode.

Patches also create a custom output directory structure, and avoid outputting un-needed files

 
==============================

Toolchain scripts originally from Yagarto ( http://www.yagarto.de )
Modified by Broadcom Inc. for WICED project.

