###############################################################################
#	Target Tool script
#	This script will download the latest list of AAVSO alerts via the TargetTool
###############################################################################

clear

###############################################################################
#	Place your TargetTool key string in a file called "aavso_targettool_token.txt"
#	Make sure it does NOT have a trailing CR/LF in the file
###############################################################################
TARGETTOOL_KEY=`cat aavso_targettool_token.txt`
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

	echo "#Getting $URL"
	echo -n "#Data retrieved at:"
	date
	# add "-D headers.txt"  to save the received headers
	curl -X GET "$URL" 							\
			-H "Authorization: $AUTH_STRING"	\
			-H "accept: application/json"   	\


	echo
}

###############################################################################
echo

OUTPUTFILE="alerts_json.txt"
Request	"https://filtergraph.com/aavso/api/v1/nighttime"	| ./replaceCRLF	> $OUTPUTFILE
Request	"https://filtergraph.com/aavso/api/v1/telescope"	| ./replaceCRLF	>> $OUTPUTFILE
Request	"https://filtergraph.com/aavso/api/v1/targets"		| ./replaceCRLF	>> $OUTPUTFILE

echo -n "Total lines in JSON response ="
wc -l $OUTPUTFILE
echo

echo -n "Total target entries ="
grep star_name $OUTPUTFILE | wc -l
echo



