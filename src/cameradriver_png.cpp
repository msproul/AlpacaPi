//**************************************************************************
//*	Name:			cameradriver_png.cpp
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
//*	References:		https://fossies.org/linux/libpng/example.c
//*					http://zarb.org/~gc/html/libpng.html
//*					http://www.labbookpages.co.uk/software/imgProc/libPNG.html
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr  3,	2020	<MLS> Created cameradriver_png.cpp
//*****************************************************************************
//*	Jan 31,	2120	<TODO> Add support for libpng
//*****************************************************************************

//#define	_ENABLE_PNGLIB_

#ifdef _ENABLE_PNGLIB_

#include	<stdio.h>
#include	<string.h>


#include	<png.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cameradriver.h"

//**************************************************************************************
void abort_(const char *errorString)
{
	fprintf(stderr, "%s\n", errorString);
	fflush(stderr);
	CONSOLE_ABORT(__FUNCTION__);
}


//**************************************************************************************
void	CameraDriver::SaveUsingPNGlib(void)
{
char			imageFileName[64];
char			imageFilePath[128];
FILE			*outputFileP;
png_structp		png_ptr;
png_infop		info_ptr;
png_colorp		palette;
png_byte		color_type;
png_byte		bit_depth;
png_bytep		*row_pointers;
int				number_of_passes;
int				yyy;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(imageFileName, cFileNameRoot);
	strcat(imageFileName, ".png");

	strcpy(imageFilePath, gImageDataDir);
	strcat(imageFilePath, "/");
	strcat(imageFilePath, imageFileName);


	outputFileP	=	fopen(imageFilePath, "wb");
	if (outputFileP != NULL)
	{
		//*	code from http://zarb.org/~gc/html/libpng.html
		//*	initialize stuff
		png_ptr	=	png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

		if (png_ptr != NULL)
		{
			info_ptr	=	png_create_info_struct(png_ptr);
			if (!info_ptr)
					abort_("[write_png_file] png_create_info_struct failed");

			if (setjmp(png_jmpbuf(png_ptr)))
					abort_("[write_png_file] Error during init_io");

			png_init_io(png_ptr, outputFileP);


			//*	write header
			if (setjmp(png_jmpbuf(png_ptr)))
					abort_("[write_png_file] Error during writing header");

			png_set_IHDR(	png_ptr,
							info_ptr,
							cCameraXsize,
							cCameraYsize,
							bit_depth,
							color_type,
							PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_BASE,
							PNG_FILTER_TYPE_BASE);

			png_write_info(png_ptr, info_ptr);


			//*	write bytes
			if (setjmp(png_jmpbuf(png_ptr)))
					abort_("[write_png_file] Error during writing bytes");

			png_write_image(png_ptr, row_pointers);


			//*	end write
			if (setjmp(png_jmpbuf(png_ptr)))
					abort_("[write_png_file] Error during end of write");

			png_write_end(png_ptr, NULL);

			//*	cleanup heap allocation
			for (yyy=0; yyy<cCameraYsize; yyy++)
			{
				free(row_pointers[yyy]);
			}
			free(row_pointers);

		}
		else
		{
			abort_("[write_png_file] png_create_write_struct failed");
		}
//-------------------------------------------------------------
		fclose(outputFileP);

		AddToDataProductsList(imageFileName, "PNG image-libpng");

	}
	else
	{
		CONSOLE_DEBUG("Failed to create file");
	}
}

#endif	//	_ENABLE_PNGLIB_




