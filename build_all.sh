###############################################################################
#	Due to all of the inter dependencies of these files,
#	each one has to do a "make clean"
#	I apologize for that.
###############################################################################
###	Jan 12,	2021	<MLS> Adding machine specific build commands
###############################################################################

RASPPI=false
PI64=false
OPENCV_OK=false

MACHINE=`uname -m`

if [ $MACHINE = "aarch64" ]
then
	RASPPI=true
	PI64=true
fi

if [ $MACHINE = "armv7l" ]
then
	RASPPI=true
fi
echo "Running on $MACHINE"

if [ -d "/usr/include/opencv" ]
then
	OPENCV_OK=true
fi

make client

################################
# if openCV is present, we can compile the clients
if $OPENCV_OK
then
	echo "Building client apps"
#	make clean switch
#	make clean sky
#	make clean focuser
#	make clean domectrl
#	make clean camera
fi


if $PI64
then
	echo "Building alpacapi server for 64 bit Raspberry Pi"
	make clean pi64
elif $RASPPI
then
	echo "Building alpacapi server for 32 bit Raspberry Pi"
	make clean pi
else
	echo "Building alpacapi server on x86"
#	make clean
#	make
fi


if $RASPPI
then
	#this only compiles on Raspberry Pi (depends on wiringPi library)
	make clean calib
	make clean rorpi
else
	make clean ror
fi


ls -lt | grep -v drwxrwxr | grep rwxr | head -12

	if [ -f alpacapi ]
	then
		echo "alpacapi server made successfully"
	fi


if $OPENCV_OK
then
	############################################
	if [ -f camera ]
	then
		echo "Camera client made successfully"
	fi
	############################################
	if [ -f domectrl ]
	then
		echo "Dome controller client made successfully"
	fi
	############################################
	if [ -f focuser ]
	then
		echo "Focuser client made successfully"
	fi
	############################################
	if [ -f switch ]
	then
		echo "Switch client made successfully"
	fi

else
	echo "OpenCV was not found so client apps were not built"
fi
