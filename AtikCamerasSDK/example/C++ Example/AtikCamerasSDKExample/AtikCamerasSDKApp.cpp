#include "stdafx.h"
#include "AtikCamerasSDKApp.h"
#include "ConsoleHelper.h"

#include <Windows.h> // Used only for Sleep


AtikCamerasSDKApp::AtikCamerasSDKApp()
{
	

}
AtikCamerasSDKApp::~AtikCamerasSDKApp()
{
}

void AtikCamerasSDKApp::Shutdown()
{
	ArtemisCoolerWarmUp(handle);
	ArtemisDisconnect(handle); // No longer needed as the DLL will take care of this!

	ConsoleHelper::ReadLine("Shutdown! - Press enter to exit");
}

void AtikCamerasSDKApp::Start()
{
	// First: Try to load the DLL:
	if (!ArtemisLoadDLL("AtikCameras.dll"))
	{
		ConsoleHelper::ReadLine("Cannot Load DLL");
		return;
	}

	// Now Check API / DLL versions
	int apiVersion = ArtemisAPIVersion();
	int dllVersion = ArtemisDLLVersion();
	if (apiVersion != dllVersion)
	{
		ConsoleHelper::ReadLine("Version do not match! API: %d DLL: %d", apiVersion, dllVersion);
		return;
	}

	ConsoleHelper::WriteLine("Artemis HSC Demo");
	ConsoleHelper::WriteLine("Version: %d", apiVersion);
	ConsoleHelper::WriteLine("");

	if (!AtemptConnectToCamera())
		return;

	WaitForCommand();
}
bool AtikCamerasSDKApp::AtemptConnectToCamera()
{
	// Connect to camera: (-1 = First avaialble camera)
	handle = ArtemisConnect(-1);
	while (handle == NULL)
	{
		string result = ConsoleHelper::ReadLine("No Camera Available. (Enter = Retry, Any text = Quit)");
		if (result == "")
			handle = ArtemisConnect(-1);
		else
			return false;
	}

	ConsoleHelper::WriteLine("Connected To Camera: ");

	ArtemisProperties(handle, &properties);
	ArtemisTemperatureSensorInfo(handle, 0, &nTempSensor);

	int moving, target, current;
	ArtemisFilterWheelInfo(handle, &nFilterWheelPositions, &moving, &current, &target);
	
	canGuide = (properties.cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GUIDE_PORT);

	PrintCameraDetails();		
	return true;
}
void AtikCamerasSDKApp::WaitForCommand()
{
	while (true)
	{
		ConsoleHelper::WriteLine("");

		string command = ConsoleHelper::ReadLine("Enter Command: (Type: 'help' for list of functions, 'end' to quit)");
		switch (GetCommand(command))
		{
		case Command::End:
			ConsoleHelper::WriteLine("Shutting down...");
			return;

		case Command::Help:
			ConsoleHelper::WriteLine("");
			ConsoleHelper::WriteLine("Command:       \t\tDescription:");
			ConsoleHelper::WriteLine("-------------- \t\t-------------------------------------------------");
			ConsoleHelper::WriteLine("End            \t\tEnds the application");
			ConsoleHelper::WriteLine("Help           \t\tLists commands");
			ConsoleHelper::WriteLine("Expose s       \t\tPerforms s-second Exposure");
			ConsoleHelper::WriteLine("GetBin         \t\tDisplays current binning");
			ConsoleHelper::WriteLine("SetBin x y     \t\tSets the binning");
			ConsoleHelper::WriteLine("GetSF          \t\tDisplays current subframe parameters");
			ConsoleHelper::WriteLine("SetSF x y w h  \t\tSets the subframe parameters");
			ConsoleHelper::WriteLine("SetSFPos x y   \t\tSets the subframe position");
			ConsoleHelper::WriteLine("SetSFSize w h  \t\tSets the subframe size");
			ConsoleHelper::WriteLine("Cool t         \t\tSets the cooling to tC");
			ConsoleHelper::WriteLine("Warm           \t\tTells the cooling to warm up");
			ConsoleHelper::WriteLine("Temp           \t\tDisplays current temp");
			ConsoleHelper::WriteLine("TempMonitor s  \t\tDisplays current temp every second for s-seconds");
			ConsoleHelper::WriteLine("GetFW          \t\tGets the current filter wheel position");
			ConsoleHelper::WriteLine("SetFW p        \t\tSets the filter wheel position to p");
			ConsoleHelper::WriteLine("Guide a        \t\tSets the guide axis to a");
			ConsoleHelper::WriteLine("GuidePort nsew \t\tSets the guide axis to a combination of nsew");
			ConsoleHelper::WriteLine("PulseGuide a t \t\tSets the guide axis to a for t-ms");
			ConsoleHelper::WriteLine("StopGuide      \t\tStops the guiding");
			break;

		case Command::Camera:
			PrintCameraDetails();
			break;

		case Command::StartExposure:
			StartExposure(command);
			break;

		case Command::GetBin:
			GetBin();
			break;

		case Command::SetBin:
			SetBin(command);
			break;

		case Command::GetSubFrame:
			GetSubFrame();
			break;

		case Command::SetSubFrame:
			SetSubFrame(command);
			break;

		case Command::SetSubFramePos:
			SetSubFramePos(command);
			break;

		case Command::SetSubFrameSize:
			SetSubFrameSize(command);
			break;

		case Command::StartCooling:
			StartCooling(command);
			break;

		case Command::WarmUp:
			WarmUp();
			break;

		case Command::GetTemp:
			GetTemp();
			break;

		case Command::TempMonitor:
			TempMonitor(command);
			break;

		case Command::GetFilterWheelPos:
			GetFilterWheelPos();
			break;

		case Command::SetFilterWheelPos:
			SetFilterWheelPos(command);
			break;

		case Command::Guide:
			Guide(command);
			break;

		case Command::GuideNSEW:
			GuideNSEW(command);
			break;

		case Command::PulseGuide:
			PulseGuide(command);
			break;

		case Command::StopGuide:
			StopGuide();
			break;

		default:
			ConsoleHelper::WriteLine("Unknown Command: Use 'help' for list of functions");
			break;
		}
	}
}

