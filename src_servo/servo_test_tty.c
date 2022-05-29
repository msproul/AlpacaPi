//******************************************************************************
//*	Name:			servo_test_tty.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Basic test program for servo_mount.c/h functions via stdin/out
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++ and led by Mark Sproul
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.
//*	You must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	May 15,	2022	<RNS> Initial port of Goddard test program for servo mount system
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> Removed globals that should be limited only to servo_mount.c
//*	May 20,	2022	<RNS> Added park state support 
//*****************************************************************************
// Notes:   M1 *MUST BE* connected to RA or Azimuth axis, M2 to Dec or Altitude
//*****************************************************************************
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#define _ENABLE_CONSOLE_DEBUG_
#include "ConsoleDebug.h"

#include "servo_std_defs.h"
#include "servo_mc_core.h"
#include "servo_time.h"
#include "servo_rc_utils.h"
#include "servo_mount_cfg.h"
#include "servo_mount.h"

#ifdef _ALPHA_OUT_
#define UNDER_TARGET 5.0
//#define RA_SENSOR_CHANGE 1
//#define DEC_SENSOR_CHANGE 2
//#define BOTH_SENSOR_CHANGE 3
#endif // _ALPHA_OUT_

// Danger, local globals ahead


//**************************************************************************
//**************************************************************************
//**************************************************************************
int main(void)
{
	double ra;
	double dec;
	double ha;
	double newRa;
	double newDec;
//	double syncRa, syncDec;
	double jd, lst;
	double tempRa;
	double tempDec;
	char inStr[256];
	char inChar;
	char *strPtr; 
	int done;
	int standbyMode;
	int status; 
	TYPE_MOVE state; 

	CONSOLE_DEBUG(__FUNCTION__);

	// Initialize the mount and read the config file
	CONSOLE_DEBUG("Entering Servo_init()");
	status = Servo_init("servo_mount.cfg", "servo_location.cfg");
	if (status != kSTATUS_OK)
	{
			CONSOLE_DEBUG("Servo_init() failed to initialize");
			return kERROR;
	}

	printf("\n");
	printf("\nInitialization Complete\n");
	Servo_get_pos(&ra, &dec);
	printf("\n");
	printf("Current RA = %lf   Dec = %lf\n", ra, dec);
	printf("Press <RETURN> to continue");
	getchar();

	// Move to the standby position even if below horizon

	printf("\n");
	printf("Initializing mount to standby mode\n");
	printf("Please wait...\n");
	Servo_ignore_horizon(true);
	Servo_get_standby_coordins(&ra, &dec);
	Servo_move_to_static(ra, dec);

	// Set the standby flag to TRUE
	standbyMode = true;

	// Greetings
	printf("\nServo mount test program version 1.0\n");

	done = false;
	while (!done)
	{
		Servo_get_pos(&ra, &dec);
		state = Servo_state(); 
		printf("\n");
		printf("=======================================================\n");
		printf("Current RA = %lf   Dec = %lf", ra, dec);
		printf("Park = %s   ", strPtr = Servo_get_park_state() ? "true" : "false");

		state = Servo_state(); 

		switch (state)
		{
			case PARKED:
				printf("  Mount is PARKED\n");
				break;
			case PARKING:
				printf("  Mount is PARKING\n");
				break;
			case STOPPED:
				printf("  Mount is STOPPED\n");
				break;
			case MOVING:
				printf("  Mount is MOVING\n");
				break;
			case TRACKING:
				printf("  Mount is TRACKING\n");
				break; 
		}
		// Display the current LST
		jd = Time_systime_to_jd();
		// get the sid time and convert to LST in HMS
		lst = Time_jd_to_gmst(jd);
		lst = Time_gmst_to_lst(lst, Time_get_lon());
		Time_deci_hours_to_hms(&lst);
		Time_check_hms(&lst);
		printf("LST = %lf\n", lst);
		printf("\n");
		printf("  Move to coordinates\n");
		printf("  Go to standby mode\n");
		printf("  Half the max values for acc/vel\n");
		printf("  Full the acc/vel values\n");
		printf("  Go to standby mode\n");
//		printf("  Sync the mount to sensor positions\n");
		printf("  Joystick slew\n");
		printf("  LST\n"); 
		printf("  Quit and park the mount\n");
		printf("\n");
		printf("Enter a command -> ");
		fgets(inStr, 256, stdin);
		printf("\n");

		// Grab the first char in the str 
		inChar = inStr[0];

		switch (inChar)
		{
		case '!':
			exit(-1);
			break;

		case 'l':
		case 'L':
			break;

		case 'h':
		case 'H':
			Servo_scale_vel(50, 50);
			Servo_scale_acc(50, 50);
			break;

		case 'f':
		case 'F':
			Servo_scale_vel(100, 100);
			Servo_scale_acc(100, 100);
			break;

		case 'M':
		case 'm':
			// Allow move only above the horizon
			Servo_ignore_horizon(false);

			// Check is the mount has been put in standby mode (ie. stopped)
			if (standbyMode == true)
			{
				// Get the standby coordinates (hourAngle, dec)
				Servo_get_standby_coordins(&ha, &dec);

				// Set the current position for a stopped mount
				Servo_set_static_pos(ha, dec);
			}

			// goto a given Ra and dec
			Servo_get_pos(&newRa, &newDec);
			do
			{
				tempRa = -1.0;
				printf("Current RA position is %lf Hms\n", newRa);
				printf("  RA must be between 0.0 and 23.5959\n");
				printf("  Enter a new RA target in HH.MMSSss ->");
				fgets(inStr, 256, stdin);
				// if nothing was entered
				if (strlen(inStr) == 0)
				{
					ra = newRa;
					printf("  Note: Ra is unchanged\n");
					break;
				}
				else
				{
					tempRa = atof(inStr);
				}
			} while (tempRa < 0.0 || tempRa > 23.595999);

			if (tempRa >= 0.0 && tempRa <= 23.595999)
			{
				ra = tempRa;
			}
			printf("\n");

			do
			{
				tempDec = -91.0;
				printf("Current Dec position is %lf Dms\n", newDec);
				printf("  Dec must be between +90.0 and -90.0\n");
				printf("  Enter a new Dec target in DDD.MMSSss ->");
				fgets(inStr, 256, stdin);
				// if nothing was entered
				if (strlen(inStr) == 0)
				{
					dec = newDec;
					printf("  Note: Dec is unchanged\n");
					break;
				}
				else
				{
					tempDec = atof(inStr);
				}
			} while (tempDec < -90.0 || tempDec > 90.0);
			if (tempDec >= -90.0 && tempDec <= 90.0)
			{
				dec = tempDec;
			}

			// Check the format of the base-60 value and copy to the ephem obj
			Time_check_hms(&ra);
			Time_check_hms(&dec);
			Time_hms_hours_to_deci(&ra);
			Time_hms_hours_to_deci(&dec);

			printf("\n");
			printf("Moving to object at Ra= %7.4lf  Dec= %7.4lf\n", ra, dec);

			Servo_move_to_coordins(ra, dec, Time_get_lat(), Time_get_lon());

			// definitely NOT in standby mode
			standbyMode = false;

			break;

		case 'G':
		case 'g':
			// Enable target below the horizon
			Servo_ignore_horizon(true);

			// Get the standby position
			Servo_get_standby_coordins(&ra, &dec);

			// Stop the mount at standby position
			Servo_move_to_static(ra, dec);

			// Set the standby flag to TRUE
			standbyMode = true;
			break;

#ifdef _ALPHA_OUT_
		case 'S':
		case 's':
			break;
			// Sync the mount to the home position
			Servo_ignore_horizon(true);

			// Sync the position of the mount only if sync is successful
			if (Servo_sync(ephem, &newRa, &newDec) == SS_OK)
			{
				// Get the home position from the config file
				Servo_get_sync_pos(&syncRa, &syncDec);

				// Update the position with the corrections so get the deltas
				eph__hms_hours_to_deci(&newRa);
				eph__hms_hours_to_deci(&syncRa);
				deltaRa = syncRa - newRa;
				eph__hms_hours_to_deci(&newDec);
				eph__hms_hours_to_deci(&syncDec);
				deltaDec = syncDec - newDec;

				printf("Old sensor position:\n");
				printf("  RA: %lf     Dec: %lf\n in decimal", syncRa, syncDec);
				printf("New sensor position:\n");
				printf("  RA: %lf     Dec: %lf\n in decimal", newRa, newDec);

				// Get the current position and reuse the newRa, newDec vars
				Servo_get_pos(&newRa, &newDec);
				eph__hms_hours_to_deci(&newRa);
				eph__hms_hours_to_deci(&newDec);

				// Add the deltas to the current position and set position
				newRa += deltaRa;
				newDec += deltaDec;
				eph__deci_hours_to_hms(&newRa);
				eph__deci_hours_to_hms(&newDec);
				// rns        Servo_set_pos(newRa, newDec);

				// get the sid time and convert to LST in HMS
				ephem->sid = eph__jd_to_gmst(ephem->jd);
				ephem->sid = eph__gmst_to_lst(ephem->sid, ephem->env->lon);

				// Get the "correct" sync pos and convert from HA to RA
				Servo_get_sync_pos(&newRa, &newDec);
				eph__hms_hours_to_deci(&newRa);
				newRa += ephem->sid;
				eph__deci_hours_to_hms(&newRa);
				eph__check_hms(&newRa);

				// Set the new position
				Servo_set_pos(newRa, newDec);

				// Absolutely not in standby mode after sync'ing
				standbyMode = FALSE;
			}
			else
			{
				printf("Error: Failed to sync mount\n");
			}
			break;
#endif // _ALPHA_OUT_

		case 'Q':
		case 'q':
			done = true;
			break;
		}

	} // of While not done

	// Get the lockdown position and move the mount to it
	Servo_ignore_horizon(true);
	Servo_get_park_coordins(&ra, &dec);
	Servo_move_to_static(ra, dec);

	// Wait for mount to be stopped
	while (Servo_state() != STOPPED)
	{
		// Sleep for one sec
		sleep(1); 
	}
	Servo_stop_all(); 
	return (0);

} // of main
