//*****************************************************************************
//*		controller_fw_common.cpp		(c) 2021 by Mark Sproul
//*
//*
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	May 28,	2021	<MLS> Created controller_fw_common.cpp
//*	May 29,	2021	<MLS> Added AlpacaGetFilterWheelStatus()
//*****************************************************************************

//*	this file gets INCLUDED at the end of either controller_telescope OR controller_skytravel

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif




//*****************************************************************************
//http://newt16:6800/api/v1/filterwheel/0/names
//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetFilterWheelStartup(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				filterWheelIdx;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevNum\t=", cAlpacaDevNum);
//	CONSOLE_DEBUG_W_NUM("cPort\t=", cPort);


	//*	Start by getting info about the filterwheel
	//===============================================================
	//*	get the filter wheel names
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/filterwheel/%d/names", cAlpacaDevNum);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										NULL,
										&jsonParser);

	if (validData)
	{
		jjj	=	0;
		while (jjj<jsonParser.tokenCount_Data)
		{
//			CONSOLE_DEBUG_W_2STR("JSON:", jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "DEVICE") == 0)
			{
				strcpy(cFilterWheelName,	"Filterwheel: ");
				strcat(cFilterWheelName,	jsonParser.dataList[jjj].valueString);
			}
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
			{

				filterWheelIdx	=	0;
				jjj++;
				while ((jjj<jsonParser.tokenCount_Data) &&
						(jsonParser.dataList[jjj].keyword[0] != ']'))
				{
					if (filterWheelIdx < kMaxFiltersPerWheel)
					{
						//*	save the filter name
						strcpy(cFilterWheelProp.Names[filterWheelIdx].FilterName, jsonParser.dataList[jjj].valueString);

//						CONSOLE_DEBUG(cFilterWheelProp.Names[filterWheelIdx].FilterName);
						filterWheelIdx++;
					}
					jjj++;
				}
				cPositionCount	=	filterWheelIdx;
			}
			jjj++;
		}
		UpdateFilterWheelInfo();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - filterwheel");
		cReadFailureCnt++;
	}
	return(validData);
}


//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetFilterWheelStatus(void)
{
bool	validData;
int		newFilterWheelPosition;
bool	rtnValidData;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
#ifdef _PARENT_IS_FILTERWHEEL_
	validData	=	AlpacaGetCommonConnectedState("filterwheel");
#endif

	validData	=	AlpacaGetIntegerValue(	"filterwheel",
											"position",
											NULL,
											&newFilterWheelPosition,
											&rtnValidData);
//	CONSOLE_DEBUG_W_BOOL("rtnValidData\t=", rtnValidData);
	if (validData && rtnValidData)
	{
//		CONSOLE_DEBUG_W_NUM("rcvd newFilterWheelPosition\t=", newFilterWheelPosition);
		cOnLine	=	true;

		//*	alpaca/ascom uses filter wheel positions from 0 -> N-1
		if ((newFilterWheelPosition >= 0) && (newFilterWheelPosition < kMaxFiltersPerWheel))
		{
			cFilterWheelProp.Position	=	newFilterWheelPosition;
			cFilterWheelProp.IsMoving	=	false;
		}
		else if (newFilterWheelPosition == -1)
		{
			cFilterWheelProp.IsMoving	=	true;
		}
		UpdateFilterWheelPosition();
	}
	else
	{
		CONSOLE_DEBUG("Failed to get filter wheel position");
	#ifdef _PARENT_IS_FILTERWHEEL_
		cOnLine	=	false;
	#endif

	#ifdef _PARENT_IS_CAMERA_
		CONSOLE_DEBUG_W_STR("This camera does NOT have a filterwheel", cWindowName);
		cHas_FilterWheel		=	false;
		UpdateFilterWheelPosition();
	#endif // _PARENT_IS_CAMERA_
	}
	SetWindowIPaddrInfo(NULL, cOnLine);
	return(validData);
}
