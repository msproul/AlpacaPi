//*****************************************************************************
//#include	"linuxerrors.h"

#ifndef	_ERRNO_H
	#include	<errno.h>
#endif // _ERRNO_H

#ifdef __cplusplus
	extern "C" {
#endif



void	GetLinuxErrorString(const int errNum, char *errorString);

#ifdef __cplusplus
}
#endif
