//*****************************************************************************
//*
//*	Name:			discovery_lib.c
//*
//*	Author:			Mark Sproul (C) 2019-2020
//*
//*	Description:	Alpaca discovery library
//*
//*	Limitations:
//*
//*	Usage notes:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 11,	2020	<MLS> Created discover_lib.c to re-organize code for multiple apps
//*	Feb 23,	2020	<MLS> Added LookupNames() to link /etc/hosts to ip addresses
//*	Mar  4,	2020	<MLS> Made GetJsonResponse() handle large buffers
//*	Apr 30,	2020	<MLS> Moved GetJsonResponse() & SendPutCommand() to new file
//*	Jun 10,	2020	<MLS> Added _INCLUDE_WIRELESS_SUBNET_ compile option flag
//*	Jun  2,	2020	<MLS> Added timeout to SendGetRequest()
//*	Aug 13,	2020	<MLS> Added ReadExternalIPlist()
//*	Aug 13,	2020	<MLS> Added Added ability to read external IP address from text file
//*	Jun 24,	2020	<MLS> Removed _INCLUDE_WIRELESS_SUBNET_
//*****************************************************************************



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"json_parse.h"

#define kAlpacaDiscoveryPORT	32227

#define	kReceiveBufferSize	1550

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define		kMaxUnitCount	32

TYPE_ALPACA_UNIT	gAlpacaUnitList[kMaxUnitCount];
int					gAlpacaUnitCnt	=	0;

//TYPE_REMOTE_DEV		gAlpacaIPaddrList[kMaxDeviceListCnt];
//int					gAlpacaDeviceCnt	=	0;

TYPE_REMOTE_DEV		gAlpacaDiscoveredList[kMaxDeviceListCnt];
int					gAlpacaDiscoveredCnt	=	0;;

static	int					gBroadcastSock;
static	struct sockaddr_in	gServer_addr;
static	struct sockaddr_in	gClient_addr;

//*****************************************************************************
static void	InitArrays(void)
{
int		iii;

	for (iii=0; iii<kMaxUnitCount; iii++)
	{
		memset(&gAlpacaUnitList[iii], 0, sizeof(TYPE_ALPACA_UNIT));
	}
	for (iii=0; iii<kMaxDeviceListCnt; iii++)
	{
		memset(&gAlpacaDiscoveredList[iii], 0, sizeof(TYPE_REMOTE_DEV));
	}
	gAlpacaUnitCnt			=	0;
	gAlpacaDiscoveredCnt	=	0;
}

//*****************************************************************************
bool	SetupBroadcast(void)
{
int				setOptRetCode;
int				bindRetCode;
struct timeval	timeoutLength;
bool			success	=	true;
int				argValue;
//	CONSOLE_DEBUG(__FUNCTION__);
	InitArrays();

	gBroadcastSock	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (gBroadcastSock  < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}


	memset(&gServer_addr, '\0', sizeof(struct sockaddr_in));
	gServer_addr.sin_family			=	AF_INET;
	gServer_addr.sin_port			=	htons(kAlpacaDiscoveryPORT);
	gServer_addr.sin_addr.s_addr	=	htonl(INADDR_BROADCAST);

	gClient_addr.sin_family			=	AF_INET;
	gClient_addr.sin_addr.s_addr	=	htonl(INADDR_ANY);
	gClient_addr.sin_port			=	htons(0);

//	inet_ntop(AF_INET, &(gServer_addr.sin_addr), str, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("gServer_addr.sin_addr", str);

//	inet_ntop(AF_INET, &(gClient_addr.sin_addr), str, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("gClient_addr.sin_addr", str);

	argValue		=	1;
	setOptRetCode	=	setsockopt(gBroadcastSock, SOL_SOCKET, (SO_BROADCAST), &argValue, sizeof(int));
	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_BROADCAST) failed");
		success	=	false;
	}

	//*	set a timeout
	timeoutLength.tv_sec	=	2;
	timeoutLength.tv_usec	=	0;
	setOptRetCode			=	setsockopt(	gBroadcastSock,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));


	bindRetCode	=	bind(gBroadcastSock, (const struct sockaddr *)&gClient_addr, sizeof(gClient_addr));
	if (bindRetCode < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	return(success);
}

