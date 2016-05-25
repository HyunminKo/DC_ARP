#include "StdAfx.h"
#include "ARPLayer.h"
#include "DC_ARP_7Dlg.h"

const DWORD CARPLayer::nRegArpSendMsg = ::RegisterWindowMessage( "ARP Send Message" ); //�޼��� ���
const DWORD CARPLayer::nRegKillRestartTimerMsg = ::RegisterWindowMessage( "ARP Send Kill Timer "); //�޼������

CARPLayer::CARPLayer(char* pName) //������
   :CBaseLayer( pName )
{
   ResetHeader();// header�κ� Reset
}


CARPLayer::~CARPLayer(void) //�Ҹ���
{
}

void CARPLayer::ResetHeader()//Header�κ� Reset�ϴ� �޼ҵ�.
{
   memset( m_sHeader.sender_mac_address , 0 , 6);//sender�� Mac�ּ� �ʱ�ȭ
   memset( m_sHeader.target_mac_address , 0 , 6);//tartget�� Mac�ּ� �ʱ�ȭ

}

void CARPLayer::setDstIpAddress( unsigned char* pAddress ) //������ Address ����
{
      memcpy(m_sHeader.target_ip_address , pAddress , 4);//���� ���̾�� ���� �ּҸ� target address�� ����
}

void CARPLayer::setSrcIpAddress( unsigned char* pAddress ) //���� ���̾�� ���� �ּҸ� source address�� ����
{
      memcpy(m_sHeader.sender_ip_address , pAddress , 4);
}

BOOL CARPLayer::Send(unsigned char* ppayload, int nlength) //���� ���̾�� ������ �޾� ARP�� ������ �Լ�
{
   if( *ppayload == 'A' )
   {// �Ѿ�� ppayload�� ���� ���� A�̸� ARP Request �Ǵ��Ѵ�.
     ppayload = NULL;//ó���� �Ѿ�� ppayload�� ����־�� �ϱ� ������ NULL�� ����
      sendCounter = 0; // ī���� �ʱ�ȭ
      int i = SearchCacheTable(); //�̹� ĳ�����̺� IP �ּҰ� �ִٸ� �ش��ϴ� ��Ʈ���� �ε��� ��ȯ

      if( i != -1 &&  cache_table.GetAt(cache_table.FindIndex(i)).state == "complete")
      {   // ĳ�����̺��� ������� �ʰ� ���°� complete �̶��
         CString exist_macAddress;
         CString caption = "!!";
         exist_macAddress.Format("�̹� �ش� IP�� ���� MAC�ּҰ� ��ġ�Ǿ� �ֽ��ϴ�\nARP Request�� �����մϴ�.\n%02x:%02x:%02x:%02x:%02x:%02x" , 
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[0] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[1] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[2] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[3] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[4] ,
            cache_table.GetAt(cache_table.FindIndex(i)).macAddress[5] );
         ::MessageBox(((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->GetSafeHwnd() , exist_macAddress ,
            caption , MB_OK | MB_ICONEXCLAMATION);   // �̹� ip�� mac �ּҰ� ��ġ�Ǿ��ִٰ� �޽����� ���� ���� 
      }
      else
      {
      //Request�� ���� ARP Header setting
      //������ table �ȿ� �˸��� ���� �־��ش�.
         m_sHeader.hard_type = htons(0x0001);
         m_sHeader.prot_type = htons(0x0800);
         m_sHeader.hard_size = 6;
         m_sHeader.prot_size = 4;
         m_sHeader.op = htons(0x0001);//request
         memcpy( m_sHeader.sender_mac_address , macMyAddress , sizeof(macMyAddress));
       //ARP ����� senderȣ��Ʈ�� ���ּ� ����
         SendMessage(HWND_BROADCAST , nRegArpSendMsg ,0 , 0);//���� ���̾ arpmessage�� broadcast�� ���� 

         return GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);//���⼭ ����� ������ ���� ���̾�� ����
      }
   }
   else if(*ppayload == 'G')
   {// �Ѿ�� ppayload�� ���� ���� G�̸� Gratuitous�� ����
      ppayload=NULL;//ó���� �Ѿ�� ppayload�� ����־�� �ϱ� ������ NULL�� ����
         m_sHeader.hard_type = htons(0x0001);
         m_sHeader.prot_type = htons(0x0800);
         m_sHeader.hard_size = 6;
         m_sHeader.prot_size = 4;
         m_sHeader.op = htons(0x0001);

         memcpy( m_sHeader.sender_mac_address , globalMacAddress , sizeof(globalMacAddress));// �� Mac Address �� �Է�
         return GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);//����� ������ ���� ���̾�� ����
   }
}

