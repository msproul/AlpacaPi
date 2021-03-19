using System;
using System.Runtime.InteropServices;
using System.Text;

namespace DotNetExample
{
    public enum ArtemisCameraConnectionState { Connecting = 1, Connected = 2, ConnectFailed = 3, Suspended = 4, Unknown = 5 }
    public enum ArtemisCameraState { Error = -1, Idle = 0, Waiting = 1, Exposing = 2, Reading = 3, Downloading = 4, Flushing = 5 }
    public enum ArtemisPrechargeMode { None = 0, ICPS = 1, Full = 2 }
    public enum ArtemisCoolingStatus { Off, Cooling, WarmingUp, Error, Unknown }
    public enum ArtemisCoolingType { None, OnOff, Power, SetPoint, Unknown }
    public enum ArtemisError { OK = 0, InvalidParameters = 1, NotConnected = 2, NotImplemented = 3, NoResponse = 4, InvalidFunction = 5, NotInitialized = 6, OperationFailed = 7, InvalidPassword = 8 }
    public enum ArtemisColourType { ARTEMIS_COLOUR_UNKNOWN, ARTEMIS_COLOUR_NONE, ARTEMIS_COLOUR_RGGB }
    public enum ArtemisExternalFilterWheelType { None = 0, EFW1 = 1, EFW2 = 2, IFW = 3 }
    public enum AtikCameraSpecificOptions
    {
        ID_GOPresetMode = 1,
        ID_GOPresetLow = 2,
        ID_GOPresetMed = 3,
        ID_GOPresetHigh = 4,
        ID_GOCustomGain = 5,
        ID_GOCustomOffset = 6,

        ID_EvenIllumination = 12,
        ID_PadData = 13,
        ID_ExposureSpeed = 14,
        ID_BitSendMode = 15,
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ArtemisSetFastCallback(IntPtr handle, int x, int y, int w, int h, int binx, int biny, IntPtr imageBuffer);

    [Flags]
    public enum ArtemisProcessing { Error = -1, None = 0, Linearise = 1, VBE = 2 }

    [StructLayout(LayoutKind.Sequential)]
    public struct ArtemisPropertiesWrapper
    {
        public int Protocol;
        public int NPixelsX;
        public int NPixelsY;
        public float PixelMicronsX;
        public float PixelMicronsY;
        public int CCDFlags;
        public int Cameraflags;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
        public String Description;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
        public String Manufacturer;
    }

    public static class AtikPInvoke
    {
        const String DLLPath = "AtikCameras.dll";

        [DllImport(DLLPath)]
        public static extern int ArtemisShutdown();

        // ----------------- DLL ------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisAPIVersion();

        [DllImport(DLLPath)]
        public static extern int ArtemisDLLVersion();

        // ------------------- Connect / Disconnect ---------------------------
        [DllImport(DLLPath)]
        public static extern IntPtr ArtemisConnect(int device);

        [DllImport(DLLPath)]
        public static extern IntPtr ArtemisQuickConnect(int device);

        [DllImport(DLLPath)]
        public static extern int ArtemisCameraConnectionState(IntPtr handle, ref ArtemisCameraConnectionState state);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisIsConnected(IntPtr handle);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDisconnect(IntPtr handle);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDisconnectAll();

        // -------------------------- Camera Info -------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisCameraSerial(IntPtr handle, out int flags, out int serial);

        [DllImport(DLLPath)]
        public static extern int ArtemisProperties(IntPtr handle, ref ArtemisPropertiesWrapper properties);

        [DllImport(DLLPath)]
        public static extern int ArtemisColourProperties(IntPtr handle, ref ArtemisColourType colourType, ref int normalOffsetX, ref int normalOffsetY, ref int previewOffsetX, ref int previewOffsetY);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDeviceName(int device, StringBuilder builder);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDeviceSerial(int device, StringBuilder builder);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDeviceIsCamera(int device);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDeviceHasFilterWheel(int device);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisDevicePresent(int device);

        [DllImport(DLLPath)]
        public static extern int ArtemisDeviceGetVIDPID(int device, out int vid, out int pid);