//*****************************************************************************
//*	a device is an individual Alpaca device, can be more than one per IP address
//*****************************************************************************
static void	UpdateRemoteList(TYPE_REMOTE_DEV *newRemoteDevice)
{
int		ii;
bool	newDevice;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	look to see if it is already in the list
	newDevice	=	true;
	for (ii=0; ii<gAlpacaDiscoveredCnt; ii++)
	{
		//*	check to see if it is already in the list
		if ((newRemoteDevice->deviceAddress.sin_addr.s_addr == gAlpacaDiscoveredList[ii].deviceAddress.sin_addr.s_addr)
			&& (strcmp(newRemoteDevice->deviceTypeStr,	gAlpacaDiscoveredList[ii].deviceTypeStr) == 0)
			&& (strcmp(newRemoteDevice->deviceNameStr,	gAlpacaDiscoveredList[ii].deviceNameStr) == 0)
			&& (newRemoteDevice->alpacaDeviceNum == gAlpacaDiscoveredList[ii].alpacaDeviceNum)
			)
		{
			//*	yep, its already here, dont bother
			newDevice	=	false;
			break;
		}
		//*	I dont want the management device type in the list
		if (strcmp(newRemoteDevice->deviceTypeStr, "management") == 0)
		{
			//*	its a management devices, dont bother
			newDevice	=	false;
			break;
		}
	}
	if (newDevice)
	{
		//*	we have a new devices, add it in (if there's room)
		if (gAlpacaDiscoveredCnt < kMaxDeviceListCnt)
		{
			gAlpacaDiscoveredList[gAlpacaDiscoveredCnt]	=	*newRemoteDevice;
			gAlpacaDiscoveredCnt++;

		}
	}
}

//*****************************************************************************
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theUnit)
{
TYPE_REMOTE_DEV	myRemoteDevice;
int				ii;
char			myVersionString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));
	memset(myVersionString, 0, sizeof(myVersionString));
	strcpy(myVersionString, "??");
	for (ii=0; ii<jsonParser->tokenCount_Data; ii++)
	{
		if (strcasecmp(jsonParser->dataList[ii].keyword, "VERSION") == 0)
		{
			strcpy(myVersionString, jsonParser->dataList[ii].valueString);
		}

		if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICETYPE") == 0)
		{
			strcpy(myRemoteDevice.deviceTypeStr, jsonParser->dataList[ii].valueString);
		}
		if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICENAME") == 0)
		{
			strcpy(myRemoteDevice.deviceNameStr, jsonParser->dataList[ii].valueString);
		}
		if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICENUMBER") == 0)
		{
			myRemoteDevice.alpacaDeviceNum	=	atoi(jsonParser->dataList[ii].valueString);
		}

		//------------------------------------
		if (strcasecmp(jsonParser->dataList[ii].keyword, "ARRAY-NEXT") == 0)
		{
			myRemoteDevice.deviceAddress	=	theUnit->deviceAddress;
			myRemoteDevice.port				=	theUnit->port;
			strcpy(myRemoteDevice.versionString, myVersionString);
			strcpy(myRemoteDevice.hostName,		theUnit->hostName);
			UpdateRemoteList(&myRemoteDevice);

			memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));

		}
	}
}

//*****************************************************************************
static void	SendGetRequest(TYPE_ALPACA_UNIT *theUnit, const char *sendData)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
int					closeRetCode;
int					shutDownRetCode;
int					recvByteCnt;
char				returnedData[kReadBuffLen];
char				xmitBuffer[kReadBuffLen];
SJP_Parser_t		jsonParser;
char				ipAddrSt[32];
struct timeval		timeoutLength;
int					setOptRetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (theUnit->deviceAddress.sin_addr.s_addr != 0)
	{
		inet_ntop(AF_INET, &(theUnit->deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);
		CONSOLE_DEBUG_W_STR("Sending get request to", ipAddrSt);

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
				perror("setsockopt(SO_BROADCAST) failed");
			}


			remoteDev.sin_addr.s_addr	=	theUnit->deviceAddress.sin_addr.s_addr;
			remoteDev.sin_family		=	AF_INET;
			remoteDev.sin_port			=	htons(theUnit->port);
			//*	Connect to remote device
			connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
			if (connRetCode >= 0)
			{
				strcpy(xmitBuffer, "GET ");
				strcat(xmitBuffer, sendData);

				strcat(xmitBuffer, " HTTP/1.1");
				strcat(xmitBuffer, "\r\n");
				strcat(xmitBuffer, "Host: 127.0.0.1:6800");
				strcat(xmitBuffer, "\r\n");
				strcat(xmitBuffer, "Connection: keep-alive");
				strcat(xmitBuffer, "\r\n");
				strcat(xmitBuffer, "Accept: text/html,application/json");
				strcat(xmitBuffer, "User-Agent: AlpacaPi");
				strcat(xmitBuffer, "\r\n");
				strcat(xmitBuffer, "\r\n");

				sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
				if (sendRetCode >= 0)
				{
					recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , 0);
					if (recvByteCnt >= 0)
					{
						returnedData[recvByteCnt]	=	0;
	//					printf("%s\r\n", returnedData);
						SJP_Init(&jsonParser);
						SJP_ParseData(&jsonParser, returnedData);
	//					SJP_DumpJsonData(&jsonParser);

						ExtractDevicesFromJSON(&jsonParser, theUnit);
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
					CONSOLE_DEBUG_W_NUM("closeRetCode\t=", closeRetCode);
					CONSOLE_DEBUG_W_NUM("errno\t=", errno);
				}
			}
			else if (errno == ECONNREFUSED)
			{
				CONSOLE_DEBUG_W_NUM("connect refused", (theUnit->deviceAddress.sin_addr.s_addr >> 24));
			}
			else
			{
	//			CONSOLE_DEBUG("connect error");
	//			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Cannot sent request to 0.0.0.0");
		CONSOLE_ABORT(__FUNCTION__);
	}
}


