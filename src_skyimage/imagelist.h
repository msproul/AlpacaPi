//**************************************************************************************
enum
{
	kImageFileType_unknown	=	0,
	kImageFileType_FITS,
	kImageFileType_PDS
};

//**************************************************************************************
typedef struct
{
	bool	validEntry;
	bool	lineSelected;
	char	DirectoryPath[512];
	char	FileName[256];
	char	FilePath[512];

	int		ImageFileType;
	//*	info from FITS header
	bool	FitsProcessed;
	double	Exposure_secs;
	double	SaturationPercent;
	int		DATAMIN;
	int		DATAMAX;

	//*	image alignment data
	int		ImageOffsetX;
	int		ImageOffsetY;
} TYPE_ImageFile;

extern	TYPE_ImageFile	*gImageList;
extern	int				gImageCount;
extern	char			gDirectoryPath[];
