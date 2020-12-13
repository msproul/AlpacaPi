//**************************************************************************
//*	Name:			cameradriver_jpeg.cpp
//*
//*	Author:			Mark Sproul (C) 2020
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
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan 29,	2020	<MLS> Started on cameradriver_jpeg.cpp
//*	Jan 29,	2020	<MLS> Can save jpegs using libjpeg instead of opencv
//*	Jan 29,	2020	<MLS> Successfully saving jpegs on NVidia/jetson
//*****************************************************************************


#ifdef _ENABLE_JPEGLIB_

#include	<stdio.h>
#include	<string.h>


#include	<jpeglib.h>
#include	<jerror.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cameradriver.h"


//**************************************************************************************
void	CameraDriver::SaveUsingJpegLib(void)
{
struct jpeg_compress_struct	jinfo;
struct jpeg_error_mgr		jerr;
FILE						*outputFile;
JSAMPROW					row_pointer[1];
int							row_stride;
char						imageFileName[64];
char						imageFilePath[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(imageFileName, cFileNameRoot);
	strcat(imageFileName, ".jpg");

	strcpy(imageFilePath, kImageDataDir);
	strcat(imageFilePath, "/");
	strcat(imageFilePath, imageFileName);

	jinfo.err	=	jpeg_std_error(&jerr);

	jpeg_create_compress(&jinfo);

	outputFile	=	fopen(imageFilePath, "wb");
	if (outputFile != NULL)
	{
		jpeg_stdio_dest(&jinfo, outputFile);

		jinfo.image_width		=	cCameraXsize;
		jinfo.image_height		=	cCameraYsize;
		jinfo.input_components	=	3;
		jinfo.in_color_space	=	JCS_RGB;

		jpeg_set_defaults(&jinfo);
		jpeg_set_quality(&jinfo, 95, TRUE);

		jpeg_start_compress(&jinfo, TRUE);

		row_stride				=	cCameraXsize * 3;

		while (jinfo.next_scanline < jinfo.image_height)
		{
			row_pointer[0]	=	&cCameraDataBuffer[jinfo.next_scanline * row_stride];
			jpeg_write_scanlines(&jinfo, row_pointer, 1);

		}
		jpeg_finish_compress(&jinfo);

		fclose(outputFile);

		AddToDataProductsList(imageFileName, "JPEG image-jpeglib");

	}
	else
	{
		CONSOLE_DEBUG("Failed to create file");
	}
}

#endif	//	_ENABLE_JPEGLIB_



