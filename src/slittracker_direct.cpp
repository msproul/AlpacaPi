//*****************************************************************************
//*		slittracker_direct.cpp
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 10,	2023	<MLS> Moved slit tracker direct code to separate file
//*****************************************************************************

#ifdef _SLIT_TRACKER_DIRECT_
	#include	<termios.h>
	#include	<fcntl.h>
	#include	"serialport.h"

#error "This needs to be moved to slittracker_direct.h"

	void	OpenSlitTrackerPort(void);
	void	GetSLitTrackerData(void);
	void	SendSlitTrackerCmd(const char *cmdBuffer);


#define	kReadBufferSize		1024
#define	kLineBuffSize		64
int				gSlitTrackerfileDesc	=	-1;				//*	port file descriptor
char			gSlitTrackerLineBuf[kLineBuffSize];
int				gSLitTrackerByteCnt		=	0;
unsigned long	gLastSlitUpdate_MS		=	0;
//*****************************************************************************
void	OpenSlitTrackerPort(void)
{
char	usbPortPath[32]	=	"/dev/ttyACM0";

	CONSOLE_DEBUG(__FUNCTION__);

	gLastSlitUpdate_MS		=	millis();
	gSlitTrackerfileDesc	=	open(usbPortPath, O_RDWR);	//* connect to port
	if (gSlitTrackerfileDesc >= 0)
	{
		Set_Serial_attribs(gSlitTrackerfileDesc, B9600, 0);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open port", usbPortPath);
	}
}

//*****************************************************************************
void	ProcessSlitTrackerLine(char *lineBuff)
{
int				clockValue;
char			*inchesPtr;
double			inchValue;
unsigned long	deltaMilliSecs;

	CONSOLE_DEBUG(lineBuff);

	if ((lineBuff[0] == '=') && (isdigit(lineBuff[1])))
	{
		clockValue	=	atoi(&lineBuff[1]);
		if ((clockValue >= 0) && (clockValue < 12))
		{
			//	0	Distance: 151.25 cm	Inches: 59.55 delta: -0.04
			inchesPtr	=	strstr(lineBuff, "Inches");
			if (inchesPtr != NULL)
			{
				inchesPtr	+=	7;
				while ((*inchesPtr == 0x20) || (*inchesPtr == 0x09))
				{
					inchesPtr++;
				}
				inchValue	=	AsciiToDouble(inchesPtr);
//				CONSOLE_DEBUG_W_DBL("inchValue\t=", inchValue);

				gSlitDistance[clockValue].distanceInches	=	inchValue;
				gSlitDistance[clockValue].validData			=	true;
				gSlitDistance[clockValue].updated			=	true;
				gSlitDistance[clockValue].readCount++;


				deltaMilliSecs	=	millis() - gLastSlitUpdate_MS;
				if (deltaMilliSecs > 1000)
				{
					gUpdateSLitWindow	=	true;
					gLastSlitUpdate_MS	=	millis();
				}
			}
			if (clockValue == 0)
			{
//				UpdateSlitLog();
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("clockValue error\t=", lineBuff);
		}
	}
}


//*****************************************************************************
void	GetSLitTrackerData(void)
{
int		readCnt;
char	readBuffer[kReadBufferSize];
char	theChar;
bool	keepGoing;
int		iii;
int		charsRead;


//	CONSOLE_DEBUG(__FUNCTION__);
	if (gSlitTrackerfileDesc >= 0)
	{
		keepGoing	=	true;
		readCnt		=	0;
		while (keepGoing && (readCnt < 10))
		{
			charsRead	=	read(gSlitTrackerfileDesc, readBuffer, (kReadBufferSize - 2));
			if (charsRead > 0)
			{
				readCnt++;
				for (iii=0; iii<charsRead; iii++)
				{
					theChar		=	readBuffer[iii];
					if ((theChar >= 0x20) || (theChar == 0x09))
					{
						if (gSLitTrackerByteCnt < (kLineBuffSize - 2))
						{
							gSlitTrackerLineBuf[gSLitTrackerByteCnt++]	=	theChar;
							gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
						}
					}
					else if (theChar == 0x0d)
					{
						gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
						if (strlen(gSlitTrackerLineBuf) > 0)
						{
							ProcessSlitTrackerLine(gSlitTrackerLineBuf);
						}
						gSLitTrackerByteCnt							=	0;
						gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
//		CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
		if (readCnt > 9)
		{
			//*	slow the read rate down
			SendSlitTrackerCmd("+");
		}
	}
	else
	{
//		CONSOLE_DEBUG("Slit tracker port not open");
	}
}


//*****************************************************************************
void	SendSlitTrackerCmd(const char *cmdBuffer)
{
int		sLen;
size_t	bytesWritten;

	CONSOLE_DEBUG_W_STR("cmdBuffer\t=", cmdBuffer);

	if (gSlitTrackerfileDesc >= 0)
	{
		sLen			=	strlen(cmdBuffer);
		bytesWritten	=	write(gSlitTrackerfileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG("write returned error");
		}
	}
}
#endif // _SLIT_TRACKER_DIRECT_
