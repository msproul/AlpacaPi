#!/bin/bash
####################################################################
#	Script to download and install PlayerOne drivers
#	for camera and filterwheel
####################################################################
#**	May 26,	2024	<MLS> Major amount of work getting install to work on a fresh system
####################################################################
clear
echo "Installing support for PlayerOne cameras and filterwheels"

PLAYERONE_DRIVER_URL="https://player-one-astronomy.com/download/softwares/"

#https://player-one-astronomy.com/download/softwares/PlayerOne_Camera_SDK_Linux_V3.6.2.tar.gz
PLAYERONE_LATESTVER_CAM="PlayerOne_Camera_SDK_Linux_V3.6.2"
PLAYERONE_LATESTVER_CAM_TAR="PlayerOne_Camera_SDK_Linux_V3.6.2.tar.gz"

#https://player-one-astronomy.com/download/softwares/PlayerOne_FilterWheel_SDK_Linux_V1.2.0.tar.gz
PLAYERONE_LATESTVER_FW="PlayerOne_FilterWheel_SDK_Linux_V1.2.0"
PLAYERONE_LATESTVER_FW_TAR="PlayerOne_FilterWheel_SDK_Linux_V1.2.0.tar.gz"


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

	if [ -d $PLAYERONE_LATESTVER_CAM ]
	then
		echo "Latest version found: $PLAYERONE_LATESTVER_CAM"
	else
		if [ -f $PLAYERONE_LATESTVER_CAM_TAR ]
		then
			echo "Tar already present: $PLAYERONE_LATESTVER_CAM_TAR"
		else
			echo "Downloading $PLAYERONE_LATESTVER_CAM_TAR"
			wget $PLAYERONE_DRIVER_URL$PLAYERONE_LATESTVER_CAM_TAR
		fi
		############################################
		#	we have the file downloaded, uncompress
		if [ -f $PLAYERONE_LATESTVER_CAM_TAR ]
		then
			tar -xvf $PLAYERONE_LATESTVER_CAM_TAR
		else
			echo "Failed to download $PLAYERONE_LATESTVER_CAM_TAR"
			exit
		fi
	fi
	if [ -d $PLAYERONE_LATESTVER_CAM ]
	then
		echo "Version $PLAYERONE_LATESTVER_CAM is present"
		CAMERA_LIB_PATH="$PLAYERONE_DIR/$PLAYERONE_LATESTVER_CAM/lib/$PLATFORM"
	else
		echo "Something failed along the way"
	fi
	echo "Creating include_camera link"
	rm include_camera
	ln -s  $PLAYERONE_LATESTVER_CAM/include/ include_camera
	cd ..


	#~/dev-mark/alpaca/PlayerOne/PlayerOne_Camera_SDK_Linux_V3.3.0/lib
	echo "Source path is $CAMERA_LIB_PATH"
	if [ -d $CAMERA_LIB_PATH ]
	then
		echo "Copying files to $LIB_DIR"
		sudo cp -p -v $CAMERA_LIB_PATH/* $LIB_DIR
	fi

	###############################################################################################
	#	check the rules file
	PLAYER_RULES_FILE="99-player_one_astronomy.rules"
	RULES_DIR="/lib/udev/rules.d"
	if [ -f $RULES_DIR/$PLAYER_RULES_FILE ]
	then
		echo "PlayerOne rules file already installed"
		ls -l $RULES_DIR | grep -i player
	else
		echo "Looking for rules file"
		RULES_PATH="$PLAYERONE_DIR/$PLAYERONE_LATESTVER_CAM/udev/$PLAYER_RULES_FILE"
		if [ -f $RULES_PATH ]
		then
			echo "Installing rules file"
			sudo cp -v -p $PLAYERONE_DIR/$PLAYERONE_LATESTVER_CAM/udev/$PLAYER_RULES_FILE /lib/udev/rules.d/
		else
			echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
			echo "Can''t find rules file: $PLAYER_RULES_FILE"
			echo "$RULES_PATH"
			exit
		fi
	fi

	###############################################################################################
	###############################################################################################
	###############################################################################################
	###############################################################################################
	###############################################################################################
	cd $PLAYERONE_DIR
	if [ -d $PLAYERONE_LATESTVER_FW ]
	then
		echo "Latest version found: $PLAYERONE_LATESTVER_FW"
	else
		if [ -f $PLAYERONE_LATESTVER_FW_TAR ]
		then
			echo "Tar already present: $PLAYERONE_LATESTVER_FW_TAR"
		else
			echo "Downloading $PLAYERONE_LATESTVER_FW_TAR"
			wget $PLAYERONE_DRIVER_URL$PLAYERONE_LATESTVER_FW_TAR
		fi
		############################################
		#	we have the file downloaded, uncompress
		if [ -f $PLAYERONE_LATESTVER_FW_TAR ]
		then
			tar -xvf $PLAYERONE_LATESTVER_FW_TAR
		else
			echo "Failed to download $PLAYERONE_LATESTVER_FW_TAR"
			exit
		fi
	fi
	if [ -d $PLAYERONE_LATESTVER_FW ]
	then
		echo "Version $PLAYERONE_LATESTVER_FW is present"
		FILTERWHEEL_LIB_PATH="$PLAYERONE_DIR/$PLAYERONE_LATESTVER_FW/lib/$PLATFORM"
	else
		echo "Something failed along the way"
	fi
	echo "Creating include_filterwheel link"
	rm include_filterwheel
	ln -s  $PLAYERONE_LATESTVER_FW/include/ include_filterwheel
	cd ..
	######################
	echo "Source path is $FILTERWHEEL_LIB_PATH"
	if [ -d $FILTERWHEEL_LIB_PATH ]
	then
		echo "Copying files to $LIB_DIR"
		sudo cp -p -v $FILTERWHEEL_LIB_PATH/* $LIB_DIR
	fi


	ls -l $PLAYERONE_DIR

else
	echo "$PLAYERONE_DIR directory was not found"
fi
