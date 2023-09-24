################################################################################
#	This script is used by alpacapi to determine what device is on what USB port
#	It became necessary in support of the Alnitak flip flat device and
#	resolving that different from the Moonlite USB devices.
#
#	You can runs this script to see what it does,
#	It gets called from the program and outputs to a file "usb_id.txt"
#	AlpacaPi then reads that file to figure out what device is on what port.
#	This is sample output when both a Moonlite focuser and an Alnitak flip-flat is installed
#
#		/dev/ttyUSB0 - FTDI_FT230X_Basic_UART_DK0DW206
#		/dev/ttyUSB1 - Optec_Inc._Flat_Fielder_A86VY5HN
#
#	script was found at
#	https://unix.stackexchange.com/questions/144029/command-to-determine-ports-of-a-device-like-dev-ttyusb0
################################################################################
#++	Sep 12,	2022	<MLS> Added documentation as what this script does
################################################################################

#!/bin/bash

for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do
    (
#		echo $sysdevpath
        syspath="${sysdevpath%/dev}"
#		echo "====$syspath"
        devname="$(udevadm info -q name -p $syspath)"
        [[ "$devname" == "bus/"* ]] && exit
        eval "$(udevadm info -q property --export -p $syspath)"
        [[ -z "$ID_SERIAL" ]] && exit
        echo "/dev/$devname - $ID_SERIAL"
    )
done
