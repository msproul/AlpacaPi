//*****************************************************************************
//*
//*	Name:			sendrequest_lib.c
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	Alpaca send request library
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
//*	Apr 30,	2020	<MLS> Created sendrequest_lib.c
//*	May 28,	2020	<MLS> Added timeout to SendPutCommand()
//*	Jun 22,	2020	<MLS> Added OpenSocketAndSendRequest()
//*	Jan 14,	2021	<MLS> Fixed GET/PUT request to have all the right header stuff
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<ctype.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"json_parse.h"


#include	"sendrequest_lib.h"

//#define		_USE_SOCKET_LINGER_

//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
int	OpenSocketAndSendRequest(	struct sockaddr_in	*deviceAddress,
								const int			port,
								const char			*get_put_string,	//*	must be either GET or PUT
								const char			*sendData,
								const char			*dataString)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
char				xmitBuffer[kReadBuffLen];
char				linebuf[100];
int					dataStrLen;
char				ipString[32];
struct timeval		timeoutLength;
int					setOptRetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(sendData);
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	set a timeout
		timeoutLength.tv_sec	=	1;
		timeoutLength.tv_usec	=	0;
		setOptRetCode			=	setsockopt(	socket_desc,
												SOL_SOCKET,
												SO_RCVTIMEO,
												&timeoutLength,
												sizeof(timeoutLength));
		if (setOptRetCode < 0)
		{
			perror("setsockopt(SO_RCVTIMEO) failed");
		}
		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			strcpy(xmitBuffer, "GET ");
			strcat(xmitBuffer, sendData);
			strcat(xmitBuffer, "\r\n");
			strcat(xmitBuffer, "User-Agent: Alpaca\r\n");
			strcat(xmitBuffer, "accept: application/json\r\n");
			if (dataString != NULL)
			{
				dataStrLen	=	strlen(dataString);
				sprintf(linebuf, "Content-Length: %d\r\n", dataStrLen);
				strcat(xmitBuffer, linebuf);
				strcat(xmitBuffer, "\r\n");

				strcat(xmitBuffer, dataString);
				strcat(xmitBuffer, "\r\n");
			}

//			CONSOLE_DEBUG(xmitBuffer);

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
				//*	success
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("sendRetCode\t=", sendRetCode);
			}
		}
		else if (errno == ECONNREFUSED)
		{

			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect error, ipaddress\t=",	ipString);
			CONSOLE_DEBUG_W_STR("connect error, send data\t=",	sendData);
			CONSOLE_DEBUG_W_NUM("errno\t\t\t=", errno);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
		CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
	}
	return(socket_desc);
}


//*****************************************************************************
bool	GetJsonResponse(	struct sockaddr_in	*deviceAddress,
							const int			port,
							const char			*sendData,
							const char			*dataString,
							SJP_Parser_t		*jsonParser)
{
bool				validData;
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
int					shutDownRetCode;
int					closeRetCode;
int					recvByteCnt;
char				returnedData[kReadBuffLen + 10];
char				xmitBuffer[kReadBuffLen + 10];
char				longBuffer[kLargeBufferSize + 10];
char				linebuf[100];
int					dataStrLen;
char				ipString[32];
#ifdef _USE_SOCKET_LINGER_
	int					setOptRetCode;
	struct linger		myLingerStruct;
#endif // _USE_SOCKET_LINGER_

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(sendData);
	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
	#ifdef _USE_SOCKET_LINGER_
		myLingerStruct.l_onoff	=	1;
		myLingerStruct.l_linger	=	2;
		setOptRetCode	=	setsockopt(	socket_desc,
										SOL_SOCKET,
										SO_LINGER,
										&myLingerStruct,
										sizeof(myLingerStruct));
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("setsockopt returned\t=",	setOptRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t\t\t=",				errno);
		}
	#endif // _USE_SOCKET_LINGER_
		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			strcpy(xmitBuffer, "GET ");
			strcat(xmitBuffer, sendData);
			strcat(xmitBuffer, "\r\n");
			strcat(xmitBuffer, "User-Agent: Alpaca\r\n");
			strcat(xmitBuffer, "accept: application/json\r\n");
			if (dataString != NULL)
			{
				dataStrLen	=	strlen(dataString);
				sprintf(linebuf, "Content-Length: %d\r\n", dataStrLen);
				strcat(xmitBuffer, linebuf);
				strcat(xmitBuffer, "\r\n");

				strcat(xmitBuffer, dataString);
				strcat(xmitBuffer, "\r\n");
			}

//			CONSOLE_DEBUG(xmitBuffer);

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
			bool	keepReading;

				keepReading		=	true;
				longBuffer[0]	=	0;
				while (keepReading && ((strlen(longBuffer) + kReadBuffLen) < kLargeBufferSize))
				{
					recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , 0);
					if (recvByteCnt > 0)
					{
						validData	=	true;
						returnedData[recvByteCnt]	=	0;
						strcat(longBuffer, returnedData);
					}
					else
					{
						keepReading		=	false;
					}
				}
				SJP_Init(jsonParser);
				SJP_ParseData(jsonParser, longBuffer);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("sendRetCode\t=", sendRetCode);
			}
			shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
			if (shutDownRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
				CONSOLE_DEBUG_W_NUM("errno\t=", errno);
			}
		}
		else if (errno == ECONNREFUSED)
		{

			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect error, ipaddress\t=",	ipString);
			CONSOLE_DEBUG_W_STR("connect error, send data\t=",	sendData);
			CONSOLE_DEBUG_W_NUM("errno\t\t\t=", errno);
		}
		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
		CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
	}
	return(validData);
}


