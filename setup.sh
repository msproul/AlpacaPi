#!/bin/bash
#############################################################
#	AlpacaPi setup
#############################################################
#	Edit History
#############################################################
#	Dec 13, 2020	<MLS> Starting on setup for github downloads
#	Dec 14, 2020	<MLS> Working on FITS install
#	Dec 16, 2020	<MLS> FITS install working
#	Feb  9,	2021	<MLS> Added support for FLIR-SDK
#############################################################

#
#		sudo apt-get install libudev-dev
#		sudo apt-get install libopencv-dev
#		sudo apt-get install libi2c-dev
#		sudo apt-get install libjpeg-dev


###########################################################
function CheckFile
{
MYPATH=$1
FILENAME=$2
ERRSTRING=$3

#echo $1
#echo $2
#echo $3

FULLPATH="$MYPATH/$FILENAME"

#echo $FULLPATH

	if [ -d $FULLPATH ]
	then
		STATUS="Present"
		ERRSTRING=""
	else
		if [ -f $FULLPATH ]
		then
			STATUS="Present"
			ERRSTRING=""
		else
			STATUS="Missing"
			let	MISSING_COUNT=$MISSING_COUNT+1
		fi
	fi
#	echo -e "\t$FILENAME\t\t$STATUS"
	printf "\t%-24s\t%s\t%s\r\n" $FILENAME $STATUS "$ERRSTRING"
}

###########################################################
function CheckOpenCV
{
OPENCV_INSTALLED=false
OPENCV_STRING="Missing"


	if [ -f "/usr/include/opencv2/highgui/highgui_c.h" ]
	then
		OPENCV_INSTALLED=true
		OPENCV_STRING="Installed in /usr/include"
	fi

	if [ -f "/usr/local/include/opencv2/highgui/highgui_c.h" ]
	then
		OPENCV_INSTALLED=true
		OPENCV_STRING="Installed in /usr/local/include"
	fi

	printf "\t%-24s\t%s\r\n" "OpenCV" "$OPENCV_STRING"

	echo "To install opencv on Raspberry Pi"
	echo "    >sudo apt-get update"
	echo "    >sudo apt-get install libopencv-dev"

}


###########################################################
function Checksystem
{
MISSING_COUNT=0
	clear
	echo "**********************************************"
	echo "*        AlpacaPi system check               *"
	echo "*                                            *"
	echo "* It is OK to run this script multiple times *"
	echo "*                                            *"
	echo "* Not all of these libraries are required    *"
	echo "* For example, if you don't use QHY cameras  *"
	echo "* then the QHY library is not needed         *"
	echo "**********************************************"
	echo "Build tools"
	CheckFile	"/usr/bin"				"gcc"			"compiler not installed"
	CheckFile	"/usr/bin"				"g++"			"compiler not installed"
	CheckFile	"/usr/bin"				"make"			"make not installed"
	CheckFile	"/usr/bin"				"cmake"			"cmake not installed"


	echo
	echo "Local files"
	CheckFile	"./"				"Makefile"			"please re-check download"
	CheckFile	"./"				"src"				"please re-check download"
	CheckFile	"./"				"src_discovery"		"please re-check download"
	CheckFile	"./"				"src_mlsLib"		"please re-check download"
	CheckFile	"./"				"src_MoonRise"		"please re-check download"
	CheckFile	"./"				"Objectfiles"
	CheckFile	"./"				"ASI_lib"
	CheckFile	"./"				"AtikCamerasSDK"
	CheckFile	"./"				"EFW_linux_mac_SDK"	"Not required"
	CheckFile	"./"				"QHY"				"Not required"
	CheckFile	"./"				"toupcamsdk"		"Not required"

	echo
	echo "System libraries"
	CheckFile	"/usr/local/include"				"fitsio.h"

	echo
	echo "Checking for openCV (Required for images and clients)"
	CheckOpenCV

	echo "*********************************************"
}

