//*****************************************************************************
//*	Jun 26,	2023	<MLS> Created common_AlpacaCmds.h
//*****************************************************************************
//#include	"common_AlpacaCmds.h"

#ifndef _COMMON_ALPACA_CMDS_H_
#define _COMMON_ALPACA_CMDS_H_

//*****************************************************************************
//*	common commands
enum
{
	//*	these enums start at 1000 to stay out of the way of the device specific commands.
	//*	this allows the driver to have a single switch statement for all the implemented commands

	kCmd_Common_action			=	1000,	//*	Invokes the specified device-specific action.
	kCmd_Common_commandblind,				//*	Transmits an arbitrary string to the device
	kCmd_Common_commandbool,				//*	Transmits an arbitrary string to the device and returns a boolean value from the device.
	kCmd_Common_commandstring,				//*	Transmits an arbitrary string to the device and returns a string value from the device.
	kCmd_Common_connected,					//*	GET--Retrieves the connected state of the device
	//--------------------------------------------------------------------------------------------------------
	//*	New commands as of Jun 18, 2023
	//All interfaces - Connect() and Disconnect() methods together with a Connecting property to manage asynchronous connection.
	//All interfaces - DeviceState property to return all device operational state values in a single call.
	kCmd_Common_Connect,
	kCmd_Common_Connecting,
	kCmd_Common_Disconnect,
	kCmd_Common_DeviceState,

	kCmd_Common_description,				//*	Device description
	kCmd_Common_driverinfo,					//*	Device driver description
	kCmd_Common_driverversion,				//*	Driver Version
	kCmd_Common_interfaceversion,			//*	The ASCOM Device interface version number that this device supports.
	kCmd_Common_name,						//*	Device name
	kCmd_Common_supportedactions,			//*	Returns the list of action names supported by this driver.
	kCmd_Common_SetupDialog,

#ifdef _INCLUDE_EXIT_COMMAND_
	//*	Added by MLS 7/20/2020
	kCmd_Common_exit,
#endif // _INCLUDE_EXIT_COMMAND_
	kCmd_Common_Extras,
	kCmd_Common_Details,
	kCmd_Common_LiveWindow,
	kCmd_Common_TemperatureLog,
	kCmd_Common_Restart,			//*	cause the driver to be destroyed and re-created

	kCmd_Common_last
};

//*****************************************************************************
//*	Extra commands
//*	these are data values that I have added to the alpaca json stream NOT in the standard Alpaca spec
//*****************************************************************************
enum
{
	kCmd_Extra_ClientTransactionID	=	2000,
	kCmd_Extra_Command,
	kCmd_Extra_Comment,
	kCmd_Extra_Device,
	kCmd_Extra_ErrorNumber,
	kCmd_Extra_ErrorMessage,
	kCmd_Extra_ReadAll,
	kCmd_Extra_ServerTransactionID,
	kCmd_Extra_Array,
	kCmd_Extra_Version,
	kCmd_Extra_UTCDate,
	kCmd_Extra_Bracket,
};
#endif // _COMMON_ALPACA_CMDS_H_
