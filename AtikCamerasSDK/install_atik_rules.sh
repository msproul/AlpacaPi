###############################################################################
#written by Mark Sproul, Nov 2019

RULES_FILE="99-atik.rules"

if [ -f /lib/udev/rules.d/$RULES_FILE ]
then
	echo "Atik rules aready installed"
	exit
fi

if [ -f $RULES_FILE ]
then
	sudo cp $RULES_FILE /lib/udev/rules.d/
	echo "You now need to reboot"
else
	echo "Rules file ($RULES_FILE) not found"
fi
