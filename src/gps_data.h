//**************************************************************************
//*	Name:			gps_data.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr  9,	2024	<MLS> Created gps_data.h
//*****************************************************************************
//#include	"gps_data.h"

#ifndef _GPS_DATA_H_
#define	_GPS_DATA_H_



#define	kMaxNMEAlen	80


//*****************************************************************************
typedef struct	//	TYPE_GPSnmea
{
	char	nmeaString[kMaxNMEAlen];

}	TYPE_GPSnmea;
#define	kMaxNMEAstrings	20

//*****************************************************************************
enum
{
	kGPS_Mode_None	=	0,
	kGPS_Mode_Serial,
	kGPS_Mode_QHYcamera,

	kGPS_Mode_Last
};

//*****************************************************************************
typedef struct	//	TYPE_QHY_GPSdata
{
//	int				GPSmode;			//*	0 = no GPS
	bool			Present;
	struct timeval	SystemTime;			//*	Used for calculating GPS/SYS time offset
	TYPE_GPSnmea	NMEAdata[kMaxNMEAstrings];
	int				NMEAdataIdx;
	int				NMEAerrCnt;			//*	number of bad NMEA packets (invalid checksum)
	bool			Status;				//	0 = not valid, 1 = locked
	bool			DateValid;
	bool			TimeValid;
	bool			LaLoValid;
	bool			AltValid;

	double			Lat;
	double			Long;
	double			Altitude;			//*	in meters
	uint32_t		SatsInView;			//*	Number of satellites in view

	//------------------------------------------------------------------
	//*	this is QHY GPS specific data
	char			CameraName[48];
	char			LibraryVersion[48];
	char			FPGAversion[48];


	uint32_t		SequenceNumber;
	uint32_t		PPSC;
	char			ShutterStartTimeStr[48];
	uint32_t		SU;					//*	start time micro seconds
	char			ShutterEndTimeStr[48];
	uint32_t		EU;					//*	End time micro seconds
	char			NowTimeStr[48];
	uint32_t		NU;					//*	Now time micro seconds
	double			Exposure_us;		//*	exposure time in micro seconds
	double			ClockDeltaSecs;
	char			SatMode1;
	char			SatMode2;
} TYPE_QHY_GPSdata;


void	GPS_StartThread(const char *serialPortPathArg = NULL, const char baudRate='9');


#endif // _GPS_DATA_H_
