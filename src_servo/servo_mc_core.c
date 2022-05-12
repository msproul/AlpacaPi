//******************************************************************************
//*	Name:			servo_mc_core.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Servo motor controller comms and helper functions library in C
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++ and led by Mark Sproul
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.
//*	You must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	Apr 05,	2022	<RNS> Created initial version from LM62x_comm.c
//*	Apr 06,	2022	<RNS> Started on the endian stuff needed for Roboclaw
//*	Apr 07,	2022	<RNS> Pulled out the note/receipt utils to make a test program
//*	Apr 08,	2022	<RNS> Note/receipt now debugged, should support both host endians
//*	Apr 09,	2022	<RNS> Simplified read/write and pulled in helper functions
//*	Apr 20,	2022	<RNS> Debugged a logic issue in MC_read_comm()
//*	Apr 19,	2022	<RNS> Reduced globals for notes and receipts, isolated comm globals
//*	May 06,	2022	<RNS> changed str_to_upper to Time_str_to_upper and renamed file servo_mc_core.c
//*	May 07,	2022	<RNS> reswizzled some setting in set_attr termios setting to not overlap with raw
//*	May 07,	2022	<RNS> Modified the calc_crc16 function to take in a crc value
//*****************************************************************************

#include	<stdio.h>
#include	<string.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<termios.h>
#include	<unistd.h>
#include	<stdint.h>
#include	<endian.h>
#include	<errno.h>
#include	"servo_std_defs.h"
#include	"servo_mc_core.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


// Endian services to handle both endians from host side
#define _MC_BIG_ENDIAN_
#ifdef _MC_BIG_ENDIAN_
	#define encode4Bytes(x) htobe32(x)
	#define encode2Bytes(x) htobe16(x)
	#define decode4Bytes(x) be32toh(x)
	#define decode2Bytes(x) be16toh(x)
#else
	#define encode4Bytes(x) (x)
	#define encode2Bytes(x) (x)
	#define decode4Bytes(x) (x)
	#define decode2Bytes(x) (x)
#endif	// Endian

#define kMAX_RETRY 3

// Handcoded Command 16 - Read Encoder Count M1
// Send: [Address, 16]
// Receive: [Enc1(4 bytes), Status, CRC(2 bytes)str_to]
uint8_t cmd16SendStr[32]	=	{0x80, 16};
// uint8_t cmd16RecStr[32]	=	{0x01, 0x02, 0x03, 0x04, 0x00, 0x2D, 0xAD};
uint8_t cmd16RecStr[32]		=	{0x05, 0x04, 0x03, 0x02, 0x00, 0x94, 0xD6};

// Handcoded Command  22 - Set Quad Encoder 1
// Send: [Address, 22, Value(4 bytes), CRC(2 bytes)]
// Receive: [0xFF]
// Write value	=	0x06070605
uint8_t cmd22SendStr[32]	=	{0x80, 22, 0x05, 0x06, 0x07, 0x06, 0x4B, 0xEA};
uint8_t cmd22RecStr[32]		=	{0xFF};

// Internal global variables visibel for this file only
static int gCommPort;
static int gCommRetries;

//*****************************************************************************
// Resets to the top of buffer, and add MC target/cmd and updates rover
//*****************************************************************************
void Note_init(uint8_t *buf, uint8_t addr, uint8_t cmd, uint8_t **rover)
{
uint8_t *tempPtr;

	tempPtr		=	buf;
	*tempPtr++	=	addr;
	*tempPtr++	=	cmd;
	*rover		=	tempPtr;
}

//*****************************************************************************
// Appends a byte to note buffer, set the rover ptr for the next append
//*****************************************************************************
void Note_add_byte(uint8_t *buf, uint8_t arg, uint8_t **rover)
{
uint8_t	*tempPtr;

	tempPtr		=	buf;
	*tempPtr++	=	arg;
	*rover		=	tempPtr;
}

//*****************************************************************************
//  Encodes and copies 2bytes arg to buffer, moves rover for next append call
//*****************************************************************************
void Note_add_word(uint8_t *buf, uint16_t arg, uint8_t **rover)
{
uint16_t *argPtr;

	argPtr	=	(uint16_t *)buf;
	*argPtr	=	encode2Bytes(arg);
	*rover	=	buf + sizeof(arg);
}

