###############################################################################
#	Driver library installation for AlpacaPi
###############################################################################
###	Mar 18,	2021	<MLS> Adding full support for QHY installation
###############################################################################
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
function Checksystem
{
MISSING_COUNT=0
	clear
	echo "**********************************************"
	echo "*        AlpacaPi library installation       *"
	echo "*                                            *"
	echo "* It is OK to run this script multiple times *"
	echo "*                                            *"
	echo "* Not all of these libraries are required    *"
	echo "* For example, if you don't use QHY cameras  *"
	echo "* then the QHY library is not needed         *"
	echo "**********************************************"
	echo
	CheckFile	"./"				"ASI_lib"
	CheckFile	"./"				"AtikCamerasSDK"	"Not required"
	CheckFile	"./"				"EFW_linux_mac_SDK"	"Not required"
	CheckFile	"./"				"FLIR-SDK"			"Not required"
	CheckFile	"./"				"QHY"				"Not required"
	CheckFile	"./"				"toupcamsdk"		"Not required"
	echo "*********************************************"
}


###########################################################
#Determine platform
ISX64=true
ISARM32=false
ISARM64=false

###########################################################
function DeterminePlatform()
{

uname -a | grep aarch64
ARMCNT=`uname -a | grep aarch64 | wc -l`
echo $ARMCNT
if [ $ARMCNT -ge 1 ]
then
	echo "Running on aarch64"
	ISARM64=true
	ISX64=false
else
	ISARM64=false
fi
uname -a | grep armv7l
ARMCNT=`uname -a | grep armv7l | wc -l`
echo $ARMCNT
if [ $ARMCNT -ge 1 ]
then
	echo "Running on armv7l"
	ISARM32=true
	ISX64=false
else
	ISARM32=false
fi

echo "Running on intel x64 : $ISX64"
echo "Running on Arm 32 bit: $ISARM32"
echo "Running on Arm 64 bit: $ISARM64"

}

###########################################################
function InstallFlir()
{
echo "***************************************************************"
FLIR_SDK_DIR="FLIR-SDK"
	if [ -d $FLIR_SDK_DIR ]
	then
		echo "FLIR SDK present, checking for installation"
		if [ -f "$FLIR_SDK_DIR/install_spinnaker_mls.sh" ]
		then
			echo -n "Would you like to install FLIR support [y/n]?"
			read YESNO
			if [ $YESNO == "y" ]
			then
				cd $FLIR_SDK_DIR
				echo "Running FLIR install script"
				./install_spinnaker_mls.sh
				cd ..
			fi
		else
			echo "FLIR install script not found"
		fi
	fi
}


###########################################################
function InstallToupTec()
{
echo "***************************************************************"
if [ -d "toupcamsdk" ]
then
	echo "Touptech SDK library found"
	echo -n "Would you like to install ToupTec support [y/n]?"
	read YESNO
	if [ $YESNO == "y" ]
	then

		#	this is for Raspberry Pi
		sudo cp -v toupcamsdk/linux/armhf/libtoupcam.so   $LIB_DIR/
	fi

else
	echo "Cant find touptech SDK"
fi
}


###############################################################################
function InstallATIK()
{
echo "***************************************************************"
ATIK_SDK_DIR="AtikCamerasSDK"
if [ -d $ATIK_SDK_DIR ]
then
	echo "AtikCamerasSDK SDK folder found"
	echo -n "Would you like to install AtikCamerasSDK support [y/n]?"
	read YESNO
	if [ $YESNO == "y" ]
	then

		if  $ISX64
		then
			ATIK_SDK_LIB_DIR=$ATIK_SDK_DIR/lib/linux
		elif  $ISARM64
		then
			ATIK_SDK_LIB_DIR=$ATIK_SDK_DIR/lib/ARM
		elif  $ISARM32
		then
			ATIK_SDK_LIB_DIR=$ATIK_SDK_DIR/lib/ARM
		else
			echo "Platform unkonwn"
			exit
		fi
		echo "Platform is '$ATIK_SDK_LIB_DIR'"

		if  $ISX64
		then
			echo "Running on X64 platform"
			if [ -d $ATIK_SDK_LIB_DIR/x64 ]
			then
				echo "Using lib/x64"
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/x64
			elif [ -d $ATIK_SDK_LIB_DIR/64 ]
			then
				echo "Using lib/64"
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/64
			fi
		elif  $ISARM64
		then
			echo "Running on ARM64 platform"
			if [ -d $ATIK_SDK_LIB_DIR/64 ]
			then
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/64
			elif [ -d $ATIK_SDK_LIB_DIR/x64 ]
			then
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/x64
			fi
		elif  $ISARM32
		then
			echo "Running on ARM32 platform"
			echo $ATIK_SDK_LIB_DIR
			if [ -d $ATIK_SDK_LIB_DIR/32 ]
			then
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/32
			elif [ -d $ATIK_SDK_LIB_DIR/x86 ]
			then
				ATIK_SDK_LIB_DIR=$ATIK_SDK_LIB_DIR/x86
			fi
		else
			exit
		fi
		echo "Platform is '$ATIK_SDK_LIB_DIR'"

		#	this is for Raspberry Pi
	#	sudo cp -v ~/dev-mark/alpaca/AtikCamerasSDK/lib/ARM/pi/pi3/x86/NoFlyCapture/libatikcameras.so   $LIB_DIR/

		sudo cp -v $ATIK_SDK_LIB_DIR/NoFlyCapture/libatikcameras.so   $LIB_DIR/

	else
		echo "Cant find AtikCamerasSDK SDK: $ATIK_SDK_DIR"
	fi
fi
}

