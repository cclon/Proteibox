//////////////////////////////////////////////////////////////////////////
// 文件名：NetworkAdapter.h
// 功能描述：网络适配器接口
// 作者：hmf3000
// 版权声明：代码可以随便用，请引用此类的程序员能够在文件的头部
//             增加一行注释：“感谢hmf3000对此类的撰写” 足以。

#pragma once

class CNetworkAdapter
{
public:
	explicit CNetworkAdapter(void);
	~CNetworkAdapter(void);

public:

// Operation
    BOOL MakeChange(int n);                      // 使当前设置生效

// Property
    CString GetIPAddress(int n) const;           // 获取IP地址
    CString GetMACAddress(int n) const;          // 获取MAC地址
	CString GetDescription(int n) const;          // 获取MAC地址
	ULONG GetDescriptionCounts() const;          // 获取数量
    void SetIPAddress( CString strIP, int n );
    void SetMACAddress( CString strMAC, int n );
	void InitLocalAdapterInfo();

private:
    
// Implementation
    BOOL RegSetIP( CString strAdapterName, CString strIPAddress, CString strNetMask );
    BOOL RegSetMAC( CString strAdapterName, CString strMACAddress );

private:
    struct ADAPTER_INFO
    {
        CString     m_strIP;
        CString     m_strMAC;
        CString     m_strNetMask;
        CString     m_strAdapterName;
		CString     m_strDescription ;
    };

	ULONG m_nCounts ;

    // 一般情况下第一个Adapter为当前正在使用的Adaper
//    vector<ADAPTER_INFO>    m_vecAdapterInfo;               // 网络适配器组
	ADAPTER_INFO   m_vecAdapterInfo[5];               // 网络适配器组
};
