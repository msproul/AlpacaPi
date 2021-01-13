###############################################################################
#	script to check and install everything for alpaca on linux or raspberry pi
###	Nov 16,	2019	<MLS> Started on install script
###############################################################################
clear
#echo "Getting ready to update (sudo apt-get update)"
#echo "Press return to continue, ^C to abort"
#read INPUT_VARIABLE
#sudo apt-get update


#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.0425.tar.bz2"
#ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.1108.tar.bz2"
ASI_TAR_FILE="ASI_linux_mac_SDK_V1.14.1119.tar.bz2"

#EFW_TAR_FILE="EFW_linux_mac_SDK_V0.3.1205.tar"
EFW_TAR_FILE="EFW_linux_mac_SDK_V0.4.1022.tar.bz2"


ATIK_ZIP_FILE="AtikCamerasSDK-129.zip"

CFITS_TAR="cfitsio_latest.tar.gz"

DEVDIR='dev-mark'
cd

if [ -d $DEVDIR ]
then
	echo "$DEVDIR already exists"
else
	mkdir $DEVDIR
fi

if [ -d $DEVDIR ]
then
	echo "$DEVDIR already exists"
else
	echo "Failed to create $DEVDIR directory"
	exit
fi



echo	"Checking to see if cfitsio is installed"
echo -n "Hit return to continue"
read WAITINPUT



if [ -d $DEVDIR/cfitsio ]
then
	echo "cfitsio already installed"
else
	if [ -f $CFITS_TAR ]
	then
		if [ -d cfitsio-3.47 ]
		then
			echo "cfitsio already untared, leaving it alone"
		else
			echo "cfitsio tar file found, untaring....."
			tar -xvf $CFITS_TAR
		fi
	else
		echo "did not find $CFITS_TAR"
	fi

	if [ -d cfitsio-3.47 ]
	then
		if [ -d $DEVDIR/cfitsio ]
		then
			echo "$DEVDIR/cfitsio exists"
		else
			mkdir $DEVDIR/cfitsio
		fi
		mv cfitsio-3.47 $DEVDIR/cfitsio/
	fi

	if [ -d $DEVDIR/cfitsio ]
	then
		echo "cfitsio appears to be installed"
	else
		echo "Run the separate cfitsio installer"
	fi

fi

echo -n "Hit return to continue"
read WAITINPUT

echo	"Checking to see if openCV is installed"
echo -n "Hit return to continue"
read WAITINPUT

#check to see if opencv is installed
if [ -d /usr/include/opencv ]
then
	echo "OpenCV appears to be installed"
else
	echo "CANT FIND OpenCV"
	echo "Installing opencv"
	sudo apt-get install libopencv-dev
fi
pwd

###############################
cd

if [ -f $ASI_TAR_FILE ]
then
	echo "Moving $ASI_TAR_FILE to $DEVDIR"
	mv $ASI_TAR_FILE $DEVDIR
else
	echo "$ASI_TAR_FILE not found"
fi

if [ -f $ATIK_ZIP_FILE ]
then
	echo "Moving $ATIK_ZIP_FILE to $DEVDIR"
	mv $ATIK_ZIP_FILE $DEVDIR
else
	echo "$ATIK_ZIP_FILE not found"
fi

if [ -f $EFW_TAR_FILE ]
then
	echo "Moving $EFW_TAR_FILE to $DEVDIR"
	mv $EFW_TAR_FILE $DEVDIR
else
	echo "$EFW_TAR_FILE not found"
fi



echo -n "Hit return to continue"
read WAITINPUT


cd $DEVDIR
pwd
ls
echo -n "Hit return to continue"
read WAITINPUT

mkdir -p alpaca

