/* ------------------------------------------------------------ *
 * file:        getbno055.c                                     *
 * purpose:     Sensor control and data extraction program for  *
 *              the Bosch BNO055 absolute orientation sensor    *
 *                                                              *
 * return:      0 on success, and -1 on errors.                 *
 *                                                              *
 * requires:	I2C headers, e.g. sudo apt install libi2c-dev   *
 *                                                              *
 * compile:	gcc -o getbno055 i2c_bno055.c getbno055.c       *
 *                                                              *
 * example:	./getbno055 -t eul  -o bno055.htm               *
 *                                                              *
 * author:      05/04/2018 Frank4DD                             *
 * ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

//#include "getbno055.h"
#include "i2c_bno055.h"

/* ------------------------------------------------------------ *
 * Global variables and defaults                                *
 * ------------------------------------------------------------ */
//bool	gBNO_verbose	=	0;
int		outflag			=	0;
int		argflag			=	0; // 1 dump, 2 reset, 3 load calib, 4 write calib
char	opr_mode[9]		=	{0};
char	pwr_mode[8]		=	{0};
char	datatype[256];
char	senaddr[256]	=	"0x28";
char	i2c_bus[256]	=	I2CBUS;
char	htmfile[256];
char	calfile[256];

/* ------------------------------------------------------------ *
 * print_usage() prints the programs commandline instructions.  *
 * ------------------------------------------------------------ */
void usage()
{
   static char const usage[] = "Usage: getbno055 [-a hex i2c-addr] [-m <opr_mode>] [-t acc|gyr|mag|eul|qua|lin|gra|inf|cal|con] [-r] [-w calfile] [-l calfile] [-o htmlfile] [-v]\n\
\n\
Command line parameters have the following format:\n\
   -a   sensor I2C bus address in hex, Example: -a 0x28 (default)\n\
   -b   I2C bus to query, Example: -b /dev/i2c-1 (default)\n\
   -d   dump the complete sensor register map content\n\
   -m   set sensor operational mode. mode arguments:\n\
           config   = configuration mode\n\
           acconly  = accelerometer only\n\
           magonly  = magnetometer only\n\
           gyronly  = gyroscope only\n\
           accmag   = accelerometer + magnetometer\n\
           accgyro  = accelerometer + gyroscope\n\
           maggyro  = magetometer + gyroscope\n\
           amg      = accelerometer + magnetometer + gyroscope\n\
           imu      = accelerometer + gyroscope fusion -> rel. orientation\n\
           compass  = accelerometer + magnetometer fusion -> abs. orientation\n\
           m4g      = accelerometer + magnetometer fusion -> rel. orientation\n\
           ndof     = accelerometer + mag + gyro fusion -> abs. orientation\n\
           ndof_fmc = ndof, using fast magnetometer calibration (FMC)\n\
   -p   set sensor power mode. mode arguments:\n\
          normal    = required sensors and MCU always on (default)\n\
          low       = enter sleep mode during motion inactivity\n\
          suspend   = sensor paused, all parts put to sleep\n\
   -r   reset sensor\n\
   -t   read and output sensor data. data type arguments:\n\
           acc = Accelerometer (X-Y-Z axis values)\n\
           gyr = Gyroscope (X-Y-Z axis values)\n\
           mag = Magnetometer (X-Y-Z axis values)\n\
           eul = Orientation E (H-R-P values as Euler angles)\n\
           qua = Orientation Q (W-X-Y-Z values as Quaternation)\n\
           gra = GravityVector (X-Y-Z axis values)\n\
           lin = Linear Accel (X-Y-Z axis values)\n\
           inf = Sensor info (23 version and state values)\n\
           cal = Calibration data (mag, gyro and accel calibration values)\n\
           con = Continuous data (eul)\n\
   -l   load sensor calibration data from file, Example -l ./bno055.cal\n\
   -w   write sensor calibration data to file, Example -w ./bno055.cal\n\
   -o   output sensor data to HTML table file, requires -t, Example: -o ./bno055.html\n\
   -h   display this message\n\
   -v   enable debug output\n\
\n\
Note: The sensor is executing calibration in the background, but only in fusion mode.\n\
\n\
Usage examples:\n\
./getbno055 -a 0x28 -t inf -v\n\
./getbno055 -t cal -v\n\
./getbno055 -t eul -o ./bno055.html\n\
./getbno055 -m ndof\n\
./getbno055 -w ./bno055.cal\n";
   printf(usage);
}

