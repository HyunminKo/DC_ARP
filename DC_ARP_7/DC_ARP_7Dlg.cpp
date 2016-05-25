
// DC_ARP_7Dlg.cpp : ���� ����
//

#include "stdafx.h"
#include "DC_ARP_7.h"
#include "DC_ARP_7Dlg.h"
#include "Proxy_Add_Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDC_ARP_7Dlg::CDC_ARP_7Dlg(CWnd* pParent /*=NULL*/)
   : CDialogEx(CDC_ARP_7Dlg::IDD, pParent),
   CBaseLayer("DC_ARP_7Dlg")
{
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

   // ���̾� ���� �� ���輳��
   m_LayerMgr.AddLayer( this );
   m_LayerMgr.AddLayer( new CARPLayer("ARPLayer"));
   m_LayerMgr.AddLayer( new CEthernetLayer( "EthernetLayer"));
   m_LayerMgr.AddLayer( new CPacketDriverLayer( "PacketDriverLayer"));

   // dialog layer �� ArpLayer�� ����
   m_LayerMgr.ConnectLayers("PacketDriverLayer ( *EthernetLayer ( *ARPLayer ) )");
   
   //���� layer�� ARPLayer�� ����
   mp_UnderLayer = m_LayerMgr.GetLayer(1);
   mp_UnderLayer->SetUpperLayer(m_LayerMgr.GetLayer(0));
   m_pdLayer = (CPacketDriverLayer*)m_LayerMgr.GetLayer(3);

   	//���� IP �ּҸ� �Է�
	ipMyAddress[0] = 169;
	ipMyAddress[1] = 254;
	ipMyAddress[2] = 246;
	ipMyAddress[3] = 245;

	((CARPLayer*)mp_UnderLayer)->setSrcIpAddress(ipMyAddress);

	//���� MAC �ּҸ� �Է�
	macMyAddress[0] = (int)strtol("44",NULL,16);
	macMyAddress[1] = (int)strtol("A8",NULL,16);
	macMyAddress[2] = (int)strtol("42",NULL,16);
	macMyAddress[3] = (int)strtol("F7",NULL,16);
	macMyAddress[4] = (int)strtol("94",NULL,16);
	macMyAddress[5] = (int)strtol("0C",NULL,16);

	GetNetWorkNameList();
	AfxBeginThread(threadCapture , this);
}

//pDX�� �̿��Ͽ� �� ��° ���ڿ� �� ��° ������ Ŭ���� ������ ����
void CDC_ARP_7Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ARP_CacheTable, ARP_Cache);
	DDX_Control(pDX, Proxy_CacheTable, Proxy_Cache);
	DDX_Control(pDX, IP_Address, IP_Address_Main);
	DDX_Control(pDX, MAC_Address, MAC_Address_Main);
}

// CDC_ARP_7Dlg �޽��� ó����
BEGIN_MESSAGE_MAP(CDC_ARP_7Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(Proxy_AddDlg, &CDC_ARP_7Dlg::OnBnClickedAdddlg)
	ON_BN_CLICKED(Proxy_deleteBtn, &CDC_ARP_7Dlg::OnBnClickeddeletebtn)
	ON_BN_CLICKED(ARP_IDeleteBtn, &CDC_ARP_7Dlg::OnBnClickedARPItemdeletebtn)
	ON_BN_CLICKED(ARP_ADeleteBtn, &CDC_ARP_7Dlg::OnBnClickedARPAlldeletebtn)
	ON_BN_CLICKED(IP_SendBtn, &CDC_ARP_7Dlg::OnBnClickedIPSendbtn)
	ON_BN_CLICKED(MAC_SendBtn, &CDC_ARP_7Dlg::OnBnClickedMACSendbtn)
	ON_REGISTERED_MESSAGE( CARPLayer::nRegArpSendMsg , OnRegArpSendMsg )
	ON_REGISTERED_MESSAGE( CARPLayer::nRegKillRestartTimerMsg , OnRegKillRestartTimerMsg )
	ON_WM_TIMER()

END_MESSAGE_MAP()




BOOL CDC_ARP_7Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	
	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	// �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	ARP_Cache.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Proxy_Cache.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//ARP Cache Table ���� ����
	ARP_Cache.InsertColumn(0 , _T("IP") , LVCFMT_LEFT , 80);
	ARP_Cache.InsertColumn(1 , _T("MAC") , LVCFMT_LEFT , 110);
	ARP_Cache.InsertColumn(2 , _T("State") , LVCFMT_LEFT , 100);
	ARP_Cache.InsertColumn(3 , _T("Time") , LVCFMT_LEFT , 50);	

	//Proxy Cache Table ���� ����
	Proxy_Cache.InsertColumn(0, _T("Device") , LVCFMT_LEFT , 95);
	Proxy_Cache.InsertColumn(1, _T("IP") , LVCFMT_LEFT , 80);
	Proxy_Cache.InsertColumn(2, _T("MAC") , LVCFMT_LEFT , 100);

	SetTimer(1, 1000 ,NULL); //table timer�� ���ؼ� node�� �ð� ���� ��ȯ 
	UpdateData( false );
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}


// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CDC_ARP_7Dlg::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

      SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

      // Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // �������� �׸��ϴ�.
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialogEx::OnPaint();
   }
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CDC_ARP_7Dlg::OnQueryDragIcon()
{
   return static_cast<HCURSOR>(m_hIcon);
}
LRESULT CDC_ARP_7Dlg::OnRegArpSendMsg(WPARAM wParam, LPARAM lParam)
{
   SetTimer(3, 2000 , NULL);   // ������ �Ͽ� ��Ŷ�� ������ �� �۵��Ѵ�
   GetDlgItem(IP_SendBtn)->EnableWindow(FALSE);   // IP�� send �ϴ� ��ư�� ��Ȱ��ȭ
   return 0;
}

LRESULT CDC_ARP_7Dlg::OnRegKillRestartTimerMsg(WPARAM wParam , LPARAM lParam)
{   // �������� ���� timer�� �ʿ� ������ �� �۵�
   KillTimer(3); //timer�� Kill �Ѵ�.
   GetDlgItem(IP_SendBtn)->EnableWindow(TRUE);   // IP�� send �ϴ� ��ư�� Ȱ��ȭ
   return 0;
}

void CDC_ARP_7Dlg::OnTimer(UINT_PTR nIDEvent)
{
   if(nIDEvent == 1)
   {
      ((CARPLayer*)mp_UnderLayer)->TableTimer();   // �ð� ���� ��ȭ��Ų��.
   }
   else if(nIDEvent == 3)
   {
      ((CARPLayer*)mp_UnderLayer)->reSend();   // reSend �Լ��� Packet�� �������Ѵ�.
   }
   __super::OnTimer(nIDEvent);
}




void CDC_ARP_7Dlg::OnBnClickedAdddlg()
{
   Proxy_Add_Dlg m_ProxyDlg;
   CString devName;
   unsigned char ip[4]; 
   unsigned char mac[6];

   // Proxy ������ �߰��Ѵ�.
   if(m_ProxyDlg.DoModal()==1)
   {   // proxy dialog�� ���� IP�� MAC�� �Է��ϰ� Ȯ�� ��ư�� ������
      devName = "interface0"; 
      memcpy(ip , m_ProxyDlg.proxy_TargetIp , 4); //Proxy dialog�� ���� ID�� ip�� �����Ͽ� �޾ƿ´�.
      memcpy(mac , m_ProxyDlg.proxy_MyMac , 6); // Proxy dialog�� ���� MAC�� mac���� �����Ͽ� �޾ƿ´�.
   
      ((CARPLayer*)m_LayerMgr.GetLayer(1))->AddProxy(devName , ip , mac); // �޾ƿ� IP�� MAC�� ���� proxy ������ �߰��ϴ� �Լ��� �����ش�.
   }
}


