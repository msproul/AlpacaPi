#!/bin/bash
###############################################################################
#	script to check and install everything for alpaca on linux or raspberry pi
###	Nov 16,	2019	<MLS> Started on install script
###	Jan 25,	2021	<MLS> Updated ZWO files
### Mar 21,	2022	<MLS> Added !/bin/bash to start of script
### Oct  1,	2022	<MLS> Added libcfitsio-dev in install script
###	Jul 20,	2024	<MLS> Removed opencv installation commands
###############################################################################
clear
echo "Getting ready to update (sudo apt-get update)"
echo "Press return to continue, ^C to abort"
read INPUT_VARIABLE
sudo apt-get update

echo "****************************************************************************"
echo "Getting ready to install libusb-1.0-0-dev"
sudo apt-get install libusb-1.0-0-dev

echo "****************************************************************************"
echo "Getting ready to install libudev-dev"
sudo apt-get install libudev-dev


echo "****************************************************************************"
echo "Getting ready to install libi2c-dev"
sudo apt-get install libi2c-dev

echo "****************************************************************************"
echo "Getting ready to install libcfitsio-dev"
sudo apt-get install libcfitsio-dev




#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.0425.tar.bz2"
#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.1108.tar.bz2"
#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.1119.tar.bz2"
#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.1227.tar.bz2"
ASI_TAR_FILE="ASI_linux_mac_SDK_V1.16.3.tar.bz2"

#EFW_TAR_FILE="EFW_linux_mac_SDK_V0.3.1205.tar"
#EFW_TAR_FILE="EFW_linux_mac_SDK_V0.4.1022.tar.bz2"
#EFW_TAR_FILE="EFW_linux_mac_SDK_V1.5.0615.tar.bz2"
EFW_TAR_FILE="EFW_linux_mac_SDK_V1.5.0615.tar.bz2"


EFW_LIB_DIR="EFW_linux_mac_SDK"
ATIK_RULES_FILE="99-atik.rules"
TOUP_ZIP_FILE="toupcamsdk.zip"


ATIK_ZIP_FILE="AtikCamerasSDK-129.zip"

CFITS_TAR="cfitsio_latest.tar.gz"

COPY="cp -p -v"
DEV_DIR="dev-mark"


##################################################################
checkfiles()
{
	echo "*******************************************************"
	echo "Checking to make sure all files are present"


	if [ -f $ASI_TAR_FILE ]
	then
		echo	"ASI Library file present and accounted for"
	else
		echo	"Can't find $ASI_TAR_FILE"
	#	exit
	fi
	if [ -f $EFW_TAR_FILE ]
	then
		echo	"EFW Library file present and accounted for"
	else
		echo	"Can't find $EFW_TAR_FILE"
	#	exit
	fi


	if [ -f $CFITSIO_FILE ]
	then
		echo	"cfitios file present and accounted for"
	else
		echo	"Can't find $CFITSIO_FILE"
	#	exit
	fi

	if [ -f $ATIK_LIB_FILE ]
	then
		echo	"ATIK file present and accounted for"
	else
		echo	"Can't find $ATIK_LIB_FILE"
	#	exit
	fi
	if [ -f $ATIK_RULES_FILE ]
	then
		echo	"ATIK rules file present and accounted for"
	else
		echo	"Can't find $ATIK_RULES_FILE"
	#	exit
	fi


	if [ -f $TOUP_ZIP_FILE ]
	then
		echo	"Toupcam file present and accounted for"
	else
		echo	"Can't find $TOUP_ZIP_FILE"
	#	exit
	fi
	echo	"------------------------------------------------------------------"
}

###############################################################################
setupDirectories()
{
#	echo	"*******************************************************"
#	echo	"Setting up directories"
#	echo -n "Hit return to continue"
#	read WAITINPUT
#	cd

#	cd
#	mkdir -p $DEV_DIR
#	mkdir -p $DEV_DIR/alpaca
#	mkdir -p $DEV_DIR/alpaca/src
#	mkdir -p $DEV_DIR/alpaca/src_discovery
#	mkdir -p $DEV_DIR/alpaca/Objectfiles
#	mkdir -p $DEV_DIR/cfitsio
#	mkdir -p $DEV_DIR/MLS_Library
	echo	"------------------------------------------------------------------"

}

