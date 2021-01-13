echo "Installing libraries into /usr/lib"

LIB_DIR="/usr/lib"

if [ -d "toupcamsdk" ]
then
	echo "Touptech SDK library found"

	#	this is for Raspberry Pi
	sudo cp -v toupcamsdk/linux/armhf/libtoupcam.so   $LIB_DIR/

else
	echo "Cant find touptech SDK"
fi


if [ -d AtikCamerasSDK ]
then
	echo "AtikCamerasSDK SDK library found"

	#	this is for Raspberry Pi
	sudo cp -v ~/dev-mark/alpaca/AtikCamerasSDK/lib/ARM/pi/pi3/x86/NoFlyCapture/libatikcameras.so   $LIB_DIR/

else
	echo "Cant find AtikCamerasSDK SDK"
fi
