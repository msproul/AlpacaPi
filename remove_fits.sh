clear
echo "***************************************************"
echo "This will remove cfitsio from your system"
echo "The purpose of this is to get ready for a new version"
echo -n "Are you sure want to remove cfitsio library? [y/n]?"
read YESNO
if [ $YESNO == "y" ]
then

	echo "Deleting existing library files"
	echo -n "Hit return to continue:"
	read INPUT_VARIABLE

	if [ -f /usr/local/include/fitsio.h ]
	then
		sudo rm -v /usr/local/lib/libcfitsio*
		sudo rm -v /usr/local/include/fitsio*.h
		sudo rm -v /usr/local/include/drvrsmem.h
		sudo rm -v /usr/local/include/longnam.h
	fi
	echo "Deleting existing directory"
	echo -n "Hit return to continue:"
	read INPUT_VARIABLE
	rm -R -v cfitsio-*/
	rm -v cfitsio_latest.tar.gz

fi
