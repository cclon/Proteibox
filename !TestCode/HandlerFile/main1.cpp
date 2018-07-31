/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/12/08 [8:12:2009 - 18:23]
* MODULE : D:\Program\R0\Coding\沙箱\SandBox\Code\HandlerFile\main1.cpp
* 
* Description:
*   R3操作文件,用来调试沙箱的驱动Object Hook过滤函数 IopParseFile_IopParseDevice_Filter()     
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <iostream.h>
#include "Hfile.h"

//////////////////////////////////////////////////////////////////////////

char g_FilePath[MAX_PATH]		= "C:\\Test\\555.txt" ;
char g_FilePathNew[MAX_PATH]	= "C:\\Test\\imadus.txt" ;


//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

// int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;

// 	printf( 
// 		"请恢复沙箱对本程序IAT:ntdll!NtOpenFile/NtQueryInformationFile/NtSetInformationFile 的InlineHook\n"
// 		"再尝试操作文件 ... \n" 
// 		) ;
// 	getchar() ;

	while ( bOK )
	{
		printf (
			"请选择操作: \n"
			"0. ESC \n"
			"1. 写文件,不存在则创建						\n"
			"2. 重命名文件								\n"
			"3. 读文件\n"
			"4. 删文件\n"
			"-------------------------------------------\n"
			);

		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;

		case 1:
			TestWriteFile ();
			break;

		case 2:
			TestRenameFile();
			break;

		case 3:
			TestReadFile();
			break;

		case 4:
			TestDeleteFile();
			break;

		default:
			break;
		}
	}

//	getchar() ;
	return 0;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

VOID
TestWriteFile(
	) 
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;

	// 创建文件
	status = HFCreateFileEx( g_FilePath, OPEN_ALWAYS );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFCreateFile(), Error, 0x%08lx \n", status ); 
		return ;
	} else {
		printf( "HFCreateFileEx OK! \"%s\" \n", g_FilePath ) ;
	}
	
	// 写文件
	status = HFWriteFile( g_FilePath, "Hello" );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFWriteFile(), Error, 0x%08lx \n", status ); 
		return ;
	} else {
		printf( "HFWriteFile OK! \"%s\" \n", g_FilePath ) ;
	}

	return ;
}



VOID
TestRenameFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;

	// 重命名文件
	status = HFRenameFile( g_FilePath, g_FilePathNew );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFRenameFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "HFRenameFile OK! \"%s\" \n", g_FilePath ) ;
	}

	return ;
}


VOID
TestReadFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;
	char Context[MAX_PATH] = "" ;

	// 读文件
	status = HFReadFile( g_FilePath, Context );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFReadFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "Context:\n%s \n\n", Context );
	}

	return ;
}


VOID
TestDeleteFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	// 删文件
	status = HFDeleteFile( g_FilePath );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFDeleteFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "HFDeleteFile OK! \"%s\" \n", g_FilePath ) ;
	}
		
	return ;
}

//////////////////////////////////////////////////////////////////////////

