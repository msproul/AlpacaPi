//*****************************************************************************
//*		controller_tasktiming.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jul 21,	2022	<MLS> Created controller_tasktiming.cpp
//*	Jul 21,	2022	<MLS> Added TaskTiming_Init(), TaskTiming_SetName()
//*	Jul 21,	2022	<MLS> Added TaskTiming_Start(), TaskTiming_Stop()
//*****************************************************************************

#ifdef _ENABLE_TASK_TIMING_


#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

////*****************************************************************************
//typedef struct
//{
//	char			taskName[48];
//	unsigned long	totalMilliSecsUsed;
//	unsigned long	startNanoSecs;
//	unsigned long	endNanoSecs;
//	unsigned long	nanoSecsAccumulator;
//
//} TYPE_TASK_TIMING;

//		TYPE_TASK_TIMING	cTaskData[kMaxTaskTiming];


//*****************************************************************************
void	Controller::TaskTiming_Init(void)
{
int		iii;

	for (iii=0; iii<kMaxTaskTiming; iii++)
	{
		memset((void *)&cTaskData[iii], 0, sizeof(TYPE_TASK_TIMING));
	}

	TaskTiming_SetName(kTask_BackgroundThread, "BackgroundThread");
}

//*****************************************************************************
void	Controller::TaskTiming_SetName(const int taskID, const char *taskName)
{
	if ((taskID >=0) && (taskID < kMaxTaskTiming))
	{
		strcpy(cTaskData[taskID].taskName, taskName);
	}
}

//*****************************************************************************
void	Controller::TaskTiming_Start(const int taskID)
{
	if ((taskID >=0) && (taskID < kMaxTaskTiming))
	{
		cTaskData[taskID].startNanoSecs	=	MSecTimer_getNanoSecs();
	}
}

//*****************************************************************************
void	Controller::TaskTiming_Stop(const int taskID)
{
unsigned long	deltaNanoSecs;

	if ((taskID >=0) && (taskID < kMaxTaskTiming))
	{
		cTaskData[taskID].endNanoSecs			=	MSecTimer_getNanoSecs();
		deltaNanoSecs							=	cTaskData[taskID].endNanoSecs - cTaskData[taskID].startNanoSecs;
		cTaskData[taskID].nanoSecsAccumulator	+=	deltaNanoSecs;

		if (cTaskData[taskID].nanoSecsAccumulator > 1000000)
		{
			cTaskData[taskID].totalMilliSecsUsed++;
			cTaskData[taskID].nanoSecsAccumulator	-=	1000000;
		}
	}
}


#endif // _ENABLE_TASK_TIMING_
