//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jul 23,	2020	<MLS> Moved SonyCallback to separate file
//*****************************************************************************
//#include	"sonyCallback.h"


#ifndef _SONY_CALLBACK_H_
#define	_SONY_CALLBACK_H_

#ifndef CAMERAREMOTE_SDK_H
	#include	"CameraRemote_SDK.h"
#endif

#ifndef ICRCAMERAOBJECTINFO_H
//	#include "ICrCameraObjectInfo.h"
#endif

#ifndef IDEVICECALLBACK_H
	#include	"IDeviceCallback.h"
#endif

//int	CreateSONY_CameraObjects(void);



//using namespace SCRSDK;



//*****************************************************************************
class SonyCallBack : public SCRSDK::IDeviceCallback
{
public:
				SonyCallBack(void *alpacaCameraDriver);
	virtual		~SonyCallBack();


public:
	// Inherited via IDeviceCallback
	virtual void OnConnected(SCRSDK::DeviceConnectionVersioin version) override;
	virtual void OnDisconnected(CrInt32u error) override;
	virtual void OnPropertyChanged() override;
	virtual void OnLvPropertyChanged() override;
	virtual void OnCompleteDownload(CrChar* filename) override;
	virtual void OnWarning(CrInt32u warning) override;
	virtual void OnError(CrInt32u error) override;

	void	*cSonyCameraDriverObjPtr;


			int		cOnConnected_Count;
			int		cOnDisconnected_Count;
			int		cOnPropertyChanged_Count;
			int		cOnLvPropertyChanged_Count;
			int		cOnCompleteDownload_Count;
			int		cOnWarning_Count;
			int		cOnError_Count;


};

#endif // _SONY_CALLBACK_H_