//*****************************************************************************
static void	PollAllDevices(void)
{
int		ii;

	CONSOLE_DEBUG(__FUNCTION__);
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
	//	SendGetRequest(&gAlpacaUnitList[ii], "/api/v1/management/0/configureddevices");
		SendGetRequest(&gAlpacaUnitList[ii], "/management/v1/configureddevices");
	//	usleep(100);
	}
}

//*****************************************************************************
//*	a unit is a single IP address that speaks alpaca
//*****************************************************************************
static void	AddUnitToList(struct sockaddr_in *deviceAddress, SJP_Parser_t *jsonParser)
{
int		ii;
bool	newUnit;
int		unitIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	newUnit		=	true;
	unitIdx		=	-1;
	//*	check to see if this IP address is already in the list
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
		if (deviceAddress->sin_addr.s_addr == gAlpacaUnitList[ii].deviceAddress.sin_addr.s_addr)
		{
			//*	yep, its already here, dont bother
			newUnit	=	false;
			unitIdx	=	ii;
			break;
		}
	}
	if (newUnit)
	{
		//*	add the new devices to our list
//		CONSOLE_DEBUG("We have a new devices")
		if (gAlpacaUnitCnt < kMaxUnitCount)
		{
			gAlpacaUnitList[gAlpacaUnitCnt].deviceAddress	=	*deviceAddress;
			//*	now find the alpaca port
			for (ii=0; ii<jsonParser->tokenCount_Data; ii++)
			{
				if (strcmp(jsonParser->dataList[ii].keyword, "ALPACAPORT") == 0)
				{
					gAlpacaUnitList[gAlpacaUnitCnt].port	=	atoi(jsonParser->dataList[ii].valueString);
					unitIdx	=	gAlpacaUnitCnt;
				}
			}
			gAlpacaUnitCnt++;
		}
	}

	if ((unitIdx >= 0) && (unitIdx < kMaxUnitCount))
	{
		//*	set the last time we heard from it
	}
}

//*****************************************************************************
static int	DeviceSort(const void *e1, const void* e2)
{
int					retValue;
TYPE_REMOTE_DEV		*entry1;
TYPE_REMOTE_DEV		*entry2;
uint32_t			address1;
uint32_t			address2;

	entry1		=	(TYPE_REMOTE_DEV *)e1;
	entry2		=	(TYPE_REMOTE_DEV *)e2;

	address1	=	entry1->deviceAddress.sin_addr.s_addr;
	address1	=	((address1 & 0x0ff) << 24) +
					(((address1 >> 8) & 0x0ff) << 16) +
					(((address1 >> 16) & 0x0ff) << 8) +
					(((address1 >> 24) & 0x0ff));

	address2	=	entry2->deviceAddress.sin_addr.s_addr;
	address2	=	((address2 & 0x0ff) << 24) +
					(((address2 >> 8) & 0x0ff) << 16) +
					(((address2 >> 16) & 0x0ff) << 8) +
					(((address2 >> 24) & 0x0ff));

	retValue	=	address1 - address2;
//	retValue	=	entry1->deviceAddress.sin_addr.s_addr - entry2->deviceAddress.sin_addr.s_addr;
	if (retValue == 0)
	{
		retValue	=	strcmp(entry1->deviceTypeStr, entry2->deviceTypeStr);
	}
	else
	{
	//	CONSOLE_DEBUG_W_HEX("addr1=",	address1);
	//	CONSOLE_DEBUG_W_HEX("addr2=",	address2);
	}
	if (retValue == 0)
	{
		retValue	=	strcmp(entry1->deviceNameStr, entry2->deviceNameStr);
	}
	return(retValue);
}

