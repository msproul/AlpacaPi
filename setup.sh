#############################################################
#	AlpacaPi setup
#############################################################
#	Edit History
#############################################################
#	Dec 13, 2020	<MLS> Starting on setup for github downloads
#############################################################

if [ -d src ]
then
	echo "src directory is present"
else
	echo "Source folder is missing, please re-check download"
fi
if [ -d Objectfiles ]
then
	echo "Objectfiles directory is present"
else
	mkdir Objectfiles
fi

if [ -f Makefile ]
then
	echo "Makefile is present"
else
	echo "'Makefile' is missing, please re-check download"
fi

if [ -d ASI_lib ]
then
	echo "ZWO ASI camera library is installed"
else
	echo "ZWO ASI camera is missing (ASI_lib)"
fi
