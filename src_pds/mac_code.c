
#ifdef __MAC__

//*****************************************************************************
struct
{
			char	*itemName;
			int		itemLength;
} PDSitemList[] =
		{
			" LINES",			6,
			" LINE_SAMPLES",	13,
			" LINE_SAMPLES",	13,

			" foo",	0
		};





long	gRecordNumber;

//*********************************************************************
//*
//*	subroutine read_variableRecord
//*	- read variable length records from input file
//*
//*********************************************************************
int read_variableRecord(short fileRefNum, char *ibuf)
{
int		length,result,nlen;
char	temp;
int		bytecount;

union //*	this union is used to swap 16 and 32 bit integers
	{
		char  ichar[4];
		short slen;
		long  llen;
	} onion;

	length	=	0;

	bytecount	=	2;
	FSRead(fileRefNum, &bytecount, &onion.ichar );

	//*	byte swap the length field
	temp  			=	onion.ichar[0];
	onion.ichar[0]	=	onion.ichar[1];
	onion.ichar[1]	=	temp;

	length 			=	onion.slen;
	bytecount		=	length+(1*length%2);
	FSRead(fileRefNum, &bytecount, ibuf);
	ibuf[length]	=	0;

	gRecordNumber++;
	return(length);
}

char	PDSlabelData[4000];
//********************************************************************
//*
//*	subroutine Skip PDS Labels - read past the pds labels
//*
//********************************************************************
void skip_PDS_labels(short frefNum)
{
char		ibuf[2048];
char		outstring[80];
int			length;
//int			iii;


	PDSlabelData[0]	=	255;
	PDSlabelData[1]	=	0;
	do
	{
		length	=	read_variableRecord(frefNum, ibuf);
		if (	(strncmp(ibuf,"SPACECRAFT",10) == 0)
			||	(strncmp(ibuf,"MISSION",7) == 0)
			||	(strncmp(ibuf,"TARGET",6) == 0)
			||	(strncmp(ibuf,"IMAGE_NUMBER",12) == 0))
		{
			strncat(PDSlabelData, ibuf, length);
			strncat(PDSlabelData, " *\r", 3);
		}
		//*****************************************************************
		//*	read to the end of the PDS labels
		//*****************************************************************
		if ((strncmp(ibuf,"END",3)) == 0 && length == 3) break;
	} while (length > 0);
}



//*****************************************************************************
void read_PDS_labels(short frefNum, short recordLen)
{
char		ibuf[2048],lineBuf[256];
short		length, labelRecCount, ii, lbfCtr;
char		*argptr;
char		theChar;
long		bytecount;
bool		eofFlag;
OSErr		iErr;

	PDSlabelData[0]	=	255;
	PDSlabelData[1]	=	0;


	eofFlag	=	false;
	if (recordLen == 0)
	{
		while (eofFlag == false)
		{
			length	=	read_variableRecord(frefNum, ibuf);
			if (length == 0)
			{
				eofFlag	=	true;
			}

			argptr	=	strchr(ibuf, '=');						// find the '='
			if (argptr != 0)
			{
				argptr++;										// skip the '='
				if (argptr[0] == 0x20)	argptr++;				// skip any spaces
				if (argptr[0] == 0x27)	argptr++;				// skip any single quote
				if (argptr[0] == '"')	argptr++;				// skip any quotes

			}
			eofFlag =	parsePDS_Label_Entry(ibuf, argptr);
		}
	}
	else
	{
		//*	we dont know how long the label area is, take a guess and hope it gets fixed
		PDSheader.label_records		=	10;

		labelRecCount	=	0;
		while ((labelRecCount < PDSheader.label_records) && (eofFlag == false))
		{
			bytecount	=	recordLen;
			iErr		=	FSRead(frefNum, &bytecount, ibuf);
			labelRecCount++;
			ibuf[bytecount]	=	0;
			if (iErr != noErr)
			{
				eofFlag	=	true;
			}
			lbfCtr	=	0;
			for (ii = 0; ii<bytecount; ii++)
			{
				theChar	=	ibuf[ii];
				if (theChar >= 0x20)
				{
					lineBuf[lbfCtr++]	=	theChar;
				}
				else if (theChar == 0x0D)
				{
					lineBuf[lbfCtr++]	=	0;
					argptr	=	strchr(lineBuf, '=');					// find the '='
					if (argptr != 0)
					{
						argptr++;										// skip the '='
						if (argptr[0] == 0x20)	argptr++;				// skip any spaces
						if (argptr[0] == 0x27)	argptr++;				// skip any single quote
						if (argptr[0] == '"')	argptr++;				// skip any quotes

					}
					eofFlag =	parsePDS_Label_Entry(lineBuf, argptr);
					lbfCtr	=	0;
				}
			}
		}
	}

}


//*****************************************************************************
//*	This routine will swap bytes of the histogram integer long words for computer
//*	hardware which stores long words in "most significant byte order first" order.
//*	This is necessary because the DECOMPRESSION routines will automatically
//*	swap the byte order for this computer hardware.
//*****************************************************************************
void swapit(long *longwords)
{
int				cnt;
unsigned char	*cp	=	(unsigned char *) longwords;
unsigned long	jjj;
int 			iii;

	for (cnt=512; cnt--;)
	{
		for (iii=4; --iii >= 0; jjj = (jjj<<8) | *(cp+iii));
		{
			*longwords++	=	jjj;
		}
	}
}


//*****************************************************************************
//*	figure out what type of label type the file is
//*
//*			LABEL_UNKNOWN
//*			VARIABLE_LENGTH,
//*			FIXED_LENGTH,
//*			STREAM
//*
//*****************************************************************************
#define		testBuffSize	2048L
short	getLabelType(short fRefNum, short *recordSize)
{
char	*bufPtr;
char	*recTypeStrPtr;
char	*recBytesStrPtr;
char	*argptr;
long	bytecount;
short	returnValue;
short	myRecSize;

	//*	set the default error conditions
	*recordSize		=	-1;
	returnValue		=	LABEL_UNKNOWN;

	bufPtr		=	(char *)NewPtr(testBuffSize + 10);
	if (bufPtr != nil)
	{
		bytecount	=	testBuffSize;
		FSRead(fRefNum, &bytecount, bufPtr);
		bufPtr[testBuffSize]	=	0;

		recTypeStrPtr	=	strstr(bufPtr, "RECORD_TYPE");
		if (recTypeStrPtr != nil)
		{
			//*	now check for the different types of records
			if (strstr(recTypeStrPtr, "VARIABLE_LENGTH") != nil)
			{
				returnValue		=	VARIABLE_LENGTH;
				*recordSize		=	0;
			}
			else if (strstr(recTypeStrPtr, "FIXED_LENGTH") != nil)
			{
				returnValue		=	FIXED_LENGTH;
				recBytesStrPtr	=	strstr(recTypeStrPtr, "RECORD_BYTES");
				if (recBytesStrPtr != nil)
				{
					argptr	=	strchr(recBytesStrPtr, '=');			// find the '='
					if (argptr != 0)
					{
						argptr++;										// skip the '='
						if (argptr[0] == 0x20)	argptr++;				// skip any spaces
						if (argptr[0] == 0x27)	argptr++;				// skip any single quote
						if (argptr[0] == '"')	argptr++;				// skip any quotes
						sscanf(argptr, "%d", &myRecSize);

						*recordSize	=	myRecSize;
					}
				}
			}
		}
		SetFPos(fRefNum, fsFromStart, 0L);
	}
	DisposePtr(bufPtr);
	return(returnValue);
}

extern	char	imagebuff[8192];		//*	: array[1..8192] of signedbyte;

