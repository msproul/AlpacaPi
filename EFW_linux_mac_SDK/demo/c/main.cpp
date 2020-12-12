#include "stdio.h"
#include "EFW_filter.h"
#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#define Sleep(a) usleep((a)*1000)
#endif
int  main()
{
	
	int EFW_count = EFWGetNum();  
	if(EFW_count <= 0)
	{
		printf("no filter wheel connected, press any key to exit\n");
		getchar();
		return -1;
	}
	else
		printf("attached filter wheel:\n");

	EFW_INFO EFWInfo;
	

	for(int i = 0; i < EFW_count; i++)
	{
		EFWGetID(i, &EFWInfo.ID);
		EFWGetProperty(EFWInfo.ID, &EFWInfo);
		printf("index %d: %s\n",i, EFWInfo.Name);
	}

	printf("\nselect one \n");

	int EFWIndex, iSelectedID;
	scanf("%d", & EFWIndex);
	EFWGetID(EFWIndex, &iSelectedID);

	if(EFWOpen(iSelectedID) != EFW_SUCCESS)
	{
		printf("open error,are you root?,press any key to exit\n");
		getchar();
		return -1;
	}

	
	if(iSelectedID < 0)
    	{
	
       		printf("open error,are you root?,press any key to exit\n");
		getchar();
		return -1;   
	 }
   	 else
	{      
		
		EFW_ERROR_CODE err;
		while(1){
			err = EFWGetProperty(iSelectedID, & EFWInfo);
			if(err != EFW_ERROR_MOVING )
				break;
			Sleep(500);
		} 
         	printf("%d slots: ", EFWInfo.slotNum);
		for(int i = 0; i < EFWInfo.slotNum; i++)
			printf("%d ", i + 1);
       
		int currentSlot;
		while(1)
    		{

        		err = EFWGetPosition(iSelectedID, &currentSlot);
        		if(err != EFW_SUCCESS || currentSlot != -1 )
        	    		break;
        		Sleep(500);
    		} 
		printf("\ncurrent position: %d\n", currentSlot + 1);

		int targetSlot;
		char c;
		printf("\nPlease input target position, type \'q\' to quit:\n");
		while(1)
		{	
			c = getchar();
     
    			if (EOF == c)
				continue;
			if(c == 'q')
				break;
			if(c >= '0' && c <= '9')
				targetSlot = c - '0';
			else
				continue;
			
			targetSlot--;
			if(targetSlot < 0 || targetSlot >=  EFWInfo.slotNum)
				continue;
			
			err = EFWSetPosition(iSelectedID, targetSlot);
			if(err == EFW_SUCCESS)
				printf("\nMoving...\n");
			while(1)
    			{
       				 err = EFWGetPosition(iSelectedID, &currentSlot);
        			if(err != EFW_SUCCESS ||currentSlot != -1 )
            				break;
        			Sleep(500);
    			} 
			printf("\nPlease input target position, type \'q\' to quit:\n");

		}
		EFWClose(iSelectedID);
		printf("main function over\n");
		return 1;
	}
}






