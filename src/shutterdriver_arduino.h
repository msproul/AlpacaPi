//*****************************************************************************
//#include	"shutterdriver_arduino.h"

#ifndef _SHUTTER_ARDUINO_H_
#define	_SHUTTER_ARDUINO_H_


#ifndef _SHUTTER_DRIVER_H_
	#include	"shutterdriver.h"
#endif

#define	kArduino_LogBufferSize	4096
#define	kArduino_LineBuffSize	256

//*****************************************************************************
class ShutterArduino: public ShutterDriver
{
	public:

		//
		// Construction
		//
						ShutterArduino(const int argDevNum);
		virtual			~ShutterArduino(void);
		virtual void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	void	Init_Hardware(void);

		virtual	int32_t	RunStateMachine(void);
	protected:
		bool	OpenArduinoConnection(void);
		bool	SendCommand(const char *theCommand);
//		int		ReadUntilChar(const int fd, char *readBuff, const int maxChars, const char terminator);
		void	ProcessArduinoLine(char *readBuffer);
		void	ProcessArduinoState(char *stateString);
		void	ReadArduinoData(void);

		virtual	bool	OpenShutter(void);
		virtual	bool	CloseShutter(void);
		virtual	bool	StopShutter(void);
		virtual	void	WatchDog_TimeOut(void);

//				void	GetArduinoLog(void);

				int		cArduinoFileDesc;				//*	port file descriptor
				char	cArduinoLineBuf[kArduino_LineBuffSize];
				int		cArduinoByteCnt;

				char	cLogBuffer[kArduino_LogBufferSize];
				int		cLogLineCnt;
				int		cNoDataCnt;

				int		cArduinoSkipCnt;

};

void	CreateShuterArduinoObjects(void);

#endif	//	_SHUTTER_ARDUINO_H_
