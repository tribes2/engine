#!/usr/bin/perl
#
# This is a script to read a Visual C++ object file and convert the calls
$len = sysread(STDIN, $data, 100000);
$data =~ s/POW/pow/;
$data =~ s/ACOS/acos/;
$data =~ s/\xE8\x00\x00\x00\x00/\xE8\xFC\xFF\xFF\xFF/g;
syswrite(STDOUT, $data, $len);
