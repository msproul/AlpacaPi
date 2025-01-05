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
//*	Jan 21,	2021	<MLS> Must use HTTP/1.0 to disable "Transfer-Encoding: chunked"
//*	Jan 30,	2021	<MLS> Changed TCP socket timeout to 4 seconds
//*	Feb  3,	2021	<MLS> Added checking for ERRNO=24 (EMFILE), too many files open
//*	Mar 13,	2021	<MLS> Added timeout to GetJsonResponse()
//*	Mar 13,	2021	<MLS> Added SetSocketTimeouts()
//*	Mar 13,	2021	<MLS> Added send timeout
//*	Sep  4,	2021	<MLS> Added microsecs arg to SetSocketTimeouts()
//*	Sep  8,	2021	<MLS> Added "Connection: close" as per suggestion from Patrick Chevalley
//*	Dec 14,	2021	<MLS> Added imagebytes option to OpenSocketAndSendRequest()
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

//#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"json_parse.h"


#include	"sendrequest_lib.h"
#include	"linuxerrors.h"

#define		kTimeOutLenSeconds	5
//#define		kTimeOutLenSeconds	1


int		gNumSocketOpenOKcnt		=	0;
int		gNumSocketOpenErrCnt	=	0;
int		gNumSocketConnOKcnt		=	0;
int		gNumSocketConnErrCnt	=	0;
bool	gReportError			=	true;
bool	gEnableDebug			=	false;

//*****************************************************************************
void	Set_SendRequestLibDebug(bool enableFlag)
{
	gEnableDebug	=	enableFlag;
}

//*****************************************************************************
static int	SetSocketTimeouts(int socket_desc, int timeOut_Secs, int timeOut_microSecs)
{
struct timeval		timeoutLength;
int					setOptRetCode;

	//*	set a timeout
	timeoutLength.tv_sec	=	timeOut_Secs;
	timeoutLength.tv_usec	=	timeOut_microSecs;
	setOptRetCode			=	setsockopt(	socket_desc,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));

	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_RCVTIMEO) failed");
	}
	setOptRetCode			=	setsockopt(	socket_desc,
											SOL_SOCKET,
											SO_SNDTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));
	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_SNDTIMEO) failed");
	}
	return(setOptRetCode);
}

//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
int	OpenSocketAndSendRequest(	struct sockaddr_in	*deviceAddress,
								const int			port,
								const char			*get_put_string,	//*	must be either GET or PUT
								const char			*sendData,
								const char			*dataString,
								const bool			includeImageBinary)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
char				xmitBuffer[kReadBuffLen];
char				linebuf[100];
int					dataStrLen;
char				ipString[32];
int					setOptRetCode;
int					so_oobinline;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("includeImageBinary\t=", includeImageBinary);
//	CONSOLE_DEBUG(sendData);
	inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		gNumSocketOpenOKcnt++;

		//*	set a timeout
		setOptRetCode	=	SetSocketTimeouts(socket_desc, kTimeOutLenSeconds, 0);
//		setOptRetCode	=	SetSocketTimeouts(socket_desc, 0, 25000);
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG("SetSocketTimeouts() failed");
		}

		//*	turn out of band off
		so_oobinline	=	0;
		setOptRetCode	=	setsockopt(	socket_desc,
										SOL_SOCKET,
										SO_OOBINLINE,
										&so_oobinline,
										sizeof(so_oobinline));
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("setsockopt() returned", setOptRetCode);
		}


		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			gNumSocketConnOKcnt++;
			//*	Must be HTTP/1.0 to disable "Transfer-Encoding: chunked"
			strcpy(xmitBuffer,	"GET ");
			strcat(xmitBuffer,	sendData);
			strcat(xmitBuffer,	" HTTP/1.0\r\n");
//			strcat(xmitBuffer,	"Host: 127.0.0.1:6800");
			sprintf(linebuf,	"Host: %s:%d\r\n", ipString, port);
			strcat(xmitBuffer,	linebuf);
			if (strlen(gUserAgentAlpacaPiStr))
			{
				//*	add User-Agent:
				strcat(xmitBuffer,	gUserAgentAlpacaPiStr);
			}
			strcat(xmitBuffer,	"Accept: text/html,application/json");
			if (includeImageBinary)
			{
				strcat(xmitBuffer,	",application/imagebytes");
			}
			strcat(xmitBuffer,	"\r\n");

			strcat(xmitBuffer,	"Connection: close\r\n");
