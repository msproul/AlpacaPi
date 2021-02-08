###########################################################################
clear
echo "############################################"
echo "Installing rules files for"
echo "	ZWO cameras"
echo "	ZWO Filter wheels"
echo "	Atik cameras"
echo "############################################"
###########################################################################
installRules()
{
RULE_DIR=$1
RULES_FILE=$2

	if [ -f /lib/udev/rules.d/$RULES_FILE ]
	then
		echo "Rules file $RULES_FILE is already installed in /lib/udev"
	elif [ -f /etc/udev/rules.d/$RULES_FILE ]
	then
		echo "Rules file $RULES_FILE is already installed in /etc/udev"
	else
		if [ -f $RULE_DIR/$RULES_FILE ]
		then
			echo "Installing $RULES_FILE"
			sudo install $RULE_DIR/$RULES_FILE /lib/udev/rules.d
		else
			echo "Cant find $RULE_DIR/$RULES_FILE"
		fi
	fi

}

#asi.rules

installRules	"ASI_lib/lib"			"asi.rules"
installRules	"EFW_linux_mac_SDK/lib"	"efw.rules"
installRules	"AtikCamerasSDK"		"99-atik.rules"
installRules	"toupcamsdk/linux/udev"	"99-toupcam.rules"
installRules	FLIR-SDK				"40-flir-spinnaker.rules"

