//******************************************************************************
//*	Name:			servo_mc_core.h
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
//  Apr 12, 2022    <RNS> Initial creation via cproto
//  Apr 20, 2022    <RNS> Cleaned up global variables declarations
//  May 06, 2022    <RNS> renamed file to servo_mc_core.h and fixed #defines
//*****************************************************************************
#ifndef _SERVO_MC_CORE_H_
#define _SERVO_MC_CORE_H_

#define kCLEAR_CRC 0

void		Note_init(uint8_t *buf, uint8_t addr, uint8_t cmd, uint8_t **rover);
void		Note_add_byte(uint8_t *buf, uint8_t arg, uint8_t **rover);
void		Note_add_word(uint8_t *buf, uint16_t arg, uint8_t **rover);
void		Note_add_dword(uint8_t *buf, uint32_t arg, uint8_t **rover);
uint8_t		Receipt_get_byte(uint8_t *buf, uint8_t **rover);
uint16_t	Receipt_get_word(uint8_t *buf, uint8_t **rover);
uint32_t	Receipt_get_dword(uint8_t *buf, uint8_t **rover);
uint8_t		MC_calc_checksum(uint8_t *data, int len);
uint16_t	MC_calc_crc16(unsigned char *packet, int numBytes, uint16_t crc);
int			MC_set_comm_attr(int port, int baudrate);
int			MC_read_comm(uint8_t *buf, size_t len);
int			MC_write_comm(uint8_t *buf, size_t len);
int			MC_init_comm(char *com, int baud);
int			MC_shutdown(void);

#endif
