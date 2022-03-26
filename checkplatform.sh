#!/bin/bash
########################################################
#	Determine Platform
#	This is used by the AlpacaPi Makefile
########################################################
MACHINE_TYPE=`uname -m`

if [ $MACHINE_TYPE == "x86_64" ]
then
	PLATFORM="x64"
elif [ $MACHINE_TYPE == "armv7l" ]
then
	PLATFORM="armv7"
elif [ $MACHINE_TYPE == "aarch64" ]
then
	PLATFORM="armv8"
fi

#uname -m
#echo "MACHINE_TYPE = $MACHINE_TYPE"
#echo "PLATFORM = $PLATFORM"

echo "$PLATFORM"
