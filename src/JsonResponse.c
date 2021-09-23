//**************************************************************************
//*	Name:			JsonResponse.c
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
//*	Apr 15,	2019	<MLS> Moved Json code to JsonResponse.c
//*	Apr 15,	2019	<MLS> Change to send directly to the socket instead of memory buffer
//*	Apr 16,	2019	<MLS> Added JsonResponse_CreateHeader()
//*	Apr 23,	2019	<MLS> Added comma option to all JsonResponse_Send_... routines
//*	Apr 23,	2019	<MLS> All JsonResponse_Send_... routines now return bytes written
//*	Apr 25,	2019	<MLS> Added JsonResponse_StartHDR() & JsonResponse_EndBlock()
//*	Apr 28,	2019	<MLS> Added "HTTP/1.0 200" to JSON header
//*	Apr 29,	2019	<MLS> Added JsonResponse_Send_ArrayStart()
//*	Apr 29,	2019	<MLS> Added _INCLUDE_DIRECT_WRITE_TO_SOCKET_
//*	Apr 29,	2019	<MLS> Added Adding buffered versions to allow single socket write.
//*	Apr 29,	2019	<MLS> Added JsonResponse_Add_HDR
//*	Apr 30,	2019	<MLS> Added JsonResponse_Add_String(), JsonResponse_Add_Int32()
//*	Apr 30,	2019	<MLS> Added JsonResponse_Add_Bool(), JsonResponse_Add_EndBlock()
//*	Apr 30,	2019	<MLS> Added JsonResponse_Add_Finish()
//*	May  1,	2019	<MLS> Added JsonResponse_Add_Double(), JsonResponse_Add_RawText()
//*	May  1,	2019	<MLS> Simple testing shows buffering is 2.5 times faster
//*	May  1,	2019	<MLS> Deleted all direct write functions
//*	May  1,	2019	<MLS> Added JsonRespnse_XmitIfFull()
//*	May 20,	2019	<MLS> JsonResponse_Add_Data()
//*	Jun 19,	2019	<MLS> Added _MAKE_JSON_PRETTY_ flag
//*	Aug 15,	2019	<MLS> Made JsonResponse_SendTextBuffer() public
//*	Mar 31,	2020	<MLS> Added JsonResponse_FinishHeader()
//*	Jun 22,	2020	<MLS> Added includeHTTPheader option to JsonResponse_Add_Finish()
//*	Jun 24,	2020	<MLS> Removed return value from JsonResponse_Add functions for speed
//*	Jun 24,	2020	<MLS> Removed some of the safety checks to increase speed
//*	Dec  7,	2020	<MLS> Fixed comma bug in JsonResponse_Add_Double()
//*	Jul 18,	2021	<MLS> Added _DEBUG_JSON_RESPONSE_
//*	Sep  5,	2021	<MLS> Added enableDebug arg to JsonRespnse_XmitIfFull()
//*****************************************************************************


#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<ctype.h>
#include	<stdint.h>


#ifdef __IAR_SYSTEMS_ICC__
	#include	<stdint.h>
	#include	<errno.h>
	#include	"Types.h"
	#include	"sockets.h"
	#include	"KIF_FreeRTOS.h"
	#define		sleep(x)	vTaskDelay( (x*1000)/portTICK_PERIOD_MS )

#else
	#include	<unistd.h>
	#include	<errno.h>
	#include	<sys/types.h>
	#include	<sys/socket.h>
	#include	<netinet/in.h>
#endif		//	__IAR_SYSTEMS_ICC__

//#define	_DEBUG_JSON_RESPONSE_

//#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//*	comment out this line to reduce the number of chars in a json packet.
//*	if enabled, tabs are added to make the json easy to visually read.
#define	_MAKE_JSON_PRETTY_

#include 	"JsonDefs.h"
#include	"JsonResponse.h"


//*****************************************************************************
void	JsonResponse_CreateHeader(	char *jsonTextBuffer, const int maxLen)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (jsonTextBuffer != NULL)
	{
		jsonTextBuffer[0]	=	0;
//		strcat(jsonTextBuffer, "\r\n");
		strcat(jsonTextBuffer, "{\r\n");
	}
}