//*****************************************************************************
bool OpenPDS_Labeled_Image(Str255 fname, short vnum, short labelType)
{
GrafPtr		tPort;
char		ibuf[2048],obuf[2048];
long		nsi,nso,nl,il;
long		hist[HISTOGRAM_SIZE+1];
long		hist2[HISTOGRAM_SIZE+1];
long		hist3[HISTOGRAM_SIZE+1];
long		total_bytes, length, long_length;
char		linei[836],lineo[836];
long		line;
long		out_bytes	=	VOYAGER_RECORD_BYTES;
short		count;
short			i, j;
Ptr				iptr;
long			TempSize;
long			bytecount;
OSErr			err;
short			fRefNum;
Str255			linebuf;
Rect			PDStextRect;
short			pdsLabelType, pdsRecordSize;


	ShowWatch();
	err 	= FSOpen(fname, vnum, &fRefNum);
	if ( err != noErr )
	{
		SysBeep(1);
		return(false);
	}
	info->vref 	= vnum;			//*	save the Volumne Number
	SaveInfo 	= info;
	iptr 	= NewPtr(sizeof(PicInfo));
	if ( iptr == nil )
	{
		PutOutOfMemMsg();
		DisposPtr(iptr);
		err 	= FSClose(fRefNum);
		return(false);
	}

	info 	= (InfoPtr)iptr;
	*info 	= *SaveInfo;
	clrPDSinfoStuf(info);


	//*****************************************************************************
	//*****************************************************************************
	//*
	//*	Read all of the PDS (Planetary Data System) header
	//*
	//*****************************************************************************
	initPDSheader();

	if (labelType == VARIABLE_LENGTH)
	{
		pdsLabelType	=	VARIABLE_LENGTH;
		pdsRecordSize	=	0;
	}
	else
	{
		pdsLabelType	=	getLabelType(fRefNum, &pdsRecordSize);
	}

	read_PDS_labels(fRefNum,  pdsRecordSize);

	out_bytes	=	PDSheader.record_Bytes;

	if (PDSheader.record_Bytes == 836)
	{
		PDS_readingFileType		= VoyagerFull;			//*	set file type that we are reading
	}
	else
	{
		PDS_readingFileType		= VikingFull;			//*	set file type that we are reading
	}

	info->LutMode 		= GrayScale;
	info->PixelsPerLine	= PDSheader.lineSamples;
	info->nlines		= PDSheader.scanLines + PDSLabelTextHeight;
	info->PicSize 		= (long)info->nlines * info->PixelsPerLine;
	TempSize 			= info->PicSize;

	info->PicBaseAddr	=	(Ptr)GetMemory(TempSize, true);
	if (info->PicBaseAddr == nil)
	{
		err 	= FSClose(fRefNum);
		DisposeInfoPtr();
		return(false);
	}

	//*	set up the color lookup table
	info->nColors 		= 256;
	info->ColorStart 	= 0;
	info->ColorWidth 	= 8;
	UpdateColors();


	SetDefaultColorTable();
	MakeNewWindow(fname, WPicture);
	SelectAll(false);
	DoOperation(EraseOp);
	info->RoiType 	=	NoRoi;
	info->Changes 	=	false;
	info->BinaryPic	=	false;
	ResetGrayMap();
	SaveOriginalColorPalette();


	setPDSinfoStuf(info, gCurrentVolName, (char *)fname);
	PDS_readingFileType	=	0;	//*	reset the flag for the next time


	//********************************************************************
	//*
	//*	process the image histogram
	//*
	//*********************************************************************

	//*	need to know record_bytes,hist_count,hist_item_type,item_count.
	total_bytes =	0;
	length		=	read_variableRecord(fRefNum, (char *)hist);
	total_bytes	=	total_bytes + length;

	if (PDSheader.record_Bytes == 836)	//*	read one more time for Voyager image
	{
		length		=	read_variableRecord(fRefNum,(char *)hist + PDSheader.record_Bytes);
		total_bytes	=	total_bytes + length;
	}


	//*********************************************************************
	//*
	//*	process the encoding histogram
	//*	don't have to byte-swap because DECOMP.C does it for us
	//*
	//*********************************************************************
	if (PDSheader.record_Bytes == 836)
	{
		length	=	read_variableRecord(fRefNum, (char *)hist);
		length	=	read_variableRecord(fRefNum, (char *)hist+836);
		length	=	read_variableRecord(fRefNum, (char *)hist+1672);
	}
	else
	{
		length	=	read_variableRecord(fRefNum, (char *)hist);
		length	=	read_variableRecord(fRefNum, (char *)hist+1204);
	}

	//********************************************************************
	//*
	//*	process the engineering summary
	//*
	//********************************************************************

	total_bytes	=	0;
	length		=	read_variableRecord(fRefNum, ibuf);

	//********************************************************************
	//*
	//*	process the line header table
	//*
	//********************************************************************

	if (	  	(PDSheader.record_Bytes == 1204)
			||	(PDSheader.record_Bytes == 1224)
			||	(PDSheader.record_Bytes == 1310)
			||	(PDSheader.record_Bytes == 1457))
	{
		long_length	=	0L;
		for (i=0;i<1056;i++)
		{
			length	=	read_variableRecord(fRefNum, ibuf);
		}
	}
	//********************************************************************
	//*
	//*	initialize the decompression
	//*
	//********************************************************************

	//*DEBUG printf("\nInitializing decompression routine...");

	decmpinit(hist);


	strcpy((Ptr)linebuf, (Ptr)"\pReading PDS file: ");
	PtoPstrcat(linebuf, fname);
	DisplayReadStatus((Ptr)linebuf, PDSlabelData);	//*	display status dialog box (themometer bar )

	//********************************************************************
	//*
	//*	read in decompress, and display the image
	//*
	//********************************************************************

	//*DEBUG printf("\nDecompressing data...\n");

	//*********************************************************************
	//*	put the name etc as the first line of the image
	//*********************************************************************
	if (PDSLabelTextHeight != 0)
	{
		GetPort(&tPort);
		SetPort((GrafPtr)info->osPort);
		TextSize(9);
		TextFont(monaco);
		TextMode(srcCopy);
		SetBackgroundColor(WhiteIndex);
		SetForegroundColor(BlackIndex);
		SetRect(&PDStextRect, 0 ,0, info->PixelsPerLine, PDSLabelTextHeight);
		FrameRect(&PDStextRect);
		MoveTo(5, 12);
		for (i=1; i<250; i++)
		{
			if (PDSlabelData[i] == 0) break;
			if (PDSlabelData[i] < 0x20) PDSlabelData[i]	=	0x20;
			if (PDSlabelData[i] == 0x20)
			{
				//*	remove duplicate spaces
				while (PDSlabelData[i+1] == 0x20)
				{
					for (j=i+1; j<250; j++)
					{
						PDSlabelData[j]	=	PDSlabelData[j+1];
					}
				}
			}
		}
		PDSlabelData[0]	=	i;
		DrawString((unsigned char *)PDSlabelData);
		SetPort(tPort);
	}
	line	= PDSLabelTextHeight;
	length	= 1;
	while ((line < PDSheader.scanLines + PDSLabelTextHeight) && (length > 0) && !CommandPeriod() )
	{

		length	=	read_variableRecord(fRefNum, ibuf);
		if (length <= 0)
		{
			break;
		}
		long_length	=	(long)length;
		decompress(ibuf, imagebuff, &long_length, &out_bytes);

		if (PDSheader.record_Bytes == 836)
		{
			//*	invert the pixels for Mac grayscale clut
			for (i=0; i<info->PixelsPerLine; i++)
			{
				imagebuff[i]	=	256 - imagebuff[i];
			}
		}
		stuffPixels(info->osPort, info->PixelsPerLine, info->nlines, line);

		if (((line+1) % 25) == 0)
		{
			UpdateReadStatus((line * 100L) / info->nlines);
			if (line < ScreenHeight)
			{
				UpdatePicWindow();
			}
		}
		line += 1;
		//*DEBUG if (line % 100 == 0) printf("line %ld\n",line);
	}
	CloseReadStatus();


	//*DEBUG printf("\n");
	err 	= FSClose(fRefNum);
	return(true);
}