BOOL CARPLayer::Receive(unsigned char* ppayload) //ARP�� �޾Ƽ� ó���ϴ� �Լ�
{
   Parp_Header pFrame = (Parp_Header)ppayload;//pFrame�� �������� �޾ƿ� ppayload�� �ִ� ������ ����

   if(memcmp(pFrame->sender_ip_address , m_sHeader.sender_ip_address, 4) == 0)
      return FALSE;   // �ڱⰡ ���� ARP Request�̸� �Լ��� �����ϸ� ��Ŷ�� ���� ���̾�� �������� �ʴ´�.


   if(ntohs(pFrame->op) == 0x0001 && memcmp(pFrame->target_ip_address , pFrame->sender_ip_address , 4 ) == 0)
   {   // Gratuitous ARP �� ���� ����
      POSITION ARP_Node = cache_table.GetHeadPosition();//ARP_Node�� cache table�� ��� ��ġ�� ����
      for( int i=0; i<cache_table.GetCount(); i++ )//ĳ�����̺��� ���鼭 Ȯ��.
      {

         if( memcmp(pFrame->sender_ip_address , cache_table.GetAt(ARP_Node).ipAddress , 4 ) == 0 )
         {   //cache table�� �ִ� ���, ip�� ��ġ �ϸ�

            /////////////////////////////////////////////
            cache_table.GetAt(ARP_Node).ttl = 200;
            /////////////////////////////////////////////
            cache_table.GetAt(ARP_Node).state = "complete";
            memcpy(cache_table.GetAt(ARP_Node).macAddress , pFrame->sender_mac_address , 6);//�ٲ� MAC Address�� ����
            updateList();//list�� ������Ʈ
            return true;
         }
         cache_table.GetNext(ARP_Node);   // ip�� ��ġ���� ������ ���� entry�� �Ѿ��.
      }
      //cache table�� �������� ������ �������� ����.
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);
      memcpy(input.macAddress , pFrame->sender_mac_address , 6);
      input.state = "complete";
      input.ttl = 190;
      cache_table.AddTail(input);//����� ��带 cache table�� ���� �߰����ش�.
      updateList();//list�� ������Ʈ
   }
   else if(ntohs(pFrame->op) == 0x0001 &&// Request�� ���� reply����
      memcmp(pFrame->target_ip_address , m_sHeader.sender_ip_address ,  4 ) == 0)
   {   // ARP�� �ް� �ڽ��� ���� ������
      m_sHeader.hard_type = htons(0x0001);
      m_sHeader.prot_type = htons(0x0800);
      m_sHeader.hard_size = 6;
      m_sHeader.prot_size = 4;
      m_sHeader.op = htons(0x0002);//reply code

      memcpy( m_sHeader.sender_mac_address , macMyAddress , 6);
      memcpy( m_sHeader.sender_ip_address , pFrame->target_ip_address , 4);
      memcpy( m_sHeader.target_ip_address , pFrame->sender_ip_address , 4);
      memcpy( m_sHeader.target_mac_address , pFrame->sender_mac_address , 6);
      // sender �� target�� ip, Mac address�� swap ���ش�.
      if(SearchCacheTable() == -1)
      {    //cache table�� �������� ������ �������� ����.
         CacheTable input;
         memcpy(input.ipAddress , pFrame->sender_ip_address , 4);
         memcpy(input.macAddress , pFrame->sender_mac_address , 6);
         input.state = "complete";
         input.ttl = 180;
         cache_table.AddTail(input);//����� ��带 cache table�� ���� �߰����ش�.
         updateList();
      }
      return mp_UnderLayer->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader), 0x02);   // ACK Packat�� ���� ���̾�� ������.
   }
   else if(ntohs(pFrame->op) == 0x0001)
   {   // ARP Request������ ���� �ּҰ� �ƴ� ��,
      POSITION Proxy_Node = proxy_table.GetHeadPosition();//Proxy_Node�� Poxcy cache table�� ��� ��ġ�� ����
      for( int i = 0 ; i < proxy_table.GetCount() ; i++)//���Ͻ� ���̺��� ���鼭 Ȯ��
      {   
         if(memcmp(pFrame->target_ip_address , proxy_table.GetAt(Proxy_Node).proxyIP , 4 ) == 0 )
         {   // Ÿ���� ip address�� proxy table�� �ִ� ip �ּҰ� ������,
            m_sHeader.hard_type = htons(0x0001);
            m_sHeader.prot_type = htons(0x0800);
            m_sHeader.hard_size = 6;
            m_sHeader.prot_size = 4;
            m_sHeader.op = htons(0x0002);//reply

            memcpy( m_sHeader.sender_mac_address , macMyAddress , 6);
            memcpy( m_sHeader.sender_ip_address , pFrame->target_ip_address , 4);
            memcpy( m_sHeader.target_ip_address , pFrame->sender_ip_address , 4);
            memcpy( m_sHeader.target_mac_address , pFrame->sender_mac_address , 6);
            // Mac �ּҴ� ����� ������ �� ���� Mac�ּҸ� ������.
            if(SearchCacheTable() == -1)
            {  //cache table�� �������� ������ �������� ����.(�޴� ���忡��)
               CacheTable input;
               memcpy(input.ipAddress , pFrame->sender_ip_address , 4);
               memcpy(input.macAddress , pFrame->sender_mac_address , 6);
               input.state = "complete";
               input.ttl = 170;
               cache_table.AddTail(input); //cache table�� �������� ������ �������� ����.
               updateList();
            }
            return mp_UnderLayer->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader), 0x02);
         }
         proxy_table.GetNext(Proxy_Node);
      }
   }//proxy ��.
    else if(ntohs(pFrame->op) == 0x0002 &&
      memcmp(pFrame->target_ip_address, m_sHeader.sender_ip_address, 4) == 0)
   {      // reply�� ���� ���
      SendMessage(HWND_BROADCAST , nRegKillRestartTimerMsg , 0, 0);// reply�� �޾����� �ٽ� �������� �ʿ䰡 �����Ƿ� ������ timer kill
      
     /* ARP table ���� */
     POSITION ARP_Node = cache_table.GetHeadPosition();
      for( int i=0; i<cache_table.GetCount(); i++ )
      {
         if( memcmp(pFrame->sender_ip_address , cache_table.GetAt(ARP_Node).ipAddress , 4 ) == 0 )
         {   //ARP table�� reply�� ���� ȣ��Ʈ�� ip�ּҰ� �ִ� ���
            cache_table.GetAt(ARP_Node).ttl = 160; //time ����
            cache_table.GetAt(ARP_Node).state = "complete";  //state�� complete�� ����
            memcpy(cache_table.GetAt(ARP_Node).macAddress , pFrame->sender_mac_address , 6); //MAC �ּ� ����
            updateList();
            return true;
         }
         cache_table.GetNext(ARP_Node);   // �� ���� entry �˻�
      }
      /* ARP table�� ���� ��� ��Ʈ�� �ϳ� ���� �߰������� */
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);  //ip �ּ� ����
      memcpy(input.macAddress , pFrame->sender_mac_address , 6);  //MAC �ּ� ����
      input.state = "complete";  //state�� completet�� ����
      input.ttl = 150;  //time ����
      cache_table.AddTail(input);
      updateList();
   }
}

