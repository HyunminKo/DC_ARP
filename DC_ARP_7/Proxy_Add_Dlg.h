#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Proxy_Add_Dlg 대화 상자입니다.

class Proxy_Add_Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(Proxy_Add_Dlg)

public:
	Proxy_Add_Dlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Proxy_Add_Dlg();

// 대화 상자 데이터입니다.
	enum { IDD = 129 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CComboBox Proxy_Device;
	CIPAddressCtrl Proxy_IP;
	CEdit Proxy_MAC;
	afx_msg void OnProxyOkClicked();

	unsigned char proxy_TargetIp[4];
	unsigned char proxy_MyMac[6];
	char proxy_MacTemp[20];
};
