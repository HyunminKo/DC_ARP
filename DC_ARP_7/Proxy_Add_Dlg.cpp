// Proxy_Add_Dlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DC_ARP_7.h"
#include "Proxy_Add_Dlg.h"
#include "afxdialogex.h"
#include <Packet32.h>

// Proxy_Add_Dlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(Proxy_Add_Dlg, CDialogEx)

// Proxy_Add_Dlg 의 표준 생성자이다. 
Proxy_Add_Dlg::Proxy_Add_Dlg(CWnd* pParent /*=NULL*/)
   : CDialogEx(Proxy_Add_Dlg::IDD, pParent)
{

}

Proxy_Add_Dlg::~Proxy_Add_Dlg()
{
}
// 다이얼로그랑 지정한 변수랑 연결시켜준다.
void Proxy_Add_Dlg::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_COMBO1, Proxy_Device);
   DDX_Control(pDX, IDC_IPADDRESS1, Proxy_IP);
   DDX_Control(pDX, IDC_EDIT1, Proxy_MAC);
}


BEGIN_MESSAGE_MAP(Proxy_Add_Dlg, CDialogEx)
   ON_BN_CLICKED(1, &Proxy_Add_Dlg::OnProxyOkClicked)
END_MESSAGE_MAP()


// Proxy_Add_Dlg 메시지 처리기입니다.

//프록시 다이얼로그에서 OK 버튼을 누르면 동작하는 함수
void Proxy_Add_Dlg::OnProxyOkClicked()
{
   
   UpdateData(true);      
   
   BYTE nField0 , nField1 , nField2 , nField3;            //IP를 잘라서 보관하기 위한 임시변수
   CString ip_Address;                              //IP 주소 전체가 들어가는 변수
   Proxy_IP.GetAddress(nField0 , nField1 ,nField2 , nField3 );         //프록시 다이얼로그에서 입력한 아이피 주소를 각각의 변수에 저장
    unsigned char ipAddress[4];

   ipAddress[0] = nField0;                        //분할된 아이피 주소를 하나하나 입력해 전체적인 IP주소를 완성
   ipAddress[1] = nField1;
   ipAddress[2] = nField2;
   ipAddress[3] = nField3;
   
   memcpy(proxy_TargetIp , ipAddress , 4);            //입력받은 전체 아이피를 Target Ip 로 복사 

   
   Proxy_MAC.GetWindowText(proxy_MacTemp , sizeof(proxy_MacTemp));      //다이얼로그 창에서 입력한 Mac 주소를 임시 변수에 저장

   sscanf(proxy_MacTemp, "%02X:%02X:%02X:%02X:%02X:%02X",            //입력받은 Mac 주소를 16진수 형태로 입력을 받아온다.
      &proxy_MyMac[0],&proxy_MyMac[1],&proxy_MyMac[2],&proxy_MyMac[3],&proxy_MyMac[4],
      &proxy_MyMac[5]);

   // ProxyDlg.h에 위치한 변수에 사용자가 입력한 값을 저장하고 dialog창을 닫습니다.
   CDialogEx::OnOK();
}