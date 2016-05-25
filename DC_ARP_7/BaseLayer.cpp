#include "StdAfx.h"
#include "DC_ARP_7.h"
#include "BaseLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBaseLayer::CBaseLayer( char* pName ) 
: m_nUpperLayerCount( 0 ), 
  mp_UnderLayer( NULL )
{
	m_pLayerName = pName ;
}

CBaseLayer::~CBaseLayer()
{

}

void CBaseLayer::SetUnderUpperLayer(CBaseLayer *pUULayer) // 주어진 토큰이 없는 함수여서 양방향으로 가정하에 구현하였습니다.
{
	if ( !pUULayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUnderUpperLayer] The variable , 'pUULayer' is NULL" ) ;
#endif
		return ;			
	}
	pUULayer->mp_UnderLayer = this; // pUULayer는 현재 계층에서 위로 push되는 계층이므로 pUULayer->mp_UnderLayer(아래계층)은 현재계층을 가리킨다.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; //this->mp_aUpperLayer[m_nUpperLayerCount++] 현재계층의 위계층은 pUULayer가 된다
}

void CBaseLayer::SetUpperUnderLayer(CBaseLayer *pUULayer)// * 토큰에 의해 활성화 되는 함수, 양방향(송,수신)을 정의 하기 위한 함수 //내가 위에
{
	if ( !pUULayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUpperUnderLayer] The variable , 'pUULayer' is NULL" ) ;
#endif
		return ;			
	}


	pUULayer->mp_UnderLayer = this; // pUULayer는 현재 계층에서 위로 push되는 계층이므로 pUULayer->mp_UnderLayer(아래계층)은 현재계층을 가리킨다.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; //this->mp_aUpperLayer[m_nUpperLayerCount++] 현재계층의 위계층은 pUULayer가 된다

}

void CBaseLayer::SetUpperLayer(CBaseLayer *pUpperLayer )
{
	if ( !pUpperLayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUpperLayer] The variable , 'pUpperLayer' is NULL" ) ;
#endif
		return ;			
	}

	// UpperLayer is added..
	this->mp_aUpperLayer[ m_nUpperLayerCount++ ] = pUpperLayer ;
}

void CBaseLayer::SetUnderLayer(CBaseLayer *pUnderLayer) 
{
	if ( !pUnderLayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUnderLayer] The variable , 'pUnderLayer' is NULL\n" ) ;
#endif
		return ;			
	}

	// UnderLayer assignment..
	this->mp_UnderLayer = pUnderLayer;
}

CBaseLayer* CBaseLayer::GetUpperLayer(int nindex)
{
	if ( nindex < 0						||
		 nindex > m_nUpperLayerCount	||
		 m_nUpperLayerCount < 0			)
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::GetUpperLayer] There is no UpperLayer in Array..\n" ) ;
#endif 
		return NULL ;
	}

	return mp_aUpperLayer[ nindex ] ;
}



CBaseLayer* CBaseLayer::GetUnderLayer()
{
	if ( !mp_UnderLayer )
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::GetUnderLayer] There is not a UnerLayer..\n" ) ;
#endif 
		return NULL ;
	}

	return mp_UnderLayer ;
}

char* CBaseLayer::GetLayerName()
{
	return m_pLayerName ; 
}
