#pragma once

//////////////////////////////////////////////////////////////////////////

#define ARRAYSIZEOF(x)	sizeof (x) / sizeof (x[0])


#define FILE_SUPERSEDE                    0x00000000
#define FILE_OPEN                         0x00000001
#define FILE_CREATE                       0x00000002
#define FILE_OPEN_IF                      0x00000003
#define FILE_OVERWRITE                    0x00000004
#define FILE_OVERWRITE_IF                 0x00000005
#define FILE_MAXIMUM_DISPOSITION          0x00000005

#define FILE_DIRECTORY_FILE               0x00000001
#define FILE_WRITE_THROUGH                0x00000002
#define FILE_SEQUENTIAL_ONLY              0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT         0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT      0x00000020
#define FILE_NON_DIRECTORY_FILE           0x00000040

#define FILE_OPEN_BY_FILE_ID              0x00002000
#define FILE_DELETE_ON_CLOSE              0x00001000

typedef struct _FILE_RENAME_INFORMATION 
{
	BOOLEAN Replace;
	HANDLE RootDir;
	ULONG FileNameLength;
	WCHAR FileName[1];

} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;


typedef struct _FILE_BASIC_INFORMATION {                    
	LARGE_INTEGER CreationTime;                             
	LARGE_INTEGER LastAccessTime;                           
	LARGE_INTEGER LastWriteTime;                            
	LARGE_INTEGER ChangeTime;                               
	ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;    

//////////////////////////////////////////////////////////////////////////

PVOID kmalloc( ULONG length );

VOID kfree( PVOID ptr );

BOOL
MmIsAddressValid (
	IN PVOID ptr,
	IN ULONG length
	);


///////////////////////////////   END OF FILE   ///////////////////////////////

