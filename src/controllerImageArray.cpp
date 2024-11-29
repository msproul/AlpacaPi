//*****************************************************************************
//*		controllerImageArray.cpp		(c) 2021 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 18,	2021	<MLS> Created controllerImageArray.cpp
//*	Dec 18,	2021	<MLS> Moved image array routines to separate file
//*	Dec 18,	2021	<MLS> Moved from image arry stuff from controller class to controller_camera
//*	Dec 18,	2021	<MLS> Added AlpacaGetImageArray_JSON() & AlpacaGetImageArray_Binary()
//*	Dec 18,	2021	<MLS> Added UpdateImageProgressBar()
//*	Dec 18,	2021	<MLS> Proved that the simulators are sending column order first
//*	Dec 18,	2021	<MLS> Spoke to Peter in the UK about it (peter@peterandjill.co.uk)
//*	May 18,	2022	<MLS> Added AlpacaGetImageArray_Binary_Int32()
//*	Feb 19,	2023	<MLS> Added AlpacaGetImageArray_Binary_Int16()
//*	Feb 19,	2023	<MLS> Changed byte order in 32 bit integer image read
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<errno.h>



#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"eventlogging.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_camera.h"

#define		kImageArrayBuffSize	15000



//*****************************************************************************
void	ParseJsonKeyWordValuePair(const char *jsonData, char *keywordStr, char *valueStr)
{
int		iii;
int		ccc;
int		sLen;
char	theChar;

	keywordStr[0]	=	0;
	valueStr[0]	=	0;
	sLen	=	strlen(jsonData);
	ccc		=	0;
	iii		=	0;
//{"Type":2

	while ((jsonData[iii] != ':') && (iii < sLen))
	{
		theChar	=	jsonData[iii];
		switch(theChar)
		{
			case '{':
			case '\"':
				break;

			default:
				keywordStr[ccc++]	=	theChar;
				keywordStr[ccc]		=	0;
				break;

		}
		iii++;
	}
	if (jsonData[iii] == ':')
	{
		iii++;
		strcpy(valueStr, &jsonData[iii]);
	}
}


//*****************************************************************************
//*		Header line= HTTP/1.1 200 OK
//*		Header line= Content-Length: 2661823
//*		Header line= Connection: close
//*		Header line= Content-Type: application/json; charset=utf-8
//*		Header line= Date: Wed, 15 Dec 2021 13:26:54 GMT
//*		Header line= Server: Kestrel
//*****************************************************************************
static void	ProcessHTTPheaderLine(char *httpHeaderLine, TYPE_HTTPheader *httpHdrStruct)
{
char	*argumentPtr;
//int		slen;

	CONSOLE_DEBUG_W_STR("httpHeaderLine\t=", httpHeaderLine);

	//*	find the ":" in the line
	argumentPtr	=	strchr(httpHeaderLine, ':');
	if (argumentPtr != NULL)
	{
		argumentPtr++;	//*	skip the ":"
		while (*argumentPtr == 0x20)
		{
			argumentPtr++;	//*	skip any spaces
		}
	}
	else
	{
		argumentPtr	=	httpHeaderLine;
	}
	if (argumentPtr != NULL)
	{
//		slen	=	strlen(argumentPtr);
//		CONSOLE_DEBUG_W_NUM("length of argument\t=", slen);
		CONSOLE_DEBUG_W_STR("argumentPtr\t=", argumentPtr);

		//*	look for specific keywords
		if (strncasecmp(httpHeaderLine,	"Content-Type:", 13) == 0)
		{
			strcpy(httpHdrStruct->ContentTypeStr, argumentPtr);

			//* look for “application/json”
			if (strcasestr(httpHdrStruct->ContentTypeStr, "json") != NULL)
			{
				CONSOLE_DEBUG("json found");
				httpHdrStruct->dataIsJson	=	true;
			}

			//* look for “application/imagebytes”
			if (strcasestr(httpHdrStruct->ContentTypeStr, "imagebytes") != NULL)
			{
				CONSOLE_DEBUG("imagebytes found");
				httpHdrStruct->dataIsBinary	=	true;
			}
		}
		else if (strncasecmp(httpHeaderLine,	"Content-Length:", 15) == 0)
		{
			strcpy(httpHdrStruct->ContentLengthStr, argumentPtr);
			httpHdrStruct->contentLength	=	atoi(argumentPtr);
		}
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
void	ControllerCamera::UpdateImageProgressBar(int maxArrayLength)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cImageArrayIndex\t=", cImageArrayIndex);

	//=================================================
	//*	deal with the progress bar
	tCurrentMillisecs	=	millis();
	tDeltaMillisecs		=	tCurrentMillisecs - tStartMillisecs;
	//*	dont do anything at first, for short downloads it slows things down
	if (tDeltaMillisecs > 1500)
	{
		tDeltaMillisecs	=	tCurrentMillisecs - tLastUpdateMillisecs;
		if (tDeltaMillisecs > 250)
		{
			UpdateDownloadProgress(cImageArrayIndex, maxArrayLength);
			tLastUpdateMillisecs	=	tCurrentMillisecs;
		}
	}
}


//*****************************************************************************
//*
//*	returns the RANK of the data found, 0 means no valid RANK
//*****************************************************************************
int		ControllerCamera::AlpacaGetIntegerArrayShortLines(	const char	*alpacaDevice,
															const int	alpacaDevNum,
															const char	*alpacaCmd,
															const char	*dataString,
															int			*uint32array,
															int			arrayLength,
															int			*actualValueCnt)
{
char			alpacaString[128];
int				shutDownRetCode;
int				closeRetCode;
bool			keepReading;
char			linebuf[kImageArrayBuffSize];
int				iii;
int				ccc;
char			theChar;
char			keywordStr[kLineBufSize];
char			valueStr[kLineBufSize];
int				imgRank;
int				myIntegerValue;
double			downLoadSeconds;
int				progressUpdateCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kReadBuffLen\t=", kReadBuffLen);
	CONSOLE_DEBUG_W_NUM("arrayLength\t=", arrayLength);

	cImgArrayType		=	-1;
	imgRank				=	0;
	cImageArrayIndex	=	0;

	tStartMillisecs			=	millis();
	tLastUpdateMillisecs	=	tStartMillisecs;

	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	cSocket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												dataString,
												READ_JSON_IMAGE);
	strcpy(cLastAlpacaCmdString, alpacaString);
	if (cSocket_desc >= 0)
	{
//		CONSOLE_DEBUG("Success: Connection open and data sent");
		SETUP_TIMING();
		START_TIMING();
		cValueFoundFlag		=	false;
		keepReading			=	true;
		ccc					=	0;
		cLinesProcessed		=	0;
		cTotalBytesRead		=	0;
		progressUpdateCnt	=	0;
		cSocketReadCnt		=	0;
		while (keepReading)
		{
			cRecvdByteCnt	=	recv(cSocket_desc, cReturnedData , kReadBuffLen , 0);
//			CONSOLE_DEBUG_W_NUM("cRecvdByteCnt\t=", cRecvdByteCnt);
			if (cRecvdByteCnt > 0)
			{
				cTotalBytesRead				+=	cRecvdByteCnt;
				cReturnedData[cRecvdByteCnt]	=	0;

				for (iii=0; iii<cRecvdByteCnt; iii++)
				{
					theChar	=	cReturnedData[iii];
					if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						linebuf[ccc]	=	0;
						if (strlen(linebuf) > 0)
						{
							JSON_ExtractKeyword_Value(linebuf, keywordStr, valueStr);

							if (strcasecmp(keywordStr, "ArrayType") == 0)
							{
								cImgArrayType	=	atoi(valueStr);
							}
							if (strcasecmp(keywordStr, "Type") == 0)
							{
								cImgArrayType	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "rank") == 0)
							{
								imgRank	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "value") == 0)
							{
							//	CONSOLE_DEBUG("value found!!!!");
								cValueFoundFlag	=	true;
							}
							else if (cValueFoundFlag)
							{
								if (isdigit(keywordStr[0]))
								{
									myIntegerValue	=	atoi(keywordStr);
									if (cImageArrayIndex < arrayLength)
									{
										uint32array[cImageArrayIndex]	=	myIntegerValue;
										cImageArrayIndex++;
									}
									else
									{
										CONSOLE_DEBUG("Ran out of room");
									}
								}
								else
								{
									CONSOLE_DEBUG_W_2STR("kw:value=", keywordStr, valueStr);
								}
							}
							cLinesProcessed++;
						}
						ccc				=	0;
						linebuf[ccc]	=	0;
					}
					else
					{
						if (ccc < kImageArrayBuffSize)
						{
							linebuf[ccc]	=	theChar;
							ccc++;
						}
						else
						{
							CONSOLE_DEBUG("Line to long");
						}
					}
				}
				cSocketReadCnt++;
			}
			else
			{
				keepReading		=	false;
			}
			//=================================================
			//*	deal with the progress bar
			UpdateImageProgressBar(arrayLength);
		}
		//*	one last time to show we are done
		UpdateDownloadProgress(cImageArrayIndex, arrayLength);


		*actualValueCnt	=	cImageArrayIndex;

		DEBUG_TIMING("Time to download image (ms)");
		CONSOLE_DEBUG_W_NUM("cSocketReadCnt\t=", cSocketReadCnt);

		cLastDownload_Bytes			=	cTotalBytesRead;
		cLastDownload_Millisecs		=	tDeltaMillisecs;
		downLoadSeconds				=	tDeltaMillisecs / 1000.0;
		if (downLoadSeconds > 0)
		{
			cLastDownload_MegaBytesPerSec	=	1.0 * cTotalBytesRead / downLoadSeconds;

		}
		else
		{
			CONSOLE_DEBUG("tDeltaMillisecs invalid");
			cLastDownload_MegaBytesPerSec	=	0.0;
		}

		CONSOLE_DEBUG_W_NUM("cImgArrayType    \t=",	cImgArrayType);
		CONSOLE_DEBUG_W_NUM("imgRank          \t=",	imgRank);
		CONSOLE_DEBUG_W_NUM("cTotalBytesRead  \t=",	cTotalBytesRead);
		CONSOLE_DEBUG_W_NUM("cLinesProcessed  \t=", cLinesProcessed);
		CONSOLE_DEBUG_W_NUM("progressUpdateCnt\t=", progressUpdateCnt);

		shutDownRetCode	=	shutdown(cSocket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		closeRetCode	=	close(cSocket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
	return(imgRank);
}

#if 0
//*****************************************************************************
static void	DumpHex(const char *dataPtr, const int numRows)
{
int		rowsPrinted;
int		ccc;
int		iii;
int		theChar;

	rowsPrinted	=	0;
	ccc			=	0;
	while (rowsPrinted < numRows)
	{
		printf("%04X\t", ccc);
		for (iii=0; iii<16; iii++)
		{
			theChar	=	dataPtr[ccc + iii] & 0x00ff;
			printf("%02X ", theChar);
			if ((iii != 0) && ((iii % 4) == 3))
			{
				printf("- ");
			}
		}
		printf("\t");

		for (iii=0; iii<16; iii++)
		{
			theChar	=	dataPtr[ccc + iii] & 0x00ff;
			if ((theChar < 0x20) || (theChar > 0x7f))
			{
				theChar	=	'.';
			}
			printf("%c ", theChar);
		}
		printf("\r\n");

		ccc	+=	16;
		rowsPrinted++;
	}
}


//*****************************************************************************
static void	DumpHexRGB(const char *dataPtr, const int numRows)
{
int		rowsPrinted;
int		ccc;
int		iii;
int		byte1;
int		byte2;

	rowsPrinted	=	0;
	ccc			=	0;
	while (rowsPrinted < numRows)
	{
		printf("%04X\t", ccc);
		iii	=	0;

		while (iii<6)
		{
			byte1	=	dataPtr[ccc + iii] & 0x00ff;
			iii++;
			byte2	=	dataPtr[ccc + iii] & 0x00ff;
			iii++;

			printf("%02X : %02X ---", byte1, byte2);
		}
		printf("\r\n");

		ccc	+=	6;
		rowsPrinted++;
	}


}
#endif // 0

//*****************************************************************************
void	ControllerCamera::AlpacaGetImageArray_Binary_Byte(	TYPE_ImageArray	*imageArray,
															int				imageArrayLen)
{
int				binaryDataValue;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cBinaryImageHdr.Rank\t=", cBinaryImageHdr.Rank);
//	CONSOLE_DEBUG_W_NUM("cRecvdByteCnt       \t=", cRecvdByteCnt);

	if (cBinaryImageHdr.Rank == 3)
	{
		while ((cData_iii < cRecvdByteCnt))
		{
			binaryDataValue	=	(cReturnedData[cData_iii] & 0x00ff) << 8;
//			CONSOLE_DEBUG_W_HEX("binaryDataValue\t=", binaryDataValue);
			if (cImageArrayIndex < imageArrayLen)
			{
				//*	deal with the individual R,G,B values
				switch(cRGBidx)
				{
					case 0:
						imageArray[cImageArrayIndex].RedValue	=	binaryDataValue & 0x00ffff;
						break;

					case 1:
						imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue & 0x00ffff;
						break;

					case 2:
						imageArray[cImageArrayIndex].BluValue	=	binaryDataValue & 0x00ffff;
						cImageArrayIndex++;
						break;
				}
				cRGBidx++;
				if (cRGBidx >= 3)
				{
					cRGBidx	=	0;
				}
			}
			cData_iii++;
		}
	}
	else if (cBinaryImageHdr.Rank == 2)
	{
		while ((cData_iii < cRecvdByteCnt))
		{
			binaryDataValue	=	(cReturnedData[cData_iii] & 0x00ff) << 8;
			imageArray[cImageArrayIndex].RedValue	=	binaryDataValue;
			imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue;
			imageArray[cImageArrayIndex].BluValue	=	binaryDataValue;
			cImageArrayIndex++;
			cData_iii++;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("cBinaryImageHdr.Rank\t=", cBinaryImageHdr.Rank);
		CONSOLE_ABORT("cBinaryImageHdr.Rank not processed correctly");
	}
}

//*****************************************************************************
void	ControllerCamera::AlpacaGetImageArray_Binary_Int16(	TYPE_ImageArray	*imageArray,
															int				imageArrayLen)
{
int				binaryDataValue;
int				dataByteIdx;

	CONSOLE_DEBUG(__FUNCTION__);

	dataByteIdx		=	0;

	if (cBinaryImageHdr.Rank == 2)
	{
		binaryDataValue	=	0;
		while ((cData_iii < cRecvdByteCnt))
		{
			switch(dataByteIdx)
			{
				case 0:
					binaryDataValue	=	cReturnedData[cData_iii] & 0x00ff;
					dataByteIdx++;
					break;

				case 1:
					binaryDataValue	+=	(((cReturnedData[cData_iii] & 0x00ff) << 8) & 0x00ff00);
					if (cImageArrayIndex < imageArrayLen)
					{
						imageArray[cImageArrayIndex].RedValue	=	binaryDataValue;
						imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue;
						imageArray[cImageArrayIndex].BluValue	=	binaryDataValue;
						cImageArrayIndex++;
					}
					dataByteIdx	=	0;
					break;

				default:
					CONSOLE_ABORT(__FUNCTION__);
					break;
			}
			cData_iii++;
		}
	}
	else if (cBinaryImageHdr.Rank == 3)
	{
		binaryDataValue	=	0;
		while ((cData_iii < cRecvdByteCnt))
		{
			switch(dataByteIdx)
			{
				case 0:
					binaryDataValue	=	cReturnedData[cData_iii] & 0x00ff;
					dataByteIdx++;
					break;

				case 1:
					binaryDataValue	+=	(((cReturnedData[cData_iii] & 0x00ff) << 8) & 0x00ff00);
					if (cImageArrayIndex < imageArrayLen)
					{
						//*	deal with the individual R,G,B values
						switch(cRGBidx)
						{
							case 0:
								imageArray[cImageArrayIndex].RedValue	=	binaryDataValue & 0x00ffff;
								break;

							case 1:
								imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue & 0x00ffff;
								break;

							case 2:
								imageArray[cImageArrayIndex].BluValue	=	binaryDataValue & 0x00ffff;
//											if ((cImageArrayIndex >= 0) && (cImageArrayIndex < 25))
//											{
//												printf("#%6d=%6d\t%6d\t%6d\r\n",	cImageArrayIndex,
//																				imageArray[cImageArrayIndex].RedValue,
//																				imageArray[cImageArrayIndex].GrnValue,
//																				imageArray[cImageArrayIndex].BluValue);
//												printf("#%6d=%6X\t%6X\t%6X\r\n",	cImageArrayIndex,
//																				imageArray[cImageArrayIndex].RedValue,
//																				imageArray[cImageArrayIndex].GrnValue,
//																				imageArray[cImageArrayIndex].BluValue);
//											}
								cImageArrayIndex++;
								break;
						}
						cRGBidx++;
						if (cRGBidx >= 3)
						{
							cRGBidx	=	0;
						}
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("cImageArrayIndex=", cImageArrayIndex);
						CONSOLE_ABORT(__FUNCTION__);
					}
					dataByteIdx	=	0;
					break;
			}
			cData_iii++;
		}
	}
	else
	{
//					LogEvent(	"camera",
//								"Binary Download",
//								NULL,
//								kASCOM_Err_Success,
//								"Invalid RANK value");
	}
}

//*****************************************************************************
void	ControllerCamera::AlpacaGetImageArray_Binary_Int32(	TYPE_ImageArray	*imageArray,
															int				imageArrayLen)
{
uint32_t		binaryDataValue;
uint32_t		*returnedDataPtr32bit;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("Rank\t=", cBinaryImageHdr.Rank);

	returnedDataPtr32bit	=	(uint32_t *)cReturnedData;

	if (cBinaryImageHdr.Rank == 3)
	{
		while ((cData_iii < cRecvdByteCnt))
		{
//			binaryDataValue	=	(cReturnedData[cData_iii] & 0x00ff) << 8;
			binaryDataValue	=	(returnedDataPtr32bit[cData_iii] & 0x00ffff);
			binaryDataValue	=	returnedDataPtr32bit[cData_iii] >> 16;
		//	CONSOLE_DEBUG_W_HEX("binaryDataValue\t=", binaryDataValue);
			if (cImageArrayIndex < imageArrayLen)
			{
				//*	deal with the individual R,G,B values
				switch(cRGBidx)
				{
					case 0:
						imageArray[cImageArrayIndex].RedValue	=	binaryDataValue & 0x00ffff;
						break;

					case 1:
						imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue & 0x00ffff;
						break;

					case 2:
						imageArray[cImageArrayIndex].BluValue	=	binaryDataValue & 0x00ffff;
						cImageArrayIndex++;
						break;
				}
				cRGBidx++;
				if (cRGBidx >= 3)
				{
					cRGBidx	=	0;
				}
			}
			cData_iii++;
		}
	}
	else if (cBinaryImageHdr.Rank == 2)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		while ((cData_iii < cRecvdByteCnt))
		{
			binaryDataValue	=	0;
			//*	Feb 19,	2023	<MLS> Changed byte order in 32 bit integer image read
		#if 0
			//*	little endian
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff);
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 8;
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 16;
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 24;
		#else
			//*	big endian
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 24;
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 16;
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff) << 8;
			binaryDataValue	+=	(cReturnedData[cData_iii++] & 0x00ff);
		#endif // 1
//			CONSOLE_DEBUG_W_HEX("binaryDataValue\t=", binaryDataValue);
			imageArray[cImageArrayIndex].RedValue	=	binaryDataValue >> 16;
			imageArray[cImageArrayIndex].GrnValue	=	binaryDataValue >> 16;
			imageArray[cImageArrayIndex].BluValue	=	binaryDataValue >> 16;
//			imageArray[cImageArrayIndex].BluValue	=	0x8123;
			cImageArrayIndex++;
		}
	}

	else
	{
		CONSOLE_DEBUG_W_NUM("Unknown rank value", cBinaryImageHdr.Rank);
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
static void WriteRawDataForDebug(const char *dataBuffer, const int arrayLength)
{
FILE	*filePointer;

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen("rawdata.bin", "w");
	if (filePointer != NULL)
	{
		fwrite(dataBuffer, arrayLength, 1, filePointer);


		fclose(filePointer);
	}
}


//*****************************************************************************
int	ControllerCamera::AlpacaGetImageArray_Binary(	TYPE_ImageArray	*imageArray,
													int				imageArrayLen,
													int				*actualValueCnt)
{
unsigned char	*binaryImgHdrPtr;
int				imgRank;
int				jjj;
char			dataTypeString[24];
int				dataBlkCount;

	CONSOLE_DEBUG("-----------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("imageArrayLen\t\t=",	imageArrayLen);

	tStartMillisecs			=	millis();
	tLastUpdateMillisecs	=	tStartMillisecs;

	imgRank			=	0;
	cRGBidx			=	0;
	dataBlkCount	=	0;
	while (cKeepReading)
	{
		dataBlkCount++;
		if (cReadBinaryHeader)
		{
			CONSOLE_DEBUG_W_NUM("contentLength\t\t=",	cHttpHdrStruct.contentLength);
			CONSOLE_DEBUG_W_NUM("cRecvdByteCnt\t\t=",	cRecvdByteCnt);
	//		DumpHex(cReturnedData, 40);

			WriteRawDataForDebug(cReturnedData, imageArrayLen);

			//*	process the binary header
			memset(&cBinaryImageHdr, 0, sizeof(TYPE_BinaryImageHdr));
			binaryImgHdrPtr	=	(unsigned char *)&cBinaryImageHdr;

			CONSOLE_DEBUG_W_NUM(	"cData_iii                  \t=", cData_iii);
			CONSOLE_DEBUG_W_SIZE(	"sizeof(TYPE_BinaryImageHdr)\t=", sizeof(TYPE_BinaryImageHdr));
			for (jjj=0; jjj < (int)sizeof(TYPE_BinaryImageHdr); jjj++)
			{
				binaryImgHdrPtr[jjj]	=	cReturnedData[cData_iii];
				cData_iii++;
			}
			CONSOLE_DEBUG_W_NUM("cData_iii                  \t=", cData_iii);
			CONSOLE_DEBUG("Imagebytes header");
//			DumpHex((char *)binaryImgHdrPtr, 6);
			CONSOLE_DEBUG("Raw data (cReturnedData)");
//			DumpHex((char *)cReturnedData, 100);

			cReadBinaryHeader	=	false;
			imgRank				=	cBinaryImageHdr.Rank;
			cImgArrayType		=	cBinaryImageHdr.TransmissionElementType;

			CONSOLE_DEBUG_W_NUM("MetadataVersion        \t=",	cBinaryImageHdr.MetadataVersion);
			CONSOLE_DEBUG_W_NUM("ErrorNumber            \t=",	cBinaryImageHdr.ErrorNumber);
			CONSOLE_DEBUG_W_NUM("ClientTransactionID    \t=",	cBinaryImageHdr.ClientTransactionID);
			CONSOLE_DEBUG_W_NUM("ServerTransactionID    \t=",	cBinaryImageHdr.ServerTransactionID);
			CONSOLE_DEBUG_W_NUM("DataStart              \t=",	cBinaryImageHdr.DataStart);
			CONSOLE_DEBUG_W_NUM("ImageElementType       \t=",	cBinaryImageHdr.ImageElementType);
			GetBinaryElementTypeString(cBinaryImageHdr.ImageElementType, dataTypeString);
			CONSOLE_DEBUG_W_STR("ImageElementType       \t=",	dataTypeString);

			CONSOLE_DEBUG_W_NUM("TransmissionElementType\t=",	cBinaryImageHdr.TransmissionElementType);
			GetBinaryElementTypeString(cBinaryImageHdr.TransmissionElementType, dataTypeString);
			CONSOLE_DEBUG_W_STR("TransmissionElementType\t=",	dataTypeString);

			CONSOLE_DEBUG_W_NUM("Rank                   \t=",	cBinaryImageHdr.Rank);
			CONSOLE_DEBUG_W_NUM("Dimension1             \t=",	cBinaryImageHdr.Dimension1);
			CONSOLE_DEBUG_W_NUM("Dimension2             \t=",	cBinaryImageHdr.Dimension2);
			CONSOLE_DEBUG_W_NUM("Dimension3             \t=",	cBinaryImageHdr.Dimension3);

//			CONSOLE_DEBUG("Image data");
//			DumpHexRGB((char *)&cReturnedData[cData_iii], 8);
//			CONSOLE_DEBUG_W_NUM("cData_iii\t\t\t=",			cData_iii);
		}


//		CONSOLE_DEBUG_W_NUM("cImageArrayIndex\t=",	cImageArrayIndex);
		//*	put the data into a long word based on the TransmissionElementType
		switch(cBinaryImageHdr.TransmissionElementType)
		{
			case kAlpacaImageData_Unknown:
			case kAlpacaImageData_Double:
			case kAlpacaImageData_Single:
			case kAlpacaImageData_Decimal:
				CONSOLE_DEBUG("Specified Binary mode not implemented yet")
//				LogEvent(	"camera",
//							"Binary Download",
//							NULL,
//							kASCOM_Err_Success,
//							"Mode not not supported yet");
				break;

			case kAlpacaImageData_Byte:
				AlpacaGetImageArray_Binary_Byte(imageArray, imageArrayLen);
				break;

			case kAlpacaImageData_Int32:
//				CONSOLE_DEBUG("kAlpacaImageData_Int32");
				AlpacaGetImageArray_Binary_Int32(imageArray, imageArrayLen);
				break;

			case kAlpacaImageData_Int64:
				CONSOLE_DEBUG("Specified Binary mode not implemented yet")
				break;

			case kAlpacaImageData_Int16:
			case kAlpacaImageData_UInt16:
				AlpacaGetImageArray_Binary_Int16(imageArray, imageArrayLen);
				break;

			default:
				CONSOLE_DEBUG_W_NUM("TransmissionElementType not supported yet:", cBinaryImageHdr.TransmissionElementType);
//				LogEvent(	"camera",
//							"Binary Download",
//							NULL,
//							kASCOM_Err_Success,
//							"Mode not not supported yet");
				break;
		}

		UpdateImageProgressBar(imageArrayLen);
		cRecvdByteCnt	=	recv(cSocket_desc, cReturnedData , kReadBuffLen , 0);
		if (cRecvdByteCnt > 0)
		{
			cSocketReadCnt++;
			cTotalBytesRead					+=	cRecvdByteCnt;
			cReturnedData[cRecvdByteCnt]	=	0;
			cData_iii	=	0;
		}
		else
		{
			cKeepReading		=	false;
		}
	}
	CONSOLE_DEBUG_W_NUM("dataBlkCount    \t=",	dataBlkCount);
	CONSOLE_DEBUG_W_NUM("imgRank         \t=",	imgRank);
	CONSOLE_DEBUG_W_NUM("cImageArrayIndex\t=",	cImageArrayIndex);
	return(imgRank);
}

//*****************************************************************************
int	ControllerCamera::AlpacaGetImageArray_JSON(	TYPE_ImageArray	*imageArray,
												int				imageArrayLen,
												int				*actualValueCnt)
{
int				imgRank;
char			theChar;
int				ccc;
int				braceCnt;	//*	{}
//int				bracktCnt;	//*	[]
//int				crCnt;
//int				lfCnt;
char			keywordStr[kLineBufSize];
char			valueStr[kLineBufSize];
char			linebuf[kReadBuffLen];
int				myIntegerValue;

	CONSOLE_DEBUG("------------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cData_iii\t=", cData_iii);

	braceCnt		=	0;
//	bracktCnt		=	0;
//	crCnt			=	0;
//	lfCnt			=	0;
	imgRank			=	3;		//*	default to color
	cRGBidx			=	0;
	ccc				=	0;
	while (cKeepReading)
	{
		//{"Type":2,"Rank":2,"Value":[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		//.......345,326,326,326,345]],"Type":2,"Rank":2,"ClientTransactionID":0,"ServerTransactionID":14087,"ErrorNumber":0,"ErrorMessage":"","DriverException":null}

		//*	dont reset cData_iii
		for (; cData_iii<cRecvdByteCnt; cData_iii++)
		{
			theChar	=	cReturnedData[cData_iii];
			if (theChar == '{')
			{
				braceCnt++;
			}
			else if (theChar == '}')
			{
				braceCnt--;
			}
//			else if (theChar == '[')
//			{
//				bracktCnt++;
//			}
//			else if (theChar == ']')
//			{
//				bracktCnt--;
//			}
//			else if (theChar == 0x0d)
//			{
//				crCnt++;
//			}
//			else if (theChar == 0x0a)
//			{
//				lfCnt++;
//			}

			if (cValueFoundFlag && ((theChar == ',') || (theChar == ']')))
			{
				linebuf[ccc]	=	0;
				if (isdigit(linebuf[0]))
				{
					myIntegerValue	=	atoi(linebuf);
					if (cImageArrayIndex < imageArrayLen)
					{
						if (imgRank == 3)
						{
							//*	deal with the individual R,G,B values
							switch(cRGBidx)
							{
								case 0:
									imageArray[cImageArrayIndex].RedValue	=	myIntegerValue;
									break;

								case 1:
									imageArray[cImageArrayIndex].GrnValue	=	myIntegerValue;
									break;

								case 2:
									imageArray[cImageArrayIndex].BluValue	=	myIntegerValue;
//											CONSOLE_DEBUG("RGB index == 2");
//											CONSOLE_DEBUG_W_STR("linebuf=", linebuf);
									break;
							}
							cRGBidx++;
							if (theChar == ']')
							{
								//*	debugging
//										if (imageArray[cImageArrayIndex].RedValue	 > 50000)
//										{
//											printf("#%6d=%6d\t%6d\t%6d\r\n",	cImageArrayIndex,
//																			imageArray[cImageArrayIndex].RedValue,
//																			imageArray[cImageArrayIndex].GrnValue,
//																			imageArray[cImageArrayIndex].BluValue);
//										}
								cRGBidx	=	0;
								cImageArrayIndex++;
							}
						}
						else
						{
							imageArray[cImageArrayIndex].RedValue	=	myIntegerValue;
							imageArray[cImageArrayIndex].GrnValue	=	myIntegerValue;
							imageArray[cImageArrayIndex].BluValue	=	myIntegerValue;
							cImageArrayIndex++;
						}
					}
					else
					{
						cRanOutOfRoomCnt++;
						if (cRanOutOfRoomCnt < 10)
						{
							CONSOLE_DEBUG("Ran out of room");
						}
					}
				}
				else
				{
				//	CONSOLE_DEBUG_W_STR("First char not digit=", linebuf);
					cFirstCharNotDigitCnt++;
				}
				ccc				=	0;
				linebuf[ccc]	=	0;

				if ((theChar == ']') || (cReturnedData[cData_iii+1] == ','))
				{
//					CONSOLE_DEBUG_W_NUM("Skipping at \t=", cData_iii);
					cData_iii++;
				}
			}

	//		else
			if (theChar == ',')
			{
				linebuf[ccc]	=	0;
				if (ccc > 0)
				{
//					CONSOLE_DEBUG_W_STR("linebuf\t=", linebuf);
				}
				if ((strlen(linebuf) > 3) && (linebuf[0] != 0x30))
				{
					ParseJsonKeyWordValuePair(linebuf, keywordStr, valueStr);
//							CONSOLE_DEBUG_W_2STR("KW:VAL\t=", keywordStr, valueStr);


					if (strcasecmp(keywordStr, "ArrayType") == 0)
					{
						cImgArrayType	=	atoi(valueStr);
					}
					else if (strcasecmp(keywordStr, "Type") == 0)
					{
						cImgArrayType	=	atoi(valueStr);
					}
					else if (strcasecmp(keywordStr, "rank") == 0)
					{
						imgRank	=	atoi(valueStr);
						CONSOLE_DEBUG_W_NUM("Found RANK\t=", imgRank);
					}
					else if (strcasecmp(keywordStr, "value") == 0)
					{
//								CONSOLE_DEBUG("value found!!!!");
						cValueFoundFlag	=	true;
					}
				}
				ccc	=	0;
			}
			else if ((theChar == 0x0d) || (theChar == 0x0a))
			{
				cLinesProcessed++;

//				//*	ignore
//				//*	check to see if there is a string
//				if (strcmp(linebuf, "400") == 0)
//				{
//				//	CONSOLE_DEBUG("Found 400");
//					ccc				=	0;
//					linebuf[ccc]	=	0;
//				}
//				else
//				{
//				int	mySlen;
//
//					mySlen	=	strlen(linebuf);
//					if ((mySlen > 1) && (linebuf[mySlen - 1] != ','))
//					{
//					//	CONSOLE_DEBUG("---------------------------------------");
//					//	CONSOLE_DEBUG(linebuf);
//					//	CONSOLE_DEBUG_W_NUM("cSocketReadCnt\t=", cSocketReadCnt);
//					//	CONSOLE_DEBUG_W_STR("cReturnedData\t=", cReturnedData);
//					}
//				}

			}
			else if (braceCnt > 0)
			{
				if (ccc < kImageArrayBuffSize)
				{
					//*	Dec 12,	2021	<MLS> Fixed bug in AlpacaGetImageArray() processing "]"
					if ((theChar != '[') && (theChar != ']') && (theChar > 0x20))
					{
						linebuf[ccc++]	=	theChar;
						linebuf[ccc]	=	0;
					}
				}
				else
				{
					CONSOLE_DEBUG("Line to long");
				}
			}
		}
		UpdateImageProgressBar(imageArrayLen);

		CONSOLE_DEBUG("Reading next packet");
		cRecvdByteCnt	=	recv(cSocket_desc, cReturnedData , kReadBuffLen , 0);
		if (cRecvdByteCnt > 0)
		{
			cSocketReadCnt++;
			cTotalBytesRead				+=	cRecvdByteCnt;
			cReturnedData[cRecvdByteCnt]	=	0;
			cData_iii	=	0;
		}
		else
		{
			cKeepReading		=	false;
		}
	}
	return(imgRank);
}


//*****************************************************************************
//*		In this routine, we are going to send the request and get the answer directly
//*		We cannot buffer the entire JSON response because it is going to be too big
//*		which prevents us from using the already developed routines
//*
//*		Open the socket
//*		Send the request
//*		Start reading the response,
//*		We have to parse the data here and not use the JSON parser
//*
//*	returns the RANK of the data found, 0 means no valid RANK
//*****************************************************************************
int	ControllerCamera::AlpacaGetImageArray(	const char		*alpacaDevice,
											const int		alpacaDevNum,
											const char		*alpacaCmd,
											const char		*dataString,
											const bool		allowBinary,
											TYPE_ImageArray	*imageArray,
											int				imageArrayLen,
											int				*actualValueCnt)
{
char			alpacaString[128];
int				shutDownRetCode;
int				closeRetCode;
char			theChar;
int				imgRank;
double			downLoadSeconds;
char			linebuf[kReadBuffLen];
bool			readingHttpHeader;
int				ccc;

	CONSOLE_DEBUG("------------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM(	"kReadBuffLen \t=", kReadBuffLen);
	CONSOLE_DEBUG_W_NUM(	"imageArrayLen\t=", imageArrayLen);
	CONSOLE_DEBUG_W_BOOL(	"allowBinary  \t=", allowBinary);

	cImgArrayType			=	-1;
	imgRank					=	2;	//*	default to 2
	cImageArrayIndex		=	0;
	cFirstCharNotDigitCnt	=	0;
	tStartMillisecs			=	millis();
	tLastUpdateMillisecs	=	tStartMillisecs;


	memset(&cHttpHdrStruct, 0, sizeof(TYPE_HTTPheader));

	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
	strcpy(cLastAlpacaCmdString, alpacaString);
	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	START_TIMING();
	cSocket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												dataString,
												allowBinary);
	if (cSocket_desc >= 0)
	{
		DEBUG_TIMING("---------------Time to send request (ms)");

		CONSOLE_DEBUG("Success: Connection open and data sent");
		cValueFoundFlag		=	false;
		cKeepReading		=	true;
		readingHttpHeader	=	true;
		cReadBinaryHeader	=	false;
		cData_iii			=	0;
		cLinesProcessed		=	0;
		cTotalBytesRead		=	0;
		cSocketReadCnt		=	0;
		cRanOutOfRoomCnt	=	0;
		ccc					=	0;
		while (cKeepReading)
		{
			cRecvdByteCnt	=	recv(cSocket_desc, cReturnedData , kReadBuffLen , 0);
			if (cRecvdByteCnt > 0)
			{
				cSocketReadCnt++;

				cTotalBytesRead					+=	cRecvdByteCnt;
				cReturnedData[cRecvdByteCnt]	=	0;

				CONSOLE_DEBUG_W_NUM("cRecvdByteCnt=", cRecvdByteCnt);

				DEBUG_TIMING("---------------Processing header (ms)");
				//----------------------------------------------------------------
				//*	this part reads and processes the HTTP header
				while (readingHttpHeader && (cData_iii < cRecvdByteCnt))
				{
					theChar	=	cReturnedData[cData_iii];
					if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						//*	null terminate the line
						linebuf[ccc]	=	0;

						if (strlen(linebuf) > 0)
						{
							//*	process the header line
							ProcessHTTPheaderLine(linebuf, &cHttpHdrStruct);
							if (cHttpHdrStruct.dataIsBinary)
							{
								cReadBinaryHeader	=	true;
							}
						}
						else
						{
							//*	Done with the header
							CONSOLE_DEBUG("Done with the header");
							readingHttpHeader	=	false;
						}

						//*	reset for next line
						ccc				=	0;
						linebuf[ccc]	=	0;

						//*	check for the lf of cr/lf
					//	if ((cReturnedData[cData_iii + 1] == 0x0a) || (cReturnedData[cData_iii + 1] == 0x0d))
						if ((cReturnedData[cData_iii + 1] == 0x0a))
						{
							cData_iii++;
						}
					}
					else
					{
						linebuf[ccc]	=	theChar;
						ccc++;
					}
					cData_iii++;
				}

				//*	check to see if we have binary data
				if (cHttpHdrStruct.dataIsBinary)
				{
					DEBUG_TIMING("---------------Data is binary, time= (ms)");
					imgRank	=	AlpacaGetImageArray_Binary(imageArray, imageArrayLen, actualValueCnt);
				}
				else
				{
					CONSOLE_DEBUG("Data is JSON");
					imgRank	=	AlpacaGetImageArray_JSON(imageArray, imageArrayLen, actualValueCnt);
				}
			}
			else
			{
				cKeepReading		=	false;
			}
			//=================================================
			//*	deal with the progress bar
			UpdateImageProgressBar(imageArrayLen);
		}
		//*	one last time to show we are done
		UpdateDownloadProgress(cImageArrayIndex, imageArrayLen);

		*actualValueCnt	=	cImageArrayIndex;
		CONSOLE_DEBUG_W_NUM("actualValueCnt\t=", *actualValueCnt);

		DEBUG_TIMING("Time to download image (ms)");
		CONSOLE_DEBUG_W_NUM("cSocketReadCnt\t=", cSocketReadCnt);

		cLastDownload_Bytes			=	cTotalBytesRead;
		cLastDownload_Millisecs		=	tDeltaMillisecs;
		downLoadSeconds				=	tDeltaMillisecs / 1000.0;
		if (downLoadSeconds > 0)
		{
			cLastDownload_MegaBytesPerSec	=	1.0 * cTotalBytesRead / downLoadSeconds;
		}
		else
		{
			CONSOLE_DEBUG("tDeltaMillisecs invalid");
			cLastDownload_MegaBytesPerSec	=	0.0;
		}
//		CONSOLE_DEBUG_W_NUM("crCnt\t=", crCnt);
//		CONSOLE_DEBUG_W_NUM("lfCnt\t=", lfCnt);

		CONSOLE_DEBUG_W_NUM("cRanOutOfRoomCnt     \t=", cRanOutOfRoomCnt);
		CONSOLE_DEBUG_W_NUM("imageArrayLen        \t=", imageArrayLen);
		CONSOLE_DEBUG_W_NUM("cImgArrayType        \t=", cImgArrayType);
		CONSOLE_DEBUG_W_NUM("imgRank              \t=",	imgRank);
		CONSOLE_DEBUG_W_NUM("cTotalBytesRead      \t=", cTotalBytesRead);
		CONSOLE_DEBUG_W_NUM("cLinesProcessed      \t=", cLinesProcessed);
		CONSOLE_DEBUG_W_NUM("cImageArrayIndex     \t=", cImageArrayIndex);
		CONSOLE_DEBUG_W_NUM("cFirstCharNotDigitCnt\t=", cFirstCharNotDigitCnt);

		shutDownRetCode	=	shutdown(cSocket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		closeRetCode	=	close(cSocket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
	return(imgRank);
}