//*****************************************************************************
//*		htmlData	= PUT /api/v1/focuser/0/moverelative HTTP/1.1
//*		Host: 127.0.0.1:6800
//*		User-Agent: curl/7.64.0
//*		accept: application/json
//*		Content-Type: application/x-www-form-urlencoded
//*		Content-Length: 13
//*****************************************************************************
bool	SendPutCommand(	struct sockaddr_in	*deviceAddress,
						const int			port,
						const char			*putCommand,
						const char			*dataString,
						SJP_Parser_t		*jsonParser)
{
bool				validData;
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
int					shutDownRetCode;
int					closeRetCode;
int					recvByteCnt;
char				returnedData[kReadBuffLen];
char				xmitBuffer[kReadBuffLen];
char				linebuf[128];
int					dataStrLen;
char				ipString[32];
struct timeval		timeoutLength;
int					setOptRetCode;

//	CONSOLE_DEBUG_W_STR("putCommand\t=", putCommand);
//	CONSOLE_DEBUG_W_STR("dataString\t=", dataString);

	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	set a timeout
		timeoutLength.tv_sec	=	2;
		timeoutLength.tv_usec	=	0;
		setOptRetCode			=	setsockopt(	socket_desc,
												SOL_SOCKET,
												SO_RCVTIMEO,
												&timeoutLength,
												sizeof(timeoutLength));
		if (setOptRetCode < 0)
		{
			perror("setsockopt(SO_BROADCAST) failed");
		}

		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
//			CONSOLE_DEBUG_W_STR("connect open", ipString);

//	PUT /api/v1/dome/0/openshutter HTTP/1.1
//	Host: test:6800
//	User-Agent: curl/7.47.0
//	accept: application/json
//	Content-Type: application/x-www-form-urlencoded
//	Content-Length: 32

//	ClientID=2&ClientTransactionID=4


			strcpy(xmitBuffer, "PUT ");
			strcat(xmitBuffer, putCommand);

		strcat(xmitBuffer, " HTTP/1.1\r\n");
		strcat(xmitBuffer, "Host: 127.0.0.1:6800\r\n");
		strcat(xmitBuffer, "User-Agent: AlpacaPi\r\n");
//		strcat(xmitBuffer, "Connection: keep-alive\r\n");
		strcat(xmitBuffer, "Accept: text/html,application/json\r\n");

			if (dataString != NULL)
			{
				dataStrLen	=	strlen(dataString);
				sprintf(linebuf, "Content-Length: %d\r\n", dataStrLen);
				strcat(xmitBuffer, linebuf);
				strcat(xmitBuffer, "\r\n");

				strcat(xmitBuffer, dataString);
				strcat(xmitBuffer, "\r\n");
			}
			else
			{
				strcat(xmitBuffer, "\r\n");
			}
			CONSOLE_DEBUG_W_STR("Sending:", xmitBuffer);

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
				recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , 0);
				if (recvByteCnt >= 0)
				{
					validData	=	true;
					returnedData[recvByteCnt]	=	0;
					SJP_Init(jsonParser);
					SJP_ParseData(jsonParser, returnedData);
				}
			}
			shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
			if (shutDownRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
				CONSOLE_DEBUG_W_NUM("errno\t=", errno);
			}
			closeRetCode	=	close(socket_desc);
			if (closeRetCode != 0)
			{
				CONSOLE_DEBUG("Close error");
			}
		}
		else if (errno == ECONNREFUSED)
		{

			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
			CONSOLE_DEBUG("connect error");
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(validData);
}



//*****************************************************************************
void	PrintIPaddressToString(const long ipAddress, char *ipString)
{
	inet_ntop(AF_INET, &ipAddress, ipString, INET_ADDRSTRLEN);

}
