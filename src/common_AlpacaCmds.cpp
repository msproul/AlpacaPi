//*****************************************************************************
//*	command table for Alpaca common commands
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created common_AlpacaCmds.cpp
//*	Jul  1,	2023	<MLS> Added gExtrasCmdTable
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _COMMON_ALPACA_CMDS_H_
	#include	"common_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gCommonCmdTable[]	=
{

	{	"action",				kCmd_Common_action,				kCmdType_PUT	},
	{	"commandblind",			kCmd_Common_commandblind,		kCmdType_PUT	},
	{	"commandbool",			kCmd_Common_commandbool,		kCmdType_PUT	},
	{	"commandstring",		kCmd_Common_commandstring,		kCmdType_PUT	},
	{	"connected",			kCmd_Common_connected,			kCmdType_BOTH	},
	//--------------------------------------------------------------------------------------------------------
	//*	New commands as of Jun 18, 2023
	//	All interfaces - Connect() and Disconnect() methods together with a Connecting property to manage asynchronous connection.
	//	All interfaces - DeviceState property to return all device operational state values in a single call.
	//*	connect, connecting, devicestate, disconnect
	{	"connect",				kCmd_Common_Connect,			kCmdType_PUT	},
	{	"connecting",			kCmd_Common_Connecting,			kCmdType_GET	},
	{	"description",			kCmd_Common_description,		kCmdType_GET	},
	{	"devicestate",			kCmd_Common_DeviceState,		kCmdType_GET	},
	{	"disconnect",			kCmd_Common_Disconnect,			kCmdType_PUT	},

	{	"driverinfo",			kCmd_Common_driverinfo,			kCmdType_GET	},
	{	"driverversion",		kCmd_Common_driverversion,		kCmdType_GET	},
	{	"interfaceversion",		kCmd_Common_interfaceversion,	kCmdType_GET	},
	{	"name",					kCmd_Common_name,				kCmdType_GET	},
	{	"supportedactions",		kCmd_Common_supportedactions,	kCmdType_GET	},

	//*	extras added by MLS
	{	"--extras",				kCmd_Common_Extras,				kCmdType_GET	},
	{	"livewindow",			kCmd_Common_LiveWindow,			kCmdType_PUT	},
	{	"temperaturelog",		kCmd_Common_TemperatureLog,		kCmdType_GET	},
	{	"restart",				kCmd_Common_Restart,			kCmdType_PUT	},

#ifdef _INCLUDE_EXIT_COMMAND_
	//*	the exit command was implemented for a special case application, it is not intended
	//*	to be used in the normal astronomy community
	{	"exit",					kCmd_Common_exit,				kCmdType_GET	},
#endif // _INCLUDE_EXIT_COMMAND_
	{	"",						-1,								0x00	}
};


//*****************************************************************************
TYPE_CmdEntry	gExtrasCmdTable[]	=
{
	{	"ClientTransactionID",	kCmd_Extra_ClientTransactionID,	kCmdType_GET	},
	{	"Command",				kCmd_Extra_Command,				kCmdType_GET	},
	{	"Comment",				kCmd_Extra_Comment,				kCmdType_GET	},
	{	"Device",				kCmd_Extra_Device,				kCmdType_GET	},
	{	"ErrorNumber",			kCmd_Extra_ErrorNumber,			kCmdType_GET	},
	{	"ErrorMessage",			kCmd_Extra_ErrorMessage,		kCmdType_GET	},
	{	"ReadAll",				kCmd_Extra_ReadAll,				kCmdType_GET	},
	{	"ServerTransactionID",	kCmd_Extra_ServerTransactionID,	kCmdType_GET	},
	{	"Version",				kCmd_Extra_Version,				kCmdType_GET	},
	{	"UTCDATE",				kCmd_Extra_UTCDate,				kCmdType_GET	},
	{	"Array",				kCmd_Extra_Array,				kCmdType_GET	},
	{	"]",					kCmd_Extra_Bracket,				kCmdType_GET	},

	{	"",						-1,								0x00			}
};
