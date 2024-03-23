//*****************************************************************************
//#include	"PDS_ReadNASAfiles.h"

#ifndef _PDS_TYPEDEFS_H_
	#include	"PDS.typedefs.h"
#endif

#ifdef __cplusplus
	extern "C" {
#endif


bool	PDS_ReadImage(const char *filePath, PDS_header_data	*pdsHeaderPtr);


#ifdef __cplusplus
}
#endif