//*****************************************************************************
// Copies 4bytes arg to buffer, moves rover for next append call
//*****************************************************************************
void Note_add_dword(uint8_t *buf, uint32_t arg, uint8_t **rover)
{
uint32_t *argPtr;

	argPtr	=	(uint32_t *)buf;
	*argPtr	=	encode4Bytes(arg);
	*rover	=	buf + sizeof(arg);
}

//*****************************************************************************
// returns a decoded byte from the current receipt buf position, updates rover
//*****************************************************************************
uint8_t Receipt_get_byte(uint8_t *buf, uint8_t **rover)
{
uint8_t *argPtr;

	argPtr	=	(uint8_t *)buf;
	*rover	=	buf + sizeof(uint8_t);
	return(*argPtr);
}

//*****************************************************************************
// returns a decoded word from the current receipt buf position, updates rover
//*****************************************************************************
uint16_t Receipt_get_word(uint8_t *buf, uint8_t **rover)
{
uint16_t *argPtr;

	argPtr	=	(uint16_t *)buf;
	*rover	=	buf + sizeof(uint16_t);
	return(decode2Bytes(*argPtr));
}

//******************************************************************************
// returns a decoded dword from the current receipt buf position, updates rover
//******************************************************************************
uint32_t Receipt_get_dword(uint8_t *buf, uint8_t **rover)
{
uint32_t *argPtr;

	argPtr	=	(uint32_t *)buf;
	*rover	=	buf + sizeof(uint32_t);
	return(decode4Bytes(*argPtr));
}

//******************************************************************************
// Sums a unsigned array of len long and returns a one byte checksum
//******************************************************************************
uint8_t MC_calc_checksum(uint8_t *data, int len)
{
uint8_t	sum	=	0;
int		iii;

	for (iii = 0; iii < len; iii++)
	{
		sum	+=	data[iii];
	}
	return(sum);
}	//* of mc__calc_checksum

