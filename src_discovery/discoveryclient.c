//*****************************************************************************
//*	Alpaca discovery client
//*	by Mark Sproul (msproul@skychariot.com)
//*
//*	Based on code by Daniel Van Noord 2019
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

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"json_parse.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"


#define kAlpacaDiscoveryPORT	32227

#define	kReceiveBufferSize	1550





//#define	kMaxDeviceListCnt	32

TYPE_ALPACA_UNIT	gAlpacaUnitList[kMaxDeviceListCnt];
int					gAlpacaUnitCnt	=	0;

TYPE_REMOTE_DEV		gAlpacaDeviceList[kMaxDeviceListCnt];
int					gALpacaDeviceCnt	=	0;;

static	int		gBroadcastSock;

//*****************************************************************************
static void	UpdateRemoteList(TYPE_REMOTE_DEV *newRemoteDevice)
{
int		ii;
bool	newDevice;


//	CONSOLE_DEBUG(__FUNCTION__);
	//*	look to see if it is already in the list
	newDevice	=	true;
	for (ii=0; ii<gALpacaDeviceCnt; ii++)
	{
		if ((newRemoteDevice->deviceAddress.sin_addr.s_addr == gAlpacaDeviceList[ii].deviceAddress.sin_addr.s_addr)
			&& (strcmp(newRemoteDevice->deviceType, gAlpacaDeviceList[ii].deviceType) == 0)
			&& (strcmp(newRemoteDevice->deviceName, gAlpacaDeviceList[ii].deviceName) == 0)
			&& (newRemoteDevice->alpacaDeviceNum == gAlpacaDeviceList[ii].alpacaDeviceNum)
			)
		{
			newDevice	=	false;
			break;
		}
		//*	I dont want the management device type in the list
		if (strcmp(newRemoteDevice->deviceType, "management") == 0)
		{
			newDevice	=	false;
			break;
		}
	}
	if (newDevice)
	{
		if (gALpacaDeviceCnt < kMaxDeviceListCnt)
		{
			gAlpacaDeviceList[gALpacaDeviceCnt]	=	*newRemoteDevice;
			gALpacaDeviceCnt++;

		}
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
		retValue	=	strcmp(entry1->deviceType, entry2->deviceType);
	}
	else
	{
	//	CONSOLE_DEBUG_W_HEX("addr1=",	address1);
	//	CONSOLE_DEBUG_W_HEX("addr2=",	address2);
	}
	if (retValue == 0)
	{
		retValue	=	strcmp(entry1->deviceName, entry2->deviceName);
	}
	return(retValue);
}


//*****************************************************************************
static	void PrintDeviceList(void)
{
int		ii;
char	ipAddrSt[32];

	qsort(gAlpacaDeviceList, gALpacaDeviceCnt, sizeof(TYPE_REMOTE_DEV), DeviceSort);

	for (ii=0; ii<gALpacaDeviceCnt; ii++)
	{
		if (ii> 0)
		{
			if (gAlpacaDeviceList[ii].deviceAddress.sin_addr.s_addr != gAlpacaDeviceList[ii-1].deviceAddress.sin_addr.s_addr)
			{
				printf("\r\n");
			}
		}
		inet_ntop(AF_INET, &(gAlpacaDeviceList[ii].deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);

		printf("%s\t",		ipAddrSt);

		printf(":%d\t",		gAlpacaDeviceList[ii].port);

		printf("%-20s\t",	gAlpacaDeviceList[ii].deviceType);
		printf("%-25s\t",	gAlpacaDeviceList[ii].deviceName);
		printf("%4d\t",		gAlpacaDeviceList[ii].alpacaDeviceNum);
		printf("%s\t",		gAlpacaDeviceList[ii].versionString);

		printf("\r\n");
	}
	fflush(stdout);
}

//*****************************************************************************
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theDevice)
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
//		CONSOLE_DEBUG_W_2STR("token", jsonParser->dataList[ii].keyword, jsonParser->dataList[ii].valueString);
		if (strcmp(jsonParser->dataList[ii].keyword, "VERSION") == 0)
		{
			strcpy(myVersionString, jsonParser->dataList[ii].valueString);
		}

		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICETYPE") == 0)
		{
			strcpy(myRemoteDevice.deviceType, jsonParser->dataList[ii].valueString);
		}
		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICENAME") == 0)
		{
			strcpy(myRemoteDevice.deviceName, jsonParser->dataList[ii].valueString);
		}
		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICENUMBER") == 0)
		{
			myRemoteDevice.alpacaDeviceNum	=	atoi(jsonParser->dataList[ii].valueString);
		}

		//------------------------------------
		if (strcmp(jsonParser->dataList[ii].keyword, "ARRAY-NEXT") == 0)
		{

			myRemoteDevice.deviceAddress	=	theDevice->deviceAddress;
			myRemoteDevice.port				=	theDevice->port;
			strcpy(myRemoteDevice.versionString, myVersionString);


			UpdateRemoteList(&myRemoteDevice);

			memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));

		}
	}
}


