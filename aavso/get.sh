#!/bin/bash
###############################################################################
#	Target Tool script
#	This script will download the latest list of AAVSO alerts via the TargetTool
#**	Jul  2,	2021	<MLS> obs_section=all now working with curl command
#**	Aug 12,	2021	<MLS> Added checking for token file, abort if not present
#**	Sep 26,	2021	<MLS> Added logging to text file
#**	Aug 22,	2022	<MLS> Updated URL to point to targettool.aavso.org
###############################################################################
clear

#echo $0
#echo $SHELL
AAVSO_TOKEN_FILE="aavso_targettool_token.txt"

LOG_FILE="aavso_retrevial_log.txt"

TARGET_TOOL_URL="https://targettool.aavso.org/TargetTool/api/v1"

#this was the URL used during development
#TARGET_TOOL_URL="https://filtergraph.com/aavso/api/v1"


###############################################################################
# first make sure "replaceCRLF" is here
if [ -f replaceCRLF ]
then
	echo "replaceCRLF is present"
else
	if [ -f replace.c ]
	then
		echo "Compiling replace.c"
		gcc replace.c -o replaceCRLF
	else
		echo "The source code file 'replace.c' is missing, please re-check your download"
#		exit
	fi
fi



###############################################################################
if [ -f $AAVSO_TOKEN_FILE ]
then
	echo "$AAVSO_TOKEN_FILE is present"
else
	if [ -f $AAVSO_TOKEN_FILE ]
	then
		echo "$AAVSO_TOKEN_FILE is present, proceeding"
	else
		echo "You must create an account and put the aavso login token in a file called $AAVSO_TOKEN_FILE"
#		echo "Refer to https://filtergraph.com/aavso/api/index# for how to create an account."
		echo "Refer to https://targettool.aavso.org/TargetTool/default/user/register?_next=/TargetTool/default/index to create an account"
		exit
	fi
fi


###############################################################################
#	Place your TargetTool key string in a file called "aavso_targettool_token.txt"
#	Make sure it does NOT have a trailing CR/LF in the file
###############################################################################
TARGETTOOL_KEY=`cat $AAVSO_TOKEN_FILE`
API_KEY="$TARGETTOOL_KEY:api_token"
echo $API_KEY
# it is important to have the -n (no NewLine) for the base64 operation
BASE64=`echo -n $API_KEY | base64`
AUTH_STRING="Basic $BASE64"
echo $BASE64
echo $AUTH_STRING



###############################################################################
function Request
{
	URL=$1
	echo "##################################################################"
	echo "#Getting $URL"
	echo -n "#Data retrieved at:"
	date

#	echo "Arg cnt=$#" >/dev/stderr
	if [ $# -eq 2 ]
	then
		DATA=$2
		echo "#URL: $URL"
		echo "#Data: $DATA"
		# add "-D headers.txt"  to save the received headers
		curl -X GET "$URL" 							\
				-d "$DATA"							\
				-H "Authorization: $AUTH_STRING"	\
				-H "accept: application/json"   	\
				-D	"returned_header.txt"

		echo "Data string = $DATA" >/dev/stderr
	else
		# add "-D headers.txt"  to save the received headers
		curl -X GET "$URL" 							\
				-H "Authorization: $AUTH_STRING"	\
				-H "accept: application/json"
	fi
	echo
}

###############################################################################
function GetAlertList
{
	OUTPUTFILE="alerts_json.txt"
	OUTPUTFILEALL="alerts_json_all.txt"
	Request	"$TARGET_TOOL_URL/nighttime"	| ./replaceCRLF	> $OUTPUTFILE
	Request	"$TARGET_TOOL_URL/telescope"	| ./replaceCRLF	>> $OUTPUTFILE

	Request	"$TARGET_TOOL_URL/targets"		| ./replaceCRLF	>> $OUTPUTFILE

	Request	"$TARGET_TOOL_URL/nighttime"	| ./replaceCRLF	> $OUTPUTFILEALL
	Request	"$TARGET_TOOL_URL/telescope"	| ./replaceCRLF	>> $OUTPUTFILEALL
	Request	"$TARGET_TOOL_URL/targets?obs_section=all"	\
																| ./replaceCRLF	>> $OUTPUTFILEALL

#	Request	"$TARGET_TOOL_URL/targets"		 > raw-json.txt

	echo -n "Total lines in JSON response ="
	wc -l $OUTPUTFILE
	echo

	echo -n "Total target entries ="
	grep star_name $OUTPUTFILE | wc -l
	echo
	echo -n "#Finished retrieved at:"	>> $OUTPUTFILE
	date								>> $OUTPUTFILE

}

###############################################################################
echo

if [ -f replaceCRLF ]
then

	GetAlertList

	ALERT_CNT=`grep star_name $OUTPUTFILE | wc -l`

	# check for failure and try again if it failed
	if [ $ALERT_CNT -eq 0 ]
	then
		echo "Trying again"
		GetAlertList

		ALERT_CNT=`grep star_name $OUTPUTFILE | wc -l`
	fi

	CURRENT_DATE=`date`
	echo -n $CURRENT_DATE >> $LOG_FILE
	echo  "   Alert count = $ALERT_CNT" >> $LOG_FILE

else
	echo "Failed to create replaceCRLF"
	if [ -f replace.c ]
	then
		echo "The source code file 'replace.c' was found. Check gcc"
	else
		echo "The source code file 'replace.c' is missing, please re-check your download"
	fi
fi