//******************************************************************************
// Calc the specific flavor of CRC16 needed for a RoboClaw... and possibly others
//******************************************************************************
uint16_t MC_calc_crc16(unsigned char *packet, int numBytes, uint16_t crc)
{
int byte;
uint8_t bit;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (byte = 0; byte < numBytes; byte++)
	{
		crc	=	crc ^ ((unsigned int)packet[byte] << 8);
		for (bit = 0; bit < 8; bit++)
		{
			if (crc & 0x8000)
			{
				crc	=	(crc << 1) ^ 0x1021;
			}
			else
			{
				crc	=	crc << 1;
			}
		}
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
	return(crc);
}

//**************************************************************************
// set_sio_attr sets the tty port to 8 data, 1 stop and no parity and raw mode
// supported bauds are 9.6K, 19.2K and 38.4K, invalid baud rates default to 9.6k
//**************************************************************************
int MC_set_comm_attr(int portFd, int baudrate)
{
	struct termios settings;
	int status;

	// Get the current terminal settings
	status	=	tcgetattr(portFd, &settings);
	if (status < 0)
	{
		return(kERROR);
	}

	// Setup the terminal control settings to a very primitive level (RAW)
	//  c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	//  c_oflag &= ~OPOST;
	//  c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	//  c_cflag &= ~(CSIZE|PARENB);
	cfmakeraw(&settings);

	// Enable the receiver, set local mode, 8bit
	settings.c_cflag	|=	(CLOCAL | CREAD | CS8);
	settings.c_cflag	&=	~CSTOPB;  // 1 stop bit
	settings.c_cflag	&=	~CRTSCTS; // Disable hardware flow control

	// Set the read timeout values
	settings.c_cc[VMIN]		=	0;
	settings.c_cc[VTIME]	=	1;

	// Set the input modes and disable any SW control flow
	settings.c_iflag &= ~(IXON | IXOFF | IXANY);

	// Set the output modes not needed after cfmakeraw()

	// Set baud rate or default to 9600
	switch (baudrate)
	{
	case 9600:
		cfsetispeed(&settings, B9600);
		cfsetospeed(&settings, B9600);
		break;
	case 19200:
		cfsetispeed(&settings, B19200);
		cfsetospeed(&settings, B19200);
		break;
	case 38400:
		cfsetispeed(&settings, B38400);
		cfsetospeed(&settings, B38400);
		break;
	default:
		cfsetispeed(&settings, B9600);
		cfsetospeed(&settings, B9600);
	}

	// Flush both the input and output buffers
	tcflush(gCommPort, TCIOFLUSH);

	// Set the terminal settings
	status	=	tcsetattr(portFd, TCSANOW, &settings);
	if (status < 0)
	{
		perror("Error in MC_set_comm_attr: ");
		return(kERROR);
	}
	return(kSTATUS_OK);
} // MC_set_comm_attr()

//**********************************************************************
//  Reads data from comm port in the buffer, returns kERROR if retries exceeded
// in[];   input data to be written
// len;     length on input data
//**********************************************************************
int MC_read_comm(uint8_t *buf, size_t len)
{
uint8_t	retryBuf[kMAX_STR_LEN];
uint8_t	*ptrA;
ssize_t	size;
ssize_t	total	=	0;
int		retValue;

	CONSOLE_DEBUG(__FUNCTION__);
	// Attempt to the read the message and set buffer ptr just-in-case
	ptrA			=	retryBuf;
	gCommRetries	=	kMAX_RETRY;

	// Loop to make sure the port does not return an error up to max retry
	do
	{
		size	=	read(gCommPort, buf, len);
		if (size < 0)
		{
			perror("Error in MC_read_comm: ");
		}
		gCommRetries--;
	} while (size <= 0 && gCommRetries > 0);

	printf("MC_read_comm: len = %d  and size = %d\n", (int) len, (int) size);

	// If the read len doesn't meet the expected length and we still have retrys left
	while ((size + total != len) && (gCommRetries > 0))
	{
		// Append the current read to the tempP
		memcpy(ptrA, buf, size);
		total += size;

		ptrA	+=	size;
		size	=	read(gCommPort, buf, len);
		gCommRetries--;
	}

	// If > max retries then error
	if (size < 0 || gCommRetries < 0)
	{
		printf("MC_read_comm: len = %d  and size = %d  retries = %d\n", (int) len, (int) size, gCommRetries);
		retValue	=	kERROR;
	}
	else
	{
		// If we had a retry attempt, need return the data from retryBuf
		if (ptrA != retryBuf)
		{
			memcpy(buf, retryBuf, total);
		}
		retValue	=	((size > total) ? size : total);
	}
	return(retValue);
} // of MC_read_comm()

//**********************************************************************
//  Write the buffer to the comm port and returns kERROR if retries exceeded
// in[];   input data to be written
// len;     length on input data
//**********************************************************************
int MC_write_comm(uint8_t *buf, size_t len)
{
ssize_t count;
int		returnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	gCommRetries	=	kMAX_RETRY;
	// Write out all data ininput buf to the commport
	tcflush(gCommPort, TCIFLUSH);
	count	=	write(gCommPort, buf, len);
	while ((count != len) && (gCommRetries > 0))
	{
		count	=	write(gCommPort, buf, len);
		gCommRetries--;
	}
	// If > max retries then error
	if (gCommRetries < 0)
	{
		returnCode	=	kERROR;
	}
	else
	{
		returnCode	=	kSTATUS_OK;
	}
	return(returnCode);
} // of MC_write_comm()

//**********************************************************************
//  Open() the comm port, pause because it's likely a USB device and
//  call attr function and returns kERROR if unsuccessful
// in[];   input data to be written
// len;     length on input data
//**********************************************************************
int MC_init_comm(char *pathName, int baud)
{
//uint8_t	buf[256];
int		status;

	// if ((gCommPort = open(pathName, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
	gCommPort	=	open(pathName, O_RDWR);
	if (gCommPort == -1)
	{
		perror("Error in MC_init_comm: ");
		// Device open failed, return neg
		return(kERROR);
	}
	// Wait for any lingering data to get to the buffer
	usleep(10000);

	status	=	MC_set_comm_attr(gCommPort, baud);
	tcflush(gCommPort, TCIOFLUSH);

	// set the globe SIO retires
	gCommRetries	=	kMAX_RETRY;

	return(status);
} // of MC_init_comm()

//********************************************************************************
// Stops all MC activitiy and releases the commm port
//********************************************************************************
int MC_shutdown(void)
{

	// Add command stuff here when it exists
	// Add command stuff here when it exists
	// Add command stuff here when it exists

	// release the port
	close(gCommPort);

	return(kSTATUS_OK);
} // of MC_shutdown()

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//#define _TEST_SERVO_MC_CORE_
#ifdef _TEST_SERVO_ME_CORE_
int	main(int argc, char **argv)
{
uint32_t	pat32		=	0x12345678;
//uint32_t	*ptr32		=	&pat32;
uint32_t	*uPtr32		=	(uint32_t *)&pat32;
uint16_t	pat16		=	0x6789;
//uint16_t	*ptr16		=	&pat16;
uint16_t	*uPtr16		=	(uint16_t *)&pat16;
uint8_t		noteBuf[64], receiptBuf[64], *ptrA, *ptrB;
uint8_t		pat8		=	0xAB;
uint8_t		buf[128];
int			iii;

	CONSOLE_DEBUG(__FUNCTION__);
	// Test encode decode macros for endianness (IA = le, RPI = be)
	printf("Servo core test program for Roboclaw interface:\n");
	printf("Encode:\n");
	printf("    pat32 = %X    encoded pat32 = %X\n", pat32, encode4Bytes(pat32));
	printf("    pat16 = %X    encoded pat16 = %X\n", pat16, encode2Bytes(pat16));
	printf("    uPrt32 = %X    encoded uPrt32 = %X\n", *uPtr32, encode4Bytes(*uPtr32));
	printf("    uPrt16 = %X    encoded uPrt16 = %X\n", *uPtr16, encode2Bytes(*uPtr16));

	printf("Decode:\n");
	printf("    pat32 = %X    decoded pat32 = %X\n", pat32, decode4Bytes(pat32));
	printf("    pat16 = %X    decoded pat16 = %X\n", pat16, decode2Bytes(pat16));
	printf("    uPrt32 = %X    decoded uPrt32 = %X\n", *uPtr32, decode4Bytes(*uPtr32));
	printf("    uPrt16 = %X    decoded uPrt16 = %X\n", *uPtr16, decode2Bytes(*uPtr16));

	// Check data before use
	printf("\nDumping note buffer to see if it's empty\n");
	ptrA	=	noteBuf;
	for (iii = 0; iii < 64; iii++)
	{
		if (iii % 8 == 0)
		{
			printf("\n");
		}
		printf("%2X   ", *ptrA);

		ptrA++;
	}
	printf("\n");

	// Test the note / receipt buffer functionality
	Note_init(noteBuf, 0x80, 0xEE, &ptrA);
	Note_add_byte(ptrA,		pat8,	&ptrB);
	Note_add_word(ptrB,		pat16,	&ptrA);
	Note_add_byte(ptrA,		pat8,	&ptrB);
	Note_add_dword(ptrB,	pat32,	&ptrA);
	Note_add_byte(ptrA,		pat8,	&ptrB);
	Note_add_word(ptrB,		pat16,	&ptrA);

	// Check data got swizzled for big endianness
	printf("\nDumping note buffer after appends\n");
	ptrA	=	noteBuf;
	for (iii = 0; iii < 64; iii++)
	{
		if (iii % 8 == 0)
		{
			printf("\n");
		}
		printf("%2X   ", *ptrA);

		ptrA++;
	}
	printf("\n");

	// Copy notes to receipts
	memcpy(receiptBuf, noteBuf, 64);

	printf("Dumping receipt array after note array was copied\n");
	printf("  Addr  = %X -> 0x80\n", Receipt_get_byte(receiptBuf, &ptrA));
	printf("  Cmd   = %X -> 0xEE\n", Receipt_get_byte(ptrA, &ptrB));
	printf("  Byte  = %X  -> pat = %X\n", Receipt_get_byte(ptrB, &ptrA), pat8);
	printf("  Word  = %X  -> pat = %X\n", Receipt_get_word(ptrA, &ptrB), pat16);
	printf("  Byte  = %X  -> pat = %X\n", Receipt_get_byte(ptrB, &ptrA), pat8);
	printf("  dword = %X  -> pat = %X\n", Receipt_get_dword(ptrA, &ptrB), pat32);
	printf("  Byte  = %X  -> pat = %X\n", Receipt_get_byte(ptrB, &ptrA), pat8);
	printf("  word  = %X  -> pat = %X\n", Receipt_get_word(ptrA, &ptrB), pat16);

	iii	=	MC_init_comm("/dev/ttyACM0", 38400);
	printf("MC_init_comm return status = %d\n", iii);
	iii	=	MC_write_comm("hello", 5);
	printf("MC_write_comm return status = %d\n", iii);
	iii	=	MC_read_comm(buf, 0);
	printf("MC_read_comm return status = %d\n", iii);
	MC_shutdown();

}
#endif // _TEST_SERVO_ME_CORE_
