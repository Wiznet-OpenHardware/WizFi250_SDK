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
	print "Usage ./resources_header.pl  <C file 1> <C file 2> ...";
	exit;
}


print "/* Auto-generated header file. Do not edit */\n";
print "#ifndef INCLUDED_RESOURCES_H_\n";
print "#define INCLUDED_RESOURCES_H_\n";

foreach $file (@ARGV)
{
    #open the file
    open INFILE, $file or die "cant open " . $file;
    
    @file_cont_array = <INFILE>;
    close INFILE;
    $file_cont = join('',@file_cont_array);
    
    while ( $file_cont =~ m/(const unsigned char \S+\[\d+\])/sgi )
    {
        print "extern $1;\n";
    }
    while ( $file_cont =~ m/(const char \S+\[\d+\])/sgi )
    {
        print "extern $1;\n";
    }
    
}

print "#endif /* ifndef INCLUDED_RESOURCES_H_ */\n";
