#include "StdAfx.h"
#include "PacketDriverLayer.h"
#include "DC_ARP_7Dlg.h"

CPacketDriverLayer::CPacketDriverLayer(char* pName)
   :CBaseLayer( pName )
{
}


CPacketDriverLayer::~CPacketDriverLayer(void)
{
}


BOOL CPacketDriverLayer :: Send(unsigned char *ppayload , int nlength){
   pcap_t* fp;      // pcap lib
   char errbuf[PCAP_ERRBUF_SIZE];
   u_char packet[100];
   int i;
   if((fp  = pcap_open_live(g_nicName, 65536 , 1 , 1000 , errbuf))==NULL)   // 네트워크 드라이버 가져오기
   {
      AfxMessageBox(_T("\nUnable to open the adapter. %s is not supported by WinPcap\n",g_nicName) );
      return FALSE;
   }


   if (pcap_sendpacket(fp, ppayload, nlength) != 0)// 가져온 네트워크에 패킷 보내기
   {
      CString errmsg;
      errmsg.Format("\nError sending the packet: %s\n", pcap_geterr(fp));
      AfxMessageBox(errmsg);

      return 3;
   }
   pcap_close(fp);   
}

BOOL CPacketDriverLayer :: Receive(unsigned char* ppayload){
   BOOLEAN rel = mp_aUpperLayer[0]->Receive(ppayload); //받은 패킷을 상위레이어(이더넷 레이어)로 전달
   return rel;
}

UINT threadCapture(LPVOID pProc){      // thread fuction must declare Global function
   char errbuf[PCAP_ERRBUF_SIZE];      // Error Log buff
   pcap_t* eth;                  // pcap dataStructure
   eth = pcap_open_live(g_nicName, 65536, 1, 1000, errbuf);   // Open Adapter
   CDC_ARP_7Dlg *p_IPCDlg = (CDC_ARP_7Dlg*)pProc; // Cast to CIPCAppDlg from LPVOID
   const BYTE* data_buffer;

   while(1){   // Infinite Loop
      struct pcap_pkthdr *header;
      int result = pcap_next_ex(eth, &header, &data_buffer);
         // Packet capture copy data_buffer(Global variable)

      switch(result){
      case 1:   // if PacketCapture complete
         p_IPCDlg->m_pdLayer->Receive((unsigned char*)data_buffer);   // Run Receive
      case 0:
      case -1:
      case -2:
      default:
         break;
      }
   }
}