void CDC_ARP_7Dlg::OnBnClickeddeletebtn()
{
      // TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
   POSITION pos;
   pos = Proxy_Cache.GetFirstSelectedItemPosition();

   int temp = Proxy_Cache.GetNextSelectedItem(pos); //Proxy Table���� ������ entry�� index�� temp�� �����Ѵ�.

   // ������ proxy entry�� ã�� �����Ѵ�.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->DeleteProxy(temp);
}


void CDC_ARP_7Dlg::OnBnClickedARPItemdeletebtn()
{
   // TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
   POSITION pos;
   pos = ARP_Cache.GetFirstSelectedItemPosition();  //ARP Table���� ������ entry�� index�� temp�� �����Ѵ�.

   int temp = ARP_Cache.GetNextSelectedItem(pos);
   // ������ ARP entry�� ã�� �����Ѵ�.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->DeleteItem(temp);
}


void CDC_ARP_7Dlg::OnBnClickedARPAlldeletebtn()
{
   // TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->AllDelete();
}


void CDC_ARP_7Dlg::OnBnClickedIPSendbtn()
{
   unsigned char check_ARP[] = "A"; 
   UpdateData(true);
   // TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
   BYTE ipField0,ipField1,ipField2,ipField3;
   unsigned char ipAddress[4];
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setSrcIpAddress(ipMyAddress);

   IP_Address_Main.GetAddress(ipField0 , ipField1 ,ipField2 , ipField3 );
   
   ipAddress[0] = ipField0;
   ipAddress[1] = ipField1;
   ipAddress[2] = ipField2;
   ipAddress[3] = ipField3;
   //�޾ƿ� IP Address�� �迭�� �־��ش�.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setDstIpAddress(ipAddress);
   // �Է��� IP�� ARP Layer�� �����༭ ���� ������Ŵ
   
   // ARP ���� ����
   
   mp_UnderLayer->Send(check_ARP,0);   // ARP���� ������ ������ Send �Լ��� �θ���.
}


void CDC_ARP_7Dlg::OnBnClickedMACSendbtn()
 {
   unsigned char check_ARP[] = "G"; 
   UpdateData(true);
   
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setSrcIpAddress(ipMyAddress);
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setDstIpAddress(ipMyAddress);
   // Desination IP �ּҿ� Sender�� IP �ּҸ� ��� �ڽ��� IP�� ����


   char macEditAddr[20];
   MAC_Address_Main.GetWindowTextA(macEditAddr , 20);

   if(strlen(macEditAddr) != 17){   // �Է¹��� MAC�ּҸ� üũ�Ѵ�. ������ FF:FF:FF:FF:FF:FF �� �����Ƿ� 17�� �´��� �˻��Ѵ�.
      this->MessageBox("Gratuitous ARP mac ����");
      return;
   }   
   sscanf(macEditAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
      &globalMacAddress[0],&globalMacAddress[1],&globalMacAddress[2],&globalMacAddress[3],&globalMacAddress[4],&globalMacAddress[5]);
   //�Է¹��� MAC�ּҸ� globalMacAddress �迭�� ���ؼ� 16������ �ٲ� ���� �����Ѵ�.

   // Gratuitous mode �� ����
   mp_UnderLayer->Send(check_ARP , 0);   // ������ Gratuitous ��带 ���Ͽ� Send�� �θ���.
}

void CDC_ARP_7Dlg::GetNetWorkNameList()
{	// ��ǻ�Ϳ� �ִ� NIC �̸�(����� �̸�)���� ����.
	int i;
	char AdapterName[8192];
	char *temp, *temp1;
	int AdapterNum = 0;
	ULONG AdapterLength;
	AdapterLength = sizeof(AdapterName);
	PacketGetAdapterNames(AdapterName,&AdapterLength);	// get all AdapterName
	temp = AdapterName;
	temp1 = AdapterName;
	i = 0;
		while ((*temp!='\0')||(*(temp-1)!='\0'))
	{
		if (*temp=='\0') 
		{
			memcpy(AdapterList[i],temp1,temp-temp1);
			temp1=temp+1;
			i++;
		}
		temp++;
	}
		i = 3;
	for( i = 0 ; AdapterList[0][i] != '\0'; i++);
	g_nicName = CString(AdapterList[0],i);
}