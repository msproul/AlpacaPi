#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "servo_std_defs.h"
#include "servo_observ_cfg.h"
#include "servo_motion_cfg.h"
#include "servo_motion.h"
#include "servo_mount_cfg.h"
#include "servo_mount.h"

int main(void)
{
int status = kSTATUS_OK; 

Servo_init(kOBSERV_CFG_FILE, kMOUNT_CFG_FILE, kMOTION_CFG_FILE);

printf("SERVO WRITE SETTING Utility - writes the current position and velocity PIDs \n");
printf("USAGE ->  <No input agrs>\n");

printf("\nSERVO WRITE SETTING Utility... writing current settings into the Roboclaw HW EEPROM.\n");

// Just use the motor0 address, it the same for both RA and Dec
status = Motion_write_settings();

if (status == kSTATUS_OK)
{
	printf(">>> Settings write sucessful\n");
}
else
{
	printf("!!! SETTINGs WRITE FAILED!\n");
}

return 0;
} // of main()

