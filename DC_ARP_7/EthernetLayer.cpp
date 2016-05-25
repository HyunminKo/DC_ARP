#include "StdAfx.h"
#include "EthernetLayer.h"
#include "ARPLayer.h"

CEthernetLayer::CEthernetLayer(char* pName)
   :CBaseLayer( pName )
{
   ResetHeader();
}


CEthernetLayer::~CEthernetLayer(void)
{
}

void CEthernetLayer::ResetHeader()
{
   memset( m_sHeader.enet_dstaddr, 0, 6 ) ;
   memset( m_sHeader.enet_srcaddr, 0, 6 ) ;
   memset( m_sHeader.padding , 0 , 18);   // packet size 를 60byte로 맞춰주는 변수값입니다.
   memset( m_sHeader.enet_data, ETHER_MAX_DATA_SIZE, 6 ) ;
   m_sHeader.enet_type = 0 ;
}

void CEthernetLayer::SetSourceAddress(unsigned char *pAddress)
{
   memcpy(m_sHeader.enet_srcaddr, pAddress, 6); // 받은 인자(*pAddress)를 6크기만큼 m_sHeader.enet_srcaddr에 복사합니다.

}

void CEthernetLayer::SetDestinAddress(unsigned char *pAddress)
{
   memcpy( m_sHeader.enet_dstaddr, pAddress, 6 ) ; // 받은 인자(*pAddress)를 6크기만큼 m_sHeader.enet_dstaddr에 복사합니다.
}

BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength , int type)
{
   char broadCast[20];
   CString broadCastMac;
   BOOL bSuccess = FALSE ;
   
   memcpy( m_sHeader.enet_data, ppayload, nlength ) ;

   if(type == 0x01) //ARP Request 일 때
   {
      broadCastMac.Format("ff:ff:ff:ff:ff:ff");
      memcpy(broadCast , (char*)(LPCTSTR)broadCastMac , 18);
      sscanf(broadCast , "%02X:%02X:%02X:%02X:%02X:%02X",
         &m_DstMac[0],&m_DstMac[1],&m_DstMac[2],&m_DstMac[3],&m_DstMac[4],
         &m_DstMac[5]);
      SetSourceAddress(macMyAddress);   // source Address 를 자신의 주소로 저장합니다.
      SetDestinAddress(m_DstMac);   // destination Address 를 BroadCast로 지정합니다.
      m_sHeader.enet_type = htons(0x0806);   // ARP Packet임을 type 값을 통해 명시해줍니다.
   }
   else if( type == 0x02 )
   {   // ARP reply 전송 일 때 수행하는 코드입니다.
      SetSourceAddress(macMyAddress);   // source Address 를 자신의 주소로 저장합니다.
      SetDestinAddress(((CARPLayer::Parp_Header)ppayload)->target_mac_address);   // destination Address 를 ACK Packet을 전송받을 상대로 지정합니다.
//      memcpy( m_sHeader.enet_dstaddr , ((CARPLayer::Parp_Header)ppayload)->target_mac_address , 4);   // ACK Packet을 전송해줄 상대를 지정합니다.
   //   memcpy( m_sHeader.enet_srcaddr , macMyAddress , sizeof(macMyAddress));
      m_sHeader.enet_type = htons(0x0806);   // ARP Packet임을 type 값을 통해 명시해줍니다.
   }

   return mp_UnderLayer->Send( (unsigned char*)&m_sHeader , nlength+ETHER_HEADER_SIZE+18);   // 하위 layer로 전송합니다.
}

BOOL CEthernetLayer::Receive( unsigned char* ppayload )
{
   PETHERNET_HEADER pFrame = (PETHERNET_HEADER)ppayload;
   
   if(ntohs(pFrame->enet_type) == 0x0806)   {// 받은 패킷의 타입이 ARP 프로토콜이면 상위레이어로 전달한다.
      return mp_aUpperLayer[0]->Receive((unsigned char*)&pFrame->enet_data);   
   }
      
   return false;
}