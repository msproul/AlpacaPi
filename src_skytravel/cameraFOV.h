//**************************************************************************************
//#include	"cameraFOV.h"

#ifndef _CAMERA_FOV_H_
#define _CAMERA_FOV_H_

#define	kMaxCamaeraFOVcnt	20

//**************************************************************************************
//*	This is for keeping track of the properties of each camera
//*	The primary use for this is drawing FOV on the screen for cameras
//**************************************************************************************
typedef struct
{
	bool					IsValid;
	bool					FOVenabled;		//*	do we display the FOV
	bool					PropertyDataValid;
	bool					HasReadAll;
	TYPE_CameraProperties	CameraProp;
	char					CameraName[64];

	//*	these are offsets from the primary scope.
	//*	only used if you have multiple scopes that are not perfectly aligned.
	//*	the values for the "PRIMARY" scope should be 0
	double					RighttAscen_Offset;		//*	units = hours (0.00 -> 23.99)
	double					Declination_Offset;		//*	units = degrees (-90.0 -> +90.0)

	double					ImgSizeX_microns;
	double					ImgSizeY_microns;

	double					PixelScale;				//*	Arc seconds / pixel

	double					FocalLen_mm;			//*	focal length in mm
	double					Aperture_mm;
	double					F_Ratio;
	double					FOV_X_arcSeconds;
	double					FOV_Y_arcSeconds;

} TYPE_CameraFOV;


#endif // _CAMERA_FOV_H_


//	Center (RA, Dec):	(273.831, 89.723)
//	Center (RA, hms):	18h 15m 19.390s
//	Center (Dec, dms):	+89° 43' 24.297"
//	Size:	1.42 x 1.07 deg
//	Radius:	0.889 deg
//	Pixel scale:	1.1 arcsec/pixel
//	Orientation:	Up is 124 degrees E of N


//	Center (RA, Dec):	(283.034, 89.711)
//	Center (RA, hms):	18h 52m 08.119s
//	Center (Dec, dms):	+89° 42' 41.187"
//	Size:	36 x 27.2 arcmin
//	Radius:	0.376 deg
//	Pixel scale:	0.464 arcsec/pixel
//	Orientation:	Up is 276 degrees E of N