###############################################################################
setupCfitsio()
{
	echo	"*******************************************************"
	echo	"Checking to see if cfitsio is installed"
	echo -n "Hit return to continue"
#	read WAITINPUT



	if [ -d $DEV_DIR/cfitsio/cfitsio-3.47 ]
	then
		echo "cfitsio already installed"
	else
		cd
		echo "looking for setup/$CFITS_TAR"
		if [ -f setup/$CFITS_TAR ]
		then
			mkdir -p $DEV_DIR/cfitsio/
			$COPY setup/$CFITS_TAR $DEV_DIR/cfitsio/
			pwd
			cd $DEV_DIR/cfitsio/
			pwd
			if [ -d cfitsio-3.47 ]
			then
				echo "cfitsio already untared, leaving it alone"
			else
				# it should be here
				if [ -f $CFITS_TAR ]
				then
					echo "cfitsio tar file found, untaring....."
					tar -xvf $CFITS_TAR
				else
					echo "For some unknown reason, the tar file is missing."
				fi
			fi
		else
			echo "did not find $CFITS_TAR"
		fi

#		if [ -d cfitsio-3.47 ]
#		then
#			if [ -d $DEV_DIR/cfitsio ]
#			then
#				echo "$DEV_DIR/cfitsio exists"
#			else
#				mkdir $DEV_DIR/cfitsio
#			fi
#			$COPY cfitsio-3.47 $DEV_DIR/cfitsio/
#		fi
#
#		if [ -d $DEV_DIR/cfitsio ]
#		then
#			echo "cfitsio appears to be installed"
#		else
#			echo "Run the separate cfitsio installer"
#		fi

	fi
	echo	"------------------------------------------------------------------"
}


###############################################################################
setupASI()
{
	echo	"*******************************************************"
	echo	"Setting up ASI library"
	echo -n "Hit return to continue"
	read WAITINPUT

	cd

	#	install the stuff for alpaca
	if [ -d $DEV_DIR/alpaca/ASI_lib/lib ]
	then
		echo "ASI_lib appears to be installed"
	else
		echo "Installing ASI library"
		echo -n "Hit return to continue"
		read WAITINPUT

		if [ -f setup/$ASI_TAR_FILE ]
		then
			echo "ASI Tar file already downloaded"
		else
			echo	"Downloading ASI_linux_mac_SDK_V1 SDK"
			echo -n "Hit return to continue"
			read WAITINPUT
			wget https://astronomy-imaging-camera.com/software/$ASI_TAR_FILE
		fi
		if [ -f setup/$ASI_TAR_FILE ]
		then
			mkdir -p $DEV_DIR/
			mkdir -p $DEV_DIR/alpaca
			mkdir -p $DEV_DIR/alpaca/ASI_lib


			$COPY setup/$ASI_TAR_FILE $DEV_DIR/alpaca/ASI_lib/
			cd
			cd $DEV_DIR/alpaca/ASI_lib
			tar xvf $ASI_TAR_FILE
		else
			echo "Download failed"
		fi
	fi
	cd
	echo	"------------------------------------------------------------------"
}

#############################################################
setupEFW()
{
	echo	"*******************************************************"
	echo	"Checking ZWO filter wheel support"
	echo -n "Hit return to continue"
	read WAITINPUT

	cd

	if [ -d $DEV_DIR/alpaca/EFW_linux_mac_SDK ]
	then
		echo "EFW_linux_mac_SDK is installed"
	else
		if [ -f setup/$EFW_TAR_FILE ]
		then
			echo "$EFW_TAR_FILE is already downloaded"
		else
			echo "Downloading EFW_linux_mac_SDK"
		#	wget https://astronomy-imaging-camera.com/software/$EFW_TAR_FILE
		fi
		if [ -f setup/$EFW_TAR_FILE ]
		then
			mkdir -p $DEV_DIR
			mkdir -p $DEV_DIR/alpaca

			if [ -d $DEV_DIR/alpaca/$EFW_LIB_DIR ]
			then
				echo "$EFW_LIB_DIR exists"
			else
				echo "Creating EFW directory"
				mkdir -p $DEV_DIR/alpaca/$EFW_LIB_DIR
				$COPY setup/$EFW_TAR_FILE $DEV_DIR/alpaca/$EFW_LIB_DIR
				cd
				cd $DEV_DIR/alpaca/$EFW_LIB_DIR
				pwd
				echo "untaring EFW_linux_mac_SDK_V0"
				tar xvf $EFW_TAR_FILE
#				if [ -f EFW_linux_mac_SDK.tar.bz2 ]
#				then
#					bunzip2 EFW_linux_mac_SDK.tar.bz2
#					if [ -f EFW_linux_mac_SDK.tar ]
#					then
#						mkdir EFW_linux_mac_SDK
#						$COPY EFW_linux_mac_SDK.tar EFW_linux_mac_SDK/
#						cdsetupASI EFW_linux_mac_SDK
#						tar xvf EFW_linux_mac_SDK.tar
#						cd ..
#						$COPY EFW_linux_mac_SDK alpaca/
#					fi
#				fi
			fi
		else
			echo "Failed to download EFW_linux_mac_SDK_V0"
		fi

	fi
	echo	"------------------------------------------------------------------"
}


