//*****************************************************************************
//*	Name:			RequestData.h
//*
//*	Author:			Mark Sproul
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  5,	2021	<MLS> Added httpCmdString to TYPE_GetPutRequestData struct
//*	Nov 29,	2022	<MLS> Added httpUserAgent to TYPE_GetPutRequestData struct
//*	Nov 29,	2022	<MLS> Added clientIs_xxx  to TYPE_GetPutRequestData struct
//*****************************************************************************
//#include	"RequestData.h"

#ifndef _REQUESTDATA_H_
#define	_REQUESTDATA_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_

#ifndef _JSON_DEFS_H_
	#include 	"JsonDefs.h"
#endif // _JSON_DEFS_H_

//*****************************************************************************
//*	the TYPE_GetPutRequestData simplifies parsing and passing of the
//*	parsed data to subroutines
#define	kHTMLbufLen			8192
#define	kDeviceTypeMaxLen	64
#define	kContentDataLen		4096
#define	kMaxCommandLen		512
#define	kHTTPbufLen			512
#define	kUserAgentLen		256

//*****************************************************************************
typedef enum
{
	kHTTPclient_NotSpecified		=	0,
	kHTTPclient_AlpacaPi,
	kHTTPclient_ASCOM_RestSharp,
	kHTTPclient_ConfomU,
	kHTTPclient_Curl,
	kHTTPclient_Mozilla,
	kHTTPclient_NotRecognized,

	kHTTPclient_last
} TYPE_Client;


//*****************************************************************************
typedef struct
{
	char				clientIPaddr[48];
	int					socket;
	int					deviceNumber;
	char				get_putIndicator;
	int					contentLength;
	char				htmlData[kHTMLbufLen];
	char				httpCmdString[kHTTPbufLen];
	char				httpUserAgent[kUserAgentLen];
	TYPE_Client			cHTTPclientType;
	bool				clientIs_AlpacaPi;		//*	flags for which client is in use
	bool				clientIs_ConformU;
	int					alpacaVersion;
	int					requestTypeEnum;
	char				deviceType[kDeviceTypeMaxLen];
	char				cmdBuffer[kMaxCommandLen];
	char				deviceCommand[kMaxCommandLen];
	char				contentData[kContentDataLen];
	TYPE_ASCOM_STATUS	alpacaErrCode;
	char				alpacaErrMsg[256];
	//*	outgoing data
	char				jsonHdrBuffer[kMaxJsonHdrLen];
	char				jsonTextBuffer[kMaxJsonBuffLen];
} TYPE_GetPutRequestData;

void	DumpRequestStructure(const char *functionName, TYPE_GetPutRequestData	*reqData);
//--int		Common_ProcessCommand(TYPE_GetPutRequestData *reqData, int cmdEnum);


#endif	//	_REQUESTDATA_H_