void AtikCamerasSDKApp::PrintCameraDetails()
{	
	ConsoleHelper::WriteLine("Name:           \t%s",		   properties.Description);
	ConsoleHelper::WriteLine("Manufacturer:   \t%s",		   properties.Manufacturer);
	ConsoleHelper::WriteLine("Pixels:         \t%d x %d",      properties.nPixelsX,      properties.nPixelsY);
	ConsoleHelper::WriteLine("Pixel Size:     \t%.2f x %.2f",  properties.PixelMicronsX, properties.PixelMicronsY);
	ConsoleHelper::WriteLine("Has Cooling:    \t%s",		   (nTempSensor > 0) ? "True" : "False");
	ConsoleHelper::WriteLine("FilterWheel:    \t%d Positions", nFilterWheelPositions);
	ConsoleHelper::WriteLine("Has Guide Port: \t%s",		   canGuide          ? "True" : "False");

}

void AtikCamerasSDKApp::StartExposure(const string text)
{
	if (SetParameters(text) != 1)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}	

	float seconds = atof(parameters[0].c_str());

	ConsoleHelper::WriteLine("Starting %.3fs Exposure:", seconds);
	int error = ArtemisStartExposure(handle, seconds);
	if (error != ARTEMIS_OK)
	{
		PrintError("Start Exposure", error);
		return;
	}
		

	int cameraState = 0;

	while (!ArtemisImageReady(handle))
	{
		int newState = ArtemisCameraState(handle);
		if (newState != cameraState)
		{
			cameraState = newState;

			switch (cameraState)
			{
			case CAMERA_ERROR:
				ConsoleHelper::WriteLine("Something went wrong!!");
				break;

			case CAMERA_IDLE:
				ConsoleHelper::WriteLine("Idle");
				break;

			case CAMERA_WAITING:
				ConsoleHelper::WriteLine("Waiting");
				break;

			case CAMERA_EXPOSING:
				ConsoleHelper::WriteLine("Exposing");
				break;

			case CAMERA_READING:
				ConsoleHelper::WriteLine("Reading");
				break;

			case CAMERA_DOWNLOADING:
				ConsoleHelper::WriteLine("Downloading");
				break;

			case CAMERA_FLUSHING:
				ConsoleHelper::WriteLine("Flushing");
				break;
			}
		}

		if (cameraState == CAMERA_DOWNLOADING)
		{
			int percent = ArtemisDownloadPercent(handle);
			ConsoleHelper::WriteLine("Download: %d %", percent);
		}
	}

	ConsoleHelper::WriteLine("Image Finished");

	int x, y, w, h, xBin, yBin;
	error = ArtemisGetImageData(handle, &x, &y, &w, &h, &xBin, &yBin);
	if (error != ARTEMIS_OK)
	{
		PrintError("Get Image Data", error);
		return;
	}

	void * imageBuffer = ArtemisImageBuffer(handle);
	if (imageBuffer == NULL)
	{
		PrintError("Get Image Buffer", error);		
		return;
	}

	ConsoleHelper::WriteLine("X:        %d", x);
	ConsoleHelper::WriteLine("Y:        %d", y);
	ConsoleHelper::WriteLine("Width:    %d", w);
	ConsoleHelper::WriteLine("Height:   %d", h);
	ConsoleHelper::WriteLine("XBin:     %d", xBin);
	ConsoleHelper::WriteLine("YBin:     %d", yBin);
	ConsoleHelper::WriteLine("Buffer:   %d (Address not size)", imageBuffer);
}
void AtikCamerasSDKApp::GetBin()
{
	int x, y;
	int error = ArtemisGetBin(handle, &x, &y);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Current Binning: X=%d, Y=%d", x, y);
	else
		PrintError("Get Binning", error);
}
void AtikCamerasSDKApp::SetBin(const string text)
{	
	if (SetParameters(text) != 2)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int x = atoi(parameters[0].c_str());
	int y = atoi(parameters[1].c_str());
	int error = ArtemisBin(handle, x, y);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set Binning: X=%d, Y=%d", x, y);
	else
		PrintError("Set Binning", error);
}
void AtikCamerasSDKApp::GetSubFrame()
{
	int x, y, w, h;
	int error = ArtemisGetSubframe(handle, &x, &y, &w, &h);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Current SubFrame: X=%d, Y=%d, Width=%d, Height=%d", x, y, w , h);
	else
		PrintError("Get SubFrame", error);
}
void AtikCamerasSDKApp::SetSubFrame(const string text)
{
	if (SetParameters(text) != 4)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int x = atoi(parameters[0].c_str());
	int y = atoi(parameters[1].c_str());
	int w = atoi(parameters[2].c_str());
	int h = atoi(parameters[3].c_str());
	int error = ArtemisSubframe(handle, x, y, w, h);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set SetSubFrame: X=%d, Y=%d, Width=%d, Height=%d", x, y, w, h);
	else
		PrintError("Set SubFrame", error);
}
void AtikCamerasSDKApp::SetSubFramePos(const string text)
{
	if (SetParameters(text) != 2)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int x = atoi(parameters[0].c_str());
	int y = atoi(parameters[1].c_str());
	int error = ArtemisSubframePos(handle, x, y);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set SubFrame Pos: X=%d, Y=%d", x, y);
	else
		PrintError("SetSubFramePos", error);
}
void AtikCamerasSDKApp::SetSubFrameSize(const string text)
{
	if (SetParameters(text) != 2)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int w = atoi(parameters[0].c_str());
	int h = atoi(parameters[1].c_str());
	int error = ArtemisSubframeSize(handle, w, h);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set SubFrame Size: Width=%d, Heigh=%d", w, h);
	else
		PrintError("SetSubFrameSize", error);
}