void CARPLayer::updateList()  //ARP Table�� ��Ʈ�� ������ ����
{
   CacheTable temp;
   int i;
   ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.DeleteAllItems();

   for(i=0 ; i < cache_table.GetCount() ; i++)  //��Ʈ���� IP�ּ�, MAC�ּ�, state, time ������ ����
   {   
      temp = cache_table.GetAt(cache_table.FindIndex(i));
      CString ipAddress , macAddress , state, ttl;

      ipAddress.Format("%d.%d.%d.%d" , temp.ipAddress[0] , temp.ipAddress[1] , 
         temp.ipAddress[2] , temp.ipAddress[3] );

      state = temp.state;
      if(state == "incomplete")  // state�� incomplete�̸� MAC �ּ� �Ʒ��� ���� ����(MAC �ּ� �𸣹Ƿ�)
      {
         macAddress.Format("??:??:??:??:??:??");
      }
      else
      {
      macAddress.Format("%02x:%02x:%02x:%02x:%02x:%02x" ,temp.macAddress[0] , temp.macAddress[1],
         temp.macAddress[2] , temp.macAddress[3] , temp.macAddress[4] , temp.macAddress[5]);
      }

      ttl.Format("%d" , temp.ttl);

     //Arp table ����
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.InsertItem( i , ipAddress , 0);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 0 , ipAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 1 , macAddress);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 2 , state);
      ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.SetItemText( i , 3 , ttl);
   }
}
/* TEST CODE�Դϴ�!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**********************
void CARPLayer::updateListtest()  //list�� arp������ �����ϴ� �Լ�
{
   CacheTable temp;
   int i;
  // ((CDC_ARP_7Dlg*)mp_aUpperLayer[0])->ARP_Cache.DeleteAllItems();

   for( i = 0 ; i < cache_table.GetCount() ; i++)
   {   // ����Ʈ�� �ִ� ��� ���鿡 ���� ������ listControl�� �߰��Ѵ�./
      temp = cache_table.GetAt(cache_table.FindIndex(i));
      CString ipAddress , macAddress , state, ttl;
      ipAddress.Format("%d.%d.%d.%d" , temp.ipAddress[0] , temp.ipAddress[1] , 
         temp.ipAddress[2] , temp.ipAddress[3] );
      state = temp.state;
      if(state == "incomplete")
      {   // ���°� incomplete��� ���ּҴ� �𸣴� �ɷ� ǥ���Ѵ�.
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
int CARPLayer::SearchCacheTable() // ã�� �ִ� IP�ּҰ� �ִ� ��Ʈ���� index�� return  
{
   int m_count;  //ARP Table�� �ִ� ��Ʈ���� ���� ���� ����
   CacheTable temp;

   if(cache_table.GetCount() == 0)  //ARP table�� �ƹ��͵� ���� ���, -1 return
   {
      return -1;
   }
   else //�ִٸ�
   {
      m_count = cache_table.GetCount();
      for(int i=0 ; i < m_count ; i ++) //ARP table�� �ִ� ��Ʈ�� ����ŭ �ݺ�
      {
         temp = cache_table.GetAt(cache_table.FindIndex(i));
         if(memcmp(temp.ipAddress , m_sHeader.target_ip_address , 4) == 0) // target IP �ּҸ� ã����, ���� �ε����� return
         {
            return i;
         }
      }
   }
   //ARP table�� ������ -1 return
   return -1;
}


void CARPLayer::reSend() //ARP Request�� ������
{
   sendCounter++;

   if(sendCounter == 2)   //send counter�� ������ Ƚ���� �ִ뿡 ���������Ƿ� ���̻� ���������� �ʴ´�.
   {
      POSITION positionNode = cache_table.GetHeadPosition();

      for(int i=0 ; i < cache_table.GetCount() ; i++)  //table�� ��Ʈ�� ����ŭ �ݺ�
      {   
         if(memcmp(m_sHeader.target_ip_address, cache_table.GetAt(positionNode).ipAddress, 4) == 0)  //ã����
         {
            cache_table.GetAt(positionNode).ttl = 5;
            cache_table.GetAt(positionNode).state = "incomplete";

            updateList();
            SendMessage(HWND_BROADCAST, nRegKillRestartTimerMsg , 0, 0); //������ timer kill
            return;
         }
         cache_table.GetNext(positionNode);
     }

      /* ARP table�� ���� ��� ��Ʈ�� �ϳ� ���� �߰������� */
      CacheTable input;
      memcpy(input.ipAddress , m_sHeader.target_ip_address , 4);
      memcpy(input.macAddress , m_sHeader.target_mac_address , 6);
      input.state = "incomplete";
      input.ttl = 180;
      cache_table.AddTail(input);
      
      updateList();
      SendMessage(HWND_BROADCAST , nRegKillRestartTimerMsg , 0, 0);  //������ timer kill
      return;
   }
   
   /* ������ */
   SendMessage(HWND_BROADCAST , nRegArpSendMsg ,0 ,0);  //�ٽ� ������
   GetUnderLayer()->Send((unsigned char*)&m_sHeader , sizeof(m_sHeader) , 0x01);
}


