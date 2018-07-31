#include "stdafx.h"
#include <windows.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi")
#include <winioctl.h>
#include <stdlib.h>


#ifndef TRACE
#define TRACE printf
#endif

int main(int argc, char* argv[])
{
	DEVINST devInst = 0;
	DEVINST devNext = 0;
	
	CONFIGRET cr = CM_Locate_DevNode(&devInst, NULL, 0);
	if (cr != CR_SUCCESS)
	{
		TRACE("Locate_devNode failed\r\n");
		system( "pause" );
		return 0;
	}
	
	printf("devInst = %X\r\n", devInst);
	BOOL done = FALSE;
	char buff[512] = {0};
	int   len = 512;
	
	while (!done)
	{
		memset(buff, 0, len);
		cr = CM_Get_DevNode_Registry_Property(
			devInst,
			CM_DRP_CLASSGUID,
			NULL,
			buff,
			(PULONG)&len,
			0 
			);

		if (cr == CR_SUCCESS)
		{
			char buff2[512] = {0};

			cr = CM_Get_DevNode_Registry_Property(
				devInst,
				CM_DRP_DEVICEDESC /*CM_DRP_FRIENDLYNAME*/,
				NULL,
				buff2,
				(PULONG)&len,
				0
				);
			if (cr == CR_SUCCESS)
			{
				printf("GUID = %s, Name = %s\r\n", buff, buff2);
			}
			
		}
		
		cr = CM_Get_Child(&devNext, devInst, 0);
		if (cr == CR_SUCCESS)
		{
			devInst = devNext;
			continue;
		}
		
		while (1)
		{
			cr = CM_Get_Sibling(&devNext, devInst, 0);
			if (cr == CR_SUCCESS)
			{
				devInst = devNext;
				break;
			}
			
			cr = CM_Get_Parent(&devNext, devInst, 0);
			if (cr == CR_SUCCESS)
			{
				devInst = devNext;
			}
			else
			{
				done = TRUE;
				break;
			}
		}
	}
	
	system( "pause" );
	return 1;
}
