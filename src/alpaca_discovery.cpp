//**************************************************************************
//*	Name:			alpaca_discovery.cpp
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*	May  3,	2020	<MLS> Created alpacadriver_discovery.cpp
//*	May  3,	2020	<MLS> Added SendDiscoveryQuery() and supporting routines
//*	May  3,	2020	<MLS> Added ProcessDiscovery()
//*****************************************************************************
//*	This set of routines allow a driver to discover other devices and query them
//*****************************************************************************

#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"discovery_lib.h"

#include	"JsonResponse.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"alpaca_defs.h"
#include	"sendrequest_lib.h"

#define	kReceiveBufferSize	1550



//**************************************************************************************
void	AlpacaDriver::HandleDiscoveryResponse(void)
{
struct sockaddr_in	fromAddress;
int					rcvCnt;
unsigned int		fromlen;
int					sendtoRetCode;
char				buf[kReceiveBufferSize + 1];
char				ipAddressStr[INET_ADDRSTRLEN];
SJP_Parser_t		jsonParser;
int					timeOutCntr;
int					iii;
int					ipPortNumber;
char				broadCastMsg[]	=	kAlpacaDiscoveryMsg;
int					closeRetCode;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	send the broadcast message to everyone
	sendtoRetCode	=	sendto(	cBroadcastSocket,
								broadCastMsg,
								strlen(broadCastMsg),
								0,
								(struct sockaddr *)&cServer_addr,
								sizeof(struct sockaddr_in));
	if (sendtoRetCode >= 0)
	{
		timeOutCntr	=	0;
		fromlen		=	sizeof(struct sockaddr_in);
		while (timeOutCntr < 2)
		{
			rcvCnt	=	recvfrom(cBroadcastSocket, buf, kReceiveBufferSize, 0, (struct sockaddr *)&fromAddress, &fromlen);
			if (rcvCnt > 0)
			{
				buf[rcvCnt]	=	0;
				SJP_Init(&jsonParser);
				SJP_ParseData(&jsonParser, buf);
				//*	we have the IP address of the device in "fromAddress"
				//*	find the port from the JSON data
				//*	now find the alpaca port
				ipPortNumber	=	-1;
				for (iii=0; iii<jsonParser.tokenCount_Data; iii++)
				{
					if (strcmp(jsonParser.dataList[iii].keyword, "ALPACAPORT") == 0)
					{
						ipPortNumber=	atoi(jsonParser.dataList[iii].valueString);
					}
				}
				if (ipPortNumber > 0)
				{
					QueryConfiguredDevices(&fromAddress, ipPortNumber);
				}


				inet_ntop(AF_INET, &(fromAddress.sin_addr), ipAddressStr, INET_ADDRSTRLEN);

//				write(1, buf, rcvCnt);
//				write(1, " ", 1);
//				write(1, ipAddressStr, strlen(ipAddressStr));
//				write(1, "\r\n", 2);
			}
			else if (rcvCnt == 0)
			{
				CONSOLE_DEBUG("no response");
			}
			else
			{
			//	perror("recvfrom");
				timeOutCntr++;
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("sendto returned error");
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		perror("sendto");
	}
	closeRetCode	=	close(cBroadcastSocket);
	if (closeRetCode != 0)
	{
		CONSOLE_DEBUG_W_NUM("close() returned an error", closeRetCode);
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);
	}
	cBroadcastSocket		=	-1;

	ReadExternalIPlist();

	cDiscoveryThreadActive	=	false;

}

//**************************************************************************************
void	AlpacaDriver::QueryConfiguredDevices(struct sockaddr_in *deviceAddress, const int ipPortNumber)
{
SJP_Parser_t	jsonParser;
bool			validData;
int				jjj;
char			deviceType[64];
int				deviceNumber;

//	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	validData	=	GetJsonResponse(	deviceAddress,
										ipPortNumber,
										"/management/v1/configureddevices",
										"",
										&jsonParser);
	if (validData)
	{
		strcpy(deviceType, "");
		deviceNumber	=	-1;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR("keyword\t=",		jsonParser.dataList[jjj].keyword);
//			CONSOLE_DEBUG_W_STR("valueString\t=",	jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "DEVICETYPE") == 0)
			{
				strcpy(deviceType, jsonParser.dataList[jjj].valueString);
//				CONSOLE_DEBUG_W_STR("deviceType\t=",		deviceType);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "DEVICENUMBER") == 0)
			{
				deviceNumber	=	atoi(jsonParser.dataList[jjj].valueString);
//				CONSOLE_DEBUG_W_NUM("deviceNumber\t=", deviceNumber);
			}
			else if (strncasecmp(jsonParser.dataList[jjj].keyword, "ARRAY", 5) == 0)
			{
				if ((strlen(deviceType) > 0) && (deviceNumber >= 0))
				{
//					CONSOLE_DEBUG_W_STR("deviceType\t=",	deviceType);
//					CONSOLE_DEBUG_W_NUM("deviceNumber\t=",	deviceNumber);

					//*	this tells the device driver what is available
					//*	it is up to the device driver to decide if it wants to know about the device
					ProcessDiscovery(	deviceAddress,
										ipPortNumber,
										deviceType,
										deviceNumber);
				}
			}
			else
			{
//				CONSOLE_DEBUG_W_STR("keyword\t=",		jsonParser.dataList[jjj].keyword);
			}
		}
	}
	else
	{
	}
}

