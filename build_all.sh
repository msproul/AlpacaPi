#!/bin/bash
###############################################################################
#	Due to all of the inter dependencies of these files,
#	each one has to do a "make clean"
#	I apologize for that.
###############################################################################
###	Jan 12,	2021	<MLS> Adding machine specific build commands
###	Jan 21,	2021	<MLS> Adding skytravel to build process
###	Mar 11,	2022	<MLS> Updated ror build
### Mar 25,	2022	<MLS> Added -j4 option to use 4 CPUs
###	Oct  1,	2022	<MLS> Updated for openCV4
###############################################################################
clear
date
RASPPI=false
PI64=false
OPENCV_V3_OK=false
OPENCV_V4_OK=false
CORES="-j4"

FILE_SEP_NAME="xxxxxxxxxxxxxxx"


LOGFILENAME="AlpacaPi_buildlog.txt"

mkdir -p Objectfiles

rm $LOGFILENAME

touch $FILE_SEP_NAME
echo "*******************************************" >> $LOGFILENAME
echo -n "Start time = " >> $LOGFILENAME
date >> $LOGFILENAME


#################################################
MACHINE=`uname -m`

echo "Machine is $MACHINE"

if [ $MACHINE == "x86_64" ]
then
	echo "Running on 64 bit X86" >> $LOGFILENAME
	PLATFORM="Linux x86_64"
	RASPPI=false
	PI64=false

	CORES="-j10"
fi

if [ $MACHINE == "aarch64" ]
then
	echo "Running on 64 bit Raspberry Pi" >> $LOGFILENAME
	RASPPI=true
	PI64=true
fi

if [ $MACHINE == "armv7l" ]
then
	RASPPI=true
fi
if [ $MACHINE == "armv8" ]
then
	RASPPI=true
fi
echo "Running on $MACHINE" >> $LOGFILENAME

#################################################
OPENCV_INCLUDE="/usr/include/opencv"

if [ -d $OPENCV_INCLUDE ]
then
	OPENCV_V3_OK=true
	echo "Open CV found at $OPENCV_INCLUDE" >> $LOGFILENAME
else

	echo "$OPENCV_INCLUDE not found" >> $LOGFILENAME
fi

#	check a second location for opencv
if [ -d "/usr/local/include/opencv" ]
then
	OPENCV_INCLUDE="/usr/local/include/opencv"
	OPENCV_V3_OK=true
	echo "Open CV found at $OPENCV_INCLUDE" >> $LOGFILENAME
fi

if [ -d "/usr/include/opencv2" ]
then
	OPENCV_INCLUDE="/usr/include/opencv2"
	OPENCV_V3_OK=true
	echo "Open CV found at $OPENCV_INCLUDE" >> $LOGFILENAME
fi

if [ -d "/usr/include/opencv4" ]
then
	OPENCV_INCLUDE="/usr/include/opencv4"
	OPENCV_V4_OK=true
	echo "Open CV Version 4 found at $OPENCV_INCLUDE"
	echo "Open CV Version 4 found at $OPENCV_INCLUDE" >> $LOGFILENAME
fi

if [ -d "/usr/local/include/opencv4" ]
then
	OPENCV_INCLUDE="/usr/local/include/opencv4"
	OPENCV_V4_OK=true
	echo "Open CV Version 4 found at $OPENCV_INCLUDE"
	echo "Open CV Version 4 found at $OPENCV_INCLUDE" >> $LOGFILENAME
fi


#################################################
#	check for JETSON
if [ -f "/sys/firmware/devicetree/base/model" ]
then
	PLATFORM=`cat /sys/firmware/devicetree/base/model`
	if [[ $PLATFORM == *"Jetson"* ]]; then
		JETSON=true
	fi
fi
echo "Platform = $PLATFORM"

echo "*************************************************** making client"
rm -f client
make clean >/dev/null; make $CORES client  >/dev/null
if [ -f "client" ]
then
	echo "client made OK"
else
	echo "client failed"
