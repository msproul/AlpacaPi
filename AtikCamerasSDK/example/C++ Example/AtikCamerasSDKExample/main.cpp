// ArtemisHSCDemoApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AtikCamerasSDKApp.h"

int _tmain(int argc, _TCHAR* argv[])
{
	AtikCamerasSDKApp app;
	app.Start();
	app.Shutdown();	

	return 0;
}

