#pragma once
#include "baselayer.h"
class CEthernetLayer :
   public CBaseLayer
{
public:

   

   typedef struct _ETHERNET_HEADER 
   {
      unsigned char   enet_dstaddr[6];
      unsigned char   enet_srcaddr[6];
      unsigned short   enet_type;
      
      unsigned char   enet_data[ ETHER_MAX_DATA_SIZE ];
      unsigned char   padding[18];
   } ETHERNET_HEADER , *PETHERNET_HEADER ;

   ETHERNET_HEADER   m_sHeader ;

   void         ResetHeader();
   BOOL         Send( unsigned char* ppayload, int nlength , int type);
   void         SetDestinAddress( unsigned char* pAddress );
   void         SetSourceAddress( unsigned char* pAddress );
   BOOL         Receive( unsigned char* ppayload);
   CEthernetLayer(char* pName);
   ~CEthernetLayer(void);
};