fi
########################################################################
# if openCV is present, we can compile the clients
if $OPENCV_V3_OK
then
	rm -f camera
	rm -f domectrl
	rm -f focuser
	rm -f rorpi
	rm -f skytravel
	rm -f switch
	echo "Building client apps (openCV 3)" >> $LOGFILENAME

	echo "*************************************************** making camera"	>/dev/stderr
	make clean >/dev/null; make $CORES camera | grep -v Wall

	echo "*************************************************** making domectrl"	>/dev/stderr
	make clean >/dev/null; make $CORES domectrl | grep -v Wall

	echo "*************************************************** making focuser"	>/dev/stderr
	make clean >/dev/null; make $CORES focuser | grep -v Wall

	echo "*************************************************** making rorpi"	>/dev/stderr
	make clean >/dev/null; make $CORES rorpi | grep -v Wall

	echo "*************************************************** making switch"	>/dev/stderr
	make clean >/dev/null; make $CORES switch | grep -v Wall

	echo "*************************************************** making skytravel"		>/dev/stderr
	make clean >/dev/null; make $CORES sky | grep -v Wall

if $RASPPI
then
	####################################################################
	#	These only compiles on Raspberry Pi (depends on wiringPi library)
	echo "*************************************************** making rorpi"			>/dev/stderr
	make clean >/dev/null; make $CORES rorpi | grep -v Wall

	echo "*************************************************** making calib"			>/dev/stderr
	make clean >/dev/null; make $CORES calib >/dev/null
fi

	if [ -f switch ]
	then
		echo "client switch made OK" >> $LOGFILENAME
	fi
	if [ -f focuser ]
	then
		echo "client focuser made OK" >> $LOGFILENAME
	fi
	if [ -f domectrl ]
	then
		echo "client domectrl made OK" >> $LOGFILENAME
	fi
	if [ -f camera ]
	then
		echo "client camera made OK" >> $LOGFILENAME
	fi
	if [ -f skytravel ]
	then
		echo "client skytravel made OK" >> $LOGFILENAME
	fi
else
	echo "OpenCV version 3 not found, skipping client apps for openCV-V3"
	echo "OpenCV version 3 not found, skipping client apps for openCV-V3" >> $LOGFILENAME
fi

########################################################################
# if openCV version 4 is present, we can compile the clients
if $OPENCV_V4_OK
then
	echo "We are using openCV version 4"
	rm -f camera
	rm -f domectrl
	rm -f focuser
	rm -f rorpi
	rm -f skytravel
	rm -f switch
	echo "Building client apps (openCV-4)" >> $LOGFILENAME
	echo "*************************************************** making camera"		>/dev/stderr
	make clean >/dev/null; make $CORES cameracv4 | grep -v Wall

	echo "*************************************************** making domectrl"		>/dev/stderr
	make clean >/dev/null; make $CORES domectrlcv4 | grep -v Wall

	echo "*************************************************** making focuser"		>/dev/stderr
	make clean >/dev/null; make $CORES focusercv4 | grep -v Wall

	echo "*************************************************** making alpaca server"		>/dev/stderr
	make clean >/dev/null; make $CORES picv4 | grep -v Wall

	if [ ! -f alpacapi ]
	then
		echo "*************************************************** making pi64"			>/dev/stderr
		make clean >/dev/null; make $CORES pi64 >/dev/null
	fi

	echo "*************************************************** making switch"		>/dev/stderr
	make clean >/dev/null; make $CORES switchcv4 | grep -v Wall

	echo "*************************************************** making skytravel"	>/dev/stderr
	make clean >/dev/null; make $CORES skycv4 | grep -v Wall

if $RASPPI
then
	####################################################################
	#	These only compiles on Raspberry Pi (depends on wiringPi library)
	echo "*************************************************** making rorpi"			>/dev/stderr
	make clean >/dev/null; make $CORES rorpi | grep -v Wall

	echo "*************************************************** making calibcv4"			>/dev/stderr
	make clean >/dev/null; make $CORES calibcv4 >/dev/null