//*****************************************************************************
void	JsonResponse_FinishHeader(	char *jsonHdrBUffer, const char *jsonTextBuffer)
{
int		contentLen;
char	lineBuff[64];

	if ((jsonHdrBUffer != NULL) && (jsonTextBuffer != NULL))
	{
		contentLen	=	strlen(jsonTextBuffer);
#ifdef _INCLUDE_HTTP_HEADER_
		jsonHdrBUffer[0]	=	0;
		strcat(jsonHdrBUffer,	"HTTP/1.0 200 OK\r\n");
		if (contentLen > 0)
		{
			sprintf(lineBuff,		"Content-Length: %d\r\n", contentLen);
			strcat(jsonHdrBUffer,	lineBuff);
		}
		else
		{
		//	sprintf(lineBuff,		"Content-Length: %d\r\n", -1);
		//	strcat(jsonHdrBUffer,	lineBuff);
		}
		strcat(jsonHdrBUffer,	"Content-type: application/json charset=utf-8\r\n");
		strcat(jsonHdrBUffer,	"Server: AlpacaPi\r\n");
//		strcat(jsonHdrBUffer,	"Accept: text/html,application/json\r\n");
//		strcat(jsonHdrBUffer,	"Accept-Language:en-US,en;q=0.8\r\n");

		strcat(jsonHdrBUffer, "\r\n");
#endif
	}
}

//*****************************************************************************
static int	JsonRespnse_XmitIfFull(	const int	socketFD,
									char		*jsonTextBuffer,
									const int	maxLen,
									const int	payloadLen,
									const bool	enableDebug)
{
size_t	bufLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		bufLen	=	strlen(jsonTextBuffer);
		if ((bufLen + payloadLen) >=  maxLen)
		{
			if (enableDebug)
			{
				CONSOLE_DEBUG("Sending Data because xmit buffer is full");
				CONSOLE_DEBUG_W_NUM("maxLen\t\t\t=", maxLen);
				CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
			}
			//*	transmit the packet and reset
			bytesWritten	=	write(socketFD, jsonTextBuffer, bufLen);
		//	CONSOLE_DEBUG(__FUNCTION__);
			if (bytesWritten < 0)
			{
				CONSOLE_DEBUG("Error writing to socket");
			}
		//	CONSOLE_DEBUG(__FUNCTION__);
			jsonTextBuffer[0]	=	0;	//*	reset the buffer
		//	CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
		}
	}
	else
	{
		CONSOLE_DEBUG("jsonTextBuffer is NULL");
	}
	return(bytesWritten);
}


//*****************************************************************************
void	JsonResponse_Add_HDR(char *jsonTextBuffer, const int maxLen)
{
	if (jsonTextBuffer != NULL)
	{
		if ((maxLen - strlen(jsonTextBuffer)) > 20)
		{
		#ifdef _MAKE_JSON_PRETTY_
			strcat(jsonTextBuffer, "\t\"hdr\":\r\n");
			strcat(jsonTextBuffer, "\t{\r\n");
		#else
			strcat(jsonTextBuffer, "\"hdr\":\r\n");
			strcat(jsonTextBuffer, "{\r\n");
		#endif
		}
	}
}

//*****************************************************************************
int	JsonResponse_Add_Data(	const int	socketFD,
								char		*jsonTextBuffer,
								const int	maxLen)
{
int		payloadLen;
int		bytesWritten;

	if (jsonTextBuffer != NULL)
	{

		payloadLen	=	20;
		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

		if ((maxLen - strlen(jsonTextBuffer)) > 20)
		{
		#ifdef _MAKE_JSON_PRETTY_
			strcat(jsonTextBuffer, "\t\"data\":\r\n");
			strcat(jsonTextBuffer, "\t{\r\n");
		#else
			strcat(jsonTextBuffer, "\"data\":\r\n");
			strcat(jsonTextBuffer, "{\r\n");
		#endif
		}
	}
	return(bytesWritten);
}



//*****************************************************************************
//*	if the buffer is getting full, it will be transmitted and the buffer will be reset
int		JsonResponse_Add_String(const int	socketFD,
								char		*jsonTextBuffer,
								const int	maxLen,
								const char	*itemName,
								const char	*stringValue,
								bool		includeTrailingComma)
{
int		payloadLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	0;
		if (itemName != NULL)
		{
			payloadLen	+=	strlen(itemName);
		}
		if (stringValue != NULL)
		{
			payloadLen	+=	strlen(stringValue);
		}
		payloadLen	+=	20;


		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer, "\t\t\"");
	#else
		strcat(jsonTextBuffer, "\"");
	#endif

		if (itemName != NULL)
		{
			strcat(jsonTextBuffer, itemName);
		}
		strcat(jsonTextBuffer, "\":\"");
		if (stringValue != NULL)
		{
			strcat(jsonTextBuffer, stringValue);
		}
		strcat(jsonTextBuffer, "\"");
		if (includeTrailingComma)
		{
			strcat(jsonTextBuffer, ",");
		}
		strcat(jsonTextBuffer, "\r\n");

	}
	return(bytesWritten);
}

