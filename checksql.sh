#!/bin/bash
########################################################
###	Mar 26,	2022	<MLS> Created checksql.sh
#	Determine sql version, mysql or mariadb
#	This is used by the AlpacaPi Makefile
########################################################
#	set the default
SQL_VERSION=mysqlclient

#figure out which one is installed.
PKGCFG_LINECNT_MYSQL=`pkg-config --cflags mysqlclient 2>/dev/null  | wc -l`
PKGCFG_LINECNT_MARIADB=`pkg-config --cflags mariadb 2>/dev/null     | wc -l`

#based on the number of lines returned, pick out the version,
#prefence given to "mysqlclient" which is opencv3
if [ $PKGCFG_LINECNT_MYSQL -eq 1 ]
then
	SQL_VERSION=mysqlclient
elif [ $PKGCFG_LINECNT_MARIADB -eq 1 ]
then
	SQL_VERSION=mariadb
fi

echo "$SQL_VERSION"