if [ -d alpaca ]
then
	#	install the stuff for alpaca
	if [ -d alpaca/ASI_lib ]
	then
		echo "ASI lib appears to be installed"
	else
		echo "********************************"
		pwd
		ls
		echo -n "Hit return to continue"
		read WAITINPUT

		if [ -f $ASI_TAR_FILE ]
		then
			echo "ASI Tar file already downloaded"
		else
			echo	"Downloading ASI_linux_mac_SDK_V1 SDK"
			echo -n "Hit return to continue"
			read WAITINPUT
			wget https://astronomy-imaging-camera.com/software/$ASI_TAR_FILE
		fi
		if [ -f $ASI_TAR_FILE ]
		then
			mv $ASI_TAR_FILE alpaca
			cd alpaca
			mkdir -P ASI_lib
			mv $ASI_TAR_FILE ASI_lib
			cd ASI_lib
			tar xvf $ASI_TAR_FILE
			cd ..
			cd ..
		else
			echo "Download failed"
		fi
	fi

	if [ -d alpaca/AtikCamerasSDK ]
	then
		echo "AtikCamerasSDK lib appears to be installed"
	else
		if [ -f AtikCamerasSDK-129.zip ]
		then
			echo "ATIK SDK is already downloaded"
		else
			wget https://www.atik-cameras.com/wp-content/uploads/2019/11/AtikCamerasSDK-129.zip
		fi
		if [ -f AtikCamerasSDK-129.zip ]
		then
			echo "Unzip AtikCamerasSDK-129.zip"
			unzip AtikCamerasSDK-129.zip
			if [ -d AtikCamerasSDK/AtikCamerasSDK ]
			then
				echo "Moving AtikCamerasSDK into alpaca folder"
				mv -v AtikCamerasSDK/AtikCamerasSDK alpaca
				rm -v AtikCamerasSDK/.DS_Store
				rmdir -v AtikCamerasSDK
			else
				echo "Failed to unzip"
			fi
		fi
	fi
	#############################################################
	echo "Checking ZWO filter wheel support"
	if [ -d alpaca/EFW_linux_mac_SDK ]
	then
		echo "EFW_linux_mac_SDK is installed"
	else
		if [ -f $EFW_TAR_FILE ]
		then
			echo "$EFW_TAR_FILE is already downloaded"
		else
			echo "Downloading EFW_linux_mac_SDK"
			wget https://astronomy-imaging-camera.com/software/$EFW_TAR_FILE
		fi
		if [ -f $EFW_TAR_FILE ]
		then
			EFW_LIB_DIR="EFW_linux_mac_SDK"
			if [ -d $EFW_LIB_DIR ]
			then
				echo "$EFW_LIB_DIR exists"
			else
				echo "Creating EFW directory"
				mkdir $EFW_LIB_DIR
			fi
			mv $EFW_TAR_FILE $EFW_LIB_DIR
			cd $EFW_LIB_DIR
			echo "untaring EFW_linux_mac_SDK_V0"
			tar xvf $EFW_TAR_FILE
			if [ -f EFW_linux_mac_SDK.tar.bz2 ]
			then
				bunzip2 EFW_linux_mac_SDK.tar.bz2
				if [ -f EFW_linux_mac_SDK.tar ]
				then
					mkdir EFW_linux_mac_SDK
					mv EFW_linux_mac_SDK.tar EFW_linux_mac_SDK/
					cd EFW_linux_mac_SDK
					tar xvf EFW_linux_mac_SDK.tar
					cd ..
					mv EFW_linux_mac_SDK alpaca/
				fi
			fi
		else
			echo "Failed to downoad EFW_linux_mac_SDK_V0"
		fi

		echo "********************************"
		pwd
		echo -n "Hit return to continue"
		read WAITINPUT
		cd ..
		if [ -d $EFW_LIB_DIR ]
		then
			pwd
			echo -n "Hit return to continue"
			read WAITINPUT
			echo "Moving $EFW_LIB_DIR into alpaca directory"
			mv $EFW_LIB_DIR alpaca
		fi
		cd alpaca
		mkdir -p src
		mkdir -p Objectfiles
		mkdir -p src_discovery
	fi
fi

echo "*****************************************"
cd

echo "checking libusb "
echo -n "Hit return to continue"
read WAITINPUT
if [ -f /usr/include/libusb-1.0/libusb.h ]
then
	echo "libusb appears to be installed"
else
	echo "installing libusb "
	sudo apt-get install libusb-1.0-0-dev
fi

#sudo apt-get install libjpeg-dev


#sudo apt-get install libudev-dev
#		sudo apt-get install libi2c-dev
#
#		example: gpio readall
#		sudo apt-get install wiringpi