//*****************************************************************************
bool OpenPDSimage(Str255 fname, short vnum)
{
GrafPtr		tPort;
char		ibuf[2048],obuf[2048];
long		nsi,nso,nl,il;
long		hist[HISTOGRAM_SIZE+1];
long		hist2[HISTOGRAM_SIZE+1];
long		hist3[HISTOGRAM_SIZE+1];
long		total_bytes, length, long_length;
char		linei[836],lineo[836];
long		line;
long		out_bytes	=	VOYAGER_RECORD_BYTES;
short		count;
short			i, j;
Ptr				iptr;
long			TempSize;
long			bytecount;
OSErr			err;
short			fRefNum;
Str255			linebuf;
Rect			PDStextRect;

	ShowWatch();
	err 	= FSOpen(fname, vnum, &fRefNum);
	if ( err != noErr )
	{
		SysBeep(1);
		return(false);
	}

	initPDSheader();
	//********************************************************************
	//*	Read all of the PDS (Planetary Data System) header
	//*******************************************************************
	read_PDS_labels(fRefNum, 0);
	out_bytes	=	PDSheader.record_Bytes;

	if (PDSheader.record_Bytes == 836)
	{
		PDS_readingFileType		= VoyagerFull;			//*	set file type that we are reading
	}
	else
	{
		PDS_readingFileType		= VikingFull;			//*	set file type that we are reading
	}

	//*DEBUG printf("\n");
	err 	= FSClose(fRefNum);
	return(true);
}



//*****************************************************************************
bool OpenPDS_LabelFile(Str255 fname, short vnum, short labelType)
{
union //*	this union is used to swap 16 and 32 bit integers
	{
		char  ichar[4];
		short slen;
		long  llen;
	} onion;
char				temp;
OSErr				err;
short				fRefNum;
char				ibuf[3000];
short				length;
short				i, crCount;
long				j1, j2;
long				bytecount, FileLength;
Ptr					iptr;
textEditInfoPtr		TEDataPtr;
long				TempSize;
short				fileRecordType;
char				*inputBuffP, *inputBuffP2;
short			vRefNum;
long			freeBytes;
enum
{
	VariableRec	=	1,
	LinesEndInCR,
	FixedLen80
};

	ShowWatch();
	PDS_readingFileType		= PDSLabelText;		//*	set file type that we are reading

	err 	= FSOpen(fname, vnum, &fRefNum);
	if ( err != noErr )
	{
		return(false);
	}

	if (GetEOF(fRefNum,&FileLength) != noErr)
	{
		FSClose(fRefNum);
		return(false);
	}

	//*	Create a new info record
	SaveInfo 	= info;
	iptr 	= NewPtr(sizeof(PicInfo));
	if ( iptr == nil )
	{
		PutOutOfMemMsg();
		DisposPtr(iptr);
		err 	= FSClose(fRefNum);
		return(false);
	}

	info 	= (InfoPtr)iptr;
	*info 	= *SaveInfo;
	clrPDSinfoStuf(info);

	info->vref 			= vnum;			//*	save the Volumne Number
	info->LutMode 		= AppleDefault;
	info->PixelsPerLine	= 600;
	info->nlines		= 400;
	info->PicSize 		= (long)info->nlines * info->PixelsPerLine;
	TempSize 			= info->PicSize;

	info->PicBaseAddr	= nil;

	//*	set up the color lookup table
	info->nColors 		= 256;
	info->ColorStart 	= 0;
	info->ColorWidth 	= 8;
	UpdateColors();


	SetDefaultColorTable();
	MakeNewWindow(fname, WTextScroll);

	SetPort((GrafPtr)info->osPort);

	TEDataPtr	= (textEditInfoPtr)info->textEditRec;


	err	=	GetVInfo(vnum, (StringPtr)gCurrentVolName, &vRefNum, &freeBytes);

	setPDSinfoStuf(info, gCurrentVolName, (char *)fname);

	//*****************************************************************
	//*	now check to see what type of data it is
	//*****************************************************************
	bytecount	=	2;
	FSRead(fRefNum, &bytecount, &onion.ichar );
	//*	byte swap the length field
	temp			=	onion.ichar[0];
	onion.ichar[0]	=	onion.ichar[1];
	onion.ichar[1]	=	temp;

	length 		= onion.slen;
	if (info->fnameExtension == '.FIT')
	{
		fileRecordType	=	FixedLen80;
	}
	else if ((length > FileLength) || (length > 0x2000) || (length == 0x0A0D))
	{
		fileRecordType	= LinesEndInCR;
	}
	else
	{
		fileRecordType	= VariableRec;
	}

	SetFPos(fRefNum, fsFromStart, 0L);

	switch (fileRecordType)
	{
		case VariableRec:
			//*	the file is variable length records, read it that way
			{
				length	=	1;
				while (length > 0)
				{
					length	=	read_variableRecord(fRefNum, ibuf);
					strncat(ibuf, "\r", 1);
					bytecount	=	length +1;
					TEStylInsert(ibuf, bytecount, nil, TEDataPtr->TEH);
					//*	read to the end of the PDS labels
//					if (((i = strncmp(ibuf,"END",3)) == 0) && length == 3)
					if (((strncmp(ibuf,"END",3)) == 0) && length == 3)
					{
						break;
					}
				}
			}
			break;
		case LinesEndInCR:
			{
				//*	the file is NOT variable length recs but lines end in CR/LF
				if (FileLength < 30000)
				{
					bytecount	=	FileLength;
				}
				else
				{
					bytecount	=	30000;
				}
				inputBuffP	=	NewPtr(bytecount + 10);
				FSRead(fRefNum, &bytecount, inputBuffP);
				j1		=	0;
				crCount	=	0;
				for (j2 = 0; j2 < bytecount; j2++)
				{
					if ((inputBuffP[j2] >= 0x20) || (inputBuffP[j2] == 0x0d))
					{
						inputBuffP[j1++]	=	inputBuffP[j2];
					}
					if (inputBuffP[j2] == 0x0d) crCount++;

				}
				bytecount	=	j1;
				if (crCount == 0)
				{
					inputBuffP2	=	NewPtr(bytecount + (bytecount/80) + 100);

					j1	=	0;
					for (j2 = 0; j2 < bytecount; j2++)
					{
						inputBuffP2[j1++]	=	inputBuffP[j2];
						if (((j2+1) % 80) == 0)
						{
							inputBuffP2[j1++]	=	0x0d;
						}
					}
					inputBuffP2[j1++]	=	0x0d;
					TEStylInsert(inputBuffP2, j1, nil, TEDataPtr->TEH);
					DisposPtr(inputBuffP2);
				}
				else
				{
					TEStylInsert(inputBuffP, j1, nil, TEDataPtr->TEH);
				}
				DisposPtr(inputBuffP);
			}
			break;
		case 	FixedLen80:
			{
			short	bytesInLine	=	0;
				//*	the file is NOT variable length recs but lines end in CR/LF
				if (FileLength < 30000)
				{
					bytecount	=	FileLength;
				}
				else
				{
					bytecount	=	30000;
				}
				inputBuffP	=	NewPtr(bytecount + 10);
				FSRead(fRefNum, &bytecount, inputBuffP);
				j1		=	0;
				crCount =	0;
				for (j2 = 0; j2 < bytecount; j2++)
				{
					if ((inputBuffP[j2] >= 0x20) || (inputBuffP[j2] == 0x0d))
					{
						inputBuffP[j1++]	=	inputBuffP[j2];
					}
					if (bytesInLine++ >= 79)
					{
						inputBuffP[j2]	=	0x0d;
						bytesInLine		=	0;
					}
					if (inputBuffP[j2] == 0x0d) crCount++;

				}
				bytecount	=	j1;
				if (crCount == 0)
				{
					inputBuffP2	=	NewPtr(bytecount + (bytecount/80) + 100);

					j1	=	0;
					for (j2 = 0; j2 < bytecount; j2++)
					{
						inputBuffP2[j1++]	=	inputBuffP[j2];
						if (((j2+1) % 80) == 0)
						{
							inputBuffP2[j1++]	=	0x0d;
						}
					}
					inputBuffP2[j1++]	=	0x0d;
					TEStylInsert(inputBuffP2, j1, nil, TEDataPtr->TEH);
					DisposPtr(inputBuffP2);
				}
				else
				{
					TEStylInsert(inputBuffP, j1, nil, TEDataPtr->TEH);
				}
				DisposPtr(inputBuffP);
			}
			break;
		}
	TextWind_SetVScroll();
	TextWind_AdjustText();
#ifdef DEBUGGING
	printf ("%ld records read\n", i);
#endif
	err 	= FSClose(fRefNum);


	return(true);
}