fi

	if [ -f switch ]
	then
		echo "client switch made OK" >> $LOGFILENAME
	fi
	if [ -f focuser ]
	then
		echo "client focuser made OK" >> $LOGFILENAME
	fi
	if [ -f domectrl ]
	then
		echo "client domectrl made OK" >> $LOGFILENAME
	fi
	if [ -f camera ]
	then
		echo "client camera made OK" >> $LOGFILENAME
	fi
	if [ -f skytravel ]
	then
		echo "client skytravel made OK" >> $LOGFILENAME
	fi
else
	echo "OpenCV-4 not found, skipping client apps for openCV-V4"
	echo "OpenCV-4 not found, skipping client apps for openCV-V4" >> $LOGFILENAME
fi


########################################################################
if $PI64
then
	echo "Building alpacapi server for 64 bit Raspberry Pi" >> $LOGFILENAME
	echo "*************************************************** making pi64" >/dev/stderr
	make clean >/dev/null; make $CORES pi64 >/dev/null
elif $RASPPI
then
	echo "Building alpacapi server for 32 bit Raspberry Pi" >> $LOGFILENAME
	echo "*************************************************** making pi" >/dev/stderr
	make clean >/dev/null; make $CORES pi >/dev/null
else
	echo "Building alpacapi server on x86" >> $LOGFILENAME
	make clean >/dev/null; make $CORES clean  >/dev/null
	echo "*************************************************** making default" >/dev/stderr
	make >/dev/null
fi
if [ -f alpacapi ]
then
	echo "'alpacapi' server made successfully" >> $LOGFILENAME
else
	echo "***************************************************" >/dev/stderr
	ERRORLOGFILE="alpacapi-errorlog.txt"
	echo "Failed to build 'alpacapi' server !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
	echo "Failed to build alpacapi server, going to try again with log files"
	if $PI64
	then
		echo "*************************************************** making pi64" >/dev/stderr
		echo "Building alpacapi server for 64 bit Raspberry Pi"
		make clean >/dev/null; make $CORES pi64 >/dev/null 2>$ERRORLOGFILE
	elif $RASPPI
	then
		echo "*************************************************** making pi" >/dev/stderr
		echo "Building alpacapi server for 32 bit Raspberry Pi"
		make clean >/dev/null; make $CORES pi  >/dev/null 2>$ERRORLOGFILE
	else
		echo "*************************************************** making alpacapi" >/dev/stderr
		echo "Building alpacapi server on x86"
		make clean >/dev/null; make $CORES alpacapi   >/dev/null 2>$ERRORLOGFILE
		make >/dev/null
	fi
fi


if $RASPPI
then
	if [ -f alpacapi-calib ]
	then
		echo "alpacapi-calib driver made successfully" >> $LOGFILENAME
	else
		echo "Failed to build alpacapi-calib driver !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
	fi
	######################################
	if [ -f ror ]
	then
		echo "rorpi driver made successfully" >> $LOGFILENAME
	else
		echo "Failed to build rorpi driver !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
	fi
fi


ls -lt | grep -v drwxrwxr |  head -12

rm -f $FILE_SEP_NAME

if [ -f alpacapi ]
then
	echo "alpacapi server made successfully" >> $LOGFILENAME
fi


############################################
if [ -f skytravel ]
then
	echo "skytravel client made successfully" >> $LOGFILENAME
else
	echo "Failed to build skytravel client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi

############################################
if [ -f camera ]
then
	echo "Camera client made successfully" >> $LOGFILENAME
else
	echo "Failed to build Camera client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi
############################################
if [ -f domectrl ]
then
	echo "Dome controller client made successfully" >> $LOGFILENAME
else
	echo "Failed to build Dome client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi
############################################
if [ -f focuser ]
then
	echo "Focuser client made successfully" >> $LOGFILENAME
else
	echo "Failed to build Focuser client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi
############################################
if [ -f switch ]
then
	echo "Switch client made successfully" >> $LOGFILENAME
else
	echo "Failed to build Switch client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi



echo -n "End time = " >> $LOGFILENAME
date >> $LOGFILENAME

echo "Log saved as $LOGFILENAME"
date
echo "-----------------------------------------------------"
cat  $LOGFILENAME

