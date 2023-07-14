//**************************************************************************************
//#include	"VideoCamInfo.h"

#ifndef _VIDEOCAMINFO_H_
#define _VIDEOCAMINFO_H_

//**************************************************************************************
typedef struct	//	TYPE_CAMERA_INFO
{
	bool	onLine;
	int		userNum;
	int		btnGrp1;
	int		btnGrp2;
	int		btnGrp3;
	int		readOutMode;
	bool	colorSupported;
	int		cameraxsize;
	int		cameraysize;
	int		videoframes;
	int		remainingseconds;
	char	cameraStateStr[32];
	char	refID[128];
	double	gigabytesFree;
	double	cpuTemp_DegF;

} TYPE_CAMERA_INFO;

extern TYPE_CAMERA_INFO	gCameraAuxData[];

#endif
