//#include	"observingconditions_driver.h"

#ifndef _ALPACA_DRIVER_H
	#include	"alpaca_driver.h"
#endif

void	Init_ObservCond(void);
int		ObservCond_RunStateMachine(void);
int		ObservCond_ProcessCommand(TYPE_GetPutRequestData *reqData);
void	ObservCond_OutputHTML(TYPE_GetPutRequestData *reqData);
