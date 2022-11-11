#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "servo_std_defs.h"
#include "servo_observ_cfg.h"
#include "servo_motion_cfg.h"
#include "servo_mount_cfg.h"
#include "servo_mount.h"

int main(int argc, char *argv[])
{
double		currRa, currDec;

Servo_init(kOBSERV_CFG_FILE, kMOUNT_CFG_FILE, kMOTION_CFG_FILE);
Servo_unpark(); 

printf("SERVO MOVE Utility - stops mount and then moves mount relative hours and degrees\n");
printf("USAGE ->  1 15 <RET> moves +1 hour in RA and +15 degs in Dec\n");
printf("USAGE ->  No input agrs just stops mount\n");

printf("\nSERVO MOVE Utility... Stopping Mount.\n");
printf("** LST:%f\n", Servo_get_lst());
Servo_stop_axes(SERVO_BOTH_AXES);

// Only allow zero or two args
switch (argc)
{
case 1:
	// Do nothing since the mount is stopped
	break;

case 3:
	Servo_ignore_horizon(true);
	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	currRa += atof(argv[1]);
	currDec += atof(argv[2]);
	printf("** Move to relative Pos delta RA = %lf  HA = %lf Dec = %lf\n", currRa, -currRa, currDec);
	Servo_move_to_coordins(currRa, currDec, Servo_get_lat(), Servo_get_lon());
	break;

default:
	return -1;
	break;
}

return 0;
} // of main()
