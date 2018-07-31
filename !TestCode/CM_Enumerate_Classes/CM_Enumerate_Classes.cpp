// CM_Enumerate_Classes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <cfgmgr32.h>

#pragma comment(lib, "SetupAPI.lib")

//////////////////////////////////////////////////////////////////////////

VOID
FindDMAChannelOccupants(
	) ;

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	FindDMAChannelOccupants() ;

	system( "pause" );
	return 0;
}



VOID
FindDMAChannelOccupants(
	)
{
    GUID    guidDevClass ;
    TCHAR   tcsDevClassName[MAX_PATH + 1];
    BYTE    pBuffer[512];
    DWORD   nRequireSize = 0;
    DWORD   dwRtn;
    BOOL    bRtn;
    int     nIndex = 0;
	
    while(true)	
    {
        dwRtn = CM_Enumerate_Classes( nIndex++, &guidDevClass, NULL );
        if(dwRtn == CR_NO_SUCH_VALUE) { break; }
		
        bRtn = SetupDiClassNameFromGuid( &guidDevClass, tcsDevClassName, MAX_PATH, &nRequireSize );
        if(!bRtn)
        {
            continue;
        }
		
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&guidDevClass, NULL, NULL, DIGCF_PRESENT);
        if(INVALID_HANDLE_VALUE == hDevInfo)
        {
            continue;
        }
		
        //Enumrate 
        int nMemberIndex = 0;
        SP_DEVINFO_DATA devInfo_data;
        devInfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
		
        while(true)
        {
			
            bRtn = SetupDiEnumDeviceInfo( hDevInfo, nMemberIndex++, &devInfo_data );
            if(!bRtn)	
            {
                int nErr = GetLastError();
                break;
            }
			
            if( !SetupDiGetDeviceRegistryProperty( hDevInfo, &devInfo_data, 0, 0, pBuffer, MAX_PATH, &nRequireSize))
			{
                continue;
			}
			
            // Open configuration 
            LOG_CONF  lcLogConf = 0;
			
            dwRtn = CM_Get_First_Log_Conf(&lcLogConf, devInfo_data.DevInst, ALLOC_LOG_CONF);
            if(CR_SUCCESS != dwRtn)
            {
                if(CR_SUCCESS != CM_Get_First_Log_Conf(&lcLogConf, devInfo_data.DevInst, BOOT_LOG_CONF))
				{
                    continue;
				}
            }
			
            RES_DES  rdResDes;
            dwRtn = CM_Get_Next_Res_Des(&rdResDes, lcLogConf, ResType_DMA, NULL, NULL);
            if(CR_SUCCESS == dwRtn)	
            {
                // check DMA information
                if(CR_SUCCESS == CM_Get_Res_Des_Data_Size(&nRequireSize, rdResDes, NULL))
                {
                    PBYTE pTempBuf;
                    pTempBuf = new BYTE[nRequireSize];
					
                    memset(pTempBuf, 0, nRequireSize);                  
					
                    if(CR_SUCCESS == CM_Get_Res_Des_Data(rdResDes, (PVOID)pTempBuf, nRequireSize, NULL))	
                    {
                        PDMA_DES pDMA = (PDMA_DES)pTempBuf;
						
                        CHAR tcsMessage[256];
                        sprintf(tcsMessage, "%s [DMA:%d]", pBuffer, pDMA->DD_Alloc_Chan);
						printf( "%s \r\n", tcsMessage );

					//	MessageBox( NULL, tcsMessage, "", MB_OK );
                    }
					
                    delete pTempBuf;
                }
				
                CM_Free_Res_Des_Handle(rdResDes);
            }
        }

        // Destroy HDEVINFO
        SetupDiDestroyDeviceInfoList(hDevInfo);     
    }
	
	return ;
}

