//*****************************************************************************
//*****************************************************************************
//#include	"controller_alpacaUnit.h"

#ifndef _CONTROLLER_ALPACAUNIT_H_
#define	_CONTROLLER_ALPACAUNIT_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#include	"controller.h"

//===========================================
#ifndef	_WINDOWTAB_ALPACAUNIT_H_
	#include	"windowtab_alpacaUnit.h"
#endif


#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#define	kDomeWindowWidth	475
#define	kDomeWindowHeight	720

#define	kMaxDevices	16
//**************************************************************************************
typedef struct
{
	char	Name[64];
	char	Type[64];
} TYPE_CONFIGURED_DEVICE;

//**************************************************************************************
class ControllerAlpacaUnit: public Controller
{
	public:
		//
		// Construction
		//
						ControllerAlpacaUnit(	const char			*argWindowName,
												TYPE_ALPACA_UNIT	*alpacaUnit);


		virtual			~ControllerAlpacaUnit(void);


		virtual	void	SetupWindowControls(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
//				bool	AlpacaGetStatus(void);
//
		virtual	bool	AlpacaProcessReadAll(			const char	*deviceType,
														const int	deviveNum,
														const char	*keywordString,
														const char	*valueString);
//		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
//														const int	deviveNum,
//														const char	*valueString);
//
		virtual	void	GetStartUpData_SubClass(void);
		virtual	void	GetStatus_SubClass(void);
		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateSupportedActions(void);
//		virtual	void	UpdateCapabilityList(void);
//
		virtual	void	ProcessConfiguredDevices(const char *keyword, const char *valueString);

				TYPE_ALPACA_UNIT	cAlpacaUnit;

//				void	AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString);
				void	GetTemperatureLogs(void);

				//--------------------------------------------
				//*	tab information
				WindowAlpacaUnit		*cAlpacaUnitTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				bool					cHasCamera;
				bool					cHasFocuser;

				int						cConfiguredDevIndex;
				TYPE_CONFIGURED_DEVICE	cDeviceList[kMaxDevices];

				double					cCPUtempLog[kTemperatureLogEntries + 10];
				int						cCPUTtempCnt;

				double					cCameraTempLog[kTemperatureLogEntries + 10];
				int						cCameraTempCnt;

				double					cFocusTempLog[kTemperatureLogEntries + 10];
				int						cFocusTempCnt;
};

void	CreateAlpacaUnitWindow(TYPE_ALPACA_UNIT	*alpacaUnit);

#endif // _CONTROLLER_ALPACAUNIT_H_

