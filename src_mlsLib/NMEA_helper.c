//*****************************************************************************
//*	Name:			NMEA_helper.c
//*
//*	Author:			(C) 2016 by Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Oct  4,	2016	<MLS> Created NMEA_helper.c to avoid duplication
//*****************************************************************************

#include	<string.h>

#include	"NMEA_helper.h"

//**************************************************************************************
short	CalculateNMEACheckSum(const char *theNmeaLine)
{
short	iii;
short	theLen;
short	theCheckSum;

	theCheckSum	=	0;
	theLen		=	strlen(theNmeaLine);
	//*	there should be an "*" before the 2-char checksum
	if (theNmeaLine[theLen-3] == '*')
	{
		theLen	-=	3;
	}
	for (iii=1; iii<theLen; iii++)
	{
		theCheckSum	=	theCheckSum ^ theNmeaLine[iii];
	}
	theCheckSum	=	theCheckSum & 0x00ff;
	return(theCheckSum);
}

//************************************************************************************************
short	Hextoi(const char theHexChar)
{
short	theValue;

	theValue	=	(short)theHexChar & 0x0f;

	if (theHexChar > '9')
	{
		theValue	+=	9;
	}
	return(theValue);
}

//**************************************************************************************
short	ExtractChecksumFromNMEAline(const char *theNmeaLine)
{
int	theLen;
int	theCheckSum;

	theCheckSum	=	0;
	theLen		=	strlen(theNmeaLine);
	if ((theLen > 5) && (theNmeaLine[0] == '$') && (theNmeaLine[theLen-3] == '*'))
	{
		theCheckSum	=	Hextoi(theNmeaLine[theLen - 2]) * 16 +
						Hextoi(theNmeaLine[theLen - 1]);
	}

	return(theCheckSum);
}
