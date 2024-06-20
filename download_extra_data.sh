#################################################################
#**	Jun  1,	2024	<MLS> Created download extras script
#################################################################
clear
checkFile()
{
	if [ -f $2 ]
	then
		echo -e "$1 --- File $1 is already present: $2"
	fi
}

D3_DIR="d3-celestial"
OPEN_NGC_DIR="OpenNGC"

echo	"This script downloads extra data for use by SkyTravel"
checkFile	"OpenNGC"	"$OPEN_NGC_DIR/NGC.csv"
checkFile	"OpenNGC"	"$OPEN_NGC_DIR/database_files/NGC.csv"

checkFile	"MilkyWay"	"$D3_DIR/data/milkyway.json"

echo -n "Press return to continue or Ctrl-C to abort"
read INPUT_WAIT


#git clone https://github.com/mattiaverga/OpenNGC


#################################################################
echo "*****************************************"
if [ -d $OPEN_NGC_DIR ]
then
	echo "The OpenNGC directory is present - $OPEN_NGC_DIR"
else
	echo "Downloading OpenNGC via git................."
	git clone https://github.com/mattiaverga/OpenNGC
fi


#################################################################
echo "*****************************************"
if [ -d $D3_DIR ]
then
	echo "The milkyway directory is present - $D3_DIR"
	if [ -d $D3_DIR/data ]
	then
		echo "The milkyway data folder is present"
		if [ -f $D3_DIR/data/milkyway.json ]
		then
			echo "The milkyway data file is present"
		else
			echo "Something is wrong, the folder exists but milkyway.json is missing"
		fi
	else
		echo "Something is wrong, the folder exists but the data folder is missing"
	fi
else
	echo "Downloading milkyway outline data via git................."
	git clone https://github.com/ofrohn/d3-celestial.git
fi