//*****************************************************************************
//* IMGINDEX.TAB;1
//* The list below presents each field in the image index table, along
//* with a 10 character terse name for each field, the full PDS data
//* dictionary name, the data type which would normally be used for
//* loading the field into a data management system, the field width and
//* the number of decimal places in numeric fields.
//*
//*				 IMAGE INDEX FLAT TABLE CONTENTS
//*
//* Field  Terse Name  Full Name            Type        Start  Width   Dec
//* -----  ----------- -------------------  ----------  -----  -----   ---
//*     1  SCNAME      SPACECRAFT_NAME      Character       2      9
//*     2  MSNPHSNM    MISSION_PHASE_NAME   Character      14     17
//*     3  TARGETNAME  TARGET_NAME          Character      34      8
//*     4  IMAGEID     IMAGE_ID             Character      45     10
//*     5  IMAGENUM    IMAGE_NUMBER         Numeric        57      8     2
//*     6  IMAGETIME   IMAGE_TIME           Character      67     20
//*     7  EARTHRCDTM  EARTH_RECEIVED_TIME  Character      90     20
//*     8  INSTRNAME   INSTRUMENT_NAME      Character     113     19
//*     9  SCANMODEID  SCAN_MODE_ID         Character     135      7
//*    10  SHUTMODEID  SHUTTER_MODE_ID      Character     145      7
//*    11  GAINMODEID  GAIN_MODE_ID         Character     155      7
//*    12  EDITMODEID  EDIT_MODE_ID         Character     165      7
//*    13  FILTERNAME  FILTER_NAME          Character     175      7
//*    14  FILTERNUM   FILTER_NUMBER        Numeric       184      4
//*    15  EXPOSUREDU  EXPOSURE_DURATION    Numeric       189      7     4
//*    16  NOTE        NOTE                 Character     198     80
//*    17  SMPLBITMSK  SAMPLE_BIT_MASK      Character     281      8
//*    18  DATAANMTYP  DATA_ANOMALY_TYPE    Character     292      6
//*    19  IMAGEVOLID  IMAGE_VOLUME_ID      Character     301      8
//*    20  IMAGEFILNM  IMAGE_FILE_NAME      Character     312     31
//*    21  BROWSVOLID  BROWSE_VOLUME_ID     Character     346      8
//*    22  BROWSFILNM  BROWSE_FILE_NAME     Character     357     38
//*
//*****************************************************************************

#define	SPACECRAFT_NAME_index			1
#define	MISSION_PHASE_NAME_index		2
#define	TARGET_NAME_index				3
#define	IMAGE_ID_index					4
#define	IMAGE_NUMBER_index				5
#define	IMAGE_TIME_index				6
#define	EARTH_RECEIVED_TIME_index		7
#define	INSTRUMENT_NAME_index			8
#define	SCAN_MODE_ID_index				9
#define	SHUTTER_MODE_ID_index			10
#define	GAIN_MODE_ID_index				11
#define	EDIT_MODE_ID_index				12
#define	FILTER_NAME_index				13
#define	FILTER_NUMBER_index				14
#define	EXPOSURE_DURATION_index			15
#define	NOTE_index						16
#define	SAMPLE_BIT_MASK_index			17
#define	DATA_ANOMALY_TYPE_index			18
#define	IMAGE_VOLUME_ID_index			19
#define	IMAGE_FILE_NAME_index			20
#define	BROWSE_VOLUME_ID_index			21
#define	BROWSE_FILE_NAME_index			22


//*****************************************************************************
typedef struct
{
	bool	selected;
	char	SPACECRAFT_NAME[16 + 2];
	char	MISSION_PHASE_NAME[32 + 2];
	char	TARGET_NAME[16 + 2];
		//*	char	IMAGE_ID[10 + 2];
	char	IMAGE_NUMBER[8 + 2];
		//*	char	IMAGE_TIME[20 + 2];
		//*	char	EARTH_RECEIVED_TIME[20 + 2];
		//*	char	INSTRUMENT_NAME[19 + 2];
		//*	char	SCAN_MODE_ID[7 + 2];
		//*	char	SHUTTER_MODE_ID[7 + 2];
		//*	char	GAIN_MODE_ID[7 + 2];
		//*	char	EDIT_MODE_ID[7 + 2];
		//*	char	FILTER_NAME[7 + 2];
		//*	char	FILTER_NUMBER[4 + 2];
		//*	char	EXPOSURE_DURATION[7 + 2];
	char	NOTE[160 + 2];
		//*	char	SAMPLE_BIT_MASK[8 + 2];
		//*	char	DATA_ANOMALY_TYPE[6 + 2];
	char	IMAGE_VOLUME_ID[8 + 2];
	char	IMAGE_FILE_NAME[31 + 2];
	char	BROWSE_VOLUME_ID[8 + 2];
	char	BROWSE_FILE_NAME[38 + 2];
	bool	ImgVolumePresentFlag;
	bool	BrsVolumePresentFlag;

	short	minLon;
	short	maxLon;
	short	minLat;
	short	maxLat;
} PDS_db_rec;

typedef			PDS_db_rec		*PDS_db_recPtr;
long			PDS_record_count;
InfoPtr			infoPtrUsedForPDSdbase;
InfoPtr			BrowseInfoPtr;


//*****************************************************************************
void getIndexedText(short index, char	data[], char *saveIt)
{
short	i, commaCount, quoteCount, adjustedIndex, offset;

	adjustedIndex	= index -1;
	commaCount		= 0;
	quoteCount		= 0;
	i				= 0;

	//*	VG_0004 record # 1699 in the data base file has an error
	//*	nothing I can do about it
	if (adjustedIndex > 0)
	{
		while ((i < 512) && (commaCount < adjustedIndex))
		{

			if (data[i] == '"')	quoteCount++;
			if ((data[i++] == ',') && ((quoteCount % 2) == 0))
			{
				commaCount++;
			}
		}
	}

	//*	does the data start with '"'?
	if (data[i] == '"')
	{
		i += 1;
		while ((data[i] != '"') && (data[i] != 0x0D) && (data[i] != 0x00))
		{
			*saveIt++	=	data[i++];
		}
	}
	else	//*	doesnt start with '"', so allow termination with ','
	{
		while ((data[i] != '"') && (data[i] != ',') && (data[i] != 0x0D) && (data[i] != 0x00))
		{
			*saveIt++	=	data[i++];
		}
	}
	*saveIt++	=	0;
	//*	get rid of trailing spaces
	while ((*saveIt == 0x00) || (*saveIt == 0x20))
	{
			*saveIt	=	0;
			*saveIt--;
	}
}

//*****************************************************************************
int getIndexedInt(index, data)
short	index;
char	data[];
{
char	theNumString[32];
short	returnValue;

	returnValue	= 0;
	getIndexedText(index, data, theNumString);
	sscanf(theNumString, "%d", &returnValue);
	return(returnValue);
}


//*****************************************************************************
OSErr GetVolInfo(short VolRefNum, int Index, char *VolName, HParamBlockRec *myHParam)
{
	//*	Get volume name and information block

	myHParam->volumeParam.ioCompletion	= NULL;
	myHParam->volumeParam.ioNamePtr		= (unsigned char *)VolName;
	myHParam->volumeParam.ioVRefNum		= VolRefNum;
	myHParam->volumeParam.ioVolIndex	= Index;
	return(PBHGetVInfo(myHParam,false));
}

