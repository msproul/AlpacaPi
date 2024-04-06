//*****************************************************************************
//#include	"NASA_moonphase.h"

#ifndef _NASA_MOONPHASE_H_
#define _NASA_MOONPHASE_H_

//*****************************************************************************
//   Date       Time    Phase    Age    Diam    Dist     RA        Dec      Slon      Slat     Elon     Elat   AxisA
//01 Jan 2024 00:00 UT  78.03  19.019  1771.3  404634  10.5867   12.7508   -55.867   -1.554   0.041   -4.685   20.699
//01 Jan 2024 01:00 UT  77.71  19.061  1771.2  404664  10.6172   12.5390   -56.372   -1.554  -0.015   -4.644   20.759
//*****************************************************************************

//*****************************************************************************
enum
{
	kMoonPhase_Date_DOM	=	0,
	kMoonPhase_Date_Month,
	kMoonPhase_Date_Year,
	kMoonPhase_Time_Time,
	kMoonPhase_UTC,
	kMoonPhase_Phase,
	kMoonPhase_Age,
	kMoonPhase_Diam,
	kMoonPhase_Dist,
	kMoonPhase_RA,
	kMoonPhase_Dec,
	kMoonPhase_Slon,
	kMoonPhase_Slat,
	kMoonPhase_Elon,
	kMoonPhase_Elat,
	kMoonPhase_AxisA

};

//*****************************************************************************
typedef struct
{
	int		Date_DOM;
	char	MonthName[8];
	int		Date_Month;
	int		Date_Year;
	int		Time_Hour;
	int		Time_Minute;
	int		Time_Second;
	double	Phase;
	char	PhaseName[16];
	double	Age;
	double	Diam;
	int		Dist;
	double	RA;
	double	Dec;
	double	SLon;
	double	SLat;
	double	ELon;
	double	ELat;
	double	AxisA;

	//*	these are for determining phases
//	bool	IsLowerThanPrev;
//	bool	IsEqualToPrev;
	bool	IsNewMoon;
	bool	IsFullMoon;
	bool	IsFirstQuater;
	bool	IsThirdQuater;
	double	PhaseDelta;

} TYPE_MoonPhase;

//*****************************************************************************
typedef struct
{
	char	FileName[32];
	int		year;
} TYPE_MoonInfoFile;


#define	kMoonPhaseRecCnt	((366 * 24) + 10)
extern	TYPE_MoonPhase		gMoonPhaseInfo[];
extern	int					gMoonPhaseCnt;

#define	kMoonFileMax		15
extern	TYPE_MoonInfoFile	gMoonInfoFileList[];
extern	int					gMoonInfoFileCnt;

int		NASA_ReadMoonPhaseDirectory(void);
int		NASA_ReadMoonPhaseData(void);
int		NASA_GetPhaseIndex(int year, int month, int day, int hour);
bool	NASA_GetMoonPhaseInfo(	int				year,
								int				month,
								int				day,
								int				hour,
								int				minute,
								int				second,
								TYPE_MoonPhase *moonPhaseInfo);
bool	NASA_GetMoonImageFilePath(int year, int month, int day, int hour, char *imagePath, char *imageFileName);
int		NASA_GetMoonImageCount(int year);
void	NASA_DownloadMoonPhaseData(void);
void	NASA_StartMoonImageDownloadThread(const int year);
bool	NASA_GetLatestDownLoadImageName(char *imageName);



#endif // _NASA_MOONPHASE_H_
