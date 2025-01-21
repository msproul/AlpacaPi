#!/bin/bash
###############################################################################
#	Script to install all the components for qsi development
#	by Mark Sproul, April 2022
###############################################################################
###	Apr  7,	2022	<MLS> Created install_qsi.h
###	Apr  8,	2022	<MLS> Added libftd2xx installation
###	Apr  8,	2022	<MLS> Tested on Raspberry Pi 64 bit
###############################################################################
#https://qsimaging.com/drivers-software/
#https://downloads.atik-cameras.com/QSI_SDK_Linux-2024.09.05.77.tar.gz

QSI_FOLDER="qsiapi-7.6.0"
QSI_TAR_FILE="qsiapi-7.6.0.tar.gz"

#	figure out platform
MACHINE_TYPE=`uname -m`

if [ $MACHINE_TYPE == "x86_64" ]
then
	PLATFORM="x64"
	FTDI_TAR_FILE="libftd2xx-x86_64-1.4.24.tgz"
elif [ $MACHINE_TYPE == "armv7l" ]
then
	PLATFORM="armv7"
	FTDI_TAR_FILE="libftd2xx-arm-v7-hf-1.4.24.tgz"
elif [ $MACHINE_TYPE == "aarch64" ]
then
	PLATFORM="armv8"
	FTDI_TAR_FILE="libftd2xx-arm-v8-1.4.24.tgz"
fi



###############################################################################
echo "Getting ready to install libudev-dev"
echo -n "Do you want to proceed with libudev-dev? [y/n]"
read YESNO
if [ "$YESNO" = "y" ]
then
	echo "Installing libudev-dev"
	sudo apt-get install libudev-dev
else
	echo "Skipping......."
fi

FTDI_FOLDER="libftd2xx"

###############################################################################
#/home/mark/dev-mark/alpaca/libftd2xx-x86_64-1.4.24
#https://ftdichip.com/drivers/d2xx-drivers/
#https://ftdichip.com/wp-content/uploads/2021/09/libftd2xx-x86_64-1.4.24.tgz
#https://ftdichip.com/wp-content/uploads/2021/09/libftd2xx-arm-v7-hf-1.4.24.tgz
#https://ftdichip.com/wp-content/uploads/2021/09/libftd2xx-arm-v8-1.4.24.tgz

echo "Installing libftd2xx"

# check to see if the folder is already present
if [ -d $FTDI_FOLDER ]
then
	echo "$FTDI_FOLDER is already present"
else
	mkdir -p $FTDI_FOLDER
fi

if [ -d $FTDI_FOLDER ]
then

	cd $FTDI_FOLDER
	if [ -f $FTDI_TAR_FILE ]
	then
		echo "$FTDI_TAR_FILE tar file is present"
	else
		echo "Downloading QSI tar file $FTDI_TAR_FILE"
		wget "https://ftdichip.com/wp-content/uploads/2021/09/$FTDI_TAR_FILE"
	fi
	if [ -d "release" ]
	then
		echo "release directory is present"
	else
		tar -xvf $FTDI_TAR_FILE
	fi

	pwd
	if [ -d "release" ]
	then
		cd release
		echo "release directory is present, getting ready to install"
		if [ -d "build" ]
		then
			pwd
			cd build
			pwd
			#Copies the libraries to a central location.
			sudo cp libftd2xx.* /usr/local/lib
			sudo chmod 0755 /usr/local/lib/libftd2xx.so.1.4.24
			#Allows non-root access to the shared object.
			sudo ln -sf /usr/local/lib/libftd2xx.so.1.4.24 /usr/local/lib/libftd2xx.so
			cd ..
			pwd
		else
			pwd
			echo "build directory is missing"
			echo "Aborting....."
			exit
		fi
		cd ..
		pwd
	else
		echo "release directory is missing"
		echo "Aborting....."
		exit
	fi

	cd ..
	pwd
else
	echo "Failed to create folder $FTDI_FOLDER"
	echo "Aborting....."
	exit
fi


pwd
#exit

###############################################################################
if [ -d $QSI_FOLDER ]
then
	echo "$QSI_FOLDER is present"
else
	echo "$QSI_FOLDER is NOT present"
	if [ -f $QSI_TAR_FILE ]
	then
		echo "$QSI_TAR_FILE tar file is present"
	else
		echo "Downloading QSI tar file $QSI_TAR_FILE"
		wget "https://qsimaging.com/downloads/qsiapi-7.6.0.tar.gz"

		tar -xvf $QSI_TAR_FILE
	fi
fi



###############################################################################
if [ -d $QSI_FOLDER ]
then
	echo "$QSI_FOLDER is present"
	ls -l | grep qsi

	cd $QSI_FOLDER
	./configure

#	To use libftd2xx, use the --enable-libftd2xx option on the ./configure command:
#	./configure --with-ftd=ftd2xx

#	To use libftdi-1.x, use the --with-ftd=ftdi1 option:
#	./configure --with-ftd=ftdi1

#	To use the legacy libfdi-0.2 use the --with-ftd=ftdi option.  This is deprecated and not recommended.
#	The ./configure defaults to libftd2xx if no option is specified.


	make all
	sudo make install
	sudo ldconfig -v
fi