//*****************************************************************************
int		JsonResponse_Add_Int32(	const int		socketFD,
								char			*jsonTextBuffer,
								const int		maxLen,
								const char		*itemName,
								const int32_t	intValue,
								bool			includeTrailingComma)
{
int		payloadLen;
char	numberString[64];
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		sprintf(numberString,	"%ld", (long)intValue);
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	strlen(numberString);
		if (itemName != NULL)
		{
			payloadLen	+=	strlen(itemName);
		}

		payloadLen	+=	20;

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer,	"\t\t\"");
	#else
		strcat(jsonTextBuffer,	"\"");
	#endif
		strcat(jsonTextBuffer,	itemName);
		strcat(jsonTextBuffer,	"\":");
		strcat(jsonTextBuffer,	numberString);
		if (includeTrailingComma)
		{
			strcat(jsonTextBuffer, ",");
		}
		strcat(jsonTextBuffer,	"\r\n");

	}
	return(bytesWritten);
}


//*****************************************************************************
int		JsonResponse_Add_Double(const int		socketFD,
								char			*jsonTextBuffer,
								const int		maxLen,
								const char		*itemName,
								const double	dblValue,
								bool			includeTrailingComma)
{
int		payloadLen;
char	numberString[64];
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		sprintf(numberString, "%f", dblValue);
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	strlen(numberString);
		if (itemName != NULL)
		{
			payloadLen	+=	strlen(itemName);
		}

		payloadLen	+=	20;

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer,	"\t\t\"");
	#else
		strcat(jsonTextBuffer,	"\"");
	#endif
		strcat(jsonTextBuffer,	itemName);
		strcat(jsonTextBuffer,	"\":");
		strcat(jsonTextBuffer,	numberString);
		if (includeTrailingComma)
		{
			strcat(jsonTextBuffer, ",");
		}
		strcat(jsonTextBuffer,	"\r\n");
	}
	return(bytesWritten);
}


//*****************************************************************************
int		JsonResponse_Add_Bool(	const int		socketFD,
								char			*jsonTextBuffer,
								const int		maxLen,
								const char		*itemName,
								const bool		boolValue,
								bool			includeTrailingComma)
{
int		payloadLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	0;
		if (itemName != NULL)
		{
			payloadLen	+=	strlen(itemName);
		}

		payloadLen	+=	20;

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer, "\t\t\"");
	#else
		strcat(jsonTextBuffer, "\"");
	#endif
		strcat(jsonTextBuffer, itemName);
		strcat(jsonTextBuffer, "\":");
		if (boolValue)
		{
			strcat(jsonTextBuffer, "true");
		}
		else
		{
			strcat(jsonTextBuffer, "false");
		}
		strcat(jsonTextBuffer, ",\r\n");

	}
	return(bytesWritten);
}

//*****************************************************************************
int		JsonResponse_Add_ArrayStart(const int		socketFD,
									char			*jsonTextBuffer,
									const int		maxLen,
									const char		*itemName)
{
int		payloadLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	0;
		if (itemName != NULL)
		{
			payloadLen	+=	strlen(itemName);
		}

		payloadLen	+=	20;

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer, "\t\t\"");
	#else
		strcat(jsonTextBuffer, "\"");
	#endif
		strcat(jsonTextBuffer, itemName);
		strcat(jsonTextBuffer, "\":[");
//		strcat(jsonTextBuffer, "\r\n");

	}
	return(bytesWritten);
}

//*****************************************************************************
int		JsonResponse_Add_ArrayEnd(	const int		socketFD,
								char			*jsonTextBuffer,
								const int		maxLen,
								bool			includeTrailingComma)
{
int		payloadLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	10;

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

		strcat(jsonTextBuffer, "]");
		if (includeTrailingComma)
		{
			strcat(jsonTextBuffer, ",");
		}
		strcat(jsonTextBuffer, "\r\n");

	}
	return(bytesWritten);
}



//*****************************************************************************
int		JsonResponse_Add_EndBlock(	const int		socketFD,
									char			*jsonTextBuffer,
									const int		maxLen,
									bool			includeTrailingComma)
{
int		payloadLen;
int		bytesWritten	=	0;

	if (jsonTextBuffer != NULL)
	{
		//*	calculate the length of what we are adding to the buffer
		payloadLen		=	8;
		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	#ifdef _MAKE_JSON_PRETTY_
		strcat(jsonTextBuffer, "\t}");
	#else
		strcat(jsonTextBuffer, "}");
	#endif
		if (includeTrailingComma)
		{
			strcat(jsonTextBuffer, ",");
		}
		strcat(jsonTextBuffer, "\r\n");

	}
	return(bytesWritten);
}