void AtikCamerasSDKApp::StartCooling(const string text)
{
	if (nTempSensor == 0)
	{
		ConsoleHelper::WriteLine("No Cooling Available");
		return;
	}		

	if (SetParameters(text) != 1)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int temp = atoi(parameters[0].c_str());

	int error = ArtemisSetCooling(handle, 100 * temp);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Cooling Set to %dC", temp);
	else
		PrintError("Cooling Set", error);
}
void AtikCamerasSDKApp::WarmUp()
{
	if (nTempSensor == 0)
	{
		ConsoleHelper::WriteLine("No Cooling Available");
		return;
	}

	int error = ArtemisCoolerWarmUp(handle);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Cooling Warming up");
	else
		PrintError("Cooling Warming up", error);
}
void AtikCamerasSDKApp::GetTemp() // Info only used by monitor!
{
	if (nTempSensor == 0)
	{
		ConsoleHelper::WriteLine("No Cooling Available");
		return;
	}

	for (int i = 0; i < nTempSensor; i++)
	{
		int temp;
		int error = ArtemisTemperatureSensorInfo(handle, i + 1, &temp);
		if (error == ARTEMIS_OK)
			ConsoleHelper::WriteLine("Sensor: %d   Temp: %.2f", i + 1, (0.01 * temp));
		else
			PrintError("Sensor", error);
	}
}
void AtikCamerasSDKApp::TempMonitor(const string text)
{
	if (nTempSensor == 0)
	{
		ConsoleHelper::WriteLine("No Cooling Available");
		return;
	}

	if (SetParameters(text) != 1)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int seconds = atoi(parameters[0].c_str());

	for (int iTime = 0; iTime < seconds; iTime++)
	{
		for (int i = 0; i < nTempSensor; i++)
		{
			int temp;
			int error = ArtemisTemperatureSensorInfo(handle, i + 1, &temp);
			if (error == ARTEMIS_OK)
				ConsoleHelper::WriteLine("Sensor: %d   Temp: %.2f    (%ds)", i + 1, (0.01 * temp), iTime);
			else
				PrintError("Sensor", error);
		}

		Sleep(1000);
	}
	
}

