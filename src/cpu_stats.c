//*****************************************************************************
//*****************************************************************************
//*	Apr 26,	2020	<MLS> Created cpu_stats.c
//*	Apr 26,	2020	<MLS> Added CPUstats_GetTemperature
//*	Apr 27,	2020	<MLS> Added CPUstats_GetUptime()
//*	Apr 27,	2020	<MLS> Added CPUstats_GetTotalRam() & CPUstats_GetFreeRam()
//*	Jun 24,	2020	<MLS> Added CPUstats_GetFreeDiskSpace()
//*	Jan 17,	2021	<MLS> Moved CPU info routines to this file, changed names
//*	Mar  8,	2021	<MLS> Added ReadUSBfsMemorySetting()
//*****************************************************************************

#include	<stdlib.h>
#include	<stdbool.h>
#include	<stdio.h>
#include	<sys/sysinfo.h>
#include	<stdint.h>
#include	<string.h>
#include	<sys/statvfs.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cpu_stats.h"

//#define _INCLUDE_MAIN_CPU_STATS_

char				gOsReleaseString[64]		=	"";
char				gCpuInfoString[64]			=	"";
char				gPlatformString[64]			=	"";
double				gBogoMipsValue				=	0.0;
//char				gFullVersionString[128];



//**************************************************************************
//*	examples
//*		PRETTY_NAME="Mendel GNU/Linux 4 (Day)"
//*		PRETTY_NAME="Ubuntu 16.04 LTS"
//*		Model		: Raspberry Pi 3 Model B Plus Rev 1.3
//*
//*	returns true on successful extraction
//**************************************************************************
static bool	ExtractArgValue(char *string, char delimChar, char *valueString)
{
int		slen;
char	*delimPtr;
bool	successFlag;

	successFlag	=	false;
	delimPtr	=	strchr(string, delimChar);
	if (delimPtr != NULL)
	{
		//*	skip the delim char
		delimPtr++;
		while ((*delimPtr == ' ') || (*delimPtr == '"'))
		{
			delimPtr++;
		}
		strcpy(valueString, delimPtr);
		slen	=	strlen(valueString);
		while (((valueString[slen] < 0x20) || (valueString[slen] == '"')) && (slen > 0))
		{
			valueString[slen]	=	0;
			slen--;
		}
		successFlag	=	true;
	}
	return(successFlag);
}


