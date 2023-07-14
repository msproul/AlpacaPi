//**************************************************************************
//*	Name:			slittracker_AlpacaCmds.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	Command table for slittracker
//*
//*****************************************************************************
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jul 10,	2023	<MLS> Created slittracker_AlpacaCmds.cpp
//*****************************************************************************


//*****************************************************************************
static TYPE_CmdEntry	gSlitTrackerCmdTable[]	=
{
//?	{	"setrate",				kCmd_SlitTracker_SetRate,			kCmdType_PUT	},

	{	"domeaddress",			kCmd_SlitTracker_DomeAddress,		kCmdType_GET	},
	{	"trackingenabled",		kCmd_SlitTracker_TrackingEnabled,	kCmdType_BOTH	},
	{	"readall",				kCmd_SlitTracker_readall,			kCmdType_GET	},
	{	"",						-1,	0x00	}
};

#ifdef _CONTROLLER_USES_ALPACA_
//*****************************************************************************
static TYPE_CmdEntry	gSlitTrackerExtrasTable[]	=
{
	{	"sensor-0",				kCmd_SlitTracker_Sesnsor0,		kCmdType_GET	},
	{	"sensor-1",				kCmd_SlitTracker_Sesnsor1,		kCmdType_GET	},
	{	"sensor-2",				kCmd_SlitTracker_Sesnsor2,		kCmdType_GET	},
	{	"sensor-3",				kCmd_SlitTracker_Sesnsor3,		kCmdType_GET	},
	{	"sensor-4",				kCmd_SlitTracker_Sesnsor4,		kCmdType_GET	},
	{	"sensor-5",				kCmd_SlitTracker_Sesnsor5,		kCmdType_GET	},
	{	"sensor-6",				kCmd_SlitTracker_Sesnsor6,		kCmdType_GET	},
	{	"sensor-7",				kCmd_SlitTracker_Sesnsor7,		kCmdType_GET	},
	{	"sensor-8",				kCmd_SlitTracker_Sesnsor8,		kCmdType_GET	},
	{	"sensor-9",				kCmd_SlitTracker_Sesnsor9,		kCmdType_GET	},
	{	"sensor-10",			kCmd_SlitTracker_Sesnsor10,		kCmdType_GET	},
	{	"sensor-11",			kCmd_SlitTracker_Sesnsor11,		kCmdType_GET	},
	{	"gravity_x",			kCmd_SlitTracker_gravity_x,		kCmdType_GET	},
	{	"gravity_y",			kCmd_SlitTracker_gravity_y,		kCmdType_GET	},
	{	"gravity_z",			kCmd_SlitTracker_gravity_z,		kCmdType_GET	},
	{	"gravity_t",			kCmd_SlitTracker_gravity_t,		kCmdType_GET	},
	{	"",						-1,	0x00											}
};
#endif // _CONTROLLER_USES_ALPACA_