//*****************************************************************************
short getMountedVolumeList(diskNames *volumes)
{
short			i;
short			volumeCount;
char			VolName[80];
HParamBlockRec	myHParam;

	//*	clear out the volume array
	for (i = 0; i < MaxVolCount; i++)
	{
		volumes[i].vOK		= false;
		volumes[i].vIndex	= 0;
		volumes[i].vName[0]	= 0;
	}

	VolName[0]	= 0x00;
	volumeCount	= 0;

	//*	get the list of all mounted volumes
	for (i = 1;i < MaxVolCount; i++)
	{
		if (GetVolInfo(0, i, VolName, &myHParam) == noErr)
		{
			volumes[volumeCount].vOK		= true;
			volumes[volumeCount].vIndex		= i;
			PtoCstrcpy(volumes[volumeCount].vName, VolName);
			volumeCount++;
		}
	}
	return(volumeCount);
}

//*****************************************************************************
* check mounted volumes and redo volume mounted flags in data base
//*****************************************************************************
void	PDS_checkMountedVolumes(void)
{
PDS_db_recPtr		PDSrecordPtr;
long				numberOfRecords, i, j;
textEditInfoPtr		TEDataPtr;
InfoPtr				SaveInfo;
short				volumeCount;
diskNames			volumes[MaxVolCount];

PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	//*	check to make sure we have a data base file open
	if ((infoPtrUsedForPDSdbase == nil) || (infoPtrUsedForPDSdbase->PictureType	!= DataBaseRecordType))
	{
		return;
	}
	SaveInfo 	= info;
	info		= infoPtrUsedForPDSdbase;

	TEDataPtr		=	(textEditInfoPtr)info->textEditRec;
	numberOfRecords	=	TEDataPtr->linesInFile;	//*	get the number of lines read

	ShowWatch();
	volumeCount	=	getMountedVolumeList(volumes);

	for (i=0; i<numberOfRecords; i++)
	{
		PDSrecordPtr						= &PDS_image_listPtr[i];
		PDSrecordPtr->ImgVolumePresentFlag	= false;
		PDSrecordPtr->BrsVolumePresentFlag	= false;
		for (j=0; j < volumeCount; j++)
		{
			if (strncmp(volumes[j].vName, PDSrecordPtr->IMAGE_VOLUME_ID,7) == 0)
			{
				PDSrecordPtr->ImgVolumePresentFlag	=	true;
			}
			if (strncmp(volumes[j].vName, PDSrecordPtr->BROWSE_VOLUME_ID,7) == 0)
			{
				PDSrecordPtr->BrsVolumePresentFlag	=	true;
			}
		}
	}
	dBaseWind_UpdateWindow(info->wptr);
	info	=	SaveInfo;
}

//*****************************************************************************
bool CheckForVolume(void * checkVolname)
{
long				i;
HParamBlockRec		myHParam;
char				VolName[80];
bool				returnFlag;
short				vnmLen;

	returnFlag	=	false;

	//*	check all of the mounted volumes
	for (i = 1;i < MaxVolCount; i++)
	{
		if (GetVolInfo(0, i, VolName, &myHParam) == noErr)
		{
			PtoCstr((unsigned char *)VolName);
			vnmLen	=	strlen(VolName);
			if (strncmp(VolName, checkVolname, vnmLen) == 0)
			{
				returnFlag	=	true;
				break;
			}
		}
	}
	return(returnFlag);
}


//*****************************************************************************
void PDS_closeDataBase(void)
{

	DisposPtr(info->PDS_dbasePtr);
	info->PDS_dbasePtr		= nil;
	infoPtrUsedForPDSdbase	= nil;
}



