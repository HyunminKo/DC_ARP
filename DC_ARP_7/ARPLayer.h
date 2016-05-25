#pragma once
#include "baselayer.h"
class CARPLayer :
   public CBaseLayer
{
public:

   //structure
   typedef struct ArpHeaderStruct
   {
      unsigned short hard_type; //2
      unsigned short prot_type; //2
      unsigned char hard_size; //1
      unsigned char prot_size;   //1
      unsigned short op; //2
      unsigned char sender_mac_address[6];
      unsigned char sender_ip_address[4];   //4
      unsigned char target_mac_address[6];
      unsigned char target_ip_address[4];

   }arp_Header , *Parp_Header;

   typedef struct CacheTableStruct{
      unsigned char macAddress[6];
      unsigned char ipAddress[4];
      CString state;
      unsigned short ttl;
   } CacheTable , *PCacheTable;

   typedef struct ProxyTableStruct {
      CString devName; 
      unsigned char proxyIP[4];
      unsigned char proxyMac[6];
   } ProxyItem;
   // field
   arp_Header m_sHeader;
   ProxyItem proxy_item;
   CList<CacheTable , CacheTable&> cache_table;
   CList<ProxyItem , ProxyItem&> proxy_table;
   unsigned short sendCounter;
   unsigned char runMode;
   // function
   BOOL         Send( unsigned char* ppayload, int nlength);
   BOOL         Receive( unsigned char* ppayload );
   void         ResetHeader();
   void         setDstIpAddress( unsigned char* pAddress );
   void         setSrcIpAddress( unsigned char* pAddress );
   void         updateList();
   void         updateListtest();
   int          SearchCacheTable();
   void         reSend();
   void         TableTimer();
   void         SetMode(char mode);
   void         AddProxy(CString devName , unsigned char *ip , unsigned char *mac);
   void         updateProxyList();
   void         DeleteProxy(int index);
   void         AllDelete();
   void         DeleteItem(int index);
   CARPLayer(char* pName);
   ~CARPLayer(void);

   static const DWORD nRegArpSendMsg;
   static const DWORD nRegKillRestartTimerMsg;
};
