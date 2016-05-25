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
   memset( m_sHeader.padding , 0 , 18);   // packet size �� 60byte�� �����ִ� �������Դϴ�.
   memset( m_sHeader.enet_data, ETHER_MAX_DATA_SIZE, 6 ) ;
   m_sHeader.enet_type = 0 ;
}

void CEthernetLayer::SetSourceAddress(unsigned char *pAddress)
{
   memcpy(m_sHeader.enet_srcaddr, pAddress, 6); // ���� ����(*pAddress)�� 6ũ�⸸ŭ m_sHeader.enet_srcaddr�� �����մϴ�.

}

void CEthernetLayer::SetDestinAddress(unsigned char *pAddress)
{
   memcpy( m_sHeader.enet_dstaddr, pAddress, 6 ) ; // ���� ����(*pAddress)�� 6ũ�⸸ŭ m_sHeader.enet_dstaddr�� �����մϴ�.
}

BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength , int type)
{
   char broadCast[20];
   CString broadCastMac;
   BOOL bSuccess = FALSE ;
   
   memcpy( m_sHeader.enet_data, ppayload, nlength ) ;

   if(type == 0x01) //ARP Request �� ��
   {
      broadCastMac.Format("ff:ff:ff:ff:ff:ff");
      memcpy(broadCast , (char*)(LPCTSTR)broadCastMac , 18);
      sscanf(broadCast , "%02X:%02X:%02X:%02X:%02X:%02X",
         &m_DstMac[0],&m_DstMac[1],&m_DstMac[2],&m_DstMac[3],&m_DstMac[4],
         &m_DstMac[5]);
      SetSourceAddress(macMyAddress);   // source Address �� �ڽ��� �ּҷ� �����մϴ�.
      SetDestinAddress(m_DstMac);   // destination Address �� BroadCast�� �����մϴ�.
      m_sHeader.enet_type = htons(0x0806);   // ARP Packet���� type ���� ���� ������ݴϴ�.
   }
   else if( type == 0x02 )
   {   // ARP reply ���� �� �� �����ϴ� �ڵ��Դϴ�.
      SetSourceAddress(macMyAddress);   // source Address �� �ڽ��� �ּҷ� �����մϴ�.
      SetDestinAddress(((CARPLayer::Parp_Header)ppayload)->target_mac_address);   // destination Address �� ACK Packet�� ���۹��� ���� �����մϴ�.
//      memcpy( m_sHeader.enet_dstaddr , ((CARPLayer::Parp_Header)ppayload)->target_mac_address , 4);   // ACK Packet�� �������� ��븦 �����մϴ�.
   //   memcpy( m_sHeader.enet_srcaddr , macMyAddress , sizeof(macMyAddress));
      m_sHeader.enet_type = htons(0x0806);   // ARP Packet���� type ���� ���� ������ݴϴ�.
   }

   return mp_UnderLayer->Send( (unsigned char*)&m_sHeader , nlength+ETHER_HEADER_SIZE+18);   // ���� layer�� �����մϴ�.
}

BOOL CEthernetLayer::Receive( unsigned char* ppayload )
{
   PETHERNET_HEADER pFrame = (PETHERNET_HEADER)ppayload;
   
   if(ntohs(pFrame->enet_type) == 0x0806)   {// ���� ��Ŷ�� Ÿ���� ARP ���������̸� �������̾�� �����Ѵ�.
      return mp_aUpperLayer[0]->Receive((unsigned char*)&pFrame->enet_data);   
   }
      
   return false;
}