void AtikCamerasSDKApp::GetFilterWheelPos()
{
	if (nFilterWheelPositions == 0)
	{
		ConsoleHelper::WriteLine("No Filter Wheel Available");
		return;
	}

	int n, moving, current, target;
	int error = ArtemisFilterWheelInfo(handle, &n, &moving, &current, &target);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Current Filter Wheel Position: %d", current);
	else
		PrintError("GetFilterWheelPos", error);
}
void AtikCamerasSDKApp::SetFilterWheelPos(const string text)
{
	if (nFilterWheelPositions == 0)
	{
		ConsoleHelper::WriteLine("No Filter Wheel Available");
		return;
	}

	if (SetParameters(text) != 1)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int target = atoi(parameters[0].c_str());
	
	int error = ArtemisFilterWheelMove(handle, target);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set Filter Wheel Position: %d", target);
	else
		PrintError("SetFilterWheelPos", error);

	while (true)
	{
		int n, moving, current, target;
		int error = ArtemisFilterWheelInfo(handle, &n, &moving, &current, &target);
		if (error != ARTEMIS_OK)
		{
			PrintError("GetFilterWheelPos", error);
			return;
		}
		
		ConsoleHelper::WriteLine("Moving to: %d\tCurrent %d", target, current);

		if (moving == 0)
		{
			ConsoleHelper::WriteLine("Done!");
			return;
		}	

		Sleep(1000);
	}
}

void AtikCamerasSDKApp::Guide(const string text)
{
	if (!canGuide)
	{
		ConsoleHelper::WriteLine("No Guide Port Available");
		return;
	}

	if (SetParameters(text) != 1)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int axis = atoi(parameters[0].c_str());

	int error = ArtemisGuide(handle, axis);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set Guide Port: %d", axis);
	else
		PrintError("Guide", error);

}
void AtikCamerasSDKApp::GuideNSEW(const string text)
{
	if (!canGuide)
	{
		ConsoleHelper::WriteLine("No Guide Port Available");
		return;
	}

	int nibble = 0;

	if (SetParameters(text) == 0)
		ConsoleHelper::WriteLine("No Direction Given");
	else
	{
		string param = parameters[0];
		int    length = param.length();
		for (int i = 0; i < length; i++)
		{
			char d = toupper(param[i]);
			if (d == 'N')
			{
				ConsoleHelper::WriteLine("Has North");
				nibble |= 1;
			}
			else if (d == 'S')
			{
				ConsoleHelper::WriteLine("Has South");
				nibble |= 2;
			}
			else if (d == 'E')
			{
				ConsoleHelper::WriteLine("Has East");
				nibble |= 4;
			}
			else if (d == 'W')
			{
				ConsoleHelper::WriteLine("Has West");
				nibble |= 8;
			}
		}
	}

	int error = ArtemisGuidePort(handle, nibble);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set Guide Port: %d", nibble);
	else
		PrintError("GuidePort", error);
}
void AtikCamerasSDKApp::PulseGuide(const string text)
{
	if (!canGuide)
	{
		ConsoleHelper::WriteLine("No Guide Port Available");
		return;
	}

	if (SetParameters(text) != 2)
	{
		ConsoleHelper::WriteLine("Cannot decode parameters");
		return;
	}

	int axis = atoi(parameters[0].c_str());
	int ms   = atoi(parameters[1].c_str());

	int error = ArtemisPulseGuide(handle, axis, ms);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Set Pulse Guide: %d %dms", axis, ms);
	else
		PrintError("PulseGuide", error);
}
void AtikCamerasSDKApp::StopGuide()
{
	if (!canGuide)
	{
		ConsoleHelper::WriteLine("No Guide Port Available");
		return;
	}

	int error = ArtemisStopGuiding(handle);
	if (error == ARTEMIS_OK)
		ConsoleHelper::WriteLine("Stop Guiding");
	else
		PrintError("StopGuide", error);
}

