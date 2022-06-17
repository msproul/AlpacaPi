#!/bin/bash
# -------------------------------------------------------- #
# loadcal_bno055.sh 20210905  contributed by robofoundry   #
#                                                          #
# This script loads previously saved calibration data at   #
# system start. This automates making the BNO055 sensor    #
# operational after reboots. It can run from /etc/rc.local #
# or add it into .bashrc to ready the sensor after login.  #
#                                                          #
# example interactive mode:                                #
# $ ./loadcal_bno055.sh                                    #
#                                                          #
# example "silent" non-interactive mode:                   #
# $ ./loadcal_bno055.sh silent                             #
#                                                          #
# example "silent" mode without any console output:        #
# $ ./loadcal_bno055.sh silent > /dev/null 2>&1            #
# -------------------------------------------------------- #
BINPATH=/home/pi/pi-bno055              # path to getbno055 binary
CALFILE=/home/pi/pi-bno055-conf/cal.cfg # path to calibration file
I2CADDR="0x28"                          # Sensor I2C address
# --------------------------------------------------------
# Verify the getbno055 program and calibration file path
# --------------------------------------------------------
[[ ! -e "$BINPATH/getbno055" ]] && { echo "No getbno055 program in $BINPATH, exiting..." >&2 ; exit 127; }
[[ ! -e "$CALFILE" ]] && { echo "No calibration file $CALFILE found, exiting..." >&2 ; exit 127; }

# --------------------------------------------------------
# Check if non-interactive "silent" mode is requested
# --------------------------------------------------------
silentmode=false
if [[ "$1" == silent ]]; then
   echo "silent"
   silentmode=true
else
   echo "user interaction mode"
fi

# --------------------------------------------------------
# Show sensor settings information before calibration load
# --------------------------------------------------------
$BINPATH/getbno055 -a $I2CADDR -t inf -v
if [ "$silentmode" = false ] ; then
   read -n1 -r -p "Press any key to continue..." key
fi

# --------------------------------------------------------
# Enable sensor configuration mode
# --------------------------------------------------------
$BINPATH/getbno055 -a $I2CADDR -m config
if [ "$silentmode" = false ] ; then
   read -n1 -r -p "Changed mode to Config, Press any key to continue..." key
fi

# --------------------------------------------------------
# Load the calibration data from a previously saved file
# --------------------------------------------------------
$BINPATH/getbno055 -a $I2CADDR -l $CALFILE
if [ "$silentmode" = false ] ; then
   read -n1 -r -p "Loaded saved calibration offsets successfully. Press any key to continue..." key
fi

# --------------------------------------------------------
# Switch sensor back into its desired operations mode
# --------------------------------------------------------
$BINPATH/getbno055 -a $I2CADDR -m ndof
if [ "$silentmode" = false ] ; then
   read -n1 -r -p "Switched back to operation mode. Press any key to continue..." key
fi

# --------------------------------------------------------
# Show sensor settings information after calibration load
# --------------------------------------------------------
$BINPATH/getbno055 -a $I2CADDR -t inf -v
if [ "$silentmode" = false ] ; then
   read -n1 -r -p "See latest calibration status after load above. Press any key to exit..." key
fi
