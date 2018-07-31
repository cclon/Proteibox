/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2008/08/13 
* MODULE : ProteinBoxDrv.H
*
* Description:
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#ifndef _PROTEINBOXDRV_H
#define _PROTEINBOXDRV_H 1

#include <devioctl.h>

///////////////////////////////////////////////////////////////////////////////


#define DEVICE_NAME L"\\Device\\devProteinBoxDrv"     // Driver Name
#define LINK_NAME   L"\\DosDevices\\ProteinBoxDrv"    // Link Name

//
// The device driver IOCTLs
//

#define IOCTL_BASE	0x800
#define MY_CTL_CODE(i) CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_HELLO				MY_CTL_CODE( 0 )
#define IOCTL_PROTEINBOX		MY_CTL_CODE( 1 )



///////////////////////////////////////////////////////////////////////////////

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString);
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp);
VOID DriverUnload(PDRIVER_OBJECT pDriverObj);
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp);



///////////////////////////////////////////////////////////////////////////////
#endif

///////////////////////////////   END OF FILE   ///////////////////////////////