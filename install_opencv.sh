###############################################################################
#	script to install opencv version 3.3.1
#	by Mark Sproul
###############################################################################
###	Mar 27,	2020	<MLS> Started on install script
###	Mar 27,	2020	<MLS> Working!!!!!
###	Jul  6,	2020	<MLS> Install of 3.3.1 failed on Raspberry-Pi 4
###	Jul  7,	2020	<MLS> Now will install 3.3.1 or 3.2.0, which ever is present
###	Jan 25,	2021	<MLS> Took 5 hours on a Jetson-Nano
###	Feb  8,	2021	<MLS> Added system update
###############################################################################

#	https://opencv.org/releases/page/4/
LOGFILENAME="opencvinstall-log.txt"
echo  "*******************************************************" >> $LOGFILENAME
echo -n "Start time=" >> $LOGFILENAME
date  >> $LOGFILENAME
clear
echo "********************************************************************"
echo "This script installs opencv version 3.3.1 on a linux machine"
echo "WARNING: The build portion can easily take 4 hours on an R-Pi!!!!"
echo "WARNING: It took 5 hours on a Jetson-nano!!!!"
echo "********************************************************************"
echo -n "Hit return to continue, ^C to abort"
read WAITINPUT

echo "First we are going to do a system update"
echo -n "Hit return to continue, ^C to abort"
read WAITINPUT
echo "	>sudo apt-get update"
echo -n "Hit return to continue, ^C to abort"
read WAITINPUT
sudo apt-get update


echo "Installing libjpeg-dev"
echo "Installing libjpeg-dev"  >> $LOGFILENAME
sudo apt-get install libjpeg-dev


echo "Checking for required programs..."
echo "Checking for make and cmake"
MAKE_VERSION_LNCNT=`make -version | wc -l`
CMAKE_VERSION_LNGCNT=`cmake -version | wc -l`

if [ $MAKE_VERSION_LNCNT -ge 1 ]
then
	echo "make is installed"
	echo "make is installed" >> $LOGFILENAME
else
	echo "make is missing!!!!!!!!!!!!!!"
	echo "make is missing!!!!!!!!!!!!!!"  >> $LOGFILENAME
	exit
fi

OPENCV_DIR="opencv"
mkdir $OPENCV_DIR
if [ -d $OPENCV_DIR ]
then
	echo "Successfully created $OPENCV_DIR directory"
else
	echo "Failed to create $OPENCV_DIR directory"
	exit
fi
cd $OPENCV_DIR
echo "Installing libgtk2.0-dev"
echo "Installing libgtk2.0-dev"  >> $LOGFILENAME

sudo apt-get install libgtk2.0-dev


if [ $CMAKE_VERSION_LNGCNT -ge 1 ]
then
	echo "cmake is installed"
	echo "cmake is installed" >> $LOGFILENAME
else
	echo "cmake is missing!!!!!!!!!!!!!!!!"
	echo -n "Do you want to install cmake? [y/n]"
	read WAITINPUT
	if [ $WAITINPUT = "y" ]
	then
		echo "Installing cmake with apt-get"
		echo "Installing cmake with apt-get" >> $LOGFILENAME
		sudo apt-get install cmake
	else
		echo "Aborting, cant install without cmake."
		echo "Aborting, cant install without cmake." >> $LOGFILENAME
		exit
	fi
fi

echo -n "Hit return to continue, ^C to abort"
read WAITINPUT

##########################################################
# lets check to see which version is available

if [ -f "opencv-3.3.1.tar.gz" ]
then
	OPENCV_TARFILE="opencv-3.3.1.tar.gz"
	OPENCV_DIR="opencv-3.3.1"
	OPENCV_WGET_FILE="https://github.com/opencv/opencv/archive/3.3.1.tar.gz"
	OPENCV_REMOTE_FILE="3.3.1.tar.gz"
	ISZIP=0
