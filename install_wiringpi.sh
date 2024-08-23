##########################################################
# script to install wiringPi
#++	Aug 23,	2024	<MLS> Created install_wiringpi.sh
##########################################################
echo "Installation of wiringPi"

MACHINE_TYPE=`uname -m`

MACHINE_IS_ARM=false

if [ $MACHINE_TYPE == "armv7l" ]
then
	MACHINE_IS_ARM=true
fi
if [ $MACHINE_TYPE == "aarch64" ]
then
	MACHINE_IS_ARM=true
fi

if [ $MACHINE_IS_ARM == true ]
then

	if [ -d WiringPi ]
	then
		echo "WiringPi already downloaded"
	else
		git clone https://github.com/WiringPi/WiringPi
	fi


	if [ -d WiringPi ]
	then
		echo "Download successful"
		cd WiringPi
		./build
	fi
else
	echo "This does not appear to be a Raspberry Pi"
fi