###############################################################################
# Latest versions as of Mar 18, 2021
#		https://www.qhyccd.com/file/repository/publish/SDK/210201/sdk_linux64_21.02.01.tgz
#		https://www.qhyccd.com/file/repository/publish/SDK/210201/sdk_Arm64_21.02.01.tgz
#		https://www.qhyccd.com/file/repository/publish/SDK/210201/sdk_arm32_21.02.01.tgz
###############################################################################
function GetQHY()
{
	echo "***************************************************************"

	if [ -d $QHY_SDK_DIR ]
	then
		echo "QHY directory already exists: $QHY_SDK_DIR"
	else
		echo -n "Would you like to download QHY support [y/n]?"
		read YESNO
		if [ $YESNO == "y" ]
		then
			echo "Downloading QHY libraries"

			mkdir -p $QHY_SDK_DIR
			if [ -d $QHY_SDK_DIR ]
			then

				if  $ISX64
				then
					echo "Running on X64 platform"
				#	QHY_TAR_FILE="sdk_linux64_21.02.01.tgz"
					QHY_SUBDIR="200626"
					QHY_TAR_FILE="sdk_linux64_20.06.26.tgz"
				#	https://www.qhyccd.com/file/repository/publish/SDK/200626/sdk_linux64_20.06.26.tgz
				elif  $ISARM64
				then
					echo "Running on ARM64 platform"
				#	QHY_TAR_FILE="sdk_Arm64_21.02.01.tgz"
					QHY_SUBDIR="200626"
					QHY_TAR_FILE="sdk_Arm64_20.06.26.tgz"
				#	https://www.qhyccd.com/file/repository/publish/SDK/200626/sdk_Arm64_20.06.26.tgz
				elif  $ISARM32
				then
					echo "Running on ARM32 platform"
				#	QHY_TAR_FILE="sdk_arm32_21.02.01.tgz"
					QHY_SUBDIR="200626"
					QHY_TAR_FILE="sdk_arm32_20.06.26.tgz"
				#	https://www.qhyccd.com/file/repository/publish/SDK/200626/sdk_arm32_20.06.26.tgz
				else
					exit
				fi

				cd $QHY_SDK_DIR
				pwd
			#	wget "https://www.qhyccd.com/file/repository/publish/SDK/210201/$QHY_TAR_FILE"
				wget "https://www.qhyccd.com/file/repository/publish/SDK/$QHY_SUBDIR/$QHY_TAR_FILE"
				ls -l
				if [ -f $QHY_TAR_FILE ]
				then
					 tar -xvf $QHY_TAR_FILE
					 mv -v sdk*/* .
				else
					echo "QHY Tar file $QHY_TAR_FILE, not found"
				fi
				cd ..
				pwd
			else
				echo "Failed to create QHY directory"
			fi
		fi
	fi
}


###############################################################################
function InstallQHY()
{
	QHY_TAR_FILE="RPI34_qhyccd_V20200219_0.tgz"

	echo "***************************************************************"
	echo "$QHY_TAR_FILE"
	if [ -d $QHY_SDK_DIR ]
	then
		echo "QHY SDK folder found: $QHY_SDK_DIR"
		echo -n "Would you like to install QHY support [y/n]?"
		read YESNO
		if [ $YESNO == "y" ]
		then
			if [ -d $QHY_SDK_DIR/include ]
			then
				echo "It looks like the library is already installed"
			else
				echo "Installing QHY libraries"
				cd $QHY_SDK_DIR
				chmod 755 install.sh
				chmod 755 uninstall.sh
				sudo ./install.sh
			#	sudo cp -a -v lib/udev/rules.d/*  	/lib/udev/rules.d
			#	sudo cp -a -v etc/udev/rules.d/*  	/etc/udev/rules.d
			#	sudo cp -a -v usr/local/lib/*		/usr/local/lib
				mkdir -p include
				cp -a -v usr/local/include/*		include/
				sudo ldconfig
				cd ..
			fi
		fi
	fi
}

clear
echo "***************************************************************"

LIB_DIR="/usr/lib"

Checksystem
DeterminePlatform
echo "Installing libraries into $LIB_DIR"


InstallATIK
InstallToupTec
InstallFlir

QHY_SDK_DIR="QHY"

GetQHY
InstallQHY
