#include "stdio.h"
#include "EAF_focuser.h"
#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h> 


#define Sleep(a) usleep((a)*1000)
#endif

bool bRun = true;
void IntHandle(int i)
{
	bRun = false;
}

int  main()
{
	
	signal(SIGINT, IntHandle);
	int EAF_count = EAFGetNum();  
	if(EAF_count <= 0)
	{
		printf("no focuser connected, press any key to exit\n");
		getchar();
		return -1;
	}
	else
		printf("attached focuser :\n");

	EAF_INFO EAFInfo;
	

	for(int i = 0; i < EAF_count; i++)
	{
		EAFGetID(i, &EAFInfo.ID);
		EAFGetProperty(EAFInfo.ID, &EAFInfo);
		printf("index %d: %s\n",i, EAFInfo.Name);
	}

	printf("\nselect one \n");

	int EAFIndex, iSelectedID;
	scanf("%d", & EAFIndex);
	EAFGetID(EAFIndex, &iSelectedID);

	if(EAFOpen(iSelectedID) != EAF_SUCCESS)
	{
		printf("open error,are you root?,press any key to exit\n");
		getchar();
		return -1;
	}
	
	float fTemp;
   	EAFGetTemp(iSelectedID, &fTemp);
	printf("temperatur=%g\n", fTemp);
		
	EAF_ERROR_CODE err;
	while(1)
	{
		err = EAFGetProperty(iSelectedID, & EAFInfo);
		if(err != EAF_ERROR_MOVING )
			break;
		Sleep(500);
	} 

	printf("Max step: %d", EAFInfo.MaxStep);

	bool bMoving = false;

	while(1)
	{
		bool pbHandControl;
		err = EAFIsMoving(iSelectedID, &bMoving, &pbHandControl);
		if(err != EAF_SUCCESS || !bMoving)
	    		break;
		Sleep(500);
	} 
	
	int currentPos;
	EAFGetPosition(iSelectedID, &currentPos);
	printf("\ncurrent position: %d\n", currentPos);

	int targetPos;
	char szInput[16];
	printf("\nPlease input target position, type \'q\' to quit:\n");
	while(1)
	{	
	//	safe_flush(stdin);
		scanf("%s", szInput);

		if(!strcmp(szInput, "q"))
			break;
		targetPos = atoi(szInput);
		printf("\nmove to: %d\n", targetPos);

		if(targetPos < 0)
			continue;

		bRun = true;
		err = EAFMove(iSelectedID, targetPos);
		if(err == EAF_SUCCESS)
			printf("\nMoving..., press CTRL+C to abort\n\n");
		while(1)
		{
			if(!bRun)
			{
				printf("\nMove is aborted\n");
				EAFStop(iSelectedID);
			}
			EAFGetPosition(iSelectedID, &currentPos);			
			printf("current position: %d\n", currentPos);
			
			bool pbHandControl;
			err = EAFIsMoving(iSelectedID, &bMoving, &pbHandControl);
			if(err != EAF_SUCCESS || !bMoving )
					break;
			Sleep(500);
		} 

		printf("\nPlease input target position, type \'q\' to quit:\n");

	}
	EAFClose(iSelectedID);
	printf("main function over\n");
	return 0;
	
}






