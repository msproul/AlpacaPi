#!/bin/bash
clear
echo "Installing support for PlayerOne cameras and filterwheels"


PLAYERONE_DIR="PlayerOne"
LIB_DIR="/usr/lib"
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
	echo "$PLAYERONE_DIR already present"
else
	mkdir $PLAYERONE_DIR
fi

#https://player-one-astronomy.com/download/softwares/PlayerOne_Camera_SDK_Linux_V3.6.2.tar.gz
PLAYERONE_DRIVER_URL="https://player-one-astronomy.com/download/softwares/"
PLAYERONE_LATESTVERSION="PlayerOne_Camera_SDK_Linux_V3.6.2"
PLAYERONE_LATESTVERSION_TAR="PlayerOne_Camera_SDK_Linux_V3.6.2.tar.gz"
if [ -d $PLAYERONE_DIR ]
then
	###############################################################################################
	echo "Deleting old versions"
	rm -v $LIB_DIR/libPlayerOneCamera*
	rm -v /usr/local/lib/libPlayerOneCamera*
	rm -v /usr/lib/libPlayerOneCamera*
	rm -v /usr/local/bin/libPlayerOneCamera*
	rm -v /usr/bin/libPlayerOneCamera*

	echo "Installing support for PlayerOne Camera"
	echo "Checking for latest version"
	cd $PLAYERONE_DIR

	CAMERA_LIB_PATH="$PLAYERONE_DIR/PlayerOne_Camera_SDK_Linux_V3.3.0/lib/$PLATFORM"

	if [ -d $PLAYERONE_LATESTVERSION ]
	then
		echo "Latest version found: $PLAYERONE_LATESTVERSION"
	else
		if [ -f $PLAYERONE_LATESTVERSION_TAR ]
		then
			echo "Tar already present: $PLAYERONE_LATESTVERSION_TAR"
		else
			echo "Downloading $PLAYERONE_LATESTVERSION_TAR"
			wget $PLAYERONE_DRIVER_URL$PLAYERONE_LATESTVERSION_TAR
		fi
		############################################
		#	we have the file downloaded, uncompress
		if [ -f $PLAYERONE_LATESTVERSION_TAR ]
		then
			tar -xvf $PLAYERONE_LATESTVERSION_TAR
		else
			echo "Failed to download $PLAYERONE_LATESTVERSION_TAR"
			exit
		fi
	fi
	if [ -d $PLAYERONE_LATESTVERSION ]
	then
		echo "Version $PLAYERONE_LATESTVERSION is present"
		CAMERA_LIB_PATH="$PLAYERONE_DIR/$PLAYERONE_LATESTVERSION/lib/$PLATFORM"
	else
		echo "Something failed along the way"
	fi
	echo "Creating include_camera link"
	rm include_camera
	ln -s  $PLAYERONE_LATESTVERSION/include/ include_camera
	cd ..


	#~/dev-mark/alpaca/PlayerOne/PlayerOne_Camera_SDK_Linux_V3.3.0/lib
	echo "Source path is $CAMERA_LIB_PATH"
	if [ -d $CAMERA_LIB_PATH ]
	then
		echo "Copying files to $LIB_DIR"
		sudo cp -p -v $CAMERA_LIB_PATH/* $LIB_DIR
	fi
	ls -l $PLAYERONE_DIR

	###############################################################################################
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


else
	echo "$PLAYERONE_DIR directory was not found"
fi