/* ------------------------------------------------------------ *
 * parseargs() checks the commandline arguments with C getopt   *
 * ------------------------------------------------------------ */
void parseargs(int argc, char* argv[])
{
	int arg;
	opterr	=	0;

	if(argc == 1) { usage(); exit(-1); }

	while ((arg = (int) getopt (argc, argv, "a:b:dm:p:rt:l:w:o:hv")) != -1)
	{
		switch (arg)
		{
			// arg -v verbose, type: flag, optional
			case 'v':
				gBNO_verbose = 1; break;

			// arg -a + sensor address, type: string
			// mandatory, example: 0x29
			case 'a':
				if(gBNO_verbose == 1) printf("Debug: arg -a, value %s\n", optarg);
				if (strlen(optarg) != 4)
				{
					printf("Error: Cannot get valid -a sensor address argument.\n");
					exit(-1);
				}
				strncpy(senaddr, optarg, sizeof(senaddr));
				break;

			// arg -b + I2C bus, type: string
			// optional, example: "/dev/i2c-1"
			case 'b':
				if(gBNO_verbose == 1) printf("Debug: arg -b, value %s\n", optarg);
				if (strlen(optarg) >= sizeof(i2c_bus))
				{
					printf("Error: invalid i2c bus argument.\n");
						exit(-1);
				}
				strncpy(i2c_bus, optarg, sizeof(i2c_bus));
				break;

			// arg -d
			// optional, dumps the complete register map data
			case 'd':
				if(gBNO_verbose == 1) printf("Debug: arg -d, value %s\n", optarg);
				argflag = 1;
				break;

			// arg -m sets operations mode, type: string
			case 'm':
				if(gBNO_verbose == 1) printf("Debug: arg -m, value %s\n", optarg);
				if (strlen(optarg) >= sizeof(opr_mode))
				{
					printf("Error: invalid opr_mode argument.\n");
					exit(-1);
				}
				strncpy(opr_mode, optarg, sizeof(opr_mode));
				break;

			// arg -p sets power mode, type: string
			case 'p':
				if(gBNO_verbose == 1) printf("Debug: arg -p, value %s\n", optarg);
				if (strlen(optarg) >= sizeof(pwr_mode))
				{
					printf("Error: invalid pwr_mode argument.\n");
					exit(-1);
				}
				strncpy(pwr_mode, optarg, sizeof(pwr_mode));
				break;

			// arg -r
			// optional, resets sensor
			case 'r':
				if(gBNO_verbose == 1) printf("Debug: arg -r, value %s\n", optarg);
				   argflag = 2;
				break;

			// arg -t + sensor component, type: string
			// mandatory, example: mag (magnetometer)
			case 't':
				if(gBNO_verbose == 1) printf("Debug: arg -t, value %s\n", optarg);
				if (strlen(optarg) != 3)
				{
					printf("Error: Cannot get valid -t data type argument.\n");
					exit(-1);
				}
				strncpy(datatype, optarg, sizeof(datatype));
				break;

			// arg -l + calibration file name, type: string
			// loads the sensor calibration from file. example: ./bno055.cal
			case 'l':
				argflag = 3;
				if(gBNO_verbose == 1) printf("Debug: arg -l, value %s\n", optarg);
				if (strlen(optarg) >= sizeof(calfile))
				{
					printf("Error: invalid calfile argument.\n");
					exit(-1);
				}
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -w + calibration file name, type: string
         // writes sensor calibration to file. example: ./bno055.cal
         case 'w':
            argflag = 4;
            if(gBNO_verbose == 1) printf("Debug: arg -w, value %s\n", optarg);
            if (strlen(optarg) >= sizeof(calfile))
			{
               printf("Error: invalid calfile argument.\n");
               exit(-1);
            }
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -o + dst HTML file, type: string, requires -t
         // writes the sensor output to file. example: /tmp/sensor.htm
         case 'o':
            outflag = 1;
            if(gBNO_verbose == 1) printf("Debug: arg -o, value %s\n", optarg);
            if (strlen(optarg) >= sizeof(htmfile))
			{
               printf("Error: invalid htmfile argument.\n");
               exit(-1);
            }
            strncpy(htmfile, optarg, sizeof(htmfile));
            break;

         // arg -h usage, type: flag, optional
         case 'h':
            usage(); exit(0);
            break;

         case '?':
            if(isprint (optopt))
               printf ("Error: Unknown option `-%c'.\n", optopt);
            else
               printf ("Error: Unknown option character `\\x%x'.\n", optopt);
            usage();
            exit(-1);
            break;

         default:
            usage();
            break;
      }
   }
}

