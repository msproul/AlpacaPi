clear
echo "Installing rules files for"
echo "	ZWO cameras"
echo "	ZWO Filter wheels"
echo "	Atik cameras"
###########################################################################
installRules()
{
RULE_DIR=$1
RULES_FILE=$2

	if [ -f /lib/udev/rules.d/$RULES_FILE ]
	then
		echo "Rules file $RULES_FILE is already installed"
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
exit


if [ -f /lib/udev/rules.d/asi.rules ]
then
	echo "ZWO ASI rules file is installed"
else
	if [ -f ASI_Lib/lib/asi.rules ]
	then
		echo "Installing asi.rules"
		sudo install ASI_Lib/lib/asi.rules /lib/udev/rules.d
	else
		echo "Cant find"
	fi
fi

#efw.rules
if [ -f /lib/udev/rules.d/efw.rules ]
then
	echo "ZWO Filter wheel rules file is installed"
else
	if [ -f EFW_linux_mac_SDK/lib/efw.rules ]
	then
		echo "Installing efw.rules"
		sudo install EFW_linux_mac_SDK/lib/efw.rules /lib/udev/rules.d
	fi
fi

#99-atik.rules
if [ -f /lib/udev/rules.d/99-atik.rules ]
then
	echo "Atik camera rules file is installed"
else
	if [ -f AtikCamerasSDK/99-atik.rules ]
	then
		echo "Installing 99-atik.rules"
		sudo install AtikCamerasSDK/99-atik.rules /lib/udev/rules.d

fi
fi