        [DllImport(DLLPath)]
        public static extern int ArtemisRefreshDevicesCount();

        // ------------------------ Column Repair ---------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisGetColumnRepairColumns(IntPtr handle, ref int nColumn, [In, Out] ushort[] columns);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetColumnRepairColumns(IntPtr handle, int nColumn, IntPtr columns);

        [DllImport(DLLPath)]
        public static extern int ArtemisClearColumnRepairColumns(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetColumnRepairFixColumns(IntPtr handle, bool fix);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetColumnRepairFixColumns(IntPtr handle, ref bool fix);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetColumnRepairCanFixColumns(IntPtr handle, ref bool fix);

        // --------------------- Exposures ----------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisStartExposure(IntPtr handle, float seconds);

        [DllImport(DLLPath)]
        public static extern int ArtemisStartExposureMS(IntPtr handle, int ms);

        [DllImport(DLLPath)]
        public static extern int ArtemisAbortExposure(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisStopExposure(IntPtr handle);

        [DllImport(DLLPath)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ArtemisImageReady(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisCameraState(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern float ArtemisExposureTimeRemaining(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisDownloadPercent(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetImageData(IntPtr handle, ref int x, ref int y, ref int width, ref int height, ref int binx, ref int biny);

        [DllImport(DLLPath)]
        public static extern IntPtr ArtemisImageBuffer(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern float ArtemisLastExposureDuration(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern IntPtr ArtemisLastStartTime(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisLastStartTimeMilliseconds(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern bool ArtemisHasFastMode(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern bool ArtemisStartFastExposure(IntPtr handle, int ms);

        [DllImport(DLLPath)]
        public static extern bool ArtemisSetFastCallback(IntPtr handle, ArtemisSetFastCallback func);



        // --------------------- Temperature ---------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisTemperatureSensorInfo(IntPtr handle, int sensor, ref int temperature);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetCooling(IntPtr handle, int setPoint);

        [DllImport(DLLPath)]
        public static extern int ArtemisCoolingInfo(IntPtr handle, ref int flags, ref int level, ref int minLevel, ref int maxLevel, ref int setPoint);

        [DllImport(DLLPath)]
        public static extern int ArtemisCoolerWarmUp(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetWindowHeaterPower(IntPtr handle, ref int windowHeaterPower);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetWindowHeaterPower(IntPtr handle, int windowHeaterPower);

        // ------------------------ Gain ----------------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisGetGain(IntPtr handle, bool isPreview, ref int gain, ref int offset);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetGain(IntPtr handle, bool isPreview, int gain, int offset);

        // ------------------------ Amplifier ----------------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisAmplifier(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern bool ArtemisGetAmplifierSwitched(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetAmplifierSwitched(IntPtr handle, bool isOn);

        // ------------------ Exposure Settings ----------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisBin(IntPtr handle, int x, int y);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetBin(IntPtr handle, ref int x, ref int y);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetMaxBin(IntPtr handle, ref int x, ref int y);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetSubframe(IntPtr handle, ref int x, ref int y, ref int w, ref int h);

        [DllImport(DLLPath)]
        public static extern int ArtemisSubframe(IntPtr handle, int x, int y, int w, int h);

        [DllImport(DLLPath)]
        public static extern int ArtemisSubframePos(IntPtr handle, int x, int y);

        [DllImport(DLLPath)]
        public static extern int ArtemisSubframeSize(IntPtr handle, int w, int h);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetSubSample(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern bool ArtemisGetContinuousExposingMode(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetContinuousExposingMode(IntPtr handle, bool bEnable);

        [DllImport(DLLPath)]
        public static extern bool ArtemisGetDarkMode(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetDarkMode(IntPtr handle, bool bEnable);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetPreview(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern int ArtemisAutoAdjustBlackLevel(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern int ArtemisPrechargeMode(IntPtr handle, int mode);

        [DllImport(DLLPath)]
        public static extern int ArtemisEightBitMode(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern int ArtemisStartOverlappedExposure(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern bool ArtemisOverlappedExposureValid(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetOverlappedExposureTime(IntPtr handle, float seconds);

        [DllImport(DLLPath)]
        public static extern int ArtemisTriggeredExposure(IntPtr handle, bool isOn);

        [DllImport(DLLPath)]
        public static extern ArtemisProcessing ArtemisGetProcessing(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetProcessing(IntPtr handle, int options);

        // --------------------------  Guiding:  -----------------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisGuidePort(IntPtr handle, int nibble);

        [DllImport(DLLPath)]
        public static extern int ArtemisGuide(IntPtr handle, int axis);

        [DllImport(DLLPath)]
        public static extern int ArtemisPulseGuide(IntPtr handle, int axis, int ms);

        [DllImport(DLLPath)]
        public static extern int ArtemisStopGuiding(IntPtr handle);

        // -------------------------  Lens  -------------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisInitializeLens(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetLensLimits(IntPtr handle, ref int apertureMin, ref int apertureMax, ref int focusMin, ref int focusMax);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetLensAperture(IntPtr handle, ref int aperature);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetLensAperture(IntPtr handle, int aperature);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetLensFocus(IntPtr handle, ref int focus);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetLensFocus(IntPtr handle, int focus);

        // ------------------------- Camera Control -------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisClearVRegs(IntPtr handle);

        // ------------------------ Overlapped Exposure ---------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisHasOverlappedExposure(IntPtr handle, ref bool hasOverlappedExposure, ref bool hasShort);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetOverlappedNone(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetOverlappedMS(IntPtr handle, float ms);

        [DllImport(DLLPath)]
        public static extern int ArtemisSetOverlappedLines(IntPtr handle, int lines);

        [DllImport(DLLPath)]
        public static extern int ArtemisGetOverlappedGap(IntPtr handle, ref float gap);


        // ---------------------- Shutter --------------------------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisCanControlShutter(IntPtr handle, ref bool canControl);

        [DllImport(DLLPath)]
        public static extern int ArtemisOpenShutter(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisCloseShutter(IntPtr handle);

        // ----------------- Internal Filter Wheel ----------------------
        [DllImport(DLLPath)]
        public static extern int ArtemisFilterWheelInfo(IntPtr handle, ref int numFilters, ref int moving, ref int currentPos, ref int targetPos);

        [DllImport(DLLPath)]
        public static extern int ArtemisFilterWheelMove(IntPtr handle, int targetPos);

        // ---------------- External Filter Wheel ------------------------
        [DllImport(DLLPath)]
        public static extern bool ArtemisEFWIsPresent(int i);

        [DllImport(DLLPath)]
        public static extern IntPtr ArtemisEFWConnect(int i);

        [DllImport(DLLPath)]
        public static extern int ArtemisEFWDisconnect(IntPtr handle);

        [DllImport(DLLPath)]
        public static extern int ArtemisEFWGetDetails(IntPtr handle, ref ArtemisExternalFilterWheelType type, StringBuilder serialNumber);

        [DllImport(DLLPath)]
        public static extern int ArtemisEFWNmrPosition(IntPtr handle, ref int nPosition);

        [DllImport(DLLPath)]
        public static extern int ArtemisEFWSetPosition(IntPtr handle, int iPosition);

        [DllImport(DLLPath)]
        public static extern int ArtemisEFWGetPosition(IntPtr handle, ref int iPosition, ref bool isMoving);

        // -------------------------- Camera Specific Options -------------------
        [DllImport(DLLPath)]
        public static extern bool ArtemisHasCameraSpecificOption(IntPtr handle, ushort id);

        [DllImport(DLLPath)]
        public static extern int ArtemisCameraSpecificOptionGetData(IntPtr handle, ushort id, [In, Out] byte[] data, int dataLength, ref int actualLength);

        [DllImport(DLLPath)]
        public static extern int ArtemisCameraSpecificOptionSetData(IntPtr handle, ushort id, [In, Out] byte[] data, int dataLength);
    }
}