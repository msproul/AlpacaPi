//*****************************************************************************
//#include	"windowtab_camgraph.h"



#ifndef	_WINDOWTAB_CAMGRAPH_H_
#define	_WINDOWTAB_CAMGRAPH_H_

#ifndef	_CONTROLLER_H_
//	#include	"controller.h"
#endif // _CONTROLLER_H_

#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H



//*****************************************************************************
enum
{
	kCamGraph_Title	=	0,
	kCamGraph_Size,
	kCamGraph_Logo,



	kCamGraph_Temp_Label,
	kCamGraph_Temperature,
	kCamGraph_CoolerChkBox,
	kCamGraph_TempGraph,
	kCamGraph_TempOutline,

	kCamGraph_AlpacaErrorMsg,
	kCamGraph_IPaddr,
	kCamGraph_Readall,

	kCamGraph_last
};




#endif // _WINDOWTAB_CAMGRAPH_H_
