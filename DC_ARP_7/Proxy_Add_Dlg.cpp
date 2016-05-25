// Proxy_Add_Dlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "DC_ARP_7.h"
#include "Proxy_Add_Dlg.h"
#include "afxdialogex.h"
#include <Packet32.h>

// Proxy_Add_Dlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(Proxy_Add_Dlg, CDialogEx)

// Proxy_Add_Dlg �� ǥ�� �������̴�. 
Proxy_Add_Dlg::Proxy_Add_Dlg(CWnd* pParent /*=NULL*/)
   : CDialogEx(Proxy_Add_Dlg::IDD, pParent)
{

}

Proxy_Add_Dlg::~Proxy_Add_Dlg()
{
}
// ���̾�α׶� ������ ������ ��������ش�.
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


// Proxy_Add_Dlg �޽��� ó�����Դϴ�.

//���Ͻ� ���̾�α׿��� OK ��ư�� ������ �����ϴ� �Լ�
void Proxy_Add_Dlg::OnProxyOkClicked()
{
   
   UpdateData(true);      
   
   BYTE nField0 , nField1 , nField2 , nField3;            //IP�� �߶� �����ϱ� ���� �ӽú���
   CString ip_Address;                              //IP �ּ� ��ü�� ���� ����
   Proxy_IP.GetAddress(nField0 , nField1 ,nField2 , nField3 );         //���Ͻ� ���̾�α׿��� �Է��� ������ �ּҸ� ������ ������ ����
    unsigned char ipAddress[4];

   ipAddress[0] = nField0;                        //���ҵ� ������ �ּҸ� �ϳ��ϳ� �Է��� ��ü���� IP�ּҸ� �ϼ�
   ipAddress[1] = nField1;
   ipAddress[2] = nField2;
   ipAddress[3] = nField3;
   
   memcpy(proxy_TargetIp , ipAddress , 4);            //�Է¹��� ��ü �����Ǹ� Target Ip �� ���� 

   
   Proxy_MAC.GetWindowText(proxy_MacTemp , sizeof(proxy_MacTemp));      //���̾�α� â���� �Է��� Mac �ּҸ� �ӽ� ������ ����

   sscanf(proxy_MacTemp, "%02X:%02X:%02X:%02X:%02X:%02X",            //�Է¹��� Mac �ּҸ� 16���� ���·� �Է��� �޾ƿ´�.
      &proxy_MyMac[0],&proxy_MyMac[1],&proxy_MyMac[2],&proxy_MyMac[3],&proxy_MyMac[4],
      &proxy_MyMac[5]);

   // ProxyDlg.h�� ��ġ�� ������ ����ڰ� �Է��� ���� �����ϰ� dialogâ�� �ݽ��ϴ�.
   CDialogEx::OnOK();
}