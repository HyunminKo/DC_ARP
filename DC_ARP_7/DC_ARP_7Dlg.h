
// DC_ARP_7Dlg.h : 헤더 파일
//

#pragma once
#include "LayerManager.h"
#include "EthernetLayer.h"
#include "ARPLayer.h"
#include "PacketDriverLayer.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"

// CDC_ARP_7Dlg 대화 상자
class CDC_ARP_7Dlg : public CDialogEx,
	public CBaseLayer
{
// 생성입니다.
public:
	CDC_ARP_7Dlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = ARP_Dlg };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private :
   CLayerManager   m_LayerMgr;
   enum {         ARP_INITIALIZING, 
               ARP_SETTING    } ;

public:
	unsigned char ipMyAddress[4]; // ip 설정 전역변수
	CListCtrl ARP_Cache;
	CListCtrl Proxy_Cache;
	CIPAddressCtrl IP_Address_Main;
	CEdit MAC_Address_Main;
	CPacketDriverLayer* m_pdLayer;
	CIPAddressCtrl m_dstIpAdr;
    CIPAddressCtrl m_srcIpAdr;
	void GetNetWorkNameList();
	LRESULT			OnRegArpSendMsg(WPARAM wParam, LPARAM lParam) ;
	LRESULT			OnRegKillRestartTimerMsg(WPARAM wParam, LPARAM lParam) ;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedAdddlg();
	afx_msg void OnBnClickeddeletebtn();
	afx_msg void OnBnClickedARPItemdeletebtn();
	afx_msg void OnBnClickedARPAlldeletebtn();
	afx_msg void OnBnClickedIPSendbtn();
	afx_msg void OnBnClickedMACSendbtn();
};