//*****************************************************************************
static void	SendGetRequest(TYPE_ALPACA_UNIT *theDevice, char *sendData)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
int					closeRetCode;
int					recvByteCnt;
char				returnedData[2000];
char				xmitBuffer[2000];
SJP_Parser_t		jsonParser;


//	CONSOLE_DEBUG(__FUNCTION__);

	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{

//		CONSOLE_DEBUG_W_NUM("Connecting to port ", theDevice->port);
//		if (theDevice->port != 3006)
//		{
//			CONSOLE_DEBUG("Port number is wrong");
//			theDevice->port	=	3006;
//		}
		remoteDev.sin_addr.s_addr	=	theDevice->deviceAddress.sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(theDevice->port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			strcpy(xmitBuffer, "GET ");
			strcat(xmitBuffer, sendData);
			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
				recvByteCnt	=	recv(socket_desc, returnedData , 2000 , 0);
				if (recvByteCnt >= 0)
				{
					returnedData[recvByteCnt]	=	0;
//					printf("%s\r\n", returnedData);
					SJP_Init(&jsonParser);
					SJP_ParseData(&jsonParser, returnedData);
//					SJP_DumpJsonData(&jsonParser);

					ExtractDevicesFromJSON(&jsonParser, theDevice);
				}
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
			CONSOLE_DEBUG_W_NUM("connect refused", (theDevice->deviceAddress.sin_addr.s_addr >> 24));
		}
		else
		{
//			CONSOLE_DEBUG("connect error");
//			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
}

//*****************************************************************************
static void	PollAllDevices(void)
{
int		ii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
	//	SendGetRequest(&gAlpacaUnitList[ii], "/api/v1/management/0/configureddevices");
		SendGetRequest(&gAlpacaUnitList[ii], "/management/v1/configureddevices");
	}
}

//*****************************************************************************
static void	AddDeviceToList(struct sockaddr_in *deviceAddress, SJP_Parser_t *jsonParser, int optionalPort)
{
int		ii;
bool	newDevice;
int		deviceIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	newDevice	=	true;
	deviceIdx	=	-1;
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
		if (deviceAddress->sin_addr.s_addr == gAlpacaUnitList[ii].deviceAddress.sin_addr.s_addr)
		{
			newDevice	=	false;
			deviceIdx	=	ii;
			break;
		}
	}
	if (newDevice)
	{
		//*	add the new devices to our list
//		CONSOLE_DEBUG("We have a new devices")
		if (gAlpacaUnitCnt < kMaxDeviceListCnt)
		{
			gAlpacaUnitList[gAlpacaUnitCnt].deviceAddress	=	*deviceAddress;

			if (optionalPort > 0)
			{
				gAlpacaUnitList[gAlpacaUnitCnt].port	=	optionalPort;
			}
			else
			{
				//*	now find the alpaca port
				for (ii=0; ii<jsonParser->tokenCount_Data; ii++)
				{
					if (strcmp(jsonParser->dataList[ii].keyword, "ALPACAPORT") == 0)
					{
						gAlpacaUnitList[gAlpacaUnitCnt].port	=	atoi(jsonParser->dataList[ii].valueString);
						deviceIdx	=	gAlpacaUnitCnt;
					}
				}
			}
			gAlpacaUnitCnt++;
		}
	}

	if ((deviceIdx >= 0) && (deviceIdx < kMaxDeviceListCnt))
	{
		//*	set the last time we heard from it
	}
}


