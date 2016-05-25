
// DC_ARP_7Dlg.cpp : 구현 파일
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

   // 레이어 생성 및 관계설정
   m_LayerMgr.AddLayer( this );
   m_LayerMgr.AddLayer( new CARPLayer("ARPLayer"));
   m_LayerMgr.AddLayer( new CEthernetLayer( "EthernetLayer"));
   m_LayerMgr.AddLayer( new CPacketDriverLayer( "PacketDriverLayer"));

   // dialog layer 와 ArpLayer를 연결
   m_LayerMgr.ConnectLayers("PacketDriverLayer ( *EthernetLayer ( *ARPLayer ) )");
   
   //현재 layer와 ARPLayer를 연결
   mp_UnderLayer = m_LayerMgr.GetLayer(1);
   mp_UnderLayer->SetUpperLayer(m_LayerMgr.GetLayer(0));
   m_pdLayer = (CPacketDriverLayer*)m_LayerMgr.GetLayer(3);

   	//나의 IP 주소를 입력
	ipMyAddress[0] = 169;
	ipMyAddress[1] = 254;
	ipMyAddress[2] = 246;
	ipMyAddress[3] = 245;

	((CARPLayer*)mp_UnderLayer)->setSrcIpAddress(ipMyAddress);

	//나의 MAC 주소를 입력
	macMyAddress[0] = (int)strtol("44",NULL,16);
	macMyAddress[1] = (int)strtol("A8",NULL,16);
	macMyAddress[2] = (int)strtol("42",NULL,16);
	macMyAddress[3] = (int)strtol("F7",NULL,16);
	macMyAddress[4] = (int)strtol("94",NULL,16);
	macMyAddress[5] = (int)strtol("0C",NULL,16);

	GetNetWorkNameList();
	AfxBeginThread(threadCapture , this);
}

//pDX를 이용하여 두 번째 인자와 세 번째 인자인 클래스 변수를 연결
void CDC_ARP_7Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ARP_CacheTable, ARP_Cache);
	DDX_Control(pDX, Proxy_CacheTable, Proxy_Cache);
	DDX_Control(pDX, IP_Address, IP_Address_Main);
	DDX_Control(pDX, MAC_Address, MAC_Address_Main);
}

// CDC_ARP_7Dlg 메시지 처리기
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

	// 추가 초기화 작업을 추가합니다.
	ARP_Cache.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Proxy_Cache.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//ARP Cache Table 분할 설정
	ARP_Cache.InsertColumn(0 , _T("IP") , LVCFMT_LEFT , 80);
	ARP_Cache.InsertColumn(1 , _T("MAC") , LVCFMT_LEFT , 110);
	ARP_Cache.InsertColumn(2 , _T("State") , LVCFMT_LEFT , 100);
	ARP_Cache.InsertColumn(3 , _T("Time") , LVCFMT_LEFT , 50);	

	//Proxy Cache Table 분할 설정
	Proxy_Cache.InsertColumn(0, _T("Device") , LVCFMT_LEFT , 95);
	Proxy_Cache.InsertColumn(1, _T("IP") , LVCFMT_LEFT , 80);
	Proxy_Cache.InsertColumn(2, _T("MAC") , LVCFMT_LEFT , 100);

	SetTimer(1, 1000 ,NULL); //table timer를 통해서 node의 시간 값을 변환 
	UpdateData( false );
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDC_ARP_7Dlg::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

      SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

      // 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // 아이콘을 그립니다.
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialogEx::OnPaint();
   }
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDC_ARP_7Dlg::OnQueryDragIcon()
{
   return static_cast<HCURSOR>(m_hIcon);
}
LRESULT CDC_ARP_7Dlg::OnRegArpSendMsg(WPARAM wParam, LPARAM lParam)
{
   SetTimer(3, 2000 , NULL);   // 전송을 하여 패킷을 보냈을 때 작동한다
   GetDlgItem(IP_SendBtn)->EnableWindow(FALSE);   // IP를 send 하는 버튼을 비활성화
   return 0;
}

LRESULT CDC_ARP_7Dlg::OnRegKillRestartTimerMsg(WPARAM wParam , LPARAM lParam)
{   // 재전송을 위한 timer가 필요 없어질 때 작동
   KillTimer(3); //timer를 Kill 한다.
   GetDlgItem(IP_SendBtn)->EnableWindow(TRUE);   // IP를 send 하는 버튼을 활성화
   return 0;
}

void CDC_ARP_7Dlg::OnTimer(UINT_PTR nIDEvent)
{
   if(nIDEvent == 1)
   {
      ((CARPLayer*)mp_UnderLayer)->TableTimer();   // 시간 값을 변화시킨다.
   }
   else if(nIDEvent == 3)
   {
      ((CARPLayer*)mp_UnderLayer)->reSend();   // reSend 함수로 Packet을 재전송한다.
   }
   __super::OnTimer(nIDEvent);
}




