#!/bin/bash
########################################################
###	Mar 26,	2022	<MLS> Created make_checkopencv.sh
#	Determine opencv version (3 or 4)
#	This is used by the AlpacaPi Makefile
########################################################
#	set the default
OPENCV_VERSION=opencv

#figure out which one is installed.
PKGCFG_LINECNT_CV3=`pkg-config --cflags opencv 2> /dev/null  | wc -l`
PKGCFG_LINECNT_CV4=`pkg-config --cflags opencv4 2> /dev/null | wc -l`

#based on the number of lines returned, pick out the version,
#prefence given to "opencv" which is opencv4
if [ $PKGCFG_LINECNT_CV4 -eq 1 ]
then
	OPENCV_VERSION=opencv4
elif [ $PKGCFG_LINECNT_CV3 -eq 1 ]
then
	OPENCV_VERSION=opencv
fi

echo "$OPENCV_VERSION"

