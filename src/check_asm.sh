#!/bin/sh

# Copyright (C) 2004-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
#
# This code is released under 2-clause BSD license. Please see the
# file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING

#=======================================================================
# This short test script compiles the file src_sinc.c into assembler
# output and the greps the assembler output for the fldcw (FPU Load
# Control Word) instruction which is very detrimental to the perfromance
# of floating point code.

echo -n "    Checking assembler output for bad code : "

if [ $# -ne 1 ]; then
	echo "Error : Need the name of the input file."
	exit 1
	fi

filename=$1

if [ ! -f $filename ];then
	echo "Error : Can't find file $filename."
	exit 1
	fi

count=`grep fldcw $filename | wc -l`

if [ $count -gt 0 ]; then
	echo -e "\n\nError : found $count bad instructions.\n\n"
	exit 1
	fi

echo "ok"

