#include "StdAfx.h"
#include "ARPLayer.h"
#include "DC_ARP_7Dlg.h"

const DWORD CARPLayer::nRegArpSendMsg = ::RegisterWindowMessage( "ARP Send Message" ); //메세지 등록
const DWORD CARPLayer::nRegKillRestartTimerMsg = ::RegisterWindowMessage( "ARP Send Kill Timer "); //메세지등록

CARPLayer::CARPLayer(char* pName) //생성자
   :CBaseLayer( pName )
{
   ResetHeader();// header부분 Reset
}


CARPLayer::~CARPLayer(void) //소멸자
{
}

void CARPLayer::ResetHeader()//Header부분 Reset하는 메소드.
{
   memset( m_sHeader.sender_mac_address , 0 , 6);//sender의 Mac주소 초기화
   memset( m_sHeader.target_mac_address , 0 , 6);//tartget의 Mac주소 초기화

}

void CARPLayer::setDstIpAddress( unsigned char* pAddress ) //목적지 Address 설정
{
      memcpy(m_sHeader.target_ip_address , pAddress , 4);//상위 레이어에서 받은 주소를 target address로 설정
}

void CARPLayer::setSrcIpAddress( unsigned char* pAddress ) //상위 레이어에서 받은 주소를 source address로 설정
{
      memcpy(m_sHeader.sender_ip_address , pAddress , 4);
}

BOOL CARPLayer::Send(unsigned char* ppayload, int nlength) //상위 레이어에서 정보를 받아 ARP를 보내는 함수
{
   if( *ppayload == 'A' )
   {// 넘어온 ppayload의 값을 보고 A이면 ARP Request 판단한다.
     ppayload = NULL;//처음에 넘어온 ppayload가 비어있어야 하기 때문에 NULL로 설정
      sendCounter = 0; // 카운터 초기화
      int i = SearchCacheTable(); //이미 캐시테이블에 IP 주소가 있다면 해당하는 엔트리의 인덱스 반환

      if( i != -1 &&  cache_table.GetAt(cache_table.FindIndex(i)).state == "complete")
      {   // 캐시테이블이 비어있지 않고 상태가 complete 이라면
         CString exist_macAddress;
         CString caption = "!!";
         exist_macAddress.Format("이미 해당 IP에 대한 MAC주소가 매치되어 있습니다\nARP Request를 종료합니다.\n%02x:%02x:%02x:%02x:%02x:%02x" , 
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[0] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[1] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[2] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[3] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[4] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[5] );
         ::MessageBox(((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->GetSafeHwnd() , exist_macAddress ,
            caption , MB_OK | MB_ICONEXCLAMATION);   // 이미 ip와 mac 주소가 매치되어있다고 메시지를 띄우고 종료 
      }
      else
      {
      //Request를 위한 ARP Header setting
      //각각의 table 안에 알맞은 값을 넣어준다.
         m_sHeader.hard_type = htons(0x0001);
         m_sHeader.prot_type = htons(0x0800);
         m_sHeader.hard_size = 6;
         m_sHeader.prot_size = 4;
         m_sHeader.op = htons(0x0001);//request
         memcpy( m_sHeader.sender_mac_address , macMyAddress , sizeof(macMyAddress));
       //ARP 헤더에 sender호스트의 맥주소 설정
         SendMessage(HWND_BROADCAST , nRegArpSendMsg ,0 , 0);//하위 레이어에 arpmessage를 broadcast로 보냄 

         return GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);//여기서 저장된 정보를 하위 레이어로 보냄
      }
   }
   else if(*ppayload == 'G')
   {// 넘어온 ppayload의 값을 보고 G이면 Gratuitous로 보냄
      ppayload=NULL;//처음에 넘어온 ppayload가 비어있어야 하기 때문에 NULL로 설정
         m_sHeader.hard_type = htons(0x0001);
         m_sHeader.prot_type = htons(0x0800);
         m_sHeader.hard_size = 6;
         m_sHeader.prot_size = 4;
         m_sHeader.op = htons(0x0001);

         memcpy( m_sHeader.sender_mac_address , globalMacAddress , sizeof(globalMacAddress));// 내 Mac Address 를 입력
         return GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);//저장된 정보를 하위 레이어로 보냄
   }
}

