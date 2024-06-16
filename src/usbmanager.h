//*****************************************************************************
//#include	"usbmanager.h"

#ifndef _USB_MANAGER_H_
#define _USB_MANAGER_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

int		USB_InitTable(void);
void	USB_DumpTable(void);
bool	USB_GetPathFromID(const char *idString, char *usbPath);

#endif // _USB_MANAGER_H_
