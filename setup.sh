#!/bin/bash
#############################################################
#	AlpacaPi setup
#############################################################
#	Edit History
#############################################################
#	Dec 13, 2020	<MLS> Starting on setup for github downloads
#	Dec 14, 2020	<MLS> Working on FITS install
#############################################################



###########################################################
function checkFile
{
MYPATH=$1
FILENAME=$2

FULLPATH="$MYPATH/$FILENAME"

#echo $FULLPATH

	if [ -d $FULLPATH ]
	then
		STATUS="Present"
	else
		if [ -f $FULLPATH ]
		then
			STATUS="Present"
		else
			STATUS="Missing"
			let	MISSING_COUNT=$MISSING_COUNT+1
		fi
	fi
#	echo -e "\t$FILENAME\t\t$STATUS"
	printf "\t%-24s\t%s\r\n" $FILENAME $STATUS
}


###########################################################
function Checksystem
{
MISSING_COUNT=0
	clear
	echo "*********************************************"
	echo "             AlpacaPi system check"
	echo ""
	echo "It is OK to run this script multiple times"
	echo "*********************************************"
	echo "Local files"
	checkFile	"./"			"Makefile"
	checkFile	"./"			"src"
	checkFile	"./"			"src_discovery"
	checkFile	"./"			"src_mlsLib"
	checkFile	"./"			"src_MoonRise"
	checkFile	"./"			"Objectfiles"
	checkFile	"./"			"ASI_lib"
	checkFile	"./"			"AtikCamerasSDK"
	checkFile	"./"			"EFW_linux_mac_SDK"
	checkFile	"./"			"QHY"
	checkFile	"./"			"toupcamsdk"

	echo
	echo "System libraries"
	checkFile	"/usr/local/include"			"fitsio.h"
	checkFile	"/usr/include/opencv2/highgui"	"highgui_c.h"

	echo "*********************************************"
}

###########################################################
function CheckFITSversion
{
	CFITSIO_PRESENT=false
	if [ -d cfitsio-3.47 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.47"
	fi

	if [ -d cfitsio-3.48 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.48"
	fi

	if [ -d cfitsio-3.49 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-3.49"
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


#	Determine platform

ARMCNT=`grep ARMv7 /proc/cpuinfo | wc -l`

if [ $ARMCNT -gt 1 ]
then
	echo "Running on ARMv7"
	ISARM=true
else
	ISARM=false
fi


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

#Checksystem

echo "MISSING_COUNT = $MISSING_COUNT"


echo "*********************************************"
echo "NOTE: This install script is not finished"
echo "Please be patient"
echo "Last updated 12/14/2020"
echo "*********************************************"