//**************************************************************************
void	CPUstats_ReadOSreleaseVersion(void)
{
FILE	*filePointer;
char	lineBuff[256];
int		slen;
char	codeName[64];
bool	codeNameFound;

	filePointer	=	fopen("/etc/os-release", "r");
	if (filePointer != NULL)
	{
		codeName[0]		=	0;
		codeNameFound	=	false;
		while (fgets(lineBuff, 200, filePointer))
		{
			slen	=	strlen(lineBuff);
			if (slen > 1)
			{
				if (strncmp(lineBuff, "PRETTY_NAME", 11) == 0)
				{
					ExtractArgValue(lineBuff, '=', gOsReleaseString);
				}
				else if (strncmp(lineBuff, "UBUNTU_CODENAME", 15) == 0)
				{
					//*	this was added for the case of the nvidia jetson board.
					codeNameFound	=	ExtractArgValue(lineBuff, '=', codeName);
				}
			}
		}
		fclose(filePointer);

		if (codeNameFound)
		{
			//*	check to see if the code name is in the release string
			if (strstr(gOsReleaseString, codeName) == NULL)
			{
				//*	it is NOT there, put it in
				strcat(gOsReleaseString, " (");
				strcat(gOsReleaseString, codeName);
				strcat(gOsReleaseString, ")");
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("/etc/os-release not found");
	}
}

//**************************************************************************
void	CPUstats_ReadInfo(void)
{
FILE	*filePointer;
char	lineBuff[256];
int		slen;
bool	stillNeedModel;
char	argValueString[64];
char	*stringPtr;

	//*	set the default value in case we fail to read /proc/cpuinfo
	strcpy(gPlatformString,	"");
	strcpy(gCpuInfoString,	"");
	stillNeedModel	=	true;

#if defined(__arm__) && !defined(_PLATFORM_STRING_)
	strcpy(gPlatformString,	"Raspberry Pi - ");
#elif defined(__i386__)
	strcpy(gPlatformString,	"32 bit x86");
#elif defined(__x86_64__)
	strcpy(gPlatformString,	"64 bit x86");
#else
	strcpy(gPlatformString,	"Unknown platform");
#endif



#if defined(__arm__)
	strcpy(gCpuInfoString,	"Arm");
#elif defined( __ARM_ARCH)
	strcpy(gCpuInfoString,	"Arm");
#elif defined( __x86_64)
	strcpy(gCpuInfoString,	"64 bit x86");
#elif defined( __i386__)
	strcpy(gCpuInfoString,	"32 bit x86");
#elif defined(__x86_64__)
	strcpy(gCpuInfoString,	"64 bit x86");
#else
	strcpy(gCpuInfoString,	"Unknown arch");
#endif

	//*	open up the cpuinfo file and get data from there
	filePointer	=	fopen("/proc/cpuinfo", "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			slen	=	strlen(lineBuff);
			if (slen > 1)
			{
				if (strncmp(lineBuff, "model name", 10) == 0)
				{
					ExtractArgValue(lineBuff, ':', gCpuInfoString);
				}
				else if (strncmp(lineBuff, "Model", 5) == 0)
				{
					//*	so far I have only found this is only on Raspberry Pi
					ExtractArgValue(lineBuff, ':', gPlatformString);
					stillNeedModel	=	false;
				}
				else if (strncmp(lineBuff, "Revision", 8) == 0)
				{
					if (stillNeedModel)
					{
						ExtractArgValue(lineBuff, ':', argValueString);
						if (strcmp(argValueString, "a020d3") == 0)
						{
							strcpy(gPlatformString,	"Raspberry Pi 3");
						}
						else
						{
							strcat(gPlatformString,	argValueString);
						}
					}
				}
				else if (strncasecmp(lineBuff, "bogomips", 8) == 0)
				{
					ExtractArgValue(lineBuff, ':', argValueString);
					gBogoMipsValue	=	atof(argValueString);
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("failed to open /proc/cpuinfo");
	}

	//=======================================================================
	filePointer	=	fopen("/sys/firmware/devicetree/base/model", "r");
	if (filePointer != NULL)
	{
		lineBuff[0]	=	0;
		stringPtr	=	fgets(lineBuff, 200, filePointer);
		if (stringPtr != NULL)
		{
			if (strlen(lineBuff) > 5)
			{
				strcpy(gPlatformString, lineBuff);
			}
		}
		else
		{
			CONSOLE_DEBUG("Error reading /sys/firmware/devicetree/base/model");
		}
		fclose(filePointer);
	}

	//*	check to see if we have a valid platform string
	if (strlen(gPlatformString) == 0)
	{
	#ifdef _PLATFORM_STRING_
		//*	_PLATFORM_STRING_ can be defined in the make file
		//	exmple, note the back slashes
		//	jetson		:	DEFINEFLAGS		+=	-D_PLATFORM_STRING_=\"Nvidia-jetson\"
		strcpy(gPlatformString, _PLATFORM_STRING_);
	#endif

	}
#if (__SIZEOF_POINTER__ == 8)
	strcat(gPlatformString, " (64 bit)");
#elif (__SIZEOF_POINTER__ == 4)
//	strcat(gPlatformString, " (32 bit)");
#endif
}


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
		CONSOLE_DEBUG_W_NUM("sysinfo returned error code:", myError);
	}
	ram_megabytes	=	mySysInfo.totalram / (1024L * 1024L);

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
		CONSOLE_DEBUG_W_NUM("sysinfo returned error code:", myError);
	}
	ram_megabytes	=	mySysInfo.freeram / (1024L * 1024L);

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
		availableSpace	=	stat.f_bsize * (stat.f_bavail / 1024L) / 1024L;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("statvfs returned error code:", retCode);
		availableSpace	=	0;
	}
//	CONSOLE_DEBUG_W_INT32("availableSpace", availableSpace);

	return (availableSpace);
}


//*****************************************************************************
//*	usbfs_memory_mb is important for the FLIR camera driver, it needs to be 1000 or greater
//*****************************************************************************
bool	ReadUSBfsMemorySetting(char *usbfsString)
{
FILE		*filePointer;
char		lineBuff[256];
struct stat	fileStatus;
char		usbFSpath[]	=	"/sys/module/usbcore/parameters/usbfs_memory_mb";
int			returnCode;
bool		foundIt;
int			iii;
int			sLen;

	foundIt		=	false;
	returnCode	=	stat(usbFSpath, &fileStatus);
	strcpy(lineBuff, "");
	if (returnCode == 0)
	{
		filePointer	=	fopen(usbFSpath, "r");
		if (filePointer != NULL)
		{
			foundIt		=	true;
			fgets(lineBuff, 200, filePointer);
			sLen	=	strlen(lineBuff);
			//*	get rid of any trailing CR/LF
			for (iii=0; iii<sLen; iii++)
			{
				if (lineBuff[iii] < 0x20)
				{
					lineBuff[iii]	=	0;
					break;
				}
			}
			strcpy(usbfsString, lineBuff);
			fclose(filePointer);
		}
	}
	return(foundIt);
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



