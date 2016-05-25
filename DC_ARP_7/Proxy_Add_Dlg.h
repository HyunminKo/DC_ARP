#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Proxy_Add_Dlg ��ȭ �����Դϴ�.

class Proxy_Add_Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(Proxy_Add_Dlg)

public:
	Proxy_Add_Dlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~Proxy_Add_Dlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = 129 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
