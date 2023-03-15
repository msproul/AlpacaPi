//*****************************************************************************
//#include	"windowtab_imageinfo.h"




#ifndef	_WINDOWTAB_IMAGEINFO_H_
#define	_WINDOWTAB_IMAGEINFO_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif


#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_


//*****************************************************************************
enum
{
	kImageInfo_Title	=	0,

	kImageInfo_Description,

	kImageInfo_AlpacaBinaryTitle,
	kImageInfo_MetadataVersion,			// Bytes 0..3 - Metadata version = 1
	kImageInfo_MetadataVersionVal,

	kImageInfo_ErrorNumber,				// Bytes 4..7 - Alpaca error number or zero for success
	kImageInfo_ErrorNumberVal,

	kImageInfo_ClientTransactionID,		// Bytes 8..11 - Client's transaction ID
	kImageInfo_ClientTransactionIDVal,

	kImageInfo_ServerTransactionID,		// Bytes 12..15 - Device's transaction ID
	kImageInfo_ServerTransactionIDVal,

	kImageInfo_DataStart,				// Bytes 16..19 - Offset of the start of the data bytes = 36 for version 1
	kImageInfo_DataStartVal,

	kImageInfo_ImageElementType,		// Bytes 20..23 - Element type of the source image array
	kImageInfo_ImageElementTypeVal,

	kImageInfo_TransmissionElementType,	// Bytes 24..27 - Element type as sent over the network
	kImageInfo_TransmissionElementTypeVal,

	kImageInfo_Rank,					// Bytes 28..31 - Image array rank
	kImageInfo_RankVal,

	kImageInfo_Dimension1,				// Bytes 32..35 - Length of image array first dimension
	kImageInfo_Dimension1Val,

	kImageInfo_Dimension2,				// Bytes 36..39 - Length of image array second dimension
	kImageInfo_Dimension2Val,

	kImageInfo_Dimension3,				// Bytes 40..43 - Length of image array third dimension (0 for 2D array)
	kImageInfo_Dimension3Val,

	kImageInfo_BinaryOutline,

	//---------------------------
	kImageInfo_DownLoadTitle,

	kImageInfo_DownLoadMBytes,
	kImageInfo_DownLoadMBytesVal,
	kImageInfo_DownLoadSeconds,
	kImageInfo_DownLoadSecondsVal,
	kImageInfo_DownLoadSpeed,
	kImageInfo_DownLoadSpeedVal,

	kImageInfo_DownLoadOutline,


	kImageInfo_last
};


//**************************************************************************************
class WindowTabImageInfo: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabImageInfo(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName,
									TYPE_BinaryImageHdr	*binaryImageHdr);
		virtual	~WindowTabImageInfo(void);

		virtual	void	SetupWindowControls(void);

};


#endif // _WINDOWTAB_IMAGEINFO_H_