//*****************************************************************************
static	void PrintDeviceList(void)
{
int		ii;
char	ipAddrSt[32];

	qsort(gAlpacaDiscoveredList, gAlpacaDiscoveredCnt, sizeof(TYPE_REMOTE_DEV), DeviceSort);

	for (ii=0; ii<gAlpacaDiscoveredCnt; ii++)
	{
		if (ii> 0)
		{
			if (gAlpacaDiscoveredList[ii].deviceAddress.sin_addr.s_addr != gAlpacaDiscoveredList[ii-1].deviceAddress.sin_addr.s_addr)
			{
				printf("\r\n");
			}
		}
		inet_ntop(AF_INET, &(gAlpacaDiscoveredList[ii].deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);

		printf("%s\t",		ipAddrSt);

		printf(":%d\t",		gAlpacaDiscoveredList[ii].port);

		printf("%-20s\t",	gAlpacaDiscoveredList[ii].deviceTypeStr);
		printf("%-20s\t",	gAlpacaDiscoveredList[ii].deviceNameStr);
		printf("%4d\t",		gAlpacaDiscoveredList[ii].alpacaDeviceNum);
		printf("%s\t",		gAlpacaDiscoveredList[ii].versionString);

		printf("\r\n");
	}
	fflush(stdout);
}

//*****************************************************************************
//*	read the /etc/hosts file and see if there are names for the address in the list
static void	LookupNames(void)
{

FILE	*filePointer;
char	lineBuff[256];
int		slen;
int		ccc;
int		iii;
char	theChar;
char	ipString[48];
char	hostAddrString[48];
char	hostName[48];
char	hostsFileName[]	=	"/etc/hosts";

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(hostsFileName, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			hostAddrString[0]	=	0;
			hostName[0]			=	0;
			slen	=	strlen(lineBuff);
			if ((slen > 10) && (lineBuff[0] != '#'))
			{
				//*	extract the address string from the hosts file
				ccc	=	0;
				for (iii=0; iii<slen; iii++)
				{
					theChar	=	lineBuff[iii];
					if (isdigit(theChar) || (theChar == '.'))
					{
						hostAddrString[ccc++]	=	theChar;
						hostAddrString[ccc]		=	0;
					}
					else
					{
						break;
					}
				}

				//*	now find the name
				while ((lineBuff[iii] <= 0x20) && (lineBuff[iii] > 0))
				{
					iii++;
				}
				//*	we should now be pointing at the name
 				ccc	=	0;
				while (iii<slen)
				{
					theChar	=	lineBuff[iii];
					if (theChar > 0x20)
					{
						hostName[ccc++]	=	theChar;
						hostName[ccc]	=	0;
					}
					iii++;
				}

				//*	now look through the address list and see if we have any matches
				if ((strlen(hostAddrString) > 1) && (strlen(hostName) > 1))
				{
					for (iii=0; iii<gAlpacaUnitCnt; iii++)
					{
						PrintIPaddressToString(gAlpacaUnitList[iii].deviceAddress.sin_addr.s_addr, ipString);
						if (strcmp(hostAddrString, ipString) == 0)
						{
							//*	they match, update the unit entry
							strcpy(gAlpacaUnitList[iii].hostName, hostName);
//							CONSOLE_DEBUG_W_STR("gAlpacaUnitList[iii].hostName\t=", gAlpacaUnitList[iii].hostName);
							break;
						}
					}
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("failed to open", hostsFileName);
	}
//	exit(0);
}


#if 0
//*****************************************************************************
void	AddAlpacaTestSiteToList(void)
{
TYPE_REMOTE_DEV		myRemoteDevice;
char				myVersionString[64];
struct sockaddr_in	deviceAddress;

	CONSOLE_DEBUG(__FUNCTION__);

	memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));
	memset(myVersionString, 0, sizeof(myVersionString));

//curl -X GET "http://52.86.219.240/ASCOMInitiative/api/v1/camera/0/imagearray?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"


	deviceAddress.sin_addr.s_addr	=	htonl((52 << 24) + (86 << 16) + (219 << 8) + 240);


	myRemoteDevice.deviceAddress			=	deviceAddress;
	myRemoteDevice.port						=	443;
	strcpy(myRemoteDevice.versionString,	"virtserver");
	strcpy(myRemoteDevice.hostName,			"Alpaca");
	strcpy(myRemoteDevice.webPrefixString,	"ASCOMInitiative");

	strcpy(myRemoteDevice.deviceTypeStr, "Camera");
	strcpy(myRemoteDevice.deviceNameStr, "Test Camera");
	myRemoteDevice.alpacaDeviceNum	=	0;


	UpdateRemoteList(&myRemoteDevice);

}
#endif // 0

//*****************************************************************************
//*	returns the number of IP addresses found that are "alpaca"
//*****************************************************************************
int	SendAlpacaQueryBroadcast(void)
{
char				broadCastMsg[]	=	"alpacadiscovery1";
struct sockaddr_in	from;
int					rcvCnt;
unsigned int		fromlen;
int					sendtoRetCode;
char				buf[kReceiveBufferSize + 1];
char				str[INET_ADDRSTRLEN];
SJP_Parser_t		jsonParser;
int					timeOutCntr;
int					alpacaIPaddrCnt;
int					bytesWritten;

	CONSOLE_DEBUG(__FUNCTION__);


	alpacaIPaddrCnt	=	0;

	//*	send the broadcast message to everyone
	sendtoRetCode	=	sendto(	gBroadcastSock,
								broadCastMsg,
								strlen(broadCastMsg),
								0,
								(struct sockaddr *)&gServer_addr,
								sizeof(struct sockaddr_in));
	if (sendtoRetCode < 0)
	{
		CONSOLE_DEBUG("sendto returned error");
		perror("sendto");
	}
	timeOutCntr	=	0;
	fromlen	=	sizeof(struct sockaddr_in);
	while (timeOutCntr < 2)
	{
		rcvCnt	=	recvfrom(gBroadcastSock, buf, kReceiveBufferSize, 0, (struct sockaddr *)&from, &fromlen);
		if (rcvCnt > 0)
		{
			buf[rcvCnt]	=	0;
//				CONSOLE_DEBUG("We have data");
//				CONSOLE_DEBUG_W_STR("buf=", buf);
			SJP_Init(&jsonParser);
			SJP_ParseData(&jsonParser, buf);
//			SJP_DumpJsonData(&jsonParser);

			AddUnitToList(&from, &jsonParser);

			inet_ntop(AF_INET, &(from.sin_addr), str, INET_ADDRSTRLEN);

			bytesWritten	=	0;
			bytesWritten	+=	write(1, buf, rcvCnt);
			bytesWritten	+=	write(1, " ", 1);
			bytesWritten	+=	write(1, str, strlen(str));
			bytesWritten	+=	write(1, "\n", 1);
		}
		else if (rcvCnt == 0)
		{
			printf("no response\r\n");
		}
		else
		{
		//	perror("recvfrom");
			timeOutCntr++;
		}
//		CONSOLE_DEBUG_W_HEX("from.sin_addr=", from.sin_addr.s_addr);
//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 24) & 0x0ff));
//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 16) & 0x0ff));
//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 8) & 0x0ff));
//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr) & 0x0ff));

	}

	ReadExternalIPlist();

	//*	find the names that go along with the addresses
	LookupNames();


	PollAllDevices();
//	AddAlpacaTestSiteToList();

	PrintDeviceList();

	return(alpacaIPaddrCnt);
}

//*****************************************************************************
void	ReadExternalIPlist(void)
{
FILE				*filePointer;
char				lineBuff[256];
char				outputIPaddr[256];
int					ii;
int					slen;
char				fileName[]	=	"external_ip_list.txt";
SJP_Parser_t		jsonParser;
struct sockaddr_in	from;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	see if there is a file listing extra IP address
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
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
				inet_pton(AF_INET, lineBuff, &(from.sin_addr));

				inet_ntop(AF_INET, &(from.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
				CONSOLE_DEBUG_W_STR("outputIPaddr\t\t=",		outputIPaddr);

				strcpy(jsonParser.dataList[0].keyword, "ALPACAPORT");
				strcpy(jsonParser.dataList[0].valueString, "6800");
				jsonParser.tokenCount_Data	=	1;

//				SJP_DumpJsonData(&jsonParser);
				AddUnitToList(&from, &jsonParser);
			}
		}
		fclose(filePointer);
	}
}