// Returns number of parameters
int AtikCamerasSDKApp::SetParameters(const string text)
{
	// Method used to find paramters after the command title

	parameters.clear();

	bool isInWord = false;
	int  iStart   = 0;

	int length = text.length();
	for (int i = 0; i < length; i++)
	{
		if (text[i] == ' ')
		{
			if (isInWord && (iStart > 0))
				parameters.push_back(text.substr(iStart, i - iStart));

			isInWord = false;
		}
		else
		{
			if (!isInWord)
			{
				iStart   = i;
				isInWord = true;
			}	
		}
	}

	if (isInWord && (iStart > 0))
		parameters.push_back(text.substr(iStart, length - 1));

	return parameters.size();
}

/*static*/ Command AtikCamerasSDKApp::GetCommand(const string text)
{
	if (StringCompare(text, "End"))
		return Command::End;

	if (StringCompare(text, "Help"))
		return Command::Help;

	if (StringCompare(text, "Camera"))
		return Command::Camera;

	if (StringCompare(text, "Expose"))
		return Command::StartExposure;

	if (StringCompare(text, "GetBin"))
		return Command::GetBin;

	if (StringCompare(text, "SetBin"))
		return Command::SetBin;

	if (StringCompare(text, "GetSF"))
		return Command::GetSubFrame;

	if (StringCompare(text, "SetSF"))
		return Command::SetSubFrame;

	if (StringCompare(text, "SetSFPos"))
		return Command::SetSubFramePos;

	if (StringCompare(text, "SetSFSize"))
		return Command::SetSubFrameSize;

	if (StringCompare(text, "Cool"))
		return Command::StartCooling;

	if (StringCompare(text, "Warm"))
		return Command::WarmUp;

	if (StringCompare(text, "Temp"))
		return Command::GetTemp;

	if (StringCompare(text, "TempMonitor"))
		return Command::TempMonitor;

	if (StringCompare(text, "GetFW"))
		return Command::GetFilterWheelPos;

	if (StringCompare(text, "SetFW"))
		return Command::SetFilterWheelPos;

	if (StringCompare(text, "Guide"))
		return Command::Guide;

	if (StringCompare(text, "GuidePort"))
		return Command::GuideNSEW;

	if (StringCompare(text, "PulseGuide"))
		return Command::PulseGuide;

	if (StringCompare(text, "StopGuide"))
		return Command::StopGuide;

	return Command::UnknownCommand;
}

// Checks the start of s1 (which may also contain parameters) against the whole of s2 (which is the command)
// If s1 is longer than s2, then the next s1 character must be a space (' ').
/*static*/ bool AtikCamerasSDKApp::StringCompare(const string s1, const string s2) 
{
	int size = s2.size();
	if (s1.size() < size)
		return false;

	for (int i = 0; i < size; i++)
	{
		if (toupper(s1[i]) != toupper(s2[i]))
			return false;
	}

	if (size == s1.size())
		return true;

	return s1[size] == ' '; // Check last entry is space for parameters!
}
/*static*/ void AtikCamerasSDKApp::PrintError(const string title, int errorCode)
{
	ConsoleHelper::WriteLine("%s FAILED! Error Code: %d", title.c_str(), errorCode);
}