//*****************************************************************************
void	AlpacaDriver::ReadExternalIPlist(void)
{
FILE				*filePointer;
char				lineBuff[256];
char				outputIPaddr[256];
int					ii;
int					slen;
char				fileName[]	=	"external_ip_list.txt";
SJP_Parser_t		jsonParser;
struct sockaddr_in	from;
int					ipPortNumber;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	see if there is a file listing extra IP address
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
		ipPortNumber	=	6800;	//*	my default port
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (ii=0; ii<slen; ii++)
			{
				if ((lineBuff[ii] == 0x0d) || (lineBuff[ii] == 0x0a))
				{
					lineBuff[ii]	=	0;
					break;
				}
			}
			CONSOLE_DEBUG_W_STR("External IP address\t=",		lineBuff);
			slen	=	strlen(lineBuff);
			if ((slen > 6) && (lineBuff[0] != '#'))
			{
				SJP_Init(&jsonParser);
		//		from.sin_addr.s_addr	=	htonl((192 << 24) + (168 << 16) + (50 << 8) + 46);
				inet_pton(AF_INET, lineBuff, &(from.sin_addr));

				inet_ntop(AF_INET, &(from.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
				CONSOLE_DEBUG_W_STR("outputIPaddr\t\t=",		outputIPaddr);

				strcpy(jsonParser.dataList[0].keyword, "ALPACAPORT");
				strcpy(jsonParser.dataList[0].valueString, "6800");
				jsonParser.tokenCount_Data	=	1;

				QueryConfiguredDevices(&from, ipPortNumber);
		//-		AddUnitToList(&from, &jsonParser);
			}
		}
		fclose(filePointer);
	}
}

//**************************************************************************************
static void	*DiscoveryResponseThread(void *arg)
{
AlpacaDriver	*parentThisPtr;

	CONSOLE_DEBUG(__FUNCTION__);
	parentThisPtr	=	(AlpacaDriver *)arg;
	if (parentThisPtr != NULL)
	{
		CONSOLE_DEBUG_W_STR("Thread started from device", parentThisPtr->cDeviceName);
		parentThisPtr->HandleDiscoveryResponse();
	}
	else
	{
		CONSOLE_DEBUG("arg is null");
	}
	CONSOLE_DEBUG("Thread exit--------------------------------------------------------");
	return(NULL);
}

//**************************************************************************************
//*	Send out a discovery protocol request
//*	This is NOT to be overridden
//*
//*		Set up and send the broadcast packet
//*		Create a thread that will process the responses and exit
//*
//**************************************************************************************
bool		AlpacaDriver::SendDiscoveryQuery(void)
{
int					setOptRetCode;
int					bindRetCode;
struct timeval		timeoutLength;
bool				success	=	true;
struct sockaddr_in	myClient_addr;
int					sockOptValue;
int					threadErr;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cDiscoveryThreadActive == false)
	{
		cBroadcastSocket	=	socket(AF_INET, SOCK_DGRAM, 0);
		if (cBroadcastSocket  >= 0)
		{
			cDiscoveryCount++;	//*	keep track of how many times we have done this

			memset(&cServer_addr, '\0', sizeof(struct sockaddr_in));
			cServer_addr.sin_family			=	AF_INET;
			cServer_addr.sin_port			=	htons(kAlpacaDiscoveryPORT);
			cServer_addr.sin_addr.s_addr	=	htonl(INADDR_BROADCAST);

			myClient_addr.sin_family		=	AF_INET;
			myClient_addr.sin_addr.s_addr	=	htonl(INADDR_ANY);
			myClient_addr.sin_port			=	htons(0);

			sockOptValue	=	1;
			setOptRetCode	=	setsockopt(cBroadcastSocket, SOL_SOCKET, (SO_BROADCAST), &sockOptValue, sizeof(int));
			if (setOptRetCode >= 0)
			{
				//*	set a timeout
				timeoutLength.tv_sec	=	1;
				timeoutLength.tv_usec	=	0;
				setOptRetCode			=	setsockopt(	cBroadcastSocket,
														SOL_SOCKET,
														SO_RCVTIMEO,
														&timeoutLength,
														sizeof(timeoutLength));


				bindRetCode	=	bind(cBroadcastSocket, (const struct sockaddr *)&myClient_addr, sizeof(myClient_addr));
				if (bindRetCode >= 0)
				{
					CONSOLE_DEBUG("Creating thread");
					threadErr	=	pthread_create(	&cDiscoveryThreadID,
													NULL,
													&DiscoveryResponseThread,
													this);
					if (threadErr == 0)
					{
						cDiscoveryThreadActive	=	true;
					}
					else
					{
						CONSOLE_DEBUG("Exiting creating thread");
					}
				}
				else
				{
					perror("bind failed");
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				perror("setsockopt(SO_BROADCAST) failed");
				success	=	false;
			}

		}
		else
		{
			perror("socket creation failed");
			exit(EXIT_FAILURE);
		}


		CONSOLE_DEBUG_W_STR("success\t=", (success ? "true" : "false"));

	//	CONSOLE_DEBUG("Exiting for now, if we got here, we are doing good");
	//	CONSOLE_DEBUG("Sleeping");
	//	sleep(5);
		CONSOLE_DEBUG_W_NUM("DONE-------------------------------------- #", cDiscoveryCount);
	//	exit(0);
	}
	else
	{
		CONSOLE_DEBUG("Thread is already active, skipping");
	}
	return(success);
}


//**************************************************************************************
void	AlpacaDriver::ProcessDiscovery(	struct sockaddr_in	*deviceAddress,
										const int			ipPortNumber,
										const char			*deviceType,
										const int			deviceNumber)
{
	//*	do nothing, this routine should be overridden
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("deviceType\t=",	deviceType);

}
