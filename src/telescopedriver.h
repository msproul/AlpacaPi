//**************************************************************************
//*	Name:			telescopedriver.h
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec  5,	2020	<MLS> Created telescopedriver.h
//*****************************************************************************
//#include	"telescopedriver.h"

#ifndef _TELESCOPE_DRIVER_H_
#define	_TELESCOPE_DRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


//**************************************************************************************
class TelescopeDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									TelescopeDriver(void);
		virtual						~TelescopeDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
//		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:

		TYPE_ASCOM_STATUS	Get_Alignmentmode(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Altitude(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_ApertureArea(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_ApertureDiameter(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_AtHome(					TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_AtPark(					TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Azimuth(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanFindHome(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanPark(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanPulseGuide(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetDeclinationRate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetGuideRates(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetPark(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetPierSide(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetRightAscensionRate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetTracking(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSlew(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSlewAltAz(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSlewAltAzAsync(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSlewAsync(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSync(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSyncAltAz(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanUnpark(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Declination(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_DeclinationRate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_DeclinationRate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_DoesRefraction(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_DoesRefraction(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_EquatorialSystem(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_FocalLength(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_GuideRateDeclination(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_GuideRateDeclination(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_GuideRateRightAscension(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_GuideRateRightAscension(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_IsPulseGuiding(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_RightAscension(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_RightAscensionRate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_RightAscensionRate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SideOfPier(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_SiderealTime(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_SiteElevation(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SiteElevation(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_SiteLatitude(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SiteLatitude(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SiteLongitude(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SiteLongitude(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Slewing(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_SlewSettleTime(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SlewSettleTime(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_TargetDeclination(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_TargetDeclination(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_TargetRightAscension(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_TargetRightAscension(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Tracking(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Tracking(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_TrackingRate(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_TrackingRate(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_TrackingRates(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_UTCdate(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_UTCdate(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//--------------------------------------------------------------------------------------------------
		//*	ASCOM docs call these METHODS
		TYPE_ASCOM_STATUS	Put_AbortSlew(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_AxisRates(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanMoveAxis(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_DestinationSideOfPier(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Put_FindHome(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_MoveAxis(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_PulseGuide(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_Park(					TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SetPark(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Put_SlewToAltAz(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SlewToAltAzAsync(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SlewToCoordinates(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SlewToCoordinatesAsync(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_SyncToAltAz(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SyncToCoordinates(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SyncToTarget(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_SlewToTarget(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_SlewToTargetAsync(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Put_UnPark(					TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		//--------------------------------------------------------------------------------------------------
		//*	extras added by MLS
		TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


				//*	these routines should be implemented by the sub-classes
				//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS		Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);

		TYPE_AlignmentModes	cAlginmentMode;
		double				cAltitude;
		double				cApertureArea;
		double				cApertureDiameter;
		bool				cAtHome;
		bool				cAtPark;
		double				cAzimuth;
		bool				cCanFindHome;
		bool				cCanMoveAxis;
		bool				cCanPark;
		bool				cCanPulseGuide;
		bool				cCanSetDeclinationRate;
		bool				cCanSetGuideRates;
		bool				cCanSetPark;
		bool				cCanSetPierSide;
		bool				cCanSetRightAscensionRate;
		bool				cCanSetTracking;
		bool				cCanSlew;
		bool				cCanSlewAltAz;
		bool				cCanSlewAltAzAsync;
		bool				cCanSlewAsync;
		bool				cCanSync;
		bool				cCanSyncAltAz;
		bool				cCanUnpark;

		bool				cTargetDec_HasBeenSet;
		double				cDeclination;
		double				cDeclinationRate;

		bool				cTargetRA_HasBeenSet;
		double				cRightAscension;
		double				cRightAscensionRate;


		bool				cDoesRefraction;
		TYPE_EquatorialCoordinateType	cEquatorialSystem;
		double				cFocalLength;
		double				cGuideRateDeclination;
		double				cGuideRateRightAscension;
		bool				cIsPulseGuiding;
		TYPE_PierSide		cSideOfPier;
		double				cSiderealTime;
		double				cSiteElevation;
		double				cSiteLatitude;
		double				cSiteLongitude;
		bool				cSlewing;
		short				cSlewSettleTime;
		double				cTargetDeclination;
		double				cTargetRightAscension;
		bool				cTracking;
		TYPE_DriveRates		cTrackingRate;
//+		double				cTrackingRates;
//+		double				cUTCDate;



};



#endif // _TELESCOPE_DRIVER_H_
