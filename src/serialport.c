//*****************************************************************************
//*	Feb 11,	2020	<MLS> Created serialport.c
//*	Mar 31,	2021	<MLS> Added Serial_Set_Blocking()
//*	Mar 31,	2021	<MLS> Changed Set_Serial_attribs() to Serial_Set_Attribs()
//*	Mar 31,	2021	<MLS> Added Serial_Send_Data()
//*****************************************************************************

#include	<errno.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<stdbool.h>
#include	<string.h>
#include	<unistd.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"serialport.h"


//*****************************************************************************
//	https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
//*****************************************************************************
int	Serial_Set_Attribs(int fd, int speed, int parity)
{
struct termios	tty;
int				tcReturnCode;
int				myReturnCode;

	if (tcgetattr (fd, &tty) != 0)
	{
		CONSOLE_DEBUG_W_NUM ("error from tcgetattr", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag		=	(tty.c_cflag & ~CSIZE) | CS8;		// 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag		&=	~IGNBRK;		// disable break processing
	tty.c_lflag		=	0;				// no signaling chars, no echo,
										// no canonical processing
	tty.c_oflag		=	0;				// no remapping, no delays
	tty.c_cc[VMIN]	=	0;				// read doesn't block
	tty.c_cc[VTIME]	=	2;				// read timeout in tenths of a second (decaseconds, 5 = 1/2 second)
//	tty.c_cc[VTIME]	=	10;				// read timeout in tenths of a second (decaseconds, 5 = 1/2 second)

	tty.c_iflag		&=	~(IXON | IXOFF | IXANY);	// shut off xocn/xoff ctrl

	tty.c_cflag		|=	(CLOCAL | CREAD);			// ignore modem controls,
													// enable reading
	tty.c_cflag		&=	~(PARENB | PARODD);			// shut off parity
	tty.c_cflag		|=	parity;
	tty.c_cflag		&=	~CSTOPB;
	tty.c_cflag		&=	~CRTSCTS;

	myReturnCode	=	0;

	tcReturnCode	=	tcsetattr (fd, TCSANOW, &tty);
	if (tcReturnCode != 0)
	{
		CONSOLE_DEBUG_W_NUM("error from tcsetattr", errno);
		myReturnCode	=	-1;
	}
	tcReturnCode	=	tcflush(fd, TCOFLUSH);
	if (tcReturnCode != 0)
	{
		CONSOLE_DEBUG_W_NUM("error from tcflush", errno);
		myReturnCode	=	-1;
	}
	return(myReturnCode);
}



//*****************************************************************************
void	Serial_Set_Blocking(int fd, int should_block)
{
struct termios tty;

	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		CONSOLE_DEBUG_W_NUM ("error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN]	=	should_block ? 1 : 0;
//	tty.c_cc[VTIME]	=	5;			// 0.5 seconds read timeout
	tty.c_cc[VTIME]	=	20;			// 0.5 seconds read timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		CONSOLE_DEBUG_W_NUM ("error %d setting term attributes", errno);
	}
}

//*****************************************************************************
int	Serial_Send_Data(int fd, const char *xmitData, bool waitFlag)
{
ssize_t	bytesWritten;
size_t	byteCount;

//	CONSOLE_DEBUG_W_STR("Sending\t=", xmitData);

	byteCount		=	strlen(xmitData);
	bytesWritten	=	write (fd, xmitData, byteCount);	// send 7 character greeting

	if (waitFlag)
	{
		usleep (byteCount * 100);	// sleep enough to transmit the data, approx 100 uS per char transmit
	}
	return(bytesWritten);
}

//*****************************************************************************
int	Serial_Read_Data(int fd, char *recvData, size_t maxDataLen)
{
ssize_t	bytesRead;

	bytesRead	=	read (fd, recvData, maxDataLen);

	return(bytesRead);
}
