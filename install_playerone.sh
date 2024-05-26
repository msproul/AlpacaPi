#!/bin/bash
PLAYERONE_DIR="PlayerOne"
LIB_DIR="/usr/lib"

#echo "Installing support for PlayerOne cameras and filterwheels"
MACHINE_TYPE=`uname -m`
echo $MACHINE_TYPE

PLATFORM="unknown"

if [ $MACHINE_TYPE == "x86_64" ]
then
	PLATFORM="x64"
elif [ $MACHINE_TYPE == "x86" ]
then
	PLATFORM="x86"
elif [ $MACHINE_TYPE == "armv7l" ]
then
	PLATFORM="arm32"
elif [ $MACHINE_TYPE == "aarch64" ]
then
	PLATFORM="arm64"
fi



if [ -d $PLAYERONE_DIR ]
then
	###########################################################
	echo "Installing support for PlayerOne filter wheel"
	if [ -d $PLAYERONE_DIR/PlayerOne_FilterWheel_SDK_Linux_V1.2.0 ]
	then
		echo "Found Playerone driver folder"
		#	copy the rules file
		sudo cp -v -p $PLAYERONE_DIR/PlayerOne_FilterWheel_SDK_Linux_V1.2.0/udev/99-player_one_astronomy.rules /lib/udev/rules.d/

		#	copy the library files
#		sudo cp -v -p -P $PLAYERONE_DIR/PlayerOne_FilterWheel_SDK_Linux_V1.2.0/lib/arm64/* /usr/local/bin
	else
		echo "Filter wheel files not found in $PLAYERONE_DIR"
	fi
	###########################################################
	echo "Deleting old versions"
	rm -v $LIB_DIR/libPlayerOneCamera*
	rm -v /usr/local/lib/libPlayerOneCamera*
	rm -v /usr/lib/libPlayerOneCamera*
	rm -v /usr/local/bin/libPlayerOneCamera*
	rm -v /usr/bin/libPlayerOneCamera*

	echo "Installing support for PlayerOne Camera"
	#~/dev-mark/alpaca/PlayerOne/PlayerOne_Camera_SDK_Linux_V3.3.0/lib
	CAMERA_LIB_PATH="$PLAYERONE_DIR/PlayerOne_Camera_SDK_Linux_V3.3.0/lib/$PLATFORM"
	echo "Source path is $CAMERA_LIB_PATH"
	if [ -d $CAMERA_LIB_PATH ]
	then
		echo "Copying files to $LIB_DIR"
		sudo cp -p -v $CAMERA_LIB_PATH/* $LIB_DIR
	fi
else
	echo "$PLAYERONE_DIR directory was not found"
fi