###########################################################
function CheckFITSversion
{
	CFITSIO_PRESENT=false
	#	Version 3.47 - May 2019
	if [ -d cfitsio-3.47 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.47"
	fi

	#	Version 3.48 - Mar 2020
	if [ -d cfitsio-3.48 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.48"
	fi

	#	Version 3.49 - Aug 2020
	if [ -d cfitsio-3.49 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.49"
	fi

	#	not released yet, but just being prepared
	if [ -d cfitsio-3.50 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.50"
	fi

}

###########################################################
function UntarFitsFile
{
	echo -n "Getting ready to untar $CFITSIO_TAR, do you want to proceed [y/n]?"
	read YESNO
	if [ $YESNO == "y" ]
	then
		tar -xvf $CFITSIO_TAR
	fi
}

###########################################################
function InstallFits
{
CFITSIO_TAR="cfitsio_latest.tar.gz"


if [ -f	"/usr/local/include/fitsio.h" ]
then
	echo "cfitsio already installed"
else
	CheckFITSversion

	if  $CFITSIO_PRESENT
	then
		echo "cfitsio already un-tared, leaving it alone, version = $FITS_FOLDER"
	else
		if [ -f $CFITSIO_TAR ]
		then
			echo "cfitsio tar file found, untaring....."
			UntarFitsFile
		else
			echo "did not find $CFITSIO_TAR"
			echo -n "Would you like to try downloading the FITS support library (cfitsio) [y/n]?"
			read YESNO
			if [ $YESNO == "y" ]
			then
				echo "Downloading $CFITSIO_TAR..........."
				wget "http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/$CFITSIO_TAR"
				UntarFitsFile
			fi
		fi
	fi
	CheckFITSversion

	echo "Fits version = $FITS_FOLDER"

	if  $CFITSIO_PRESENT
	then
		echo "We are now ready to build and install cfitsio"
		echo -n "Some of this will use require super user (sudo). Would you like to proceed [y/n]?"
		read YESNO
		if [ $YESNO == "y" ]
		then
			if [ -d $FITS_FOLDER ]
			then
				cd $FITS_FOLDER
				pwd
				./configure --prefix=/usr/local
				make
				sudo make install
				sudo /sbin/ldconfig -v

				make testprog
				if [ -f testprog ]
				then
					echo "testprog Make success"
					sleep 5
					./testprog > testprog.lis
					DIFFCNT=`diff testprog.lis testprog.out | wc -l`
					echo $DIFFCNT
					if [ "$DIFFCNT" -eq "0" ]
					then
						echo "Install OK"
					else
						echo "Install Failed (testprog)"
						diff testprog.lis testprog.out | wc
					fi
					cmp testprog.fit testprog.std | wc
				else
					echo "Failed to make testprog"
				fi
			else
				echo "Could not find directory ($FITS_FOLDER)"
			fi
		fi
	fi
fi
}


##################################################################
setupJPEGlib()
{
	echo	"*******************************************************"
	echo "Getting ready to install libjpeg-dev"
	sudo apt-get install libjpeg-dev

}

###########################################################
function CheckForFLIR()
{
	if [ -d "FLIR-SDK" ]
	then
		echo "FLIR SDK present, checking for installation"
		cd FLIR-SDK
		if [ -f "install_spinnaker_mls.sh" ]
		then
			echo -n "Would you like to install FLIR support [y/n]?"
			read YESNO
			if [ $YESNO == "y" ]
			then
				echo "Running FLIR install script"
				./install_spinnaker_mls.sh
			fi
		else
			echo "FLIR install script not found"
		fi
	fi

}



##############################################################
#	Determine platform

ARMCNT=`grep ARMv7 /proc/cpuinfo | wc -l`

if [ $ARMCNT -gt 1 ]
then
	echo "Running on ARMv7"
	ISARM=true
else
	ISARM=false
fi

#install udev
sudo apt-get install libudev-dev

mkdir -p Objectfiles
Checksystem


if [ ! -f Makefile ]
then
	echo "'Makefile' is missing, please re-check download"
fi

if [ ! -d ASI_lib ]
then
	echo "ZWO ASI camera is missing (ASI_lib)"
fi

InstallFits

CheckForFLIR

Checksystem

echo "*************************************************************"
echo "Installing libusb-1.0-0-dev"
sudo apt-get install libusb-1.0-0-dev


echo "MISSING_COUNT = $MISSING_COUNT"


echo "*********************************************"
echo "NOTE: This install script is not finished"
echo "Please be patient"
echo "Last updated 12/15/2020"
echo "*********************************************"
