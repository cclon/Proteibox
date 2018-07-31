// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


//////////////////////////////////////////////////////////////////////////

void SampleCreateDoc()
{
	::CoInitialize(NULL);	// COM 初始化
				// 如果是MFC程序，可以使用AfxOleInit()替代
	
	HRESULT hr;		// 函数执行返回值
	IStorage *pStg = NULL;	// 根存储接口指针
	IStorage *pSub = NULL;	// 子存储接口指针
	IStream *pStm = NULL;	// 流接口指针
	
	hr = ::StgCreateDocfile(	// 建立复合文件
		L"c:\\360.stg",	// 文件名称
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,	// 打开方式
		0,		// 保留参数
		&pStg);		// 取得根存储接口指针

	if ( !SUCCEEDED ( hr ) ) 
	{
		printf( "StgCreateDocfile() failed \n" );
		goto _END_ ;
	}
	
	hr = pStg->CreateStorage(	// 建立子存储
		L"SubStg",	// 子存储名称
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
		0,0,
		&pSub);		// 取得子存储接口指针
	if ( !SUCCEEDED ( hr ) ) 
	{ 
		printf( "pStg->CreateStorage() failed \n" );
		goto _END_ ; 
	}
	
	hr = pSub->CreateStream(	// 建立流
		L"Stm",		// 流名称
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
		0,0,
		&pStm);		// 取得流接口指针
	if ( !SUCCEEDED ( hr ) )
	{ 
		printf( "pSub->CreateStream() failed \n" );
		goto _END_ ; 
	}

	hr = pStm->Write(		// 向流中写入数据
		"Hello",		// 数据地址
		5,		// 字节长度(注意，没有写入字符串结尾的\0)
		NULL);		// 不需要得到实际写入的字节长度
	if ( !SUCCEEDED ( hr ) ) 
	{ 
		printf( "pStm->Write() failed \n" );
		goto _END_ ; 
	}

	printf( "ok!!! \n\n" );
	
_END_ :
	if( pStm )	pStm->Release();// 释放流指针
	if( pSub )	pSub->Release();// 释放子存储指针
	if( pStg )	pStg->Release();// 释放根存储指针

	::CoUninitialize() ;		// COM 释放
		// 如果使用 AfxOleInit(),则不调用该函数
}



void GetWallpaper()
{
	WCHAR   wszWallpaper [MAX_PATH];
	HRESULT hr;
	IActiveDesktop* pIAD;
	
	// 1. 初始化COM库（让Windows加载DLLs）。通常是在程序的InitInstance()中调用
	// CoInitialize ( NULL )或其它启动代码。MFC程序使用AfxOleInit()。
	
	CoInitialize ( NULL );
	
	// 2. 使用外壳提供的活动桌面组件对象类创建COM对象。
	// 第四个参数通知COM需要什么接口(这里是IActiveDesktop).
	
	hr = CoCreateInstance (
		CLSID_ActiveDesktop,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop,
		(void**) &pIAD
		);
	
	if ( SUCCEEDED(hr) )
	{
		// 3. 如果COM对象被创建成功，则调用这个对象的GetWallpaper() 方法。
		hr = pIAD->GetWallpaper ( wszWallpaper, MAX_PATH, 0 );
		
		if ( SUCCEEDED(hr) )
		{
            // 4. 如果 GetWallpaper() 成功，则输出它返回的文件名字。
            // 注意这里使用wcout 来显示Unicode 串wszWallpaper.  wcout 是
            // Unicode 专用，功能与cout.相同。

			printf( "Wallpaper path is:\n %ws \n\n", wszWallpaper );
		//	wcout << L"Wallpaper path is:\n    " << wszWallpaper << endl << endl;
        }
        else
        {
            cout << "GetWallpaper() failed." << endl;
        }
		
        // 5. 释放接口。
        pIAD->Release();
    }
    else
    {
        cout << "CoCreateInstance() failed." << endl;
    }
	
    // 6. 收回COM库。MFC 程序不用这一步，它自动完成。
	::CoUninitialize() ;		// COM 释放
	
	return ;
}