//			strcat(xmitBuffer,	"Accept: application/json, text/json, text/x-json, text/javascript, application/xml, text/xml\r\n");
//			strcat(xmitBuffer,	"Accept-Language: en-US,en;q=0.5\r\n");

			strcat(xmitBuffer, "\r\n");
//
			if (dataString != NULL)
			{
				dataStrLen	=	strlen(dataString);
				sprintf(linebuf, "Content-Length: %d\r\n", dataStrLen);
				strcat(xmitBuffer, linebuf);
				strcat(xmitBuffer, "\r\n");

				strcat(xmitBuffer, dataString);
				strcat(xmitBuffer, "\r\n");
			}
			//*	this EXTRA CR/LF is VERY important for Alpaca Remote Server
			strcat(xmitBuffer, "\r\n");

//			CONSOLE_DEBUG(xmitBuffer);

//			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , MSG_NOSIGNAL);
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
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
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
//	CONSOLE_DEBUG("exit");
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
bool				keepReading;
int					setOptRetCode;
int					readLoopCnt;		//*	for debugging
int					readSuccessCnt;		//*	for debugging
int					parseReturnCode;

	if (gEnableDebug)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "------start-------");
		CONSOLE_DEBUG(sendData);
		CONSOLE_DEBUG_W_SIZE("sizeof(xmitBuffer)  \t=", sizeof(xmitBuffer));
		CONSOLE_DEBUG_W_SIZE("sizeof(returnedData)\t=", sizeof(returnedData));
		CONSOLE_DEBUG_W_SIZE("sizeof(longBuffer)  \t=", sizeof(longBuffer));
	}
	inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

	SETUP_TIMING();

	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		if (gEnableDebug)
		{
			CONSOLE_DEBUG("Setting Timeout");
		}
		//*	set a timeout
		setOptRetCode	=	SetSocketTimeouts(socket_desc, kTimeOutLenSeconds, 0);
	//	setOptRetCode	=	SetSocketTimeouts(socket_desc, 0, 25000);
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG("SetSocketTimeouts() failed");
		}
		gNumSocketOpenOKcnt++;
		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);

		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
//			CONSOLE_DEBUG("Connected");

			gNumSocketConnOKcnt++;
//	GET /api/v1/camera/0/supportedactions HTTP/1.1
//	Host: newt16:6800
//	User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:71.0) Gecko/20100101 Firefox/71.0
//	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//	Accept-Language: en-US,en;q=0.5
//	Accept-Encoding: gzip, deflate
//	Connection: keep-alive
//	Upgrade-Insecure-Requests: 1
//	Cache-Control: max-age=0

//	GET /api/v1/camera/0/supportedactions HTTP/1.1
//	Host: ascom:11111
//	User-Agent: AlpacaPi
//	Accept: text/html,application/json
//	Accept-Language: en-US,en;q=0.5
//	Connection: keep-alive


			if (gEnableDebug)
			{
				CONSOLE_DEBUG("Building xmitBuffer");
			}
			strcpy(xmitBuffer,	"GET ");
			strcat(xmitBuffer,	sendData);
			strcat(xmitBuffer,	" HTTP/1.0\r\n");
//			strcat(xmitBuffer,	"Host: ascom:11111\r\n");
			sprintf(linebuf,	"Host: %s:%d\r\n", ipString, port);
			strcat(xmitBuffer,	linebuf);
//			strcat(xmitBuffer,	"User-Agent: AlpacaPi\r\n");
			if (strlen(gUserAgentAlpacaPiStr))
			{
				//*	add User-Agent:
				strcat(xmitBuffer,	gUserAgentAlpacaPiStr);
			}
			strcat(xmitBuffer,	"Accept: text/html,application/json\r\n");

			strcat(xmitBuffer,	"Accept-Language: en-US,en;q=0.5\r\n");
			strcat(xmitBuffer,	"Connection: close\r\n");

			if (gEnableDebug)
			{
				CONSOLE_DEBUG_W_SIZE("length xmitBuffer\t=", strlen(xmitBuffer));
			}