###############################################################################
setupATIK()
{
	echo	"*******************************************************"
	echo	"Setting up ATIK libraries"
	echo -n "Hit return to continue"
	read WAITINPUT

	cd

	if [ -d $DEV_DIR/alpaca/AtikCamerasSDK ]
	then
		echo "AtikCamerasSDK lib appears to be installed"
	else
		if [ -f setup/$ATIK_ZIP_FILE ]
		then
			echo "Copying $ATIK_ZIP_FILE to $DEV_DIR"
			$COPY setup/$ATIK_ZIP_FILE $DEV_DIR/alpaca

			cd $DEV_DIR/alpaca
			if [ -f $ATIK_ZIP_FILE ]
			then
				echo "Unzip $ATIK_ZIP_FILE"
				unzip $ATIK_ZIP_FILE
				if [ -d AtikCamerasSDK ]
				then
					echo "Unzip success"
					if [ -d AtikCamerasSDK/AtikCamerasSDK ]
					then
						cd AtikCamerasSDK/AtikCamerasSDK
						mv -v * ../
						rm -v .DS_Store
						cd ..
						rm -v .DS_Store
						rmdir -v AtikCamerasSDK
					#	$COPY AtikCamerasSDK/AtikCamerasSDK alpaca
					fi
				else
					echo "Failed to unzip"
				fi
			fi
		else
			echo "$ATIK_ZIP_FILE not found"
		fi
	fi
	echo	"------------------------------------------------------------------"
}

#############################################################
setupTOUP()
{
	echo	"*******************************************************"
	echo	"Checking Toupcam support"
	echo -n "Hit return to continue"
	read WAITINPUT

	cd
	if [ -d $DEV_DIR/toupcamsdk/linux ]
	then
		echo "toupcamsdk lib appears to be installed"
	else
		if [ -f setup/$TOUP_ZIP_FILE ]

		then
			mkdir -p $DEV_DIR/alpaca/toupcamsdk
			echo "Copying $TOUP_ZIP_FILE to $DEV_DIR/alpaca/toupcamsdk"
			$COPY setup/$TOUP_ZIP_FILE $DEV_DIR/alpaca/toupcamsdk/

			cd $DEV_DIR/alpaca/toupcamsdk
			if [ -f $TOUP_ZIP_FILE ]
			then
				echo "Unzip $TOUP_ZIP_FILE"
				unzip $TOUP_ZIP_FILE
				cd ..
				if [ -d toupcamsdk ]
				then
					echo "Unzip success"
				else
					echo "Failed to unzip"
				fi
			else
				echo "For some unknown reason, $TOUP_ZIP_FILE is missing."
			fi
		else
			echo "$TOUP_ZIP_FILE not found"
		fi
	fi
	echo	"------------------------------------------------------------------"

}

##################################################################
setupJPEGlib()
{
	echo	"*******************************************************"
	echo "Getting ready to install libjpeg-dev"
	sudo apt-get install libjpeg-dev

}

###############################################################################
setupLibUSB()
{
	echo	"*******************************************************"
	echo "checking libusb "
	echo -n "Hit return to continue"
	read WAITINPUT

	if [ -f /usr/include/libusb-1.0/libusb.h ]
	then
		echo "libusb appears to be installed"
	else
		echo "installing libusb "
		sudo apt-get -y install libusb-1.0-0-dev
	fi
	echo	"------------------------------------------------------------------"
}

###############################################################################
	./install_rules.sh

	checkfiles
	setupDirectories
	setupCfitsio
	setupASI
	setupEFW
	setupATIK
	setupLibUSB
	setupTOUP

	checkfiles


		sudo apt-get -y install libudev-dev
#		sudo apt-get -y install libi2c-dev
#
#		example: gpio readall
#		sudo apt-get install wiringpi
