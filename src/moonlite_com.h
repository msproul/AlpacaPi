//*****************************************************************************
//#include	"moonlite_com.h"

#ifndef	_MOONLITE_COM_H_
#define	_MOONLITE_COM_H_
#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif
#ifdef __cplusplus
	extern "C" {
#endif


#define	kStepsPerRev_WR25 374920	//	steps per revolution
#define	kStepsPerRev_WR30 444080	//	steps per revolution
#define	kStepsPerRev_WR35 505960	//	steps per revolution


//*****************************************************************************
enum
{
	kMoonLite_NiteCrawler	=	0,
	kMoonLite_HighRes,
};

//*****************************************************************************
typedef struct
{
	char			usbPortPath[48];
	int				model;
	int				fileDesc;				//*	port file descriptor
	char			deviceModelString[64];
	char			deviceVersion[64];
	char			deviceSerialNum[64];
	bool			openOK;
	int32_t			stepsPerRev;
	int				invalidStringErrCnt;
	unsigned char	switchBits;
	unsigned char	auxSwitchBits;

	bool			switchIN;
	bool			switchOUT;
	bool			switchROT;
	bool			switchAUX1;
	bool			switchAUX2;

} TYPE_MOONLITECOM;


int		MoonLite_CountFocusers(void);
void	MoonLite_InitCom(					TYPE_MOONLITECOM *moonliteCom, int serialPortIndex);
bool	MoonLite_OpenFocuserConnection(		TYPE_MOONLITECOM *moonliteCom, bool checkForNiteCrawler);
bool	MoonLite_CloseFocuserConnection(	TYPE_MOONLITECOM *moonliteCom);
bool	MoonLite_FlushReadBuffer(			TYPE_MOONLITECOM *moonliteCom);
bool	MoonLite_GetPosition(	TYPE_MOONLITECOM	*moonliteCom,
								const int			axisNumber,
								int32_t				*valueToUpdate);
bool	MoonLite_SetSPostion(	TYPE_MOONLITECOM	*moonliteCom,
								const int			axisNumber,
								int32_t				newPosition);

bool	MoonLite_GetTemperature(TYPE_MOONLITECOM	*moonliteCom,
								double				*returnTemp_degC);
bool	MoonLite_GetVoltage(	TYPE_MOONLITECOM	*moonliteCom,
								double				*returnVoltage);


bool	MoonLite_GetSwiches(	TYPE_MOONLITECOM	*moonliteCom,
								unsigned char		*switchBits);

bool	MoonLite_GetAuxSwiches(	TYPE_MOONLITECOM	*moonliteCom,
								unsigned char		*auxSwitchBits);

bool	MoonLite_StopAxis(		TYPE_MOONLITECOM	*moonliteCom, int axisNumber);
bool	MoonLite_StopMotors(	TYPE_MOONLITECOM	*moonliteCom);


bool	MoonLite_GetNC_Color(	TYPE_MOONLITECOM	*moonliteCom,
								uint16_t			whichColor,
								uint16_t			*color16bit);


#ifdef __cplusplus
}
#endif


#endif	//	_MOONLITE_COM_H_