BOOL CARPLayer::Receive(unsigned char* ppayload) //ARP를 받아서 처리하는 함수
{
   Parp_Header pFrame = (Parp_Header)ppayload;//pFrame에 하위에서 받아온 ppayload에 있는 정보를 저장

   if(memcmp(pFrame->sender_ip_address , m_sHeader.sender_ip_address, 4) == 0)
      return FALSE;   // 자기가 보낸 ARP Request이면 함수를 리턴하며 패킷을 상위 레이어로 전송하지 않는다.


   if(ntohs(pFrame->op) == 0x0001 && memcmp(pFrame->target_ip_address , pFrame->sender_ip_address , 4 ) == 0)
   {   // Gratuitous ARP 에 대한 대응
      POSITION ARP_Node = cache_table.GetHeadPosition();//ARP_Node에 cache table의 헤드 위치를 저장
      for( int i=0; i<cache_table.GetCount(); i++ )//캐시테이블을 돌면서 확인.
      {

         if( memcmp(pFrame->sender_ip_address , cache_table.GetAt(ARP_Node).ipAddress , 4 ) == 0 )
         {   //cache table에 있는 경우, ip가 일치 하면

            /////////////////////////////////////////////
            cache_table.GetAt(ARP_Node).ttl = 200;
            /////////////////////////////////////////////
            cache_table.GetAt(ARP_Node).state = "complete";
            memcpy(cache_table.GetAt(ARP_Node).macAddress , pFrame->sender_mac_address , 6);//바뀐 MAC Address를 저장
            updateList();//list를 업데이트
            return true;
         }
         cache_table.GetNext(ARP_Node);   // ip가 일치하지 않으면 다음 entry로 넘어간다.
      }
      //cache table에 존재하지 않으면 정보들을 저장.
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);
      memcpy(input.macAddress , pFrame->sender_mac_address , 6);
      input.state = "complete";
      input.ttl = 190;
      cache_table.AddTail(input);//저장된 노드를 cache table의 끝에 추가해준다.
      updateList();//list를 업데이트
   }
   else if(ntohs(pFrame->op) == 0x0001 &&// Request에 대한 reply반응
      memcmp(pFrame->target_ip_address , m_sHeader.sender_ip_address ,  4 ) == 0)
   {   // ARP를 받고 자신의 것이 맞으면
      m_sHeader.hard_type = htons(0x0001);
      m_sHeader.prot_type = htons(0x0800);
      m_sHeader.hard_size = 6;
      m_sHeader.prot_size = 4;
      m_sHeader.op = htons(0x0002);//reply code

      memcpy( m_sHeader.sender_mac_address , macMyAddress , 6);
      memcpy( m_sHeader.sender_ip_address , pFrame->target_ip_address , 4);
      memcpy( m_sHeader.target_ip_address , pFrame->sender_ip_address , 4);
      memcpy( m_sHeader.target_mac_address , pFrame->sender_mac_address , 6);
      // sender 와 target의 ip, Mac address를 swap 해준다.
      if(SearchCacheTable() == -1)
      {    //cache table에 존재하지 않으면 정보들을 저장.
         CacheTable input;
         memcpy(input.ipAddress , pFrame->sender_ip_address , 4);
         memcpy(input.macAddress , pFrame->sender_mac_address , 6);
         input.state = "complete";
         input.ttl = 180;
         cache_table.AddTail(input);//저장된 노드를 cache table의 끝에 추가해준다.
         updateList();
      }
      return mp_UnderLayer->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader), 0x02);   // ACK Packat을 하위 레이어로 보낸다.
   }
   else if(ntohs(pFrame->op) == 0x0001)
   {   // ARP Request이지만 나의 주소가 아닐 때,
      POSITION Proxy_Node = proxy_table.GetHeadPosition();//Proxy_Node에 Poxcy cache table의 헤드 위치를 저장
      for( int i = 0 ; i < proxy_table.GetCount() ; i++)//프록시 테이블을 돌면서 확인
      {   
         if(memcmp(pFrame->target_ip_address , proxy_table.GetAt(Proxy_Node).proxyIP , 4 ) == 0 )
         {   // 타겟의 ip address와 proxy table에 있는 ip 주소가 같으면,
            m_sHeader.hard_type = htons(0x0001);
            m_sHeader.prot_type = htons(0x0800);
            m_sHeader.hard_size = 6;
            m_sHeader.prot_size = 4;
            m_sHeader.op = htons(0x0002);//reply

            memcpy( m_sHeader.sender_mac_address , macMyAddress , 6);
            memcpy( m_sHeader.sender_ip_address , pFrame->target_ip_address , 4);
            memcpy( m_sHeader.target_ip_address , pFrame->sender_ip_address , 4);
            memcpy( m_sHeader.target_mac_address , pFrame->sender_mac_address , 6);
            // Mac 주소는 라우터 역할이 된 것의 Mac주소를 보낸다.
            if(SearchCacheTable() == -1)
            {  //cache table에 존재하지 않으면 정보들을 저장.(받는 입장에서)
               CacheTable input;
               memcpy(input.ipAddress , pFrame->sender_ip_address , 4);
               memcpy(input.macAddress , pFrame->sender_mac_address , 6);
               input.state = "complete";
               input.ttl = 170;
               cache_table.AddTail(input); //cache table에 존재하지 않으면 정보들을 저장.
               updateList();
            }
            return mp_UnderLayer->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader), 0x02);
         }
         proxy_table.GetNext(Proxy_Node);
      }
   }//proxy 끝.
    else if(ntohs(pFrame->op) == 0x0002 &&
      memcmp(pFrame->target_ip_address, m_sHeader.sender_ip_address, 4) == 0)
   {      // reply를 받은 경우
      SendMessage(HWND_BROADCAST , nRegKillRestartTimerMsg , 0, 0);// reply를 받았으니 다시 재전송할 필요가 없으므로 재전송 timer kill
      
     /* ARP table 갱신 */
     POSITION ARP_Node = cache_table.GetHeadPosition();
      for( int i=0; i<cache_table.GetCount(); i++ )
      {
         if( memcmp(pFrame->sender_ip_address , cache_table.GetAt(ARP_Node).ipAddress , 4 ) == 0 )
         {   //ARP table에 reply를 보낸 호스트의 ip주소가 있는 경우
            cache_table.GetAt(ARP_Node).ttl = 160; //time 설정
            cache_table.GetAt(ARP_Node).state = "complete";  //state를 complete로 변경
            memcpy(cache_table.GetAt(ARP_Node).macAddress , pFrame->sender_mac_address , 6); //MAC 주소 기입
            updateList();
            return true;
         }
         cache_table.GetNext(ARP_Node);   // 그 다음 entry 검사
      }
      /* ARP table에 없는 경우 엔트리 하나 새로 추가시켜줌 */
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);  //ip 주소 기입
      memcpy(input.macAddress , pFrame->sender_mac_address , 6);  //MAC 주소 기입
      input.state = "complete";  //state를 completet로 변경
      input.ttl = 150;  //time 설정
      cache_table.AddTail(input);
      updateList();
   }
}

