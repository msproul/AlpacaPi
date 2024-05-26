//**************************************************************************
//*	Name:			filterwheeldriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 18,	2019	<MLS> Created filterwheeldriver.c
//*	Apr 18,	2019	<MLS> Started on Filterwheel
//*	Apr 18,	2019	<MLS> Filterwheel driver working
//*	Apr 21,	2019	<MLS> Added support for filter description file "filters.txt"
//*	Nov 10,	2019	<MLS> Switching filter wheel to C++
//*	Nov 11,	2019	<MLS> Multiple filter wheels will now work
//*	Nov 11,	2019	<MLS> Changed the filter name table to start with 1, ignore slot 0
//*	Nov 16,	2019	<MLS> Added Read_CurrentFilterName()
//*	Dec 13,	2019	<MLS> Updated Get_Names() to deal with table starting at 1
//*	Jan  1,	2020	<MLS> Filterwheel description list was not being initialized
//*	Mar 22,	2020	<MLS> Added variables for current filter wheel filter name
//*	Mar 31,	2020	<MLS> Added Get_Focusoffsets()
//*	Apr  1,	2020	<MLS> Switching filter numbers to 0 -> N-1 as per ASCOM
//*	Apr  1,	2020	<MLS> Updated Get_Names() to deal with table starting at 0
//*	Apr  1,	2020	<MLS> CONFORM-filterwheel -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	May 22,	2020	<MLS> Fixed JSON formating error in filter wheel names output
//*	Mar 21,	2021	<MLS> Working on FilterWheel driver to prevent ZWO EFW from hanging
//*	Apr 30,	2021	<MLS> Added "Filter #" if name not specified to names list
//*	Jun 15,	2021	<MLS> Fixed filterwheel ifdefs
//*	Jun 22,	2021	<MLS> Updated filtwheel driver cCommonProp.InterfaceVersion to 2
//*	AUg  5,	2022	<MLS> Added Get_Readall() to filterwheel driver
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to filterwheel driver
//*	May 17,	2024	<MLS> Added http error 400 processing to filterwheel driver
//*	May 17,	2024	<MLS> Added http error 400 to Put_Position()
//*	May 17,	2024	<MLS> CONFORMU-filterwheel -> PASSED!!!!!!!!!!!!!!
//*****************************************************************************

#if defined(_ENABLE_FILTERWHEEL_) || defined(_ENABLE_FILTERWHEEL_ZWO_) || defined(_ENABLE_FILTERWHEEL_ATIK_)

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"
#include	"filterwheeldriver.h"

#ifdef _ENABLE_FILTERWHEEL_ATIK_
	#include	"filterwheeldriver_ATIK.h"
#endif
#ifdef _ENABLE_FILTERWHEEL_PLAYERONE_
	#include	"filterwheeldriver_Play1.h"
#endif
#ifdef _ENABLE_FILTERWHEEL_QHY_
	#include	"filterwheeldriver_QHY.h"
#endif
#ifdef _ENABLE_FILTERWHEEL_SIMULATOR_
	#include	"filterwheeldriver_sim.h"
#endif
#ifdef _ENABLE_FILTERWHEEL_ZWO_
	#include	"filterwheeldriver_ZWO.h"
#endif

#include	"filterwheel_AlpacaCmds.h"
#include	"filterwheel_AlpacaCmds.cpp"


//**************************************************************************************
//*	this will get moved to the individual implementations later
void	CreateFilterWheelObjects(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_FILTERWHEEL_ATIK_
	CreateFilterWheelObjects_ATIK();
#endif

#ifdef _ENABLE_FILTERWHEEL_PLAYERONE_
	CreateFilterWheelObjects_PlayerOne();
#endif

#ifdef _ENABLE_FILTERWHEEL_QHY_
	CreateFilterWheelObjects_QHY();
#endif

#ifdef _ENABLE_FILTERWHEEL_ZWO_
	CreateFilterWheelObjects_ZWO();
#endif

#ifdef _ENABLE_FILTERWHEEL_SIMULATOR_
	CreateFilterWheelObjects_SIM();
#endif

}



