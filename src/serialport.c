//*****************************************************************************
//*	Feb 11,	2020	<MLS> Created serialport.c
//*****************************************************************************

#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"serialport.h"


//*****************************************************************************
//	https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
//*****************************************************************************
int	Set_Serial_attribs(int fd, int speed, int parity)
{
struct termios tty;
int		tcReturnCode;
int		myReturnCode;

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