void CARPLayer::updateList()  //ARP Table의 엔트리 정보를 갱신
{
   CacheTable temp;
   int i;
   ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.DeleteAllItems();

   for(i=0 ; i < cache_table.GetCount() ; i++)  //엔트리의 IP주소, MAC주소, state, time 정보를 세팅
   {   
      temp = cache_table.GetAt(cache_table.FindIndex(i));
      CString ipAddress , macAddress , state, ttl;

      ipAddress.Format("%d.%d.%d.%d" , temp.ipAddress[0] , temp.ipAddress[1] , 
         temp.ipAddress[2] , temp.ipAddress[3] );

      state = temp.state;
      if(state == "incomplete")  // state가 incomplete이면 MAC 주소 아래와 같이 기입(MAC 주소 모르므로)
      {
         macAddress.Format("??:??:??:??:??:??");
      }
      else
      {
      macAddress.Format("%02x:%02x:%02x:%02x:%02x:%02x" ,temp.macAddress[0] , temp.macAddress[1],
         temp.macAddress[2] , temp.macAddress[3] , temp.macAddress[4] , temp.macAddress[5]);
      }

      ttl.Format("%d" , temp.ttl);

     //Arp table 갱신
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.InsertItem( i , ipAddress , 0);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 0 , ipAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 1 , macAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 2 , state);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 3 , ttl);
   }
}
/* TEST CODE입니다!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**********************
void CARPLayer::updateListtest()  //list의 arp정보를 갱신하는 함수
{
   CacheTable temp;
   int i;
  // ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.DeleteAllItems();

   for( i = 0 ; i < cache_table.GetCount() ; i++)
   {   // 리스트에 있는 모든 노드들에 대한 정보를 listControl에 추가한다./
      temp = cache_table.GetAt(cache_table.FindIndex(i));
      CString ipAddress , macAddress , state, ttl;
      ipAddress.Format("%d.%d.%d.%d" , temp.ipAddress[0] , temp.ipAddress[1] , 
         temp.ipAddress[2] , temp.ipAddress[3] );
      state = temp.state;
      if(state == "incomplete")
      {   // 상태가 incomplete라면 맥주소는 모르는 걸로 표시한다.
         macAddress.Format("??:??:??:??:??:??");
      }
      else
      {
      macAddress.Format("%02x:%02x:%02x:%02x:%02x:%02x" ,temp.macAddress[0] , temp.macAddress[1],
         temp.macAddress[2] , temp.macAddress[3] , temp.macAddress[4] , temp.macAddress[5]);
      }

      ttl.Format("%d" , temp.ttl);

     // ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.InsertItem( i , ipAddress , 0);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 0 , ipAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 1 , macAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 2 , state);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 3 , ttl);
   }
}*/
int CARPLayer::SearchCacheTable() // 찾고 있는 IP주소가 있는 엔트리의 index를 return  
{
   int m_count;  //ARP Table에 있는 엔트리들 수를 받을 변수
   CacheTable temp;

   if(cache_table.GetCount() == 0)  //ARP table에 아무것도 없는 경우, -1 return
   {
      return -1;
   }
   else //있다면
   {
      m_count = cache_table.GetCount();
      for(int i=0 ; i < m_count ; i ++) //ARP table에 있는 엔트리 수만큼 반복
      {
         temp = cache_table.GetAt(cache_table.FindIndex(i));
         if(memcmp(temp.ipAddress , m_sHeader.target_ip_address , 4) == 0) // target IP 주소를 찾으면, 현재 인덱스를 return
         {
            return i;
         }
      }
   }
   //ARP table에 없으면 -1 return
   return -1;
}