//**************************************************************************************
FilterwheelDriver::FilterwheelDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Filterwheel)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Filterwheel");
	strcpy(cCommonProp.Description,	"Generic filterwheel");
	cCommonProp.InterfaceVersion	=	2;
	cDriverCmdTablePtr				=	gFilterwheelCmdTable;

	strcpy(cFilterWheelCurrName, "none");

	//*	initialize all of the filter wheel properties to 0
	memset(&cFilterWheelProp, 0, sizeof(TYPE_FilterWheelProperties));

	cFilterWheelState			=	kFilterWheelState_OK;

	cFilterWheelConnected		=	false;
	cNumberOfPositions			=	0;
	cFilterWheelProp.Position	=	0;
	cSuccesfullOpens			=	0;
	cSuccesfullCloses			=	0;
	cOpenFailures				=	0;
	cCloseFailures				=	0;
	cFilterWheelIsOpen			=	false;

	ReadFilterNamesTextFile();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FilterwheelDriver::~FilterwheelDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	FilterwheelDriver::ReadFilterNamesTextFile(void)
{
FILE			*filePointer;
char			lineBuff[256];
int				filterIndex;
int				ii;
int				slen;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	see if there is a file describing the filters currently installed
	filePointer	=	fopen("filters.txt", "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG("filters.txt is present");

		LogEvent(	"filterwheel",
					"filters.txt is present",
					NULL,
					kASCOM_Err_Success,
					"");

		//*	zero based indexing
		filterIndex	=	0;

		while ((fgets(lineBuff, 200, filePointer)) && (filterIndex < kMaxFiltersPerWheel))
		{
			if (lineBuff[0] != '#')
			{
				//*	get rid of the trailing CR/LF
				slen	=	strlen(lineBuff);
				for (ii=0; ii<slen; ii++)
				{
					if (lineBuff[ii] < 0x20)
					{
						lineBuff[ii]	=	0;
						break;
					}
				}
				slen	=	slen;
				if ((slen > 1) && (slen < kMaxFWnameLen))
				{
				//	strcpy(cFilterDef[filterIndex].filterDesciption, lineBuff);
					strcpy(cFilterWheelProp.Names[filterIndex].FilterName, lineBuff);
				}
				filterIndex++;
			}
		}
		fclose(filePointer);
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					cmdEnumValue;
int					cmdType;
char				alpacaErrMsg[256];
int					mySocket;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
#endif // _DEBUG_CONFORM_

	strcpy(alpacaErrMsg, "");

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

//	DumpRequestStructure(__FUNCTION__, reqData);

	alpacaErrCode	=	kASCOM_Err_Success;


	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cCommonProp.Name,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gFilterwheelCmdTable, &cmdType);
//	CONSOLE_DEBUG_W_NUM("cmdEnumValue\t=", cmdEnumValue);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gFilterwheelCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Filterwheel specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Filterwheel_focusoffsets:		//*	Filter focus offsets
			alpacaErrCode	=	Get_Focusoffsets(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Filterwheel_names:			//*	Filter wheel filter names
			alpacaErrCode	=	Get_Names(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Filterwheel_position:			//*	GET-Returns the current filter wheel position
												//*	PUT-Sets the filter wheel position
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Position(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Position(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Filterwheel_readall:
			alpacaErrCode	=	Get_Readall(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ClientTransactionID",
									reqData->ClientTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ServerTransactionID",
									gServerTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Int32(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorNumber",
								alpacaErrCode,
								INCLUDE_COMMA);


		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorMessage",
								alpacaErrMsg,
								NO_COMMA);

		JsonResponse_Add_Finish(mySocket,
								reqData->httpRetCode,
								reqData->jsonTextBuffer,
								(cHttpHeaderSent == false));
	}
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG_W_STR("Output JSON\t=", reqData->jsonTextBuffer);
#endif // _DEBUG_CONFORM_

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Get_Focusoffsets(TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					ii;
int					mySocketFD;
char				lineBuffer[256];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;


		if (cNumberOfPositions > 0)
		{
			JsonResponse_Add_ArrayStart(mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										responseString);

			JsonResponse_Add_RawText(	mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"\r\n");
			for (ii=0; ii<cNumberOfPositions; ii++)
			{
			//	sprintf(lineBuffer, "\t\t\t%d", cFilterDef[ii].focusOffset);
				sprintf(lineBuffer, "\t\t\t%d", cFilterWheelProp.FocusOffsets[ii]);

				if (ii < (cNumberOfPositions - 1))
				{
					strcat(lineBuffer, ",");
					strcat(lineBuffer, "\r\n");
				}
				else
				{
					strcat(lineBuffer, " ");
				}

				JsonResponse_Add_RawText(	mySocketFD,
											reqData->jsonTextBuffer,
											kMaxJsonBuffLen,
											lineBuffer);

			}
			JsonResponse_Add_ArrayEnd(	mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										INCLUDE_COMMA);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Get_Names(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					ii;
int					mySocketFD;
char				lineBuffer[256];
char				filterNameBuff[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;

		if (cNumberOfPositions > 0)
		{
		//	JsonResponse_Add_RawText(mySocketFD,
		//							reqData->jsonTextBuffer,
		//							kMaxJsonBuffLen,
		//							"\t\"Value\": \r\n\t[\r\n");
			JsonResponse_Add_ArrayStart(mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										responseString);
			JsonResponse_Add_RawText(	mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"\r\n");
			for (ii=0; ii<cNumberOfPositions; ii++)
			{
				strcpy(lineBuffer, "\t\t\t\"");
			//	strcat(lineBuffer, cFilterDef[ii].filterDesciption);

				if (strlen(cFilterWheelProp.Names[ii].FilterName) > 0)
				{
					strcat(lineBuffer, cFilterWheelProp.Names[ii].FilterName);
				}
				else
				{
					sprintf(filterNameBuff, "Filter %d", (ii + 1));
					strcat(lineBuffer, filterNameBuff);
				}

				strcat(lineBuffer, "\"");
				if (ii < (cNumberOfPositions - 1))
				{
					strcat(lineBuffer, ",");
					strcat(lineBuffer, "\r\n");
				}
				else
				{
					strcat(lineBuffer, " ");
				}

				JsonResponse_Add_RawText(	mySocketFD,
											reqData->jsonTextBuffer,
											kMaxJsonBuffLen,
											lineBuffer);

			}
			JsonResponse_Add_ArrayEnd(	mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										INCLUDE_COMMA);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Get_Position(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					curState;
int					myFilterPosition;

//	CONSOLE_DEBUG(__FUNCTION__);
	curState		=	Read_CurrentFWstate();
	if (curState == kFilterWheelState_Moving)
	{
		//*	https://ascom-standards.org/Help/Developer/html/P_ASCOM_DriverAccess_FilterWheel_Position.htm
		//*	states that it is mandatory to return -1 if the wheel is moving

		myFilterPosition	=	-1;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Filter wheel is moving");
	}
	else
	{
		alpacaErrCode		=	Read_CurrentFilterPositon();
		myFilterPosition	=	cFilterWheelProp.Position;
	}

	//*	the positions are number 0 -> [4,7], same as the devices
	//*	we want to represent the slot numbers as they are on the devices
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							myFilterPosition,
							INCLUDE_COMMA);

	if (myFilterPosition >= 0)
	{
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"FilterName",
							//	cFilterDef[myFilterPosition].filterDesciption,
								cFilterWheelProp.Names[myFilterPosition].FilterName,
								INCLUDE_COMMA);
	}
	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
//*	curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/ASCOMAlpacaAPI/1.0.0-oas3/filterwheel/0/position"
//*			-H  "accept: application/json"
//*			-H  "Content-Type: application/x-www-form-urlencoded"
//*			-d "Position=3&ClientID=&ClientTransactionID="
//*	curl -X PUT "http://192.168.1.220:6800/api/v1.0.0-oas3/filterwheel/0/position"
//*			-H  "accept: application/json"
//*			-H  "Content-Type: application/x-www-form-urlencoded"
//*			-d "Position=3&ClientID=&ClientTransactionID="
//*	filterwheel/0/interfaceversion?ClientTransactionID=2&ClientID=12498 HTTP/1.1
//****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Put_Position(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					newPosition;
bool				positionFound;
char				poisitonString[32];
char				commentString[128];

//	CONSOLE_DEBUG_W_STR("contentData\t=",	reqData->contentData);
//	CONSOLE_DEBUG_W_NUM("max positions\t=",	cNumberOfPositions);

	positionFound		=	GetKeyWordArgument(	reqData->contentData,
												"Position",
												poisitonString,
												16);

	if (positionFound)
	{
		if (IsValidNumericString(poisitonString))
		{
			newPosition		=	AsciiToDouble(poisitonString);

			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//*	newPosition starts at "0"
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if ((newPosition >= 0) && (newPosition < cNumberOfPositions))
			{
				alpacaErrCode	=	Set_CurrentFilterPositon(newPosition);
				if (alpacaErrCode == 0)
				{
					cFilterWheelProp.Position	=	newPosition;
				}
				else
				{
					CONSOLE_DEBUG("Failed to open filter wheel");
				}
		#ifdef _INCLUDE_ALPACA_EXTENSIONS_
				//*	NOTE: the cFilterDef slot 0 is not used.
				sprintf(commentString, "#%d-%s",
										newPosition,
									//	cFilterDef[newPosition].filterDesciption
										cFilterWheelProp.Names[newPosition].FilterName
										);

				JsonResponse_Add_String(reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"Comment",
										commentString,
										INCLUDE_COMMA);
		#endif // _INCLUDE_ALPACA_EXTENSIONS_
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "New position is out of range");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "New position is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		//*	improperly formatted request
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Position is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
bool	FilterwheelDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
	DeviceState_Add_Int(socketFD,	jsonTextBuffer, maxLen,	"Position",	cFilterWheelProp.Position,	true);

	return(true);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;


	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);

		alpacaErrCode	=	Get_Position(reqData,		alpacaErrMsg,	"position");
		alpacaErrCode	=	Get_Names(reqData,			alpacaErrMsg,	"names");
		alpacaErrCode	=	Get_Focusoffsets(reqData,	alpacaErrMsg,	"focusoffsets");

		alpacaErrCode	=	kASCOM_Err_Success;
		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


#pragma mark -

//*****************************************************************************
void	FilterwheelDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
int			iii;
char		lineBuffer[128];
bool		isConnected;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H2>Filter Wheel</H2>\r\n");

		isConnected		=	IsFilterwheelConnected();
		if (isConnected)
		{
			SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");


			SocketWriteData(mySocketFD,	"<TR>\r\n");

			SocketWriteData(mySocketFD,	"\t<TD>");
			SocketWriteData(mySocketFD,	cCommonProp.Name);
			SocketWriteData(mySocketFD,	"</TD>\r\n");

			sprintf(lineBuffer,	"\t<TD><CENTER>Slots=%d</TD>\r\n",	cNumberOfPositions);
			SocketWriteData(mySocketFD,	lineBuffer);

			Read_CurrentFilterPositon();
			sprintf(lineBuffer,	"\t<TD><CENTER>Current position=%d</TD>\r\n",	cFilterWheelProp.Position);
			SocketWriteData(mySocketFD,	lineBuffer);

			SocketWriteData(mySocketFD,	"</TR>\r\n");


			//*******************************************************
			//*	list out the filter names
			for (iii=0; iii < cNumberOfPositions; iii++)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");

			//	sprintf(lineBuffer,	"\t<TD>Slot#%d</TD><TD>%s</TD>\r\n",	iii, cFilterDef[iii].filterDesciption);
				sprintf(lineBuffer,	"\t<TD>Slot#%d</TD><TD>%s</TD>\r\n",	iii, cFilterWheelProp.Names[iii].FilterName);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"</TR>\r\n");
			}
			SocketWriteData(mySocketFD,	"</TABLE>\r\n");
			SocketWriteData(mySocketFD,	"<P>\r\n");

		}
		else
		{
			SocketWriteData(mySocketFD,	"<H3>Filter wheel is not connected</H3>\r\n");
		}

		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
		SocketWriteData(mySocketFD,	"<TR><TH COLSPAN=2><CENTER>Driver operations</TH></TR>\r\n");


		sprintf(lineBuffer,	"\t<TR><TD><CENTER>Succesful Opens</TD><TD>%d</TD></TR>\r\n",	cSuccesfullOpens);
		SocketWriteData(mySocketFD,	lineBuffer);

		sprintf(lineBuffer,	"\t<TR><TD><CENTER>Successful Closes</TD><TD>%d</TD></TR>\r\n",	cSuccesfullCloses);
		SocketWriteData(mySocketFD,	lineBuffer);

		sprintf(lineBuffer,	"\t<TR><TD><CENTER>Open Failures</TD><TD>%d</TD></TR>\r\n",	cOpenFailures);
		SocketWriteData(mySocketFD,	lineBuffer);

		sprintf(lineBuffer,	"\t<TR><TD><CENTER>Close Failures</TD><TD>%d</TD></TR>\r\n",	cCloseFailures);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"<P>\r\n");

		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	FilterwheelDriver::RunStateMachine(void)
{
	return(5 * 1000 * 1000);
}

//*****************************************************************************
bool	FilterwheelDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gFilterwheelCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
int	FilterwheelDriver::Read_CurrentFWstate(void)
{
	//*	this routine should be overloaded
	CONSOLE_DEBUG("This routine should be overloaded !!!!!!");
	return(cFilterWheelState);
}

//*****************************************************************************
//*	return -1 if unable to determine position
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Read_CurrentFilterPositon(int *rtnCurrentPosition)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_FailedUnknown;

	CONSOLE_DEBUG("This routine should be overloaded !!!!!!");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelDriver::Read_CurrentFilterName(char *rtnCurrentName)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (rtnCurrentName != NULL)
	{
//?		strcpy(cFilterWheelCurrName, cFilterDef[cFilterWheelProp.Position].filterDesciption);
		strcpy(rtnCurrentName, cFilterWheelCurrName);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_FailedUnknown;
	}

	return(alpacaErrCode);
}


//*****************************************************************************
//*	this accepts a position starting with 1
//*	if the specific device uses 0 as the starting location, this routine must adjust
//*****************************************************************************
TYPE_ASCOM_STATUS		FilterwheelDriver::Set_CurrentFilterPositon(const int newPosition)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
bool	FilterwheelDriver::IsFilterwheelConnected(void)
{
	return(cFilterWheelConnected);
}


#endif	//	_ENABLE_FILTERWHEEL_