/* ----------------------------------------------------------- *
 *  print_calstat() - Read and print calibration status        *
 * ----------------------------------------------------------- */
void print_calstat()
{
struct bnocal bnoc;
   /* -------------------------------------------------------- *
	*  Check the sensors calibration state                     *
	* -------------------------------------------------------- */
   int res = get_calstatus(&bnoc);
   if(res != 0)
	{
		printf("Error: Cannot read calibration state.\n");
		exit(-1);
	}

	/* -------------------------------------------------------- *
	*  Convert the status code into a status message           *
	* -------------------------------------------------------- */
	printf("Sensor System Calibration = ");
    switch(bnoc.scal_st)
	{
	  case 0:
		 printf("Uncalibrated\n");
		 break;
	  case 1:
		 printf("Minimal Calibrated\n");
	     break;
	  case 2:
		 printf("Mostly Calibrated\n");
		 break;
	  case 3:
		 printf("Fully calibrated\n");
	     break;
   }

   printf("    Gyroscope Calibration = ");
   switch(bnoc.gcal_st)
   {
	  case 0:
		 printf("Uncalibrated\n");
		 break;
	  case 1:
		 printf("Minimal Calibrated\n");
		 break;
	  case 2:
		 printf("Mostly Calibrated\n");
		 break;
	  case 3:
		 printf("Fully calibrated\n");
		 break;
   }

   printf("Accelerometer Calibration = ");
   switch(bnoc.acal_st)
   {
	  case 0:
		 printf("Uncalibrated\n");
		 break;
	  case 1:
	     printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }

   printf(" Magnetometer Calibration = ");
   switch(bnoc.mcal_st)
   {
      case 0:
         printf("Uncalibrated\n");
         break;
      case 1:
         printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }
}


