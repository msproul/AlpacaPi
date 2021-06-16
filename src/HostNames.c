//*****************************************************************************
//*
//*	Name:			HostNames.c
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
//*	Jan 16,	2021	<MLS> Created HostNames.c
//*****************************************************************************

#include	<ctype.h>
#include	<string.h>
#include	<stdio.h>
#include	<stdbool.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"HostNames.h"

//*****************************************************************************
typedef struct
{
	char				ipAddressStr[32];
	char				hostNameStr[32];
	struct sockaddr_in	ipAddress;


} TYPE_HOSTNAME;

#define	kMaxHostNames	32
static TYPE_HOSTNAME	gHostNameTable[kMaxHostNames];
static int				gHostNameCount	=	0;

//*****************************************************************************
//*	read the /etc/hosts file into memory
//*****************************************************************************
static void	ReadHostNameFile(void)
{
FILE	*filePointer;
char	lineBuff[256];
int		slen;
int		ccc;
int		iii;
char	theChar;
char	hostAddrString[48];
char	hostName[48];
char	hostsFileName[]	=	"/etc/hosts";
struct sockaddr_in	myIPaddress;

//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kMaxHostNames; iii++)
	{
		memset(&gHostNameTable[iii], 0, sizeof(TYPE_HOSTNAME));
	}
	gHostNameCount	=	0;

	filePointer	=	fopen(hostsFileName, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			hostAddrString[0]	=	0;
			hostName[0]			=	0;
			slen	=	strlen(lineBuff);
			if ((slen > 10) && (lineBuff[0] != '#') && isdigit(lineBuff[0]))
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
				if (gHostNameCount < kMaxHostNames)
				{
//					CONSOLE_DEBUG_W_2STR("New entry", hostAddrString, hostName);
					strcpy(gHostNameTable[gHostNameCount].hostNameStr,	hostName);
					strcpy(gHostNameTable[gHostNameCount].ipAddressStr,	hostAddrString);

					inet_pton(AF_INET, hostAddrString, &(myIPaddress.sin_addr));

					gHostNameTable[gHostNameCount].ipAddress	=	myIPaddress;
					gHostNameCount++;
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("failed to open", hostsFileName);
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
bool	LookupNameFromString(const char *ipAddrStr, char *returnedHostName)
{
int		iii;
bool	foundIt;

//	CONSOLE_DEBUG(__FUNCTION__);

	foundIt	=	false;
	if (gHostNameCount == 0)
	{
		ReadHostNameFile();
	}
	//*	now see if we can find this address in the table
	iii			=	0;
	while ((foundIt == false) && (iii < gHostNameCount))
	{
		if (strcmp(ipAddrStr, gHostNameTable[iii].ipAddressStr) == 0)
		{
			strcpy(returnedHostName, gHostNameTable[iii].hostNameStr);
			foundIt		=	true;

//			CONSOLE_DEBUG_W_STR("returnedHostName", returnedHostName);

		}
		iii++;
	}

	return(foundIt);
}


//*****************************************************************************
bool	LookupNameFromIPaddr(uint32_t ipAddrNum, char *returnedHostName)
{
int		iii;
bool	foundIt;

//	CONSOLE_DEBUG(__FUNCTION__);

	foundIt	=	false;
	if (gHostNameCount == 0)
	{
		ReadHostNameFile();
	}
	//*	now see if we can find this address in the table
	iii			=	0;
	while ((foundIt == false) && (iii < gHostNameCount))
	{
		if (ipAddrNum == gHostNameTable[iii].ipAddress.sin_addr.s_addr)
		{
			strcpy(returnedHostName, gHostNameTable[iii].hostNameStr);
			foundIt		=	true;

		//	CONSOLE_DEBUG_W_STR("returnedHostName", returnedHostName);
		}
		iii++;
	}

	return(foundIt);
}