//			if (strstr(sendData, "switch") !=  NULL)
//			{
//				CONSOLE_DEBUG_W_STR("xmitBuffer\t=", xmitBuffer);
//			}

			if (dataString != NULL)
			{
//				CONSOLE_DEBUG_W_STR("dataString\t=", dataString);
				dataStrLen	=	strlen(dataString);
				sprintf(linebuf, "Content-Length: %d\r\n", dataStrLen);
				strcat(xmitBuffer, linebuf);
				strcat(xmitBuffer, "\r\n");

				strcat(xmitBuffer, dataString);
				strcat(xmitBuffer, "\r\n");
			}
			//*	this EXTRA CR/LF is VERY important for Alpaca Remote Server
			strcat(xmitBuffer, "\r\n");

			if (gEnableDebug)
			{
				CONSOLE_DEBUG_W_SIZE("length xmitBuffer\t=", strlen(xmitBuffer));
			}

//			CONSOLE_DEBUG(xmitBuffer);

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , MSG_NOSIGNAL);
			if (sendRetCode >= 0)
			{
				if (gEnableDebug)
				{
					CONSOLE_DEBUG("Request sent");
				}
				keepReading		=	true;
				longBuffer[0]	=	0;
				readLoopCnt		=	0;
				readSuccessCnt	=	0;
				while (keepReading && ((strlen(longBuffer) + kReadBuffLen) < kLargeBufferSize))
				{
					readLoopCnt++;
				//	recvByteCnt	=	recv(socket_desc, returnedData, kReadBuffLen, 0);
					recvByteCnt	=	recv(socket_desc, returnedData, kReadBuffLen, MSG_NOSIGNAL);
					if (gEnableDebug)
					{
						CONSOLE_DEBUG_W_NUM("errno      \t=",	errno);
						CONSOLE_DEBUG_W_NUM("recvByteCnt\t=",	recvByteCnt);
					}
					if (recvByteCnt > 0)
					{
						readSuccessCnt++;
						returnedData[recvByteCnt]	=	0;
//						CONSOLE_DEBUG_W_NUM("recvByteCnt\t=",	recvByteCnt);
//						if (strstr(sendData, "switch") !=  NULL)
//						{
//							CONSOLE_DEBUG_W_STR("returnedData\t=", returnedData);
//						}
						validData	=	true;

//						CONSOLE_DEBUG_W_STR("returnedData\t=",	returnedData);

						strcat(longBuffer, returnedData);
					}
					else
					{
						keepReading		=	false;
					}
				}
				if (gEnableDebug)
				{
					CONSOLE_DEBUG_W_NUM("readLoopCnt   \t=",	readLoopCnt);
					CONSOLE_DEBUG_W_NUM("readSuccessCnt\t=",	readSuccessCnt);
					CONSOLE_DEBUG_W_STR("longBuffer    \t=",	longBuffer);
				}
				SJP_Init(jsonParser);
				parseReturnCode	=	SJP_ParseData(jsonParser, longBuffer);
				if ((parseReturnCode != 0) || gEnableDebug)
				{
					CONSOLE_DEBUG_W_NUM("parseReturnCode   \t=",	parseReturnCode);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("sendRetCode\t=", sendRetCode);
			}
			if (gEnableDebug)
			{
				CONSOLE_DEBUG("Calling shutdown");
			}
			shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
			if (shutDownRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
				CONSOLE_DEBUG_W_NUM("errno\t=", errno);
				CONSOLE_DEBUG(strerror(errno));

			}
//			CONSOLE_DEBUG(__FUNCTION__);
		}
		else if (errno == ECONNREFUSED)
		{
		char	portString[32];

			sprintf(portString, ":%d", port);
			CONSOLE_DEBUG_W_2STR("connect refused", ipString, portString);
		}
		else
		{
		char	errorString[64];

			CONSOLE_DEBUG_W_STR("connect error, ipaddress\t=",	ipString);
			CONSOLE_DEBUG_W_STR("connect error, send data\t=",	sendData);
		//	CONSOLE_DEBUG_W_NUM("errno\t\t\t=", errno);

			GetLinuxErrorString(errno, errorString);

			CONSOLE_DEBUG_W_STR("Error message\t\t=",	errorString);
		}
		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