//**************************************************************************************
int main(int argc, char *argv[])
{
struct bnocal	bnoc;
int				res = -1;       // res = function retcode: 0=OK, -1 = Error

	/* ---------------------------------------------------------- *
	* Process the cmdline parameters                             *
	* ---------------------------------------------------------- */
	parseargs(argc, argv);

	/* ----------------------------------------------------------- *
	* get current time (now), write program start if verbose      *
	* ----------------------------------------------------------- */
	time_t tsnow = time(NULL);
	if(gBNO_verbose == 1) printf("Debug: ts=[%lld] date=%s", (long long) tsnow, ctime(&tsnow));

	/* ----------------------------------------------------------- *
	* "-a" open the I2C bus and connect to the sensor i2c address *
	* ----------------------------------------------------------- */
	get_i2cbus(i2c_bus, senaddr);

	/* ----------------------------------------------------------- *
	*  "-d" dump the register map content and exit the program    *
	* ----------------------------------------------------------- */
	if(argflag == 1)
	{
		res = bno_dump();
		if(res != 0)
		{
			printf("Error: could not dump the register maps.\n");
			exit(-1);
		}
		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-r" reset the sensor and exit the program                 *
	* ----------------------------------------------------------- */
	if(argflag == 2)
	{
		res = bno_reset();
		if(res != 0)
		{
			printf("Error: could not reset the sensor.\n");
			exit(-1);
		}
		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-m" set the sensor operational mode and exit the program  *
	* ----------------------------------------------------------- */
	if(strlen(opr_mode) > 0)
	{
		opmode_t newmode;
		if(strcmp(opr_mode, "config")   == 0) newmode = config;
		else if(strcmp(opr_mode, "acconly")  == 0) newmode = acconly;
		else if(strcmp(opr_mode, "magonly")  == 0) newmode = magonly;
		else if(strcmp(opr_mode, "gyronly")  == 0) newmode = gyronly;
		else if(strcmp(opr_mode, "accmag")   == 0) newmode = accmag;
		else if(strcmp(opr_mode, "accgyro")  == 0) newmode = accgyro;
		else if(strcmp(opr_mode, "maggyro")  == 0) newmode = maggyro;
		else if(strcmp(opr_mode, "amg")      == 0) newmode = amg;
		else if(strcmp(opr_mode, "imu")      == 0) newmode = imu;
		else if(strcmp(opr_mode, "compass")  == 0) newmode = compass;
		else if(strcmp(opr_mode, "m4g")      == 0) newmode = m4g;
		else if(strcmp(opr_mode, "ndof")     == 0) newmode = ndof;
		else if(strcmp(opr_mode, "ndof_fmc") == 0) newmode = ndof_fmc;
		else
		{
			printf("Error: invalid operations mode %s.\n", opr_mode);
			exit(-1);
		}

		res = set_mode(newmode);
		if(res != 0)
		{
			printf("Error: could not set sensor mode %s [0x%02X].\n", opr_mode, newmode);
			exit(-1);
		}
		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-p" set the sensor power mode and exit the program        *
	* ----------------------------------------------------------- */
	if(strlen(pwr_mode) > 0)
	{
		power_t newmode;
		if(strcmp(pwr_mode, "normal")   == 0) newmode = normal;
		else if(strcmp(pwr_mode, "low")  == 0) newmode = low;
		else if(strcmp(pwr_mode, "suspend")  == 0) newmode = suspend;
		else
		{
			printf("Error: invalid power mode %s.\n", pwr_mode);
			exit(-1);
		}

		if(newmode == get_power())
		{
			if(gBNO_verbose == 1) printf("Debug: Sensor already in mode %s [0x%02X].\n", pwr_mode, newmode);
			exit(0);
		}

		res = set_power(newmode);
		if(res != 0)
		{
			printf("Error: could not set power mode %s [0x%02X].\n", pwr_mode, newmode);
			exit(-1);
		}
		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-l" loads the sensor calibration data from file.          *
	* To update calibration data, sensor must be in CONFIG mode.  *
	* ----------------------------------------------------------- */
	if(argflag == 3) load_cal(calfile);

	/* ----------------------------------------------------------- *
	* -t "cal"  print the sensor calibration data                 *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "cal") == 0)
	{
		/* -------------------------------------------------------- *
		*  Read the sensors calibration state                      *
		* -------------------------------------------------------- */
		res = get_calstatus(&bnoc);
		if(res != 0)
		{
			printf("Error: Cannot read calibration state.\n");
			exit(-1);
		}
		/* -------------------------------------------------------- *
		*  Read the sensors calibration offset                     *
		* -------------------------------------------------------- */
		res = get_caloffset(&bnoc);
		if(res != 0)
		{
			printf("Error: Cannot read calibration data.\n");
			exit(-1);
		}

		/* -------------------------------------------------------- *
		*  Print the calibration data line                         *
		* -------------------------------------------------------- */
		printf("sys [S:%d]", bnoc.scal_st);
		printf(" acc [S:%d ", bnoc.acal_st);
		printf("X:%d Y:%d Z:%d", bnoc.aoff_x, bnoc.aoff_y, bnoc.aoff_z);
		printf(" R:%d]", bnoc.acc_rad);

		printf(" mag [S:%d ", bnoc.mcal_st);
		printf("X:%d Y:%d Z:%d", bnoc.moff_x, bnoc.moff_y, bnoc.moff_z);
		printf(" R:%d]", bnoc.mag_rad);

		printf(" gyr [S:%d ", bnoc.gcal_st);
		printf("X:%d Y:%d Z:%d]\n", bnoc.goff_x, bnoc.goff_y, bnoc.goff_z);

		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-w" writes sensor calibration data to file.               *
	* ----------------------------------------------------------- */
	if (argflag == 4)
	{
		/* -------------------------------------------------------- *
		*  Check the sensors calibration state                     *
		* -------------------------------------------------------- */
		res = get_calstatus(&bnoc);
		if(res != 0)
		{
			printf("Error: Cannot read calibration state.\n");
			exit(-1);
		}
		/* -------------------------------------------------------- *
		*  Only save data if the sensor is fully calibrated (3)    *
		* -------------------------------------------------------- */
		if(bnoc.scal_st == 3) save_cal(calfile);
		else printf("Error: Sensor not fully calibrated, abort writing to file %s.\n", calfile);
	}


	/* ----------------------------------------------------------- *
	* -t "inf"  print the sensor configuration                    *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "inf") == 0)
	{
		struct bnoinf bnoi;
		res = get_inf(&bnoi);
		if(res != 0)
		{
			printf("Error: Cannot read sensor version data.\n");
			exit(-1);
		}

		/* ----------------------------------------------------------- *
		* print the formatted output strings to stdout                *
		* ----------------------------------------------------------- */
		printf("\nBN0055 Information at %s", ctime(&tsnow));
		printf("----------------------------------------------\n");
		printf("   Chip Version ID = 0x%02X\n", bnoi.chip_id);
		printf("  Accelerometer ID = 0x%02X\n", bnoi.acc_id);
		printf("      Gyroscope ID = 0x%02X\n", bnoi.gyr_id);
		printf("   Magnetoscope ID = 0x%02X\n", bnoi.mag_id);
		printf("  Software Version = %d.%d\n", bnoi.sw_msb, bnoi.sw_lsb);
		printf("   Operations Mode = "); print_mode(bnoi.opr_mode);
		printf("        Power Mode = "); print_power(bnoi.pwr_mode);
		printf("Axis Configuration = "); print_remap_conf(bnoi.axr_conf);
		printf("   Axis Remap Sign = "); print_remap_sign(bnoi.axr_sign);
		printf("System Status Code = "); print_sstat(bnoi.sys_stat);
		printf("System Clocksource = "); print_clksrc();

		printf("Accelerometer Test = ");
		if((bnoi.selftest >> 0) & 0x01) printf("OK\n");
		else printf("FAIL\n");

		printf(" Magnetometer Test = ");
		if((bnoi.selftest >> 1) & 0x01) printf("OK\n");
		else printf("FAIL\n");

		printf("    Gyroscope Test = ");
		if((bnoi.selftest >> 2) & 0x01) printf("OK\n");
		else printf("FAIL\n");

		printf("MCU Cortex M0 Test = ");
		if((bnoi.selftest >> 3) & 0x01) printf("OK\n");
		else printf("FAIL\n");

		printf(" System Error Code = ");
		switch(bnoi.sys_err)
		{
			case 0x00:
				printf("No Error\n");
				break;
			case 0x01:
				printf("Peripheral initialization error\n");
				break;
			case 0x02:
				printf("System initializion error\n");
				break;
			case 0x03:
				printf("Selftest result failed\n");
				break;
			case 0x04:
				printf("Register map value out of range\n");
				break;
			case 0x05:
				printf("Register map address out of range\n");
				break;
			case 0x06:
				printf("Register map write error\n");
				break;
			case 0x07:
				printf("BNO low power mode not available\n");
				break;
			case 0x08:
				printf("Accelerometer power mode not available\n");
				break;
			case 0x09:
				printf("Fusion algorithm configuration error\n");
				break;
			case 0x0A:
				printf("Sensor configuration error\n");
				break;
		}

		print_unit(bnoi.unitsel);

		printf("Sensor Temperature = ");
		if(bnoi.opr_mode > 0)
		{
			if((bnoi.unitsel >> 4) & 0x01) printf("%d°F\n", bnoi.temp_val);
			else printf("%d°C\n",bnoi.temp_val);
		}
		else  printf("no data in CONFIG mode\n");

		printf("\n----------------------------------------------\n");
		struct bnoaconf bnoac;
		if(get_acc_conf(&bnoac) == 0) print_acc_conf(&bnoac);

		printf("\n----------------------------------------------\n");
		print_calstat();
		exit(0);
	}

	/* ----------------------------------------------------------- *
	*  "-t acc " reads accelerometer data from the sensor.        *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "acc") == 0)
	{
		struct bnoacc bnod;
		res = get_acc(&bnod);
		if(res != 0)
		{
			printf("Error: Cannot read accelerometer data.\n");
			exit(-1);
		}

		//* -----------------------------------------------------------
		//* print the formatted output string to stdout (Example below)
		//* ACC -45.00 264.00 939.00 (ACC X Y Z)
		//* -----------------------------------------------------------
		printf("ACC %3.2f %3.2f %3.2f\n", bnod.adata_x, bnod.adata_y, bnod.adata_z);

		if(outflag == 1)
		{
			//* --------------------------------------------------------
			//*  Open the html file for writing accelerometer data
			//* --------------------------------------------------------
			FILE *html;
			if(! (html=fopen(htmfile, "w")))
			{
				printf("Error open %s for writing.\n", htmfile);
				exit(-1);
			}
			fprintf(html, "<table><tr>\n");
			fprintf(html, "<td class=\"sensordata\">Accelerometer X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_x);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Accelerometer Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_y);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Accelerometer Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_z);
			fprintf(html, "</tr></table>\n");
			fclose(html);
		}
	} /* End reading Accelerometer */

	/* ----------------------------------------------------------- *
	*  "-t gyr" reads gyroscope data from the sensor.             *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "gyr") == 0)
	{
		struct bnogyr bnod;
		res = get_gyr(&bnod);
		if(res != 0)
		{
			printf("Error: Cannot read gyroscope data.\n");
			exit(-1);
		}

		//* -----------------------------------------------------------
		//* print the formatted output string to stdout (Example below)
		//* GYR 0.00 0.06 -0.12 (GYR X Y Z)
		//* -----------------------------------------------------------
		printf("GYR %3.2f %3.2f %3.2f\n", bnod.gdata_x, bnod.gdata_y, bnod.gdata_z);

		if(outflag == 1)
		{
			//* --------------------------------------------------------
			//*  Open the html file for writing gyroscope data
			//* --------------------------------------------------------
			FILE *html;
			if(! (html=fopen(htmfile, "w")))
			{
				printf("Error open %s for writing.\n", htmfile);
				exit(-1);
			}
			fprintf(html, "<table><tr>\n");
			fprintf(html, "<td class=\"sensordata\">Gyroscope X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gdata_x);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Gyroscope Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gdata_y);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Gyroscope Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gdata_z);
			fprintf(html, "</tr></table>\n");
			fclose(html);
		}
	} /* End reading Gyroscope */

	/* ----------------------------------------------------------- *
	*  "-t mag" reads magnetometer data from the sensor.          *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "mag") == 0)
	{
		struct bnomag bnod;
		res = get_mag(&bnod);
		if(res != 0)
		{
			printf("Error: Cannot read magnetometer data.\n");
			exit(-1);
		}

		/* ----------------------------------------------------------- *
		* print the formatted output string to stdout (Example below) *
		* MAG -220.00 50.62 -345.62 (MAG X Y Z in Micro Tesla)        *
		* ----------------------------------------------------------- */
		printf("MAG %3.2f %3.2f %3.2f\n", bnod.mdata_x, bnod.mdata_y, bnod.mdata_z);

		if(outflag == 1)
		{
			/* -------------------------------------------------------- *
			*  Open the html file for writing Magnetometer data        *
			* -------------------------------------------------------- */
			FILE *html;
			if(! (html=fopen(htmfile, "w")))
			{
				printf("Error open %s for writing.\n", htmfile);
				exit(-1);
			}
			fprintf(html, "<table><tr>\n");
			fprintf(html, "<td class=\"sensordata\">Magnetometer X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.mdata_x);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Magnetometer Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.mdata_y);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Magentometer Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.mdata_z);
			fprintf(html, "</tr></table>\n");
			fclose(html);
		}
	} /* End reading Magnetometer data */

	/* ----------------------------------------------------------- *
	*  "-t eul" reads the Euler Orientation from the sensor.      *
	* This requires the sensor to be in fusion mode (mode > 7).   *
	* ----------------------------------------------------------- */
	if(strcmp(datatype, "eul") == 0)
	{

		int mode = get_mode();
		if(mode < 8)
		{
			printf("Error getting Euler data, sensor mode %d is not a fusion mode.\n", mode);
			exit(-1);
		}

		struct bnoeul bnod;
		res = get_eul(&bnod);
		if(res != 0)
		{
			printf("Error: Cannot read Euler orientation data.\n");
			exit(-1);
		}

		/* ----------------------------------------------------------- *
		* print the formatted output string to stdout (Example below) *
		* EUL 66.06 -3.00 -15.56 (EUL H R P in Degrees)               *
		* ----------------------------------------------------------- */
		printf("EUL %3.4f %3.4f %3.4f\n", bnod.eul_head, bnod.eul_roll, bnod.eul_pitc);

		if(outflag == 1)
		{
			/* -------------------------------------------------------- *
			*  Open the html file for writing Euler Orientation data   *
			* -------------------------------------------------------- */
			FILE *html;
			if(! (html=fopen(htmfile, "w")))
			{
				printf("Error open %s for writing.\n", htmfile);
				exit(-1);
			}
			fprintf(html, "<table><tr>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Heading:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_head);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Roll:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_roll);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Pitch:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_pitc);
			fprintf(html, "</tr></table>\n");
			fclose(html);
		}
	} /* End reading Euler Orientation */

  /* ----------------------------------------------------------- *
    *  "-t con"                                                   *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "con") == 0)
	{

      int mode = get_mode();
      if(mode < 8)
		{
         printf("Error getting Euler data, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnoeul bnod;
      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * EUL 66.06 -3.00 -15.56 (EUL H R P in Degrees)               *
       * ----------------------------------------------------------- */
      while(1)
		{
        clock_t t;
        t = clock();

        res = get_eul(&bnod);
        if(res != 0)
			{
           printf("Error: Cannot read Euler orientation data.\n");
           continue;
        }

        printf("EUL %3.4f %3.4f %3.4f\n", bnod.eul_head, bnod.eul_roll, bnod.eul_pitc);

        if(outflag == 1)
		{
			/* -------------------------------------------------------- *
			*  Open the html file for writing Euler Orientation data   *
			* -------------------------------------------------------- */
			FILE *html;
			if(! (html=fopen(htmfile, "w")))
			{
				printf("Error open %s for writing.\n", htmfile);
				exit(-1);
			}
			fprintf(html, "<table><tr>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Heading:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_head);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Roll:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_roll);
			fprintf(html, "<td class=\"sensorspace\"></td>\n");
			fprintf(html, "<td class=\"sensordata\">Euler Pitch:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_pitc);
			fprintf(html, "</tr></table>\n");
			fclose(html);
			}

//			t = clock() - t;
//			double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
//			printf("Sensor reading took %f seconds \n", time_taken);
		}

	} /* End reading continuous data */

   /* ----------------------------------------------------------- *
    *  "-t qua" reads the Quaternation data from the sensor.      *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "qua") == 0)
	{

      int mode = get_mode();
      if(mode < 8)
		{
         printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnoqua bnod;
      res = get_qua(&bnod);
      if(res != 0)
		{
         printf("Error: Cannot read Quaternation data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * QUA 0.83 0.13 -0.05 -0.54 (QUA W X Y Z)                     *
       * ----------------------------------------------------------- */
      printf("QUA %3.2f %3.2f %3.2f %3.2f\n", bnod.quater_w, bnod.quater_x, bnod.quater_y, bnod.quater_z);

      if(outflag == 1)
		{
         /* -------------------------------------------------------- *
          *  Open the html file for writing Quaternation data        *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w")))
         {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation W:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_w);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_x);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_y);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_z);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Quaternation data */

   /* ----------------------------------------------------------- *
    *  "-t gra " reads gravity vector data from the sensor.       *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "gra") == 0)
	{

      int mode = get_mode();
      if(mode < 8)
		{
         printf("Error getting Gravity Vector, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnogra bnod;
      res = get_gra(&bnod);
      if(res != 0)
		{
         printf("Error: Cannot read gravity vector data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * GRA -3.19 16.38 58.94 (GRA X Y Z)                           *
       * ----------------------------------------------------------- */
      printf("GRA %3.2f %3.2f %3.2f\n", bnod.gravityx, bnod.gravityy, bnod.gravityz);

      if(outflag == 1)
		{
         /* -------------------------------------------------------- *
          *  Open the html file for writing gravity vector data      *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w")))
         {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Gravity Vector X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gravityx);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Gravity Vector Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gravityy);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Gravity Vector Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.gravityz);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Gravity  Vector */

   /* ----------------------------------------------------------- *
    *  "-t lin " reads linear acceleration data from the sensor.  *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "lin") == 0)
	{

      int mode = get_mode();
      if(mode < 8)
		{
         printf("Error getting Linear Acceleration, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnolin bnod;
      res = get_lin(&bnod);
      if(res != 0)
		{
         printf("Error: Cannot read linear acceleration data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * LIN 0.44 0.19 -0.38 (LIN X Y Z)                             *
       * ----------------------------------------------------------- */
      printf("LIN %3.2f %3.2f %3.2f\n", bnod.linacc_x, bnod.linacc_y, bnod.linacc_z);

      if(outflag == 1)
		{
         /* -------------------------------------------------------- *
          *  Open the html file for writing linear acceleration data *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w")))
         {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Linear Acceleration X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.linacc_x);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Linear Acceleration Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.linacc_y);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Linear Acceleration Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.linacc_z);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Linear Acceleration */

   exit(0);
}