//*****************************************************************************
bool OpenPDSdatabaseFile(Str255 fname, short vnum)
{
OSErr				err;
short				fRefNum;
char				ibuf[525];
long		 		length;
long				bytecount, FileLength;
long				numberOfRecords, i, j;
PDS_db_recPtr		PDSrecordPtr;
long				TempSize;
Ptr					iptr;
textEditInfoPtr		TEDataPtr;
StScrpHandle		hStScrap;
Str255				linebuf;
long				lineCtr	=	0;
char				tempStr[100];
short				vRefNum;
long				freeBytes;
PDS_db_recPtr		PDS_image_listPtr;

	ShowWatch();

	if (vnum == 0)		//*	if vnum is 0, then this is not the 'connected' drive
	{
		for (i=1; ((i<10) && (fname[i] != ':')); i++)
		{
			gCurrentVolName[i]	=	fname[i];
		}
		gCurrentVolName[0]	=	i-1;
	}
	else
	{
		err	=	GetVInfo(vnum, (StringPtr)gCurrentVolName, &vRefNum, &freeBytes);
	}
	PDSdBASEtype	= getPDSdBASEtype(gCurrentVolName, (char *)fname);

	if (PDSdBASEtype == 0)	//*	we have a problem, we dont know what type it is
	{
		//*	try to figure it out by reading the first few bytes of the file
		//*	the offsets into the first line are hard coded and were obtained
		//*	by looking at the file manually
		err 		= FSOpen(fname, vnum, &fRefNum);
		bytecount	= 50;
		err 		= FSRead(fRefNum, &bytecount, ibuf );
		if 		(strncmp(&ibuf[1], "VOYAGER", 7) == 0)	PDSdBASEtype	= Voyager_db_ImageDBase;
		else if	(strncmp(&ibuf[21], "VIKING", 6) == 0)	PDSdBASEtype	= Viking_db_ImageDBase;

		FSClose(fRefNum);
	}

	if (PDSdBASEtype == 0)
	{
		SysBeep(1);
		return;
	}

	//*	if it is the Magellan MDIR Geometry table (GEOM), read it as text file
	if (PDSdBASEtype == Magellan_db_MDIR_Geometry)
	{
		OpenPDS_LabelFile(fname, vnum, VARIABLE_LENGTH);
		return;
	}


	err 	= FSOpen(fname, vnum, &fRefNum);
	if ( err != noErr )
	{
		return(false);
	}

	if (GetEOF(fRefNum,&FileLength) != noErr)
	{
		FSClose(fRefNum);
		return(false);
	}
	numberOfRecords	=	FileLength / PDS_Info_BytesPerRecord;
	PDS_image_listPtr	= (PDS_db_recPtr)NewPtr((numberOfRecords + 1) * sizeof(PDS_db_rec));
	PDS_record_count	= numberOfRecords;
	if (PDS_image_listPtr == nil)
	{
		PutOutOfMemMsg();
		FSClose(fRefNum);
		return(false);
	}


	//*	Create a new info record
	SaveInfo 	=	info;
	iptr 		=	NewPtr(sizeof(PicInfo));
	if ( iptr == nil )
	{
		PutOutOfMemMsg();
		DisposPtr(iptr);
		err 	= FSClose(fRefNum);
		return(false);
	}

	info 	= (InfoPtr)iptr;
	*info 	= *SaveInfo;

	PDS_readingFileType	=	PDSdBASEtype;
	setPDSinfoStuf(info, gCurrentVolName, (char *)fname);
	PDS_readingFileType	=	0;	//*	reset the flag for the next time

	infoPtrUsedForPDSdbase	= info;					//*	save which 'info' we are using

	info->PDS_dbaseType		= PDSdBASEtype;
	info->PDS_dbasePtr		= (Ptr)PDS_image_listPtr;
	info->PDS_dbaseRecCnt	= PDS_record_count;

	info->vref 			= vnum;						//*	save the Volumne Number
	info->PictureType	= DataBaseRecordType;		//*	DataBase Records
	info->LutMode 		= GrayScale;
	info->PixelsPerLine	= ScreenWidth - 10;
	info->nlines		= ScreenHeight - PicTopBase;
	info->PicSize 		= (long)info->nlines * info->PixelsPerLine;
	TempSize 			= info->PicSize;

	info->PicBaseAddr	= nil;

	//*	set up the color lookup table
	info->nColors 		= 256;
	info->ColorStart 	= 0;
	info->ColorWidth 	= 8;
	UpdateColors();


	SetDefaultColorTable();
	MakeNewWindow(fname, WDataBase);
	ResetGrayMap();
	SaveOriginalColorPalette();

	SetPort((GrafPtr)info->osPort);

	TEDataPtr	= (textEditInfoPtr)info->textEditRec;

	strcpy((Ptr)linebuf, (Ptr)"\pReading PDS database file: ");
	PtoPstrcat(linebuf, fname);

	sprintf (tempStr, "Reading %ld database records", numberOfRecords);
	CtoPstr(tempStr);
	DisplayReadStatus((Ptr)linebuf, (Ptr)tempStr);	//*	display status dialog box (themometer bar )

	for (i =0; ((i<numberOfRecords) && (err == noErr) && !CommandPeriod()); i++)
	{
		if (((i+1) % 40) == 0)
		{
			UpdateReadStatus((i * 100L) / numberOfRecords);
		}
		bytecount	=	PDS_Info_BytesPerRecord;
		err			=	FSRead(fRefNum, &bytecount, ibuf );
		//*	get ptr to current record
		PDSrecordPtr	=	&PDS_image_listPtr[i];

		memset(PDSrecordPtr, 0, sizeof(PDS_db_rec));	//*	zero out the record

		switch (PDSdBASEtype)
		{
			case Voyager_db_ImageDBase:
				getIndexedText(SPACECRAFT_NAME_index, 	ibuf, 	PDSrecordPtr->SPACECRAFT_NAME);
				getIndexedText(MISSION_PHASE_NAME_index, ibuf,	PDSrecordPtr->MISSION_PHASE_NAME);
				getIndexedText(TARGET_NAME_index, 		ibuf,	PDSrecordPtr->TARGET_NAME);
				getIndexedText(IMAGE_NUMBER_index, 		ibuf,	PDSrecordPtr->IMAGE_NUMBER);
				getIndexedText(NOTE_index, 				ibuf,	PDSrecordPtr->NOTE);
				getIndexedText(IMAGE_VOLUME_ID_index,	ibuf,	PDSrecordPtr->IMAGE_VOLUME_ID);
				getIndexedText(IMAGE_FILE_NAME_index,	ibuf,	PDSrecordPtr->IMAGE_FILE_NAME);
				getIndexedText(BROWSE_VOLUME_ID_index,	ibuf,	PDSrecordPtr->BROWSE_VOLUME_ID);
				getIndexedText(BROWSE_FILE_NAME_index,	ibuf,	PDSrecordPtr->BROWSE_FILE_NAME);
				break;

			case Viking_db_ImageDBase:
				getIndexedText(3, 		ibuf, 	PDSrecordPtr->SPACECRAFT_NAME);
				getIndexedText(4, 		ibuf,	PDSrecordPtr->MISSION_PHASE_NAME);
				getIndexedText(5, 		ibuf,	PDSrecordPtr->TARGET_NAME);
				getIndexedText(2, 		ibuf,	PDSrecordPtr->IMAGE_NUMBER);
				getIndexedText(15, 		ibuf,	PDSrecordPtr->NOTE);
				getIndexedText(16,		ibuf,	PDSrecordPtr->IMAGE_VOLUME_ID);
				getIndexedText(17,		ibuf,	PDSrecordPtr->IMAGE_FILE_NAME);
				getIndexedText(18,		ibuf,	PDSrecordPtr->BROWSE_VOLUME_ID);
				getIndexedText(19,		ibuf,	PDSrecordPtr->BROWSE_FILE_NAME);
				break;

			case Magellan_db_Venus_Features:			//*	GEO.TAB
				getIndexedText(5, 		ibuf,	PDSrecordPtr->TARGET_NAME);
				getIndexedText(6, 		ibuf,	PDSrecordPtr->NOTE);
				PDSrecordPtr->minLat	=		getIndexedInt(1, ibuf);
				PDSrecordPtr->maxLat	=		getIndexedInt(2, ibuf);
				PDSrecordPtr->minLon	=		getIndexedInt(3, ibuf);
				PDSrecordPtr->maxLon	=		getIndexedInt(4, ibuf);
				break;

			case Magellan_db_CD_Contents:				//*	CONTENTS.TAB

				break;

		 	case Magellan_db_MDIR_Products_list:		//*	MCUMDIR.TAB
				getIndexedText(3, 		ibuf,	PDSrecordPtr->TARGET_NAME);
				getIndexedText(1,		ibuf,	PDSrecordPtr->IMAGE_VOLUME_ID);
				getIndexedText(2,		ibuf,	PDSrecordPtr->IMAGE_FILE_NAME);
				break;

			case Magellan_db_FrameLatLong_List:		//*	FRAME.TAB
				getIndexedText(7, 		ibuf,	PDSrecordPtr->TARGET_NAME);
				getIndexedText(5,		ibuf,	PDSrecordPtr->IMAGE_VOLUME_ID);
				getIndexedText(6,		ibuf,	PDSrecordPtr->IMAGE_FILE_NAME);
				strncpy(PDSrecordPtr->NOTE, ibuf, 35);	//*	get the lat/long values
				break;

		}
	}
	CloseReadStatus();

	TEDataPtr->linesInFile	= i;			//*	save the number of lines read
	PDS_dbaseNoSelection();					//*	make sure none of them are selected
	dBaseWind_SetVScroll();
	dBaseWind_SetView(info->wptr, false);
	err 	= FSClose(fRefNum);
	PDS_checkMountedVolumes();

	return(true);
}


//*****************************************************************************
void PDS_dbaseNoSelection()
{
PDS_db_recPtr		PDSrecordPtr;
textEditInfoPtr		TEDataPtr;
short				i;
PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;
	TEDataPtr			= (textEditInfoPtr)info->textEditRec;

	for (i=0; i< TEDataPtr->linesInFile; i++)
	{
		//*	get ptr to current record
		PDSrecordPtr	=	&PDS_image_listPtr[i];

		PDSrecordPtr->selected	=	false;
	}
}

//*****************************************************************************
void setPDS_dbaseEntrySelected(long	lineIndex, bool selectedValue)
{
PDS_db_recPtr		PDSrecordPtr;
long				mylineIndex;
PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	//*	Bullet proffing, check for range of value
	mylineIndex	=	lineIndex;
	if (mylineIndex < 0)
	{
		mylineIndex	=	0;
	}
	else if (mylineIndex >= PDS_record_count)
	{
		mylineIndex	=	PDS_record_count-1;
	}
	//*	get ptr to current record
	PDSrecordPtr	=	&PDS_image_listPtr[mylineIndex];

	PDSrecordPtr->selected	=	selectedValue;
}

//*****************************************************************************
//* get a single line of data from the index
//*****************************************************************************
void getPDS_dbaseData(long	lineIndex, char	*lineBuff, bool	*selectedFlag, bool	*volPresentFlag)
{
PDS_db_recPtr		PDSrecordPtr;
PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	//*	get ptr to current record
	PDSrecordPtr	=	&PDS_image_listPtr[lineIndex];

	*selectedFlag	= PDSrecordPtr->selected;
	*volPresentFlag	= PDSrecordPtr->ImgVolumePresentFlag;

	switch(info->PDS_dbaseType)
	{
		case Voyager_db_ImageDBase:
		case Viking_db_ImageDBase:
			sprintf(lineBuff, "%4ld-%s: %s - %s - %s - %s\r",(lineIndex +1),
							PDSrecordPtr->IMAGE_VOLUME_ID,
							PDSrecordPtr->SPACECRAFT_NAME,
						//*	PDSrecordPtr->MISSION_PHASE_NAME,
							PDSrecordPtr->IMAGE_NUMBER,
							PDSrecordPtr->TARGET_NAME,
							PDSrecordPtr->NOTE
							);
			break;

		case Magellan_db_MDIR_Geometry:		//*	GEOM.TAB;1		Geometry file for one MIDR
			sprintf(lineBuff, "%4ld-Not finished\r",(lineIndex +1));
			break;

		case Magellan_db_Venus_Features:		//*	GEO.TAB;1		Venus Features
			sprintf(lineBuff, "%4ld-%-8s == %4d -> %4d : %4d ->%4d: %s \r",(lineIndex +1),
							PDSrecordPtr->TARGET_NAME,
							PDSrecordPtr->minLat,
							PDSrecordPtr->maxLat,
							PDSrecordPtr->minLon,
							PDSrecordPtr->maxLon,
							PDSrecordPtr->NOTE
							);
			*volPresentFlag	= true;
			break;

		case Magellan_db_CD_Contents:			//*	CONTENTS.TAB;1
			sprintf(lineBuff, "%4ld-Not finished\r",(lineIndex +1));
			break;

		case Magellan_db_MDIR_Products_list:	//*	MCUMDIR.TAB;1
			sprintf(lineBuff, "%4ld-%s: %s - %s\r",(lineIndex +1),
							PDSrecordPtr->IMAGE_VOLUME_ID,
							PDSrecordPtr->TARGET_NAME,
							PDSrecordPtr->IMAGE_FILE_NAME
							);
			*volPresentFlag	= true;
			break;

		case Magellan_db_FrameLatLong_List:	//*	FRAME.TAB;1
			sprintf(lineBuff, "%4ld-%s: %s - %s - %s - %s\r",(lineIndex +1),
							PDSrecordPtr->IMAGE_VOLUME_ID,
							PDSrecordPtr->SPACECRAFT_NAME,
							PDSrecordPtr->IMAGE_NUMBER,
							PDSrecordPtr->TARGET_NAME,
							PDSrecordPtr->NOTE
							);
			*volPresentFlag	= true;
			break;

		default:
			sprintf(lineBuff, "%4ld-Internal Error\r",(lineIndex +1));
			break;

	}
}