void CARPLayer::reSend() //ARP Request를 재전송
{
   sendCounter++;

   if(sendCounter == 2)   //send counter가 재전송 횟수의 최대에 도달했으므로 더이상 재전송하지 않는다.
   {
      POSITION positionNode = cache_table.GetHeadPosition();

      for(int i=0 ; i < cache_table.GetCount() ; i++)  //table의 엔트리 수만큼 반복
      {   
         if(memcmp(m_sHeader.target_ip_address, cache_table.GetAt(positionNode).ipAddress, 4) == 0)  //찾으면
         {
            cache_table.GetAt(positionNode).ttl = 5;
            cache_table.GetAt(positionNode).state = "incomplete";

            updateList();
            SendMessage(HWND_BROADCAST, nRegKillRestartTimerMsg , 0, 0); //재전송 timer kill
            return;
         }
         cache_table.GetNext(positionNode);
     }

      /* ARP table에 없는 경우 엔트리 하나 새로 추가시켜줌 */
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);
      memcpy(input.macAddress , m_sHeader.target_mac_address , 6);
      input.state = "incomplete";
      input.ttl = 180;
      cache_table.AddTail(input);
      
      updateList();
      SendMessage(HWND_BROADCAST , nRegKillRestartTimerMsg , 0, 0);  //재전송 timer kill
      return;
   }
   
   /* 재전송 */
   SendMessage(HWND_BROADCAST , nRegArpSendMsg ,0 ,0);  //다시 보내기
   GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);
}


