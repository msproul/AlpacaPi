#include <stdio.h>
#include <stdlib.h>

#define	kMaxLineLen	10000
//*********************************************************************
int main()
{
int		theChar;
int		theChar2;
int		theChar3;
int		theChar4;
char	lineBuff[kMaxLineLen + 100];
int		ccc;
int		replaceCnt;
int		charCnt;
int		brktCntInLine;

	printf("\n");
	printf("#JSON data reformatted by %s, written by Mark Sproul, msproul@skychariot.com\r\n", __FILE__);
	printf("#The JSON data must be in this format to be understood by SkyTravel\n");

	theChar			=	0;
	ccc				=	0;
	replaceCnt		=	0;
	charCnt			=	0;
	brktCntInLine	=	0;
	while (theChar != EOF)
	{
		theChar	=	getc(stdin);
		if (theChar != EOF)
		{
			charCnt++;
			if (theChar == '\\')
			{
				theChar2	=	getc(stdin);
				theChar3	=	getc(stdin);
				theChar4	=	getc(stdin);
				charCnt		+=	3;
				if ((theChar2 == 'r') && (theChar3 == '\\') && (theChar4 == 'n'))
				{
					lineBuff[ccc++]	=	0;
					printf("%s\r\n", lineBuff);
					ccc		=	0;
					replaceCnt++;
					brktCntInLine	=	0;	//	reset for new line
				}
				else if (ccc < kMaxLineLen)
				{
					lineBuff[ccc++]	=	theChar;
					lineBuff[ccc++]	=	theChar2;
					lineBuff[ccc++]	=	theChar3;
					lineBuff[ccc++]	=	theChar4;
				}
			}
			else if (theChar == '{')
			{
				lineBuff[ccc++]	=	0;
				printf("%s\r\n", lineBuff);
				printf("\t{\r\n");

				ccc				=	0;
				lineBuff[ccc++]	=	0x09;
				lineBuff[ccc++]	=	0x09;

				brktCntInLine	=	0;	//	reset for new line

			}
			else if (theChar == '}')
			{
				lineBuff[ccc++]	=	0;
				printf("%s\r\n", lineBuff);

			//	printf("\t}\r\n");

				ccc				=	0;
				lineBuff[ccc++]	=	0x09;
				lineBuff[ccc++]	=	theChar;

				brktCntInLine	=	0;	//	reset for new line

			}
			else if (theChar == ',')
			{
				if (brktCntInLine < 1)
				{
					lineBuff[ccc++]	=	theChar;
					lineBuff[ccc++]	=	0;
					printf("%s\r\n", lineBuff);
					ccc				=	0;
					lineBuff[ccc++]	=	0x09;
					lineBuff[ccc++]	=	0x09;
					theChar2	=	getc(stdin);
					charCnt		+=	1;
					if (theChar2 != 0x20)
					{
						lineBuff[ccc++]	=	theChar2;
					}
				}
				else
				{
					lineBuff[ccc++]	=	theChar;
				}
			}
			else if (ccc < kMaxLineLen)
			{
				if ((theChar == '[') && (ccc == 0))
				{
					lineBuff[ccc++]	=	0x09;
					lineBuff[ccc++]	=	0x09;
					lineBuff[ccc++]	=	0x09;
				}

				lineBuff[ccc++]	=	theChar;

				if (theChar == '[')
				{
					brktCntInLine++;
				}
				else if (theChar == ']')
				{
					brktCntInLine--;
				}
				if (brktCntInLine < 0)
				{
					brktCntInLine	=	0;
				}
			}
			else
			{
				printf("OVERFLOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
				exit(0);
			}
		}
	}
	if (ccc > 0)
	{
		lineBuff[ccc++]	=	0;
		printf("%s\r\n", lineBuff);
	}
	printf("\r\n");
	fprintf(stderr, "Chars read=%d\r\n", charCnt);
	fprintf(stderr, "replaceCnt=%d\r\n", replaceCnt);

	return(0);
}