void CARPLayer::TableTimer() //ARP Table ��Ʈ������ ttl ���� 1�� ���ҽ�Ŵ.
{
   POSITION entry = cache_table.GetHeadPosition();
   POSITION deleteEntry;

   if( cache_table.GetCount() != 0 )
   {
      for( int i=0; i<cache_table.GetCount(); i++ )  //Table�� �ִ� ��Ʈ�� ����ŭ �ݺ�
      {
         cache_table.GetAt(entry).ttl--;   // ttl ���� 1�ʾ� ����

       /* time �� Ȯ�� - 0 �� ��Ʈ���� table���� �������Ѿ� �ϹǷ� */
         if ( cache_table.GetAt(entry).ttl == 0 )  //time ���� 0�� �� ��Ʈ���� Table���� ������Ŵ
         {
            deleteEntry = entry;
            cache_table.GetNext(entry);
            cache_table.RemoveAt(deleteEntry);
         }
         else  //�ƴ϶�� ���� ��Ʈ�� ������
         {
            cache_table.GetNext(entry);
         }      
     }
     //��� ��Ʈ���� �۾��� �Ϸ��ϸ� Table update
      updateList();
   }
}

void CARPLayer::AddProxy(CString devName , unsigned char *ip , unsigned char *mac)
{   //Proxy table�� �Է¹��� ����� �߰�
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

void CARPLayer::updateProxyList() //Proxy table�� ��Ʈ�� ���� ����
{
   // Proxy_Cache
   // proxy table �� ���� listControl�� �����Ѵ�. ���� ����� updateList �Լ��� �����ϴ�.
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

void CARPLayer::DeleteProxy(int index) // Proxy table�� ��Ʈ�� �ϳ��� ����
{
      proxy_table.RemoveAt(proxy_table.FindIndex(index));
      updateProxyList();      
}

void CARPLayer::AllDelete() //ARP table�� �ִ� ��� ��Ʈ���� ����
{
   cache_table.RemoveAll();      
   updateList();
}

void CARPLayer::DeleteItem(int index) //ARP table�� ��Ʈ�� �ϳ��� ����
{
   cache_table.RemoveAt(cache_table.FindIndex(index)); 
   updateList();
}