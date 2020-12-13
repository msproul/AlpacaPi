//*****************************************************************************
//*****************************************************************************
//*	Apr 26,	2020	<MLS> Created cpu_stats.c
//*	Apr 26,	2020	<MLS> Added CPUstats_GetTemperature
//*	Apr 27,	2020	<MLS> Added CPUstats_GetUptime()
//*	Apr 27,	2020	<MLS> Added CPUstats_GetTotalRam() & CPUstats_GetFreeRam()
//*	Jun 24,	2020	<MLS> Added CPUstats_GetFreeDiskSpace()
//*****************************************************************************

#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/sysinfo.h>
#include	<stdint.h>
#include	<string.h>
#include	<sys/statvfs.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cpu_stats.h"

//#define _INCLUDE_MAIN_CPU_STATS_

//*****************************************************************************
//	Returns a double with the temp in deg C
//	And the temp in a text string
//*****************************************************************************
double	CPUstats_GetTemperature(char *theTempString)
{
double	cpuTemp_DegC;
double	cpuTemp_DegF;
long	millidegLng;
FILE	*filePtr;

	cpuTemp_DegC	=	0.0;
	filePtr	=	fopen("/sys/class/thermal/thermal_zone0/temp","r");
	if (filePtr != NULL)
	{
		fscanf(filePtr,	"%ld",	&millidegLng);

		cpuTemp_DegC	=	millidegLng / 1000.0;
		cpuTemp_DegF	=	((cpuTemp_DegC * (9.0/5.0)) + 32);

		if (theTempString != NULL)
		{
			sprintf(theTempString,"CPU temp is %3.3f deg C / %3.3f deg F", cpuTemp_DegC, cpuTemp_DegF);
		}
		fclose(filePtr);
	}
	return(cpuTemp_DegC);
}

//*****************************************************************************
//*	struct sysinfo {
//*	    long uptime;             //* Seconds since boot
//*	    unsigned long loads[3];  //* 1, 5, and 15 minute load averages
//*	    unsigned long totalram;  //* Total usable main memory size
//*		unsigned long freeram;   //* Available memory size
//*		unsigned long sharedram; //* Amount of shared memory
//*		unsigned long bufferram; //* Memory used by buffers
//*		unsigned long totalswap; //* Total swap space size
//*		unsigned long freeswap;  //* swap space still available
//*		unsigned short procs;    //* Number of current processes
//*		unsigned long totalhigh; //* Total high memory size
//*		unsigned long freehigh;  //* Available high memory size
//*		unsigned int mem_unit;   //* Memory unit size in bytes
//*		char _f[20-2*sizeof(long)-sizeof(int)]; //* Padding for libc5
//*****************************************************************************
//*	returns uptime in seconds since boot
//*****************************************************************************
uint32_t	CPUstats_GetUptime(void)
{
struct sysinfo	mySysInfo;
int				myError;
uint32_t		upTime_seconds;

	myError	=	sysinfo(&mySysInfo);
	if (myError != 0)
	{
		printf("code error = %d\n", myError);
	}
	upTime_seconds	=	mySysInfo.uptime;
	return(upTime_seconds);
}

//*****************************************************************************
//*	returns value in Megabytes
//*****************************************************************************
uint32_t	CPUstats_GetTotalRam(void)
{
struct sysinfo	mySysInfo;
int				myError;
uint32_t		ram_megabytes;

	myError	=	sysinfo(&mySysInfo);
	if (myError != 0)
	{
		printf("code error = %d\n", myError);
	}
	ram_megabytes	=	mySysInfo.totalram / (1024 * 1024);

	return(ram_megabytes);
}

//*****************************************************************************
//*	returns value in Megabytes
//*****************************************************************************
uint32_t	CPUstats_GetFreeRam(void)
{
struct sysinfo	mySysInfo;
int				myError;
uint32_t		ram_megabytes;

	myError	=	sysinfo(&mySysInfo);
	if (myError != 0)
	{
		printf("code error = %d\n", myError);
	}
	ram_megabytes	=	mySysInfo.freeram / (1024 * 1024);

	return(ram_megabytes);
}

//**************************************************************************************
//*	returns the available disk space in megabytes
//**************************************************************************************
uint32_t CPUstats_GetFreeDiskSpace(const char* path)
{
struct statvfs	stat;
int				retCode;
uint32_t		availableSpace;

	retCode	=	statvfs(path, &stat);
	if (retCode == 0)
	{
//		CONSOLE_DEBUG_W_INT32("stat.f_bsize\t=",	stat.f_bsize);
//		CONSOLE_DEBUG_W_INT32("stat.f_bavail\t=",	stat.f_bavail);
		// the available size is f_bsize * f_bavail
		//*	raspberry pi still uses 32 bit ints, make the math work
		availableSpace	=	stat.f_bsize * (stat.f_bavail / 1024) / 1024;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("statvfs returned error code:", retCode);
		availableSpace	=	0;
	}
//	CONSOLE_DEBUG_W_INT32("availableSpace", availableSpace);

	return (availableSpace);
}



#ifdef _INCLUDE_MAIN_CPU_STATS_

//*****************************************************************************
void	DumpSysInfo(void)
{
struct sysinfo	mySysInfo;
int				myError;
uint32_t		ram_megabytes;

	memset(&mySysInfo, 0, sizeof(mySysInfo));

	myError	=	sysinfo(&mySysInfo);
	if (myError == 0)
	{
//*	    long uptime;             //* Seconds since boot
//*	    unsigned long loads[3];  //* 1, 5, and 15 minute load averages
//*	    unsigned long totalram;  //* Total usable main memory size
//*		unsigned long freeram;   //* Available memory size
//*		unsigned long sharedram; //* Amount of shared memory
//*		unsigned long bufferram; //* Memory used by buffers
//*		unsigned long totalswap; //* Total swap space size
//*		unsigned long freeswap;  //* swap space still available
//*		unsigned short procs;    //* Number of current processes
//*		unsigned long totalhigh; //* Total high memory size
//*		unsigned long freehigh;  //* Available high memory size
//*		unsigned int mem_unit;   //* Memory unit size in bytes

		printf("totalram\t=\t%ld\r\n",	mySysInfo.totalram);

		printf("freeram\t\t=\t%ld\r\n",	mySysInfo.freeram);

		printf("sharedram\t=\t%ld\r\n",	mySysInfo.sharedram);

		printf("bufferram\t=\t%ld\r\n",	mySysInfo.bufferram);

		printf("totalswap\t=\t%ld\r\n",	mySysInfo.totalswap);

		printf("procs\t\t=\t%ld\r\n",	mySysInfo.procs);

		printf("totalhigh\t=\t%ld\r\n",	mySysInfo.totalhigh);

		printf("freehigh\t=\t%ld\r\n",	mySysInfo.freehigh);

		printf("mem_unit\t=\t%ld\r\n",	mySysInfo.mem_unit);
	}
	ram_megabytes	=	mySysInfo.freehigh / (1024 * 1024);


}
//*****************************************************************************
int main(int argc, char *argv[])
{
double		cputempDegC;
char		theTempString[48];
uint32_t	ram_megabytes;

	cputempDegC	=	CPUstats_GetTemperature(theTempString);
	printf("%s\r\n", theTempString);

	ram_megabytes	=	CPUstats_GetTotalRam();
	printf("total ram\t=\t%ld\r\n", ram_megabytes);

	DumpSysInfo();
}
#endif // _INCLUDE_MAIN_CPU_STATS_



