#!/bin/bash
########################################################
###	Mar 26,	2022	<MLS> Created make_checksql.sh
#	Determine sql version, mysql or mariadb
#	This is used by the AlpacaPi Makefile
########################################################
#	set the default
SQL_VERSION=sql_not_installed
MACHINE_IS_ARM=false



MACHINE_TYPE=`uname -m`

if [ $MACHINE_TYPE == "armv7l" ]
then
	MACHINE_IS_ARM=true
fi
if [ $MACHINE_TYPE == "aarch64" ]
then
	MACHINE_IS_ARM=true
fi


#figure out which one is installed.
PKGCFG_LINECNT_MYSQL=`pkg-config --cflags mysqlclient 2>/dev/null  | wc -l`
PKGCFG_LINECNT_MARIADB=`pkg-config --cflags mariadb 2>/dev/null     | wc -l`

#based on the number of lines returned, pick out the version,
################################################
################################################
if [ $MACHINE_IS_ARM == true ]
then

	#	if we are on arm, preference is given to mariadb
	if [ $PKGCFG_LINECNT_MYSQL -eq 1 ]
	then
		SQL_VERSION=mysqlclient
	fi

	#	if mariaDB is present, it will override
	if [ $PKGCFG_LINECNT_MARIADB -eq 1 ]
	then
		SQL_VERSION=mariadb
	fi
else
	# not on ARM, preference given to "mysqlclient"
	if [ $PKGCFG_LINECNT_MYSQL -eq 1 ]
	then
		SQL_VERSION=mysqlclient
	elif [ $PKGCFG_LINECNT_MARIADB -eq 1 ]
	then
		SQL_VERSION=mariadb
	fi
fi
echo "$SQL_VERSION"

