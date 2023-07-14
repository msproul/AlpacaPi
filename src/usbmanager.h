//*****************************************************************************
//#include	"usbmanager.h"

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

int		USB_InitTable(void);
bool	USB_GetPathFromID(const char *idString, char *usbPath);
