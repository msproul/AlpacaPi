//*****************************************************************************
//#include	"PDS.typedefs.h"

#ifndef _PDS_TYPEDEFS_H_
#define _PDS_TYPEDEFS_H_

#ifdef __cplusplus
	extern "C" {
#endif


#define		MaxVolCount	16

//***************************************
//* Volume prefixes for CD-ROM series
//*		Voyager		= VG_0001	->	VG_0012
//*		Viking		= VO_1001	->	VO_1008
//*		Viking		= VO_2001	->	VO_2007
//*		Magellan	= MG_0001	->	MG_0067
//*		Magellan	= MG_2001	->	MG_2015
//*		Magellan	= MG_3001

//***************************************
enum
{
		VOYAGER	=	1,
		VIKING_1,
		VIKING_2,
//		MAGELLAN,
		MAGELLAN_0,
		MAGELLAN_1,
		MAGELLAN_2,
		MAGELLAN_3,
		GALILEO,
		JEDI
};

//**************************************************
//*	types of PDS labels
enum
{
		LABEL_UNKNOWN	=	0,
		VARIABLE_LENGTH,
		FIXED_LENGTH,
		STREAM
	};

//**************************************************
typedef struct
{
	bool	vOK;
	char	vName[36];
	int		vIndex;
	int		vRefNum;
} diskNames;

//**************************************************
typedef struct
	{
		bool	vOnLine;
		char	vName[8];
		char	dirName[10];
//+		Rect	MgRect;
	} MagellanRectangles;

//**************************************************
enum
{
	Browse = 0,
	Image
};


//************************************************************
// PDS file types
enum
{
//*	Non image types
		notImage	=	0,
		PDSLabelText,				//*	label or text files

//*	Image types
		VoyagerBrowse,				//* 200 x 200, 	3200 byte offset, non compressed
		VoyagerFull,				//* 800 x 800 	compressed, header and image
		VikingBrowse,				//* 300 x 264, 	3300 byte offset, non compressed
		VikingFull,					//* ? x ? 		compressed, header and image
		MagellanVenusMercator,		//*
		MagellanBrowse,				//* 1024 x 896	non compressed
		MagellanFull,				//* 1024 x 1024	non compressed
		JEDIHaleyComet,				//*	256 x 256	may vary


//*	DataBase types

		Voyager_db_ImageDBase,			//* INDEX:IMGINDEX.TAB;1
		Viking_db_ImageDBase,			//*	INDEX:IMGINDEX.TAB;1
		Magellan_db_MDIR_Geometry,		//* GEOM.TAB;1		Geometry file for one MIDR (useless)
		Magellan_db_Venus_Features,		//* GEO.TAB;1		Venus Features
		Magellan_db_CD_Contents,		//* CONTENTS.TAB;1	contents of current CD only
		Magellan_db_MDIR_Products_list,	//* MCUMDIR.TAB;1	cumulative directory
		Magellan_db_FrameLatLong_List,	//* FRAME.TAB;1		frame lat long
		dbase_last

	};

#define		kHistogramEntries		256
#define		HISTOGRAM_SIZE			511

//*****************************************************************************
typedef struct
{
	char	headerLine[88];
} TYPE_HeaderTxt;

#define	kMaxHeaderLineCnt	200

//*****************************************************************************
typedef struct
{
	int			cdROMtype;
	char		volumeName[64];			//*	Name of CD-Rom
	char		imageFileName[512];		//*	file name if the image is a separate file
	int			record_Type;			//*	type of record	: Variable, ?
	int			record_Bytes;			//*	number of bytes per record (after un-compressing)
	int			format;					//*	number of bits per pixel
	int			file_records;			//*	total number of records in file
	int			label_records;			//*	number of records in label (header)
	char		spaceCraft_name[64];	//*	name of space craft
	//*	image information
	int			scanLines;				//*	number of scan lines in image
	int			lineSamples;			//*	number of samples per line
	int			imageLocationFlag;		//*	location of image 0 = means this file, 1 = other file
	int			imageOffset;			//*	offset record for image
	int			histogramOffset;		//*	offset record for histogram
	uint32_t	imgHistogram[kHistogramEntries];

	uint8_t		*imageData;

	size_t		labelSize;
	int			lineSuffixBytes;
	int			linePrefixBytes;
	size_t		headerBytesRead;
	char		targetBody[64];

	//*	compression information
	bool		imageIsCompressed;
	int			encodeHistOffset;
	uint32_t	encodingHistogram[HISTOGRAM_SIZE + 100];

	TYPE_HeaderTxt	HeaderData[kMaxHeaderLineCnt];
	int				HeaderLineCnt;

} PDS_header_data;

void	PDS_DumpHeader(PDS_header_data *pdsHeaderPtr);

//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage] OBJECT                           = ENCODING_HISTOGRAM
//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage]  ITEMS                           = 511
//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage]  ITEM_TYPE                       = VAX_INTEGER
//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage]  ITEM_BITS                       = 32
//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage] END_OBJECT

#ifdef __cplusplus
}
#endif

#endif // _PDS_TYPEDEFS_H_