//*****************************************************************************
int main(int argc, char *argv[])
{
char				broadCastMsg[]	=	"alpacadiscovery1";
struct sockaddr_in	servaddr;
struct sockaddr_in	cliaddr;
struct sockaddr_in	from;
int					rcvCnt;
unsigned int		fromlen;
int					setOptRetCode;
int					bindRetCode;
int					sendtoRetCode;
char				buf[kReceiveBufferSize + 1];
char				str[INET_ADDRSTRLEN];
SJP_Parser_t		jsonParser;
struct timeval		timeoutLength;
int					timeOutCntr;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("argc\t=", argc);

	gBroadcastSock	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (gBroadcastSock  < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, '\0', sizeof(struct sockaddr_in));
	servaddr.sin_family			=	AF_INET;
	servaddr.sin_port			=	htons(kAlpacaDiscoveryPORT);
	servaddr.sin_addr.s_addr	=	htonl(INADDR_BROADCAST);

	cliaddr.sin_family			=	AF_INET;
	cliaddr.sin_addr.s_addr		=	htonl(INADDR_ANY);
	cliaddr.sin_port			=	htons(0);

//	inet_ntop(AF_INET, &(servaddr.sin_addr), str, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("servaddr.sin_addr", str);

//	inet_ntop(AF_INET, &(cliaddr.sin_addr), str, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("cliaddr.sin_addr", str);


	setOptRetCode	=	setsockopt(gBroadcastSock, SOL_SOCKET, (SO_BROADCAST), &(int) { 1 }, sizeof(int));
	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_BROADCAST) failed");
	}

	//*	set a timeout
	timeoutLength.tv_sec	=	1;
	timeoutLength.tv_usec	=	0;
	setOptRetCode			=	setsockopt(	gBroadcastSock,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));


	bindRetCode	=	bind(gBroadcastSock, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	if (bindRetCode < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	sendtoRetCode	=	0;
//	while (sendtoRetCode >= 0)
	{
		printf("*******************************************************************************\r\n");
//		CONSOLE_DEBUG("Calling sendto");
		//*	send the broadcast message to everyone
		sendtoRetCode	=	sendto(	gBroadcastSock,
									broadCastMsg,
									strlen(broadCastMsg),
									0,
									(struct sockaddr *)&servaddr,
									sizeof(struct sockaddr_in));
		if (sendtoRetCode < 0)
		{
			CONSOLE_DEBUG("sendto returned error");
			perror("sendto");
		}
		timeOutCntr	=	0;
		fromlen		=	sizeof(struct sockaddr_in);
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
//				SJP_DumpJsonData(&jsonParser);


				AddDeviceToList(&from, &jsonParser, -1);

				inet_ntop(AF_INET, &(from.sin_addr), str, INET_ADDRSTRLEN);

				write(1, buf, rcvCnt);
				write(1, " ", 1);
				write(1, str, strlen(str));
				write(1, "\n", 1);
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

		if ((argc >= 2) && (argv[1][0] == 'x'))
		{
			//*	this is for my custom network
			from.sin_addr.s_addr	=	htonl((192 << 24) + (168 << 16) + (50 << 8) + 46);
			inet_ntop(AF_INET, &(from.sin_addr), str, INET_ADDRSTRLEN);
		//	CONSOLE_DEBUG_W_STR("str=", str);
			AddDeviceToList(&from, &jsonParser, 6800);

			from.sin_addr.s_addr	=	htonl((192 << 24) + (168 << 16) + (50 << 8) + 77);
			inet_ntop(AF_INET, &(from.sin_addr), str, INET_ADDRSTRLEN);
		//	CONSOLE_DEBUG_W_STR("str=", str);
			AddDeviceToList(&from, &jsonParser, 6800);
		}
//		ReadExternalIPlist();

		PollAllDevices();

		PrintDeviceList();


		printf("\r\n");
		printf("Total alpaca devices found = %d\r\n", gAlpacaUnitCnt);
	}
	return(0);
}
