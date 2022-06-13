
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "servo_std_defs.h"
#include "servo_mc_core.h"
#include "servo_time.h"
#include "servo_rc_utils.h"
#include "servo_mount_cfg.h"
#include "servo_mount.h"

// Minimum globals needed for the test program, copied from servo_mount.c 
extern TYPE_MOUNT_CONFIG gMountConfig;
static char gDebugInfoCOP[] = "00000";


int main(int argc, char *argv[])
{
	double startRa, startDec, endRa, endDec;
	double lst;
	double raVec, decVec;
	char mount;
	bool flip;
	char buf[128];
	FILE *inFile;

	printf("TEST Servo_calc_optimal_path() program 1.0 - Flip Window = %lf\n\n", gMountConfig.flipWin);
	// printf("Input-> Mount LST startRa startDec endRa endDec\n");

    // Init the data structure 
    Servo_init(NULL, NULL);

	if (argc != 2)
	{
		printf("Error! Must specify input file\n");
		exit(-1);
	}

	inFile = fopen(argv[1], "r");

	fgets(buf, 128, inFile);
	sscanf(buf, "%c %lf %lf %lf %lf %lf", &mount, &lst, &startRa, &startDec, &endRa, &endDec);

	switch (mount)
	{
	case 'G':
	case 'g':
		gMountConfig.mount = kGERMAN;
		break;

	case 'F':
	case 'f':
		gMountConfig.mount = kFORK;
		break;

	case '#':
	case 'Q':
	case 'q':
		// Do nothing, let the while loop handle it
		break;

	default:
		printf("Error!  Input line format *'%s'* unsupported\n", buf);
		fclose(inFile);
		exit(-1);
		break;
	}

	while (mount != 'Q' || mount != 'q')
	{

		switch (mount)
		{
		case '#':
			// Comment line, just print it
			printf("%s", buf);
			break;

		case 'G':
		case 'g':
			// GEM mount
			printf("Mount:%c LST:%.2lf sRA:%.2lf sDec:%.2lf eRA:%.2lf eDec:%.2lf\n", mount, lst, startRa, startDec, endRa, endDec);
			flip = Servo_calc_optimal_path(startRa, startDec, lst, endRa, endDec, &raVec, &decVec);
			printf("-> result move Flip= %d raVec= %.2lf decVec= %.2lf  %s\n\n", flip, raVec, decVec, gDebugInfoCOP);
			break;

		case 'F':
		case 'f':
			// FORK mount
			printf("Mount:%c LST:%.2lf sRA:%.2lf sDec:%.2lf eRA:%.2lf eDec:%.2lf\n", mount, lst, startRa, startDec, endRa, endDec);
			flip = Servo_calc_optimal_path(startRa, startDec, lst, endRa, endDec, &raVec, &decVec);
			printf("-> result move Flip= %d raVec= %.2lf decVec= %.2lf  %s\n\n", flip, raVec, decVec, gDebugInfoCOP);
			break;

		case 'Q':
		case 'q':
			fclose(inFile);
			exit(0);
			break;

		default:
			printf("Error!  Input line format *'%s'* unsupported\n", buf);
			fclose(inFile);
			exit(-1);
			break;
		} // of switch

		// Get the next line from input file and parse
		fgets(buf, 128, inFile);
		if (feof(inFile))
		{
			exit(0);
		}

		sscanf(buf, "%c %lf %lf %lf %lf %lf", &mount, &lst, &startRa, &startDec, &endRa, &endDec);

	} // of while

	// Should never get here, but just in case
	fclose(inFile);
	exit(0);
}