//*****************************************************************************
void VenusFeaturesDoubleClick(long lineIndex)
{
PDS_db_recPtr		PDSrecordPtr;
Rect				featureRect;
PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	//*	get ptr to current record
	PDSrecordPtr	=	&PDS_image_listPtr[lineIndex];

	featureRect.left	=	PDSrecordPtr->minLon;
	featureRect.right	=	PDSrecordPtr->maxLon;
	featureRect.top		=	PDSrecordPtr->maxLat;
	featureRect.bottom	=	PDSrecordPtr->minLat;

	MercatorFrameRect(&featureRect);

	//*						PDSrecordPtr->TARGET_NAME,
	//*						PDSrecordPtr->NOTE

}

//*****************************************************************************
PDS_FixFileName(char *filenameString, Str255 nameBuff)
{
short			i,j;
bool			inDirNameFlg;

	//**************************************************************
	//*	the file names look like this
	//*	"VG_0004 :[HELENE]C3401041.IMQ           "
	//*	"VG_0004 :[SATURN.C3470XXX]C3470640.IMQ  "
	//*	change the string to conform to Mac conventions
	//**************************************************************
	i	=	0;
	j	=	0;
	inDirNameFlg	=	false;
	while (i < strlen(filenameString))
	{
		if (filenameString[i] == '[')			//*	ignore "["
		{
			inDirNameFlg	=	true;
		}
		else if (filenameString[i] == ']')		//*	change "]" to ':'
		{
			nameBuff[j++]	=	':';
			inDirNameFlg	=	false;
		}
		else if ((filenameString[i] == '.') && (inDirNameFlg == true))
		{
			nameBuff[j++]	=	':';		//*	change '.' to ':'
			//*	but only when in the directory section of the name
		}
		else if (filenameString[i] != ' ')
		{
			nameBuff[j++]	=	filenameString[i];	//*	copy others
		}
		i++;
	}
	if (gUseSemicolonOne)
	{
		nameBuff[j++]	=	';';		//*	adde ";1"
		nameBuff[j++]	=	'1';
	}
	nameBuff[j++]	=	0x00;
	CtoPstr((Ptr)nameBuff);

}



//*****************************************************************************
//*	If 'info' is also the Browse info ptr, set it to nil
//*	this gets called from the close window code
//*****************************************************************************
PDS_checkForBrowseWindow()
{
	if (info == BrowseInfoPtr)
	{
		BrowseInfoPtr	=	nil;
	}
}


//*****************************************************************************
PDS_VolumeNotMounted(void *volName)
{
short	itemHit;
	itemHit	= doSimpleDialog(PDS_volumeDialog, PDS_volumeDialogLAST);
	if (itemHit == upDateVolumes)
	{
		PDS_checkMountedVolumes();
	}
}

//*****************************************************************************
bool	OpenIndexedVoyagerPDSfile(long lineIndex, short whichFile)
{
bool			returnFlag, okFlag;
PDS_db_recPtr	PDSrecordPtr;
char			tempbuf[256];
Str255			nameBuff;
InfoPtr			SaveInfo;
OSErr			err;
long			TempSize, fileSize;
short			browseWidth,browseHeidth;
short			fRefNum;
short			theRefNum;
OSType			theExtension;
short			i, fnameLen;
PDS_db_recPtr	PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	//*	get ptr to current record
	PDSrecordPtr	=	&PDS_image_listPtr[lineIndex];

	ShowWatch();
	if (whichFile == Browse)
	{
		sprintf(tempbuf, "%s:%s",	PDSrecordPtr->BROWSE_VOLUME_ID,
					PDSrecordPtr->BROWSE_FILE_NAME );
		PDS_FixFileName(tempbuf, nameBuff);

		//*	if the volume present flag is false, check again anyway
		if (PDSrecordPtr->BrsVolumePresentFlag == false)
		{
			PDSrecordPtr->BrsVolumePresentFlag	=	CheckForVolume(PDSrecordPtr->BROWSE_VOLUME_ID);
		}

		if (PDSrecordPtr->BrsVolumePresentFlag == false)
		{
			PtoCstr(nameBuff);
			sprintf(tempbuf, "Sorry, file '%s' could not be opened.  The volume is not mounted", nameBuff);
			PutMessage(CtoPstr(tempbuf));
			return(false);
		}

		SaveInfo	= info;

		//*	open the file first
		//*	we have to figure out what size it is so we can figure out what
		//*	size the image is
		err		= FSOpen(nameBuff, theRefNum, &fRefNum);
		if (err != noErr)
		{
			return(false);
		}
		err		= GetEOF(fRefNum, &fileSize);

		if (fileSize == 82500L)
		{
			browseWidth		= 300;
			browseHeidth	= 264;
			info->ImageDataOffset	= 3300;
		}
		else
		{
			browseWidth		= 200;
			browseHeidth	= 200;
			info->ImageDataOffset	= 3200;
		}

		if (BrowseInfoPtr == nil)
		{
			okFlag	=	NewPicWindow("\pBrowse Window", browseWidth, browseHeidth);
			if ( okFlag == false )
			{
				info	= SaveInfo;
				return(false);
			}
			info->LutMode 	= GrayScale;
			ResetGrayMap();
			SaveOriginalColorPalette();

			BrowseInfoPtr	=	info;
		}
		info		= BrowseInfoPtr;

		TempSize	= 1L * browseWidth * browseHeidth;
		err			= SetFPos(fRefNum, fsFromStart, info->ImageDataOffset);
		err 		= FSRead(fRefNum, &TempSize, info->PicBaseAddr);
		if (err != noErr)
		{
			SysBeep(1);
		}
		err 		= FSClose(fRefNum);



		strncpy(gCurrentVolName, PDSrecordPtr->BROWSE_VOLUME_ID, 8);
		gCurrentVolName[7]	=	0;
		CtoPstr(gCurrentVolName);

		PDS_readingFileType	=	VoyagerBrowse;
		setPDSinfoStuf(info, gCurrentVolName, (char *)nameBuff);
		PDS_readingFileType	=	0;	//*	reset the flag for the next time

		if (info->PDS_volPrefix == 'VG_0')
		{
			InvertPic();
		}
		UpdatePicWindow();
		info		=	SaveInfo;
		returnFlag	=	true;
	}
	else
	{
		sprintf(tempbuf, "%s:%s",	PDSrecordPtr->IMAGE_VOLUME_ID,
					PDSrecordPtr->IMAGE_FILE_NAME );

		PDS_FixFileName(tempbuf, nameBuff);

		//*	is that volume on line
		if (PDSrecordPtr->ImgVolumePresentFlag == true)
		{
			returnFlag	=	OpenPDS_LabelFile(nameBuff, 0, VARIABLE_LENGTH);
			if (returnFlag == true)
			{
				//*	Magellan CD-ROM has seperate label and image files
				theExtension	=	getFnameExtension((char *)nameBuff);	//*	find the extension of the file name

				if (theExtension == '.IMQ')	//*	read normal IMQ file
				{
					returnFlag	=	OpenPDS_Labeled_Image(nameBuff, 0, VARIABLE_LENGTH);
				}
				else if (theExtension == '.LBL')	//*	replace ".LBL" with ".IMG"
				{
					fnameLen	=	nameBuff[0];			//*	get the length of the file name
					for (i = fnameLen; i > 0; i--)
					{
						if (nameBuff[i] == '.') break;	//*	find the "."
					}
					nameBuff[i+1]	=	'I';				//*	replace the extension
					nameBuff[i+2]	=	'M';
					nameBuff[i+3]	=	'G';

					WhatToOpen				= OpenCustom;
					ImportCustomWidth		= 1024;
					ImportCustomHeight		= 1024;
					ImportCustomOffset		= 0;
					returnFlag	=	OpenFile(nameBuff, 0);
					strncpy(gCurrentVolName, PDSrecordPtr->IMAGE_VOLUME_ID, 8);
					gCurrentVolName[7]	=	0;
					CtoPstr(gCurrentVolName);

					setPDSinfoStuf(info, gCurrentVolName, (char *)nameBuff);
					PDS_readingFileType	=	0;	//*	reset the flag for the next time

				}
				else	//*	for now, leave this the default, we may have to change it later
				{
					returnFlag	=	OpenPDS_Labeled_Image(nameBuff, 0, VARIABLE_LENGTH);
				}
			}
			else
			{
				PtoCstr(nameBuff);
				sprintf(tempbuf, "Sorry, file '%s' could not be opened.  The volume is probably not mounted", nameBuff);
				PutMessage(CtoPstr(tempbuf));
			}
		}
		else
		{
			PDS_VolumeNotMounted(PDSrecordPtr->IMAGE_VOLUME_ID);
			returnFlag	=	false;
		}
	}
	return(returnFlag);
}

