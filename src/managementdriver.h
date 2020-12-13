//**************************************************************************
//*	Name:			managementdriver.h
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 20,	2019	<MLS> Created managment driver
//*****************************************************************************
//#include	"managementdriver.h"

#ifndef _MANAGEMENT_DRIVER_H_
#define	_MANAGEMENT_DRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


//**************************************************************************************
class ManagementDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									ManagementDriver(const int argDevNum);
		virtual						~ManagementDriver(void);

		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:

			TYPE_ASCOM_STATUS		Get_Apiversions(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS		Get_Description(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS		Get_Configureddevices(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS		Get_Libraries(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS		Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

};

//*****************************************************************************
//Filter wheel commands
//*****************************************************************************
enum
{
	kCmd_Managment_apiversions	=	0,	//*
	kCmd_Managment_description,			//*
	kCmd_Managment_configureddevices,	//*

	kCmd_Managment_Extras,
	kCmd_Managment_cpustats,
	kCmd_Managment_libraries,
	kCmd_Managment_readall,


	kCmd_Managment_last

};



void	CreateManagementObject(void);


#endif	//	_DOME_DRIVER_H_