//*****************************************************************************
int		JsonResponse_Add_RawText(	const int		socketFD,
									char			*jsonTextBuffer,
									const int		maxLen,
									const char		*rawTextBuffer)
{
int		payloadLen;
int		bytesWritten	=	0;

#ifdef _DEBUG_JSON_RESPONSE_
	if ((jsonTextBuffer != NULL) && (rawTextBuffer != NULL))
	{

		//*	calculate the length of what we are adding to the buffer
		payloadLen	=	strlen(rawTextBuffer);
		CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
		CONSOLE_DEBUG_W_NUM("payloadLen            \t=", payloadLen);

		bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen);
		CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));

		strcat(jsonTextBuffer, rawTextBuffer);
		CONSOLE_DEBUG(__FUNCTION__);
	}
	else
	{
		CONSOLE_DEBUG("Internal error");
	}
#else
	//*	calculate the length of what we are adding to the buffer
	payloadLen	=	strlen(rawTextBuffer);

	bytesWritten	=	JsonRespnse_XmitIfFull(socketFD, jsonTextBuffer, maxLen, payloadLen, false);

	strcat(jsonTextBuffer, rawTextBuffer);
#endif // _DEBUG_JSON_RESPONSE_
	return(bytesWritten);
}

//*****************************************************************************
//*	returns bytes written
//*****************************************************************************
int			JsonResponse_Add_Finish(const int		socketFD,
									char			*jsonTextBuffer,
									const int		maxLen,
									bool			includeHeader)
{
char	fullDataBuffer[kMaxJsonBuffLen];
int		bytesWritten	=	0;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
//	CONSOLE_DEBUG_W_NUM("kMaxJsonBuffLen      \t=", kMaxJsonBuffLen);

	fullDataBuffer[0]	=	0;

	if (jsonTextBuffer != NULL)
	{
//CONSOLE_DEBUG(__FUNCTION__);
		fullDataBuffer[0]	=	0;
		strcat(jsonTextBuffer, "}\r\n");
//CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));

		if (strlen(jsonTextBuffer) >= sizeof(fullDataBuffer))
		{
			CONSOLE_DEBUG("Buffer overflow!!!!!!!!!!!");
			CONSOLE_ABORT("Buffer overflow!!!!!!!!!!!");
		}

CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
		if (includeHeader)
		{
			JsonResponse_FinishHeader(	fullDataBuffer, jsonTextBuffer);
		}
		else
		{
			fullDataBuffer[0]	=	0;
		}
CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
CONSOLE_DEBUG_W_NUM("len of fullDataBuffer\t=", strlen(fullDataBuffer));
		strcat(fullDataBuffer, jsonTextBuffer);
CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
CONSOLE_DEBUG_W_NUM("len of fullDataBuffer\t=", strlen(fullDataBuffer));

//		CONSOLE_DEBUG_W_STR("jsonTextBuffer=", jsonTextBuffer);
//		CONSOLE_DEBUG_W_STR("Full json message=\r\n", fullDataBuffer);

		bytesWritten	=	JsonResponse_SendTextBuffer(socketFD, fullDataBuffer);
//		bytesWritten	=	JsonResponse_SendTextBuffer(socketFD, jsonTextBuffer);
		jsonTextBuffer[0]	=	0;

	}
	else
	{
		CONSOLE_DEBUG("jsonTextBuffer was NULL");
	}
	return(bytesWritten);
}

//*****************************************************************************
int	JsonResponse_SendTextBuffer(const int socketFD, char *jsonTextBuffer)
{
int		bytesWritten	=	0;
size_t	bufLen;
int		tryCount;
bool	keepTrying;

	if (jsonTextBuffer != NULL)
	{
CONSOLE_DEBUG_W_NUM("len of jsonTextBuffer\t=", strlen(jsonTextBuffer));
//		CONSOLE_DEBUG_W_STR("jsonTextBuffer\t=", jsonTextBuffer);
		bufLen			=	strlen(jsonTextBuffer);
		tryCount		=	0;
		keepTrying		=	true;
		while (keepTrying && (tryCount < 10))
		{
//CONSOLE_DEBUG_W_NUM("SSIZE_MAX\t=", SSIZE_MAX);
CONSOLE_DEBUG_W_NUM("bufLen   \t=", bufLen);
CONSOLE_DEBUG_W_NUM("Calling write with socketFD=", socketFD);
			bytesWritten	=	write(socketFD, jsonTextBuffer, bufLen);
CONSOLE_DEBUG_W_NUM("bytesWritten=", bytesWritten);
			if (bytesWritten > 0)
			{
				keepTrying			=	false;
				jsonTextBuffer[0]	=	0;	//*	reset the buffer
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("tryCount\t=", tryCount);
				CONSOLE_DEBUG_W_NUM("Error writting to socket, socketFD\t=", socketFD);
				CONSOLE_DEBUG_W_NUM("Error writting to socket, errno\t=", errno);
			//	sleep(1);
			}
			tryCount++;
		}
	}
	else
	{
		CONSOLE_DEBUG("jsonTextBuffer was NULL");
	}
	return(bytesWritten);
}
