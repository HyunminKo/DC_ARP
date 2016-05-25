#pragma once
#include "baselayer.h"
#include <pcap.h>
#include <Packet32.h>
#include <NtDDNdis.h>
#include <conio.h>


class CPacketDriverLayer :
   public CBaseLayer
{
   //DECLARE_DYNAMIC(CPacketDriverLayer)

public:
   
   BOOL Send(unsigned char* ppayload , int nlength);
   BOOL Receive(unsigned char* a);
   pcap_t* eth;

   CPacketDriverLayer(char* pName);
   ~CPacketDriverLayer(void);
};