void CreateWallpaperLnk ()
{
	LPCSTR sWallpaper = "C:\\WINDOWS\\Web\\Wallpaper\\Bliss.bmp" ;
	HRESULT hr ;
	IShellLink*   pISL;
	IPersistFile* pIPF;
	
    // 1. 初始化COM库(让Windows 加载DLLs). 通常在InitInstance()中调用
    // CoInitialize ( NULL )或其它启动代码。MFC 程序使用AfxOleInit() 。
	
    CoInitialize ( NULL );
	
    // 2. 使用外壳提供的Shell Link组件对象类创建COM对象。.
    // 第四个参数通知COM 需要什么接口(这里是IShellLink)。
	
    hr = CoCreateInstance ( 
		CLSID_ShellLink,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(void**) &pISL 
		);
	
    if ( SUCCEEDED(hr) )
    {
        // 3. 设置快捷方式目标(墙纸文件)的路径。
        hr = pISL->SetPath ( sWallpaper );
		
        if ( SUCCEEDED(hr) )
        {
            // 4. 获取这个对象的第二个接口(IPersistFile)。
            hr = pISL->QueryInterface ( IID_IPersistFile, (void**) &pIPF );
			
            if ( SUCCEEDED(hr) )
            {
                // 5. 调用Save() 方法保存某个文件得快捷方式。第一个参数是
                // Unicode 串。
                hr = pIPF->Save ( L"C:\\wallpaper.lnk", FALSE );
				if ( SUCCEEDED(hr) )
				{
					printf( "ok!!! \n\n" );
				}
				
                // 6a. 释放IPersistFile 接口。
                pIPF->Release();
            }
        }
		
        // 6. 释放IShellLink 接口。
        pISL->Release();
    }
	
    // 输出错误信息部分这里省略。
	
    // 7. 收回COM 库。MFC 程序不用这一步，它自动完成。
    CoUninitialize();


	return ;
}


void CLSID2ProgID()
{
	::CoInitialize( NULL );
	
	HRESULT hr;
	CLSID clsid = {0x06BE7323,0xEF34,0x11d1,{0xAC,0xD8,0,0xC0,0x4F,0xA3,0x10,0x9}}; 
	LPOLESTR lpwProgID = NULL;
	IMalloc * pMalloc = NULL;

	hr = ::ProgIDFromCLSID( clsid, &lpwProgID );
	if ( ! SUCCEEDED(hr) )
	{
		printf( "::ProgIDFromCLSID() failed: 0x%08lx \n\n", hr );
		goto _end_ ;
	}
	
	printf( "lpwProgID: %ws \n\n", lpwProgID );
	
	hr = ::CoGetMalloc( 1, &pMalloc );  // 取得 IMalloc
	if ( SUCCEEDED(hr) )
	{
		pMalloc->Free( lpwProgID );  // 释放ProgID内存
		pMalloc->Release();          // 释放IMalloc
	}

_end_ :
	::CoUninitialize();
}



//////////////////////////////////////////////////////////////////////////


int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;

	system( "title=Test for xx:COM" );

	while ( bOK )
	{
		printf (
			"请选择操作: \n"
			"0. ESC \n"
			"1. 调用COM接口写文件到c:\\360.stg			\n"
			"2. 得到桌面墙纸的路径						\n"
			"3. 创建桌面墙纸快捷方式至c:\\wallpaper.lnk \n"
			"4. clsid to progid							\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;
			
		case 1 :
			SampleCreateDoc() ;
			break;

		case 2 :
			GetWallpaper() ;
			break;
		
		case 3 :
			CreateWallpaperLnk() ;
			break;

		case 4 :
			CLSID2ProgID() ;
			break;
			
		default:
			break;
		}
	}
	
	//	getchar() ;
	return 0;
}