//		CONSOLE_DEBUG(__FUNCTION__);
	}
	else
	{
		if (errno == EMFILE)
		{
			CONSOLE_DEBUG("Too many files open!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
			CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
		}
	}
	if (gEnableDebug)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
	}
	DEBUG_TIMING("Delta time for GetJsonResponse()=");
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
int					setOptRetCode;

//	CONSOLE_DEBUG_W_STR("putCommand\t=", putCommand);
//	CONSOLE_DEBUG_W_STR("dataString\t=", dataString);
//	CONSOLE_ABORT(__FUNCTION__);

	inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	set a timeout
		setOptRetCode	=	SetSocketTimeouts(socket_desc, kTimeOutLenSeconds, 0);
	//	setOptRetCode	=	SetSocketTimeouts(socket_desc, 0, 25000);
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG("SetSocketTimeouts() failed");
		}

		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
//			CONSOLE_DEBUG_W_STR("connect open", ipString);

			//	PUT /api/v1/dome/0/openshutter HTTP/1.1
			//	Host: test:6800
			//	User-Agent: curl/7.47.0
			//	accept: application/json
			//	Content-Type: application/x-www-form-urlencoded
			//	Content-Length: 32

			//	ClientID=2&ClientTransactionID=4


			//PUT /api/v1/camera/0/connected HTTP/1.1
			//Host: newt16:6800
			//User-Agent: curl/7.58.0
			//accept: application/json
			//Content-Length: 14
			//Content-Type: application/x-www-form-urlencoded
			//
			//Connected=true

			//PUT /api/v1/camera/0/connected HTTP/1.0
			//Host: 127.0.0.1:6800
			//User-Agent: AlpacaPi
			//Accept: text/html,application/json
			//Content-Length: 47
			//
			//Connected=true&ClientID=1&ClientTransactionID=1

			//PUT /api/v1/camera/0/connected HTTP/1.0
			//Host: 127.0.0.1:32323
			//User-Agent: AlpacaPi
			//Accept: text/html,application/json
			//Content-Length: 47
			//
			//Connected=true&ClientID=1&ClientTransactionID=1

			strcpy(xmitBuffer,	"PUT ");
			strcat(xmitBuffer,	putCommand);
			strcat(xmitBuffer,	" HTTP/1.0\r\n");
//			strcat(xmitBuffer,	"Host: 192.168.1.156:32323\r\n");
			sprintf(linebuf,	"Host: %s:%d\r\n", ipString, port);
			strcat(xmitBuffer,	linebuf);
//			strcat(xmitBuffer,	"User-Agent: AlpacaPi\r\n");
			if (strlen(gUserAgentAlpacaPiStr))
			{
				//*	add User-Agent:
				strcat(xmitBuffer,	gUserAgentAlpacaPiStr);
			}
			strcat(xmitBuffer,	"Connection: close\r\n");
			strcat(xmitBuffer,	"Accept: text/html,application/json\r\n");
			strcat(xmitBuffer,	"Content-Type: application/x-www-form-urlencoded\r\n");

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
				//*	this EXTRA CR/LF is VERY important for Alpaca Remote Server
				strcat(xmitBuffer, "\r\n");
			}
			strcat(xmitBuffer, "\r\n");
//			CONSOLE_DEBUG_W_STR("Sending:", xmitBuffer);

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , MSG_NOSIGNAL);
			if (sendRetCode >= 0)
			{
				recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , MSG_NOSIGNAL);
				if (recvByteCnt >= 0)
				{
//					CONSOLE_DEBUG("Setting validData to true");
					validData					=	true;
					returnedData[recvByteCnt]	=	0;
					SJP_Init(jsonParser);
					SJP_ParseData(jsonParser, returnedData);
//					CONSOLE_DEBUG_W_STR("returnedData=\r\n", returnedData);
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
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
			CONSOLE_DEBUG("connect error");
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else
	{
		if (errno == EMFILE)
		{
			CONSOLE_DEBUG("Too many files open!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
			CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
		}
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, (validData ? "Valid Data" : "Not Valid"));
	return(validData);
}



//*****************************************************************************
void	PrintIPaddressToString(const long ipAddress, char *ipString)
{
	inet_ntop(AF_INET, &ipAddress, ipString, INET_ADDRSTRLEN);
	if (strlen(ipString) >= 32)
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

