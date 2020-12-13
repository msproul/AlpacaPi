//**************************************************************************************
//*		commoncolor.c
//*
//*	this is common code shared between the driver code and the controller code
//*	so that the background and text colors match
//*	between what is running on the remote machine and the local machine
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 16,	2020	<MLS> Added commoncolor.c
//*	Apr 16,	2020	<MLS> Added GetDefaultColors() for use both by client and server
//**************************************************************************************

#include	<stdio.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<string.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"commoncolor.h"



#define	SetRGBColor(rgbPtr, r, g, b)	\
	rgbPtr->red		=	r & 0x00ff;		\
	rgbPtr->grn		=	g & 0x00ff;		\
	rgbPtr->blu		=	b & 0x00ff;


//**************************************************************************************
void	GetDefaultColors(	const char	overRideChar,
							const char	*windowName,
							RGBcolor	*bgColor,
							RGBcolor	*txColor)
{
	if (overRideChar != 0)
	{
		switch(tolower(overRideChar))
		{
			case 'r':	//*	red
				SetRGBColor(	bgColor,	255,	0,		0);
				SetRGBColor(	txColor,	0,		0,		0);
				break;

			case 'g':	//*	green
				SetRGBColor(	bgColor,	0x028,	0x0ba,	0x01d);	//*	#28ba1d Moonlite green
				SetRGBColor(	txColor,	0,		0,		0);
				break;

			case 'b':	//*	blue
				SetRGBColor(	bgColor,	0,		0,		255);
				SetRGBColor(	txColor,	0,		0,		0);
				break;

			case 'k':	//*	black
				SetRGBColor(	bgColor,	0,		0,		0);
				SetRGBColor(	txColor,	255,	255,	255);
				break;

			case 'p':	//*	purple
			case 'v':	//*	violet
				SetRGBColor(	bgColor,	0x072,	0x003,	0x0A6);	//	#7203A6
				SetRGBColor(	txColor,	255,	255,	255);
				break;

			case 'a':	//*	gold
			case 'y':	//*	yellow
				SetRGBColor(	bgColor,	(0x0CD - 0x10),	(0x0AC - 0x10),	(0x006));
				SetRGBColor(	txColor,	0,		0,		0);
				break;

		}
	}
	else if (strcasestr(windowName, "newt16") != NULL)
	{
		//*	purple
		SetRGBColor(	bgColor,	0x072,	0x003,	0x0A6);	//	#7203A6
		SetRGBColor(	txColor,	255,	255,	255);
	}
	else if ((strcasestr(windowName, "wo71") != NULL) || (strcasestr(windowName, "pi-1") != NULL))
	{
		//*	red
		SetRGBColor(	bgColor,	255,	0,		0);
		SetRGBColor(	txColor,	0,		0,		0);

	}
	else if (strcasestr(windowName, "wo102") != NULL)
	{
		//*	gold
		SetRGBColor(	bgColor,	(0x0CD - 0x10),	(0x0AC - 0x10),	(0x006));
		SetRGBColor(	txColor,	0,		0,		0);
		CONSOLE_DEBUG_W_HEX("bgColor->red\t=",	bgColor->red);
		CONSOLE_DEBUG_W_HEX("bgColor->grn\t=",	bgColor->grn);
		CONSOLE_DEBUG_W_HEX("bgColor->blu\t=",	bgColor->blu);
	}
	else if (strcasestr(windowName, "pi") != NULL)
	{
		//*	red
		SetRGBColor(	bgColor,	255,	0,		0);
		SetRGBColor(	txColor,	0,		0,		0);
	}
	else if (strcasestr(windowName, "door") != NULL)
	{
		//*	Moonlite green
		SetRGBColor(	bgColor,	0x028,	0x0ba,	0x01d);	//*	#28ba1d Moonlite green
		SetRGBColor(	txColor,	0,		0,		0);
	}
	else if (strcasestr(windowName, "jetson") != NULL)
	{
		//*	Moonlite green
		SetRGBColor(	bgColor,	0x028,	0x0ba,	0x01d);	//*	#28ba1d Moonlite green
		SetRGBColor(	txColor,	0,		0,		0);
	}
	else if ((strcasestr(windowName, "finder") != NULL) || (strcasestr(windowName, "CCTV") != NULL))
	{
		//*	Blue
		SetRGBColor(	bgColor,	0,		0,		255);
		SetRGBColor(	txColor,	255,	255,	255);
	}
	else if (strcasestr(windowName, "tty") != NULL)
	{
		SetRGBColor(	bgColor,	0,		0,		0);
		SetRGBColor(	txColor,	255,	255,	255);
	}
	else
	{
		//*	alpaca blue = 0x002157
		SetRGBColor(	bgColor,	0,		0x21,	0x57);
		SetRGBColor(	txColor,	255,	255,	255);
	#ifdef _JETSON_
		//*	dark green
		SetRGBColor(	bgColor,	0x008,	0x07a,	0x00d);
		SetRGBColor(	txColor,	0,		0,		0);
	#endif
	}
}