void CDC_ARP_7Dlg::OnBnClickedAdddlg()
{
   Proxy_Add_Dlg m_ProxyDlg;
   CString devName;
   unsigned char ip[4]; 
   unsigned char mac[6];

   // Proxy 정보를 추가한다.
   if(m_ProxyDlg.DoModal()==1)
   {   // proxy dialog를 띄우고 IP와 MAC을 입력하고 확인 버튼을 누르면
      devName = "interface0"; 
      memcpy(ip , m_ProxyDlg.proxy_TargetIp , 4); //Proxy dialog에 적은 ID를 ip로 복사하여 받아온다.
      memcpy(mac , m_ProxyDlg.proxy_MyMac , 6); // Proxy dialog에 적은 MAC을 mac으로 복사하여 받아온다.
   
      ((CARPLayer*)m_LayerMgr.GetLayer(1))->AddProxy(devName , ip , mac); // 받아온 IP와 MAC을 통해 proxy 정보를 추가하는 함수로 보내준다.
   }
}


void CDC_ARP_7Dlg::OnBnClickeddeletebtn()
{
      // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
   POSITION pos;
   pos = Proxy_Cache.GetFirstSelectedItemPosition();

   int temp = Proxy_Cache.GetNextSelectedItem(pos); //Proxy Table에서 선택한 entry의 index를 temp로 설정한다.

   // 선택한 proxy entry를 찾아 삭제한다.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->DeleteProxy(temp);
}


void CDC_ARP_7Dlg::OnBnClickedARPItemdeletebtn()
{
   // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
   POSITION pos;
   pos = ARP_Cache.GetFirstSelectedItemPosition();  //ARP Table에서 선택한 entry의 index를 temp로 설정한다.

   int temp = ARP_Cache.GetNextSelectedItem(pos);
   // 선택한 ARP entry를 찾아 삭제한다.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->DeleteItem(temp);
}


void CDC_ARP_7Dlg::OnBnClickedARPAlldeletebtn()
{
   // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->AllDelete();
}


void CDC_ARP_7Dlg::OnBnClickedIPSendbtn()
{
   unsigned char check_ARP[] = "A"; 
   UpdateData(true);
   // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
   BYTE ipField0,ipField1,ipField2,ipField3;
   unsigned char ipAddress[4];
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setSrcIpAddress(ipMyAddress);

   IP_Address_Main.GetAddress(ipField0 , ipField1 ,ipField2 , ipField3 );
   
   ipAddress[0] = ipField0;
   ipAddress[1] = ipField1;
   ipAddress[2] = ipField2;
   ipAddress[3] = ipField3;
   //받아온 IP Address를 배열로 넣어준다.
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setDstIpAddress(ipAddress);
   // 입력한 IP를 ARP Layer에 보내줘서 값을 설정시킴
   
   // ARP 모드로 설정
   
   mp_UnderLayer->Send(check_ARP,0);   // ARP모드로 설정한 정보로 Send 함수를 부른다.
}


void CDC_ARP_7Dlg::OnBnClickedMACSendbtn()
 {
   unsigned char check_ARP[] = "G"; 
   UpdateData(true);
   
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setSrcIpAddress(ipMyAddress);
   ((CARPLayer*)m_LayerMgr.GetLayer(1))->setDstIpAddress(ipMyAddress);
   // Desination IP 주소와 Sender의 IP 주소를 모두 자신의 IP로 설정


   char macEditAddr[20];
   MAC_Address_Main.GetWindowTextA(macEditAddr , 20);

   if(strlen(macEditAddr) != 17){   // 입력받은 MAC주소를 체크한다. 형식이 FF:FF:FF:FF:FF:FF 와 같으므로 17이 맞는지 검사한다.
      this->MessageBox("Gratuitous ARP mac 에러");
      return;
   }   
   sscanf(macEditAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
      &globalMacAddress[0],&globalMacAddress[1],&globalMacAddress[2],&globalMacAddress[3],&globalMacAddress[4],&globalMacAddress[5]);
   //입력받은 MAC주소를 globalMacAddress 배열을 통해서 16진수로 바꾼 다음 저장한다.

   // Gratuitous mode 로 설정
   mp_UnderLayer->Send(check_ARP , 0);   // 설정한 Gratuitous 모드를 통하여 Send를 부른다.
}

void CDC_ARP_7Dlg::GetNetWorkNameList()
{	// 컴퓨터에 있는 NIC 이름(어댑터 이름)들을 얻어옴.
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