//*****************************************************************************
void PDS_OpenBrowse()
{
long				lineNumOnScreen, lineIndex;
textEditInfoPtr		TEDataPtr;
PDS_db_recPtr		PDSrecordPtr;
PDS_db_recPtr		PDS_image_listPtr;

	PDS_image_listPtr	= (PDS_db_recPtr)info->PDS_dbasePtr;

	TEDataPtr	= (textEditInfoPtr)info->textEditRec;
	if (TEDataPtr != nil)
	{
		lineIndex	= TEDataPtr->selectedIndex;

		//*	Bullet proffing, check for range of value
		if (lineIndex < 0)
		{
			lineIndex	=	0;
		}
		else if (lineIndex >= PDS_record_count)
		{
			lineIndex	=	PDS_record_count-1;
		}

		//*	get ptr to current record
		PDSrecordPtr	=	&PDS_image_listPtr[lineIndex];

		if (PDSrecordPtr->BrsVolumePresentFlag == false)
		{
			PDS_VolumeNotMounted(PDSrecordPtr->BROWSE_VOLUME_ID);
		}

		if (PDSrecordPtr->BrsVolumePresentFlag == true)
		{
			OpenIndexedVoyagerPDSfile(lineIndex, Browse);
		}
	}
	else
	{
		SysBeep(1);
	}
}


//*****************************************************************************
//*	args are pascal strings
void setPDSinfoStuf(InfoPtr theInfoPtr, char *volname, char *fname)
{

	theInfoPtr->fnameExtension	=	getFnameExtension((char *)fname);
	strncpy(theInfoPtr->PDS_volName, volname,8);
	theInfoPtr->PDS_volName[0]	=	7;	//*	force Pascal string notation
	theInfoPtr->PDS_volName[8]	=	0;
	theInfoPtr->PDS_volPrefix	=	(volname[1] << 8)
								+	(volname[2]);
	theInfoPtr->PDS_volPrefix	=	(theInfoPtr->PDS_volPrefix   << 16)
								+	(volname[3] << 8)
								+ 	(volname[4]);
	theInfoPtr->PDS_imageType	=	PDS_readingFileType;
	theInfoPtr->PDS_dbaseType	=	0;
	theInfoPtr->PDS_dbasePtr	=	0;
	theInfoPtr->PDS_dbaseRecCnt	=	0;

	//*	debuging code
	if (theInfoPtr->PDS_imageType == 0)
	{
		SysBeep(1);
	}
}

//*****************************************************************************
void clrPDSinfoStuf(InfoPtr theInfoPtr)
{
	theInfoPtr->fnameExtension	=	0;
	theInfoPtr->PDS_volName[0]	=	0;
	theInfoPtr->PDS_volPrefix	=	0;
	theInfoPtr->PDS_imageType	=	0;
	theInfoPtr->PDS_dbaseType	=	0;
	theInfoPtr->PDS_dbasePtr	=	0;
	theInfoPtr->PDS_dbaseRecCnt	=	0;
	theInfoPtr->imageMinLat		=	0;
	theInfoPtr->imageMaxLat		=	0;
	theInfoPtr->imageMinLon		=	0;
	theInfoPtr->imageMaxLon		=	0;
}

//*****************************************************************************
bool	PDS_is_this_an_Image(InfoPtr theInfoPtr)
{
bool	returnFlag;
	returnFlag	=	true;
	if (	(theInfoPtr->PDS_imageType	==	PDSLabelText) ||
	 		(theInfoPtr->PDS_imageType	>=	Voyager_db_ImageDBase))
	 {
	 	returnFlag	=	false;
	}
	return(returnFlag);
}

//***************************************************************
void	doPDS_DbaseMenu(int MenuItem)
{
Str255		fileName;
short		volumeCount, magellanVolCnt;
short		i, j;
diskNames	volumes[MaxVolCount];

	switch ( MenuItem )
	{
		case PDS_dbOpenVoyagerViking:
			break;

		case PDS_dbOpenMagellanFeatureList:
			//***************************************************************
			//*	find the first mounted Magellan CD-ROM
			volumeCount			=	getMountedVolumeList(volumes);
			magellanVolCnt		=	0;
			for (j=0; j < volumeCount; j++)
			{
				if (strncmp(volumes[j].vName, "MG_00",5) == 0)
				{
					strcpy((char *)fileName, volumes[j].vName);
					magellanVolCnt++;	//*	increment the number of magellan CDs we have
				}
			}
			if (magellanVolCnt > 0)
			{
				strcat((char *)fileName, ":GEO.TAB");
				CtoPstr((Ptr)fileName);
				OpenPDSdatabaseFile(fileName, 0);
			}
			else
			{
				SysBeep(1);
			}
			break;
	}
}

//***************************************************************
UpdatePDSdbaseMenu()
{
char		volName[32];
short		volumeCount, VG_VolCnt, VO_VolCnt, MG_VolCnt;
short		volType;
short		j;
diskNames	volumes[MaxVolCount];

	volumeCount		=	getMountedVolumeList(volumes);
	VG_VolCnt		=	0;
	VO_VolCnt		=	0;
	MG_VolCnt		=	0;

	for (j=0; j < volumeCount; j++)
	{
		CtoPstrcpy(volName, volumes[j].vName);
		volType	=	PDS_GetCDROMtype(volName);
		switch(volType)
		{
			case VOYAGER:	VG_VolCnt++;	break;
			case VIKING_1:	VO_VolCnt++;	break;
			case VIKING_2:	VO_VolCnt++;	break;
			case MAGELLAN:	MG_VolCnt++;	break;
		}
	}
	SetMenuItem(OpenPDSdbaseMenuH, PDS_dbOpenVoyagerViking,			(VG_VolCnt+VO_VolCnt) > 0);
	SetMenuItem(OpenPDSdbaseMenuH, PDS_dbOpenMagellanFeatureList,	MG_VolCnt > 0);
}
#endif // __MAC__