elif [ -f "opencv-3.2.0.tar.gz" ]
then
	OPENCV_TARFILE="opencv-3.2.0.tar.gz"
	OPENCV_DIR="opencv-3.2.0"
	OPENCV_WGET_FILE="https://github.com/opencv/opencv/archive/3.2.0.tar.gz"
	OPENCV_REMOTE_FILE="3.2.0.tar.gz"
	ISZIP=0
else
	echo "Valid version of Opencv was not found, tyring to download!!!!!"
	echo "Getting ready to try downloading"
	OPENCV_TARFILE="opencv-3.3.1.tar.gz"
	OPENCV_DIR="opencv-3.3.1"
	OPENCV_WGET_FILE="https://github.com/opencv/opencv/archive/3.3.1.tar.gz"
	OPENCV_REMOTE_FILE="3.3.1.tar.gz"
	ISZIP=0
#	exit
fi

echo
echo
echo "Getting ready to install $OPENCV_DIR"
echo -n "Hit return to continue, ^C to abort"
read WAITINPUT

#OPENCV_TARFILE="opencv-3.4.9.zip"
#OPENCV_DIR="opencv-3.4.9"
#OPENCV_WGET_FILE="https://github.com/opencv/opencv/archive/3.4.9.zip"
#OPENCV_REMOTE_FILE="3.4.9.zip"
#ISZIP=1

#OPENCV_TARFILE="opencv-3.4.0.zip"
#OPENCV_DIR="opencv-3.4.0"
#OPENCV_WGET_FILE="https://github.com/opencv/opencv/archive/3.4.0.tar.gz"
#OPENCV_REMOTE_FILE="3.4.0.tar.gz"
#ISZIP=0




if [ -f $OPENCV_TARFILE ]
then
	echo "$OPENCV_TARFILE is present"
else

	echo "Downloading $OPENCV_WGET_FILE" >> $LOGFILENAME
	echo "Getting ready to download $OPENCV_TARFILE"
	echo -n "Hit return to continue, ^C to abort"
	read WAITINPUT
	wget $OPENCV_WGET_FILE
	mv $OPENCV_REMOTE_FILE $OPENCV_TARFILE
fi

if [ -d $OPENCV_DIR ]
then
	echo "$OPENCV_DIR already exists"
else
	if [ -f $OPENCV_TARFILE ]
	then
		if [ $ISZIP -eq 1]
		then
			echo "Unzipping $OPENCV_TARFILE"
			echo -n "Hit return to continue"
			read WAITINPUT
			unzip $OPENCV_TARFILE
		else
			echo "un-tarring $OPENCV_TARFILE"
			tar xvf $OPENCV_TARFILE
		fi
	else
		echo "Failed to find $OPENCV_TARFILE"
	fi
fi
ls -l

echo "Getting ready to build"
echo -n "Hit return to continue"
read WAITINPUT


if [ -d $OPENCV_DIR ]
then
	if [ -d "$OPENCV_DIR/CMakeFiles" ]
	then
		echo "It appears that cmake has already been run"
	else
		echo "Getting ready to run cmake"
		echo -n "Hit return to continue"
		read WAITINPUT
		time cmake $OPENCV_DIR
	fi


	if [ -f Makefile ]
	then
		echo "********************************************************"
		echo "Getting ready to run make"
		echo "WARNING: This could easily take 4 hours!!!!"
		echo "WARNING: It took 5 hours on a Jetson-nano!!!!"
		echo "********************************************************"
		echo -n "Hit return to continue"
		read WAITINPUT
		date  >> $LOGFILENAME
		time make all

		echo "Getting ready to install"
		echo -n "Hit return to continue"
		read WAITINPUT
		sudo make install
		sudo ldconfig
	else
		echo "Makefile is missing"
	fi
	date  >> $LOGFILENAME

#export LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib

#or to make the change permanent you can add /usr/local/lib to /etc/ld.so.conf
#(or something it includes) and run ldconfig as root.

fi












