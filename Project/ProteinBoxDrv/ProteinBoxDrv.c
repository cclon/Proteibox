/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/03/31 
* MODULE : ProteinBoxDrv.C
*
* Description:
*   
*   蛋白质盒 的主驱动                       
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#include "struct.h"
#include "ProteinBoxDrv.h"
#include "DoWork.h"
#include "DispatchIoctl.h"

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

NTSTATUS 
DriverEntry(
	PDRIVER_OBJECT pDriverObj, 
	PUNICODE_STRING pRegistryString
	)
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ustrLinkName;
	UNICODE_STRING ustrDevName;    
	PDEVICE_OBJECT pDevObj;

	dprintf("[ProteinBoxDrv] DriverEntry\n");
	
	pDriverObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	pDriverObj->DriverUnload = DriverUnload;


	RtlInitUnicodeString(&ustrDevName, DEVICE_NAME);
	status = IoCreateDevice(pDriverObj, 
				0,
				&ustrDevName, 
				FILE_DEVICE_UNKNOWN,
				0,
				FALSE,
				&pDevObj);

	if(!NT_SUCCESS(status))	{
		dprintf("[ProteinBoxDrv] IoCreateDevice = 0x%x\n", status);
		return status;
	}

	RtlInitUnicodeString(&ustrLinkName, LINK_NAME);
	status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);  
	if(!NT_SUCCESS(status)) {
		dprintf("[ProteinBoxDrv] IoCreateSymbolicLink = 0x%x\n", status);
		IoDeleteDevice(pDevObj);  
		return status;
	}
	

	//
	// 添加执行代码
	//

	DoWork( TRUE );

	return STATUS_SUCCESS;
}



VOID 
DriverUnload(
	PDRIVER_OBJECT pDriverObj
	)
{	
	UNICODE_STRING strLink;
	RtlInitUnicodeString(&strLink, LINK_NAME);

	//
	// 添加卸载代码
	//

	DoWork( FALSE );
	
	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(pDriverObj->DeviceObject);
	dprintf("[ProteinBoxDrv] Unloaded\n");
}



NTSTATUS 
DispatchCreate(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
	)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	dprintf("[ProteinBoxDrv] IRP_MJ_CREATE\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}



NTSTATUS 
DispatchClose(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
	)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	dprintf("[ProteinBoxDrv] IRP_MJ_CLOSE\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}



NTSTATUS 
DispatchIoctl(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
	)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch( uIoControlCode )
	{
		case IOCTL_HELLO : {
			
			dprintf("[ProteinBoxDrv] Hello\n");
			status = STATUS_SUCCESS;
		}
		break;

		//
		// 添加执行代码
		//

		case IOCTL_PROTEINBOX : {
			
		//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX\n" );

			if ( KeGetCurrentIrql() || UserMode != ExGetPreviousMode() )
			{
				status = STATUS_INVALID_LEVEL ;
				break ;
			}

			if ( uInSize < 8 )
			{
				status = STATUS_INVALID_DEVICE_REQUEST ;
				break ;
			}

			status = Handler_DispacthIoctl_PROTEINBOX( pIoBuffer );
		}
		break;

	}

	if(status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;
	else
		pIrp->IoStatus.Information = 0;
	
	/////////////////////////////////////
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                                           +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--





///////////////////////////////   END OF FILE   ///////////////////////////////
