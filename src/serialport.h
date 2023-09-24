//*****************************************************************************

//#include	"serialport.h"


#ifdef __cplusplus
	extern "C" {
#endif


int		Serial_Set_Attribs(int fd, int speed, int parity);
void	Serial_Set_Blocking (int fd, int should_block);
int		Serial_Set_RTS(int fd, bool rtsState);
int		Serial_Send_Data(int fd, const char *xmitData, bool waitFlag);
int		Serial_Read_Data(int fd, char *recvData, size_t maxDataLen);



#ifdef __cplusplus
}
#endif