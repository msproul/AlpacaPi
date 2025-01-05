#!/bin/bash
#############################################################
#	AlpacaPi setup
#############################################################
#	Edit History
#############################################################
#++	Sep 14,	2023	<MLS> Created install_fits.sh
#++	Jul 18,	2024	<MLS> Updated to handle cfitsio-4.4.1
#++	Dec 31,	2024	<MLS> Updated to handle cfitsio-4.5.0
#############################################################
#	This will get the default version
#	sudo apt-get install libcfitsio-dev
#	https://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/
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

	#	Version 4.0.0 - Sep 2021
	if [ -d cfitsio-4.0.0 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-4.0.0"
	fi

	#	Version 4.1.0 - Sep 2021
	if [ -d cfitsio-4.1.0 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-4.1.0"
	fi

	if [ -d cfitsio-4.4.1 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-4.4.1"
	fi

	if [ -d cfitsio-4.5.0 ]
	then
		CFITSIO_PRESENT=true
		FITS_FOLDER="cfitsio-4.5.0"
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
function CheckForFITSIO
{
	FITS_INSTALLED=false
	FITS_LOCATION="not-found"

	if [ -f	"/usr/local/include/fitsio.h" ]
	then
		FITS_INSTALLED=true
		FITS_LOCATION="/usr/local/include"
	fi

	if [ -f	"/usr/include/fitsio.h" ]
	then
		FITS_INSTALLED=true
		FITS_LOCATION="/usr/include"
	fi
}

###########################################################
function DownloadFITStarFile
{
	if [ -f $CFITSIO_TAR ]
	then
		echo "FITS tar file already present $CFITSIO_TAR"
	else
		wget "http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio_latest.tar.gz"
	fi

}

###########################################################
function InstallFits
{
	CFITSIO_TAR="cfitsio_latest.tar.gz"

	DownloadFITStarFile

	CheckForFITSIO

	echo "FITS_INSTALLED = $FITS_INSTALLED"
if [ $FITS_INSTALLED == true ]
then
	echo "Fits located at $FITS_LOCATION"
	grep CFITSIO_VERSION $FITS_LOCATION/fitsio.h
	exit
else
	echo "We will now try to install fitsio"
fi


#if [ -f	"/usr/local/include/fitsio.h" ]
if [ $FITS_INSTALLED == true ]
then
	echo "cfitsio already installed"
else
	CheckFITSversion

	if  [ $CFITSIO_PRESENT == true ]
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

	if  [ $CFITSIO_PRESENT == true ]
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

InstallFits

