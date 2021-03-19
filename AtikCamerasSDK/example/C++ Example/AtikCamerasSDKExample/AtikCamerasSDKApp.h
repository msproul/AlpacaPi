#pragma once

#include "AtikCameras.h"

#include <string>
#include <vector>

using namespace std;

enum Command 
{ 
	End, 
	Help, 
	Camera, 
	StartExposure, 
	GetBin, SetBin, 
	GetSubFrame, SetSubFrame, SetSubFramePos, SetSubFrameSize, 
	StartCooling, WarmUp, GetTemp, TempMonitor, 
	GetFilterWheelPos, SetFilterWheelPos, 
	Guide, GuideNSEW, PulseGuide, StopGuide,
	UnknownCommand 
};

class AtikCamerasSDKApp
{
public:
	AtikCamerasSDKApp();
	~AtikCamerasSDKApp();
	
	void Start();
	void Shutdown();

private:
	bool AtemptConnectToCamera();
	void WaitForCommand();

	// Camera Info:
	void PrintCameraDetails();

	// Exposures
	void StartExposure(const string text);
	void GetBin();
	void SetBin(const string text);
	void GetSubFrame();
	void SetSubFrame(const string text);
	void SetSubFramePos(const string text);
	void SetSubFrameSize(const string text);

	// Cooling:
	void StartCooling(const string text);
	void WarmUp();
	void GetTemp();
	void TempMonitor(const string text);

	// FilterWheel:
	void GetFilterWheelPos();
	void SetFilterWheelPos(const string text);

	// Guiding:
	void Guide(const string text);
	void GuideNSEW(const string text);
	void PulseGuide(const string text);
	void StopGuide();

	// Helpfull methods:
	int SetParameters(const string text);

	static Command GetCommand(const string text);
	static bool    StringCompare(const string s1, const string s2);

	static void PrintError(const string title, int errorCode);

private:
	bool isInit;
	ArtemisHandle handle;	

	ARTEMISPROPERTIES properties;	
	int nTempSensor;
	int nFilterWheelPositions;
	bool canGuide;
	
	vector<string> parameters;
};

