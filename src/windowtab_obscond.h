//*****************************************************************************
//#include	"windowtab_obscond.h"



#ifndef	_WINDOWTAB_OBSCOND_H_
#define	_WINDOWTAB_OBSCOND_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H_

#define	kBoxesPerSwitch	5
//*****************************************************************************
enum
{
	kObsCond_Title	=	0,

	kObsCond_Connected,

	kObsCond_Averageperiod_Title,
	kObsCond_Averageperiod,
	kObsCond_Averageperiod_Units,

	kObsCond_Cloudcover_Title,
	kObsCond_Cloudcover,
	kObsCond_Cloudcover_Units,

	kObsCond_Dewpoint_Title,
	kObsCond_Dewpoint,
	kObsCond_Dewpoint_Units,

	kObsCond_Humidity_Title,
	kObsCond_Humidity,
	kObsCond_Humidity_Units,

	kObsCond_Pressure_hPa_Title,
	kObsCond_Pressure_hPa,
	kObsCond_Pressure_hPa_Units,

	kObsCond_RainRate_Title,
	kObsCond_RainRate,
	kObsCond_RainRate_Units,

	kObsCond_SkyBrightness_Title,
	kObsCond_SkyBrightness,
	kObsCond_SkyBrightness_Units,

	kObsCond_SkyQuality_Title,
	kObsCond_SkyQuality,
	kObsCond_SkyQuality_Units,

	kObsCond_SkyTemperature_Title,
	kObsCond_SkyTemperature,
	kObsCond_SkyTemperature_Units,

	kObsCond_StarFWHM_Title,
	kObsCond_StarFWHM,
	kObsCond_StarFWHM_Units,

	kObsCond_Temperature_Title,
	kObsCond_Temperature,
	kObsCond_Temperature_Units,

	kObsCond_TempDegreesF_Title,
	kObsCond_TempDegreesF,
	kObsCond_TempDegreesF_Units,

	kObsCond_WindDirection_Title,
	kObsCond_WindDirection,
	kObsCond_WindDirection_Units,

	kObsCond_WindGust_Title,
	kObsCond_WindGust,
	kObsCond_WindGust_Units,

	kObsCond_WindSpeed_Title,
	kObsCond_WindSpeed,
	kObsCond_WindSpeed_Units,

	kObsCond_LastCmdString,
	kObsCond_AlpacaLogo,
	kObsCond_AlpacaErrorMsg,
	kObsCond_IPaddr,
	kObsCond_Readall,


	kObsCond_last
};

#define	kMaxSwitchControlBoxes	12

//**************************************************************************************
class WindowTabObsCond: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabObsCond(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabObsCond(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

				void	UpdateObservationValues(TYPE_ObsConditionProperties *obsCondProp);

				bool	cFirstRead;

				int		cValidSwitchCount;

};


#endif // _WINDOWTAB_SWITCH_H_
