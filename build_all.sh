###############################################################################
#	Due to all of the inter dependencies of these files,
#	each one has to do a "make clean"
#	I apologize for that.
###############################################################################
###	Jan 12,	2021	<MLS> Adding machine specific build commands
###	Jan 21,	2021	<MLS> Adding skytravel to build process
###############################################################################

RASPPI=false
PI64=false
OPENCV_OK=false

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

if [ $MACHINE = "aarch64" ]
then
	echo "Running on 64 bit Raspberry Pi" >> $LOGFILENAME
	RASPPI=true
	PI64=true
fi

if [ $MACHINE = "armv7l" ]
then
	RASPPI=true
fi
echo "Running on $MACHINE" >> $LOGFILENAME

#################################################
OPENCV_INCLUDE="/usr/include/opencv"

if [ -d $OPENCV_INCLUDE ]
then
	OPENCV_OK=true
	echo "Open CV found at $OPENCV_INCLUDE" >> $LOGFILENAME
else

	echo "$OPENCV_INCLUDE not found" >> $LOGFILENAME
fi

#	check a second location for opencv
if [ -d "/usr/local/include/opencv" ]
then
	OPENCV_INCLUDE="/usr/local/include/opencv"
	OPENCV_OK=true
	echo "Open CV found at $OPENCV_INCLUDE" >> $LOGFILENAME
fi


#################################################
#	check for JETSON
if [ -f "/sys/firmware/devicetree/base/model" ]
then
	PLATFORM=`cat /sys/firmware/devicetree/base/model`
	if [[ $PLATFORM == *"Jetson"* ]]; then
		JETSON=true
	fi
else
	PLATFORM="unknown"
fi
echo "$PLATFORM"

echo "*************************************************** making client"
make clean client  >/dev/null

################################
# if openCV is present, we can compile the clients
if $OPENCV_OK
then
	echo "Building client apps" >> $LOGFILENAME
	echo "*************************************************** making switch"	>/dev/stderr
	make clean switch >/dev/null
	echo "*************************************************** making sky"		>/dev/stderr
	make clean sky >/dev/null
	echo "*************************************************** making focuser"	>/dev/stderr
	make clean focuser >/dev/null
	echo "*************************************************** making domectrl"	>/dev/stderr
	make clean domectrl >/dev/null
	echo "*************************************************** making camera"	>/dev/stderr
	make clean camera >/dev/null
	echo "*************************************************** making ror"	>/dev/stderr
	make clean ror >/dev/null
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
	echo "OpenCV not found, skipping client apps" >> $LOGFILENAME
fi


if $PI64
then
	echo "Building alpacapi server for 64 bit Raspberry Pi" >> $LOGFILENAME
	echo "*************************************************** making pi64" >/dev/stderr
	make clean pi64 >/dev/null
elif $RASPPI
then
	echo "Building alpacapi server for 32 bit Raspberry Pi" >> $LOGFILENAME
	echo "*************************************************** making pi" >/dev/stderr
	make clean pi >/dev/null
else
	echo "Building alpacapi server on x86" >> $LOGFILENAME
	make clean
	echo "*************************************************** making default" >/dev/stderr
	make >/dev/null
fi
if [ -f alpacapi ]
then
	echo "'alpacapi' server made successfully" >> $LOGFILENAME
else
	ERRORLOGFILE="alpacapi-errorlog.txt"
	echo "Failed to build 'alpacapi' server !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
	echo "Failed to build alpacapi server, going to try again with log files"
	if $PI64
	then
		echo "Building alpacapi server for 64 bit Raspberry Pi"
		make clean pi64 >/dev/null 2>$ERRORLOGFILE
	elif $RASPPI
	then
		echo "Building alpacapi server for 32 bit Raspberry Pi"
		make clean pi  >/dev/null 2>$ERRORLOGFILE
	else
		echo "Building alpacapi server on x86"
		make clean   >/dev/null 2>$ERRORLOGFILE
		make >/dev/null
	fi
fi


if $RASPPI
then
	##################################
	#this only compiles on Raspberry Pi (depends on wiringPi library)
	make clean calib >/dev/null
	if [ -f alpacapi-calib ]
	then
		echo "alpacapi-calib driver made successfully" >> $LOGFILENAME
	else
		echo "Failed to build alpacapi-calib driver !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
	fi
	##################################
	make clean rorpi >/dev/null
else
	make clean ror >/dev/null
fi
if [ -f ror ]
then
	echo "rorpi driver made successfully" >> $LOGFILENAME
else
	echo "Failed to build rorpi driver !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
fi


ls -lt | grep -v drwxrwxr |  head -12

rm -f $FILE_SEP_NAME

if [ -f alpacapi ]
then
	echo "alpacapi server made successfully" >> $LOGFILENAME
fi


if $OPENCV_OK
then
	##############################################
	#	lets try sky travel
	if [ -d src_skytravel ]
	then
		make clean sky >/dev/null
		if [ -f skytravel ]
		then
			echo "skytravel client made successfully" >> $LOGFILENAME
		else
			echo "Failed to build skytravel client !!!!!!!!!!!!!!!!!!!!!" >> $LOGFILENAME
		fi
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

else
	echo "OpenCV was not found so client apps were not built" >> $LOGFILENAME
fi


echo -n "End time = " >> $LOGFILENAME
date >> $LOGFILENAME


cat  $LOGFILENAME

echo "Log saved as $LOGFILENAME"