void CARPLayer::TableTimer() //ARP Table 엔트리들의 ttl 값을 1씩 감소시킴.
{
   POSITION entry = cache_table.GetHeadPosition();
   POSITION deleteEntry;

   if( cache_table.GetCount() != 0 )
   {
      for( int i=0; i<cache_table.GetCount(); i++ )  //Table에 있는 엔트리 수만큼 반복
      {
         cache_table.GetAt(entry).ttl--;   // ttl 값을 1초씩 줄임

       /* time 값 확인 - 0 된 엔트리는 table에서 삭제시켜야 하므로 */
         if ( cache_table.GetAt(entry).ttl == 0 )  //time 값이 0이 된 엔트리는 Table에서 삭제시킴
         {
            deleteEntry = entry;
            cache_table.GetNext(entry);
            cache_table.RemoveAt(deleteEntry);
         }
         else  //아니라면 다음 엔트리 가져옴
         {
            cache_table.GetNext(entry);
         }      
     }
     //모든 엔트리에 작업을 완료하면 Table update
      updateList();
   }
}

void CARPLayer::AddProxy(CString devName , unsigned char *ip , unsigned char *mac)
{   //Proxy table에 입력받은 내용들 추가
   ProxyItem temp;
   int i;
   
   CString Name ,  Ip , Mac ;
   Name = devName; 
   temp.devName = devName;
   memcpy(temp.proxyIP , ip , 4);
   memcpy(temp.proxyMac , mac , 6);
   proxy_table.AddTail(temp);

   updateProxyList();
}

void CARPLayer::updateProxyList() //Proxy table의 엔트리 정보 갱신
{
   // Proxy_Cache
   // proxy table 에 대한 listControl을 갱신한다. 갱신 방법은 updateList 함수와 동일하다.
   ProxyItem temp;
   int i;
   ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->Proxy_Cache.DeleteAllItems();

   for( i = 0 ; i < proxy_table.GetCount() ; i++)
   {
      temp = proxy_table.GetAt(proxy_table.FindIndex(i));
      CString ipAddress , macAddress  ;
      ipAddress.Format("%d.%d.%d.%d" , temp.proxyIP[0] , temp.proxyIP[1] , 
         temp.proxyIP[2] , temp.proxyIP[3] );

      macAddress.Format("%02x:%02x:%02x:%02x:%02x:%02x" ,temp.proxyMac[0] , temp.proxyMac[1],
         temp.proxyMac[2] , temp.proxyMac[3] , temp.proxyMac[4] , temp.proxyMac[5]);

      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->Proxy_Cache.InsertItem( i , temp.devName , i);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->Proxy_Cache.SetItemText( i , 0 , temp.devName);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->Proxy_Cache.SetItemText( i , 1 , ipAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->Proxy_Cache.SetItemText( i , 2 , macAddress);
   }
}

void CARPLayer::DeleteProxy(int index) // Proxy table의 엔트리 하나를 삭제
{
      proxy_table.RemoveAt(proxy_table.FindIndex(index));
      updateProxyList();      
}

void CARPLayer::AllDelete() //ARP table에 있는 모든 엔트리를 삭제
{
   cache_table.RemoveAll();      
   updateList();
}

void CARPLayer::DeleteItem(int index) //ARP table의 엔트리 하나를 삭제
{
   cache_table.RemoveAt(cache_table.FindIndex(index)); 
   updateList();
}