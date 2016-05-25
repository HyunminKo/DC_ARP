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

void CBaseLayer::SetUnderUpperLayer(CBaseLayer *pUULayer) // �־��� ��ū�� ���� �Լ����� ��������� �����Ͽ� �����Ͽ����ϴ�.
{
	if ( !pUULayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUnderUpperLayer] The variable , 'pUULayer' is NULL" ) ;
#endif
		return ;			
	}
	pUULayer->mp_UnderLayer = this; // pUULayer�� ���� �������� ���� push�Ǵ� �����̹Ƿ� pUULayer->mp_UnderLayer(�Ʒ�����)�� ��������� ����Ų��.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; //this->mp_aUpperLayer[m_nUpperLayerCount++] ��������� �������� pUULayer�� �ȴ�
}

void CBaseLayer::SetUpperUnderLayer(CBaseLayer *pUULayer)// * ��ū�� ���� Ȱ��ȭ �Ǵ� �Լ�, �����(��,����)�� ���� �ϱ� ���� �Լ� //���� ����
{
	if ( !pUULayer ) // if the pointer is null, 
	{
#ifdef _DEBUG
		TRACE( "[CBaseLayer::SetUpperUnderLayer] The variable , 'pUULayer' is NULL" ) ;
#endif
		return ;			
	}


	pUULayer->mp_UnderLayer = this; // pUULayer�� ���� �������� ���� push�Ǵ� �����̹Ƿ� pUULayer->mp_UnderLayer(�Ʒ�����)�� ��������� ����Ų��.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; //this->mp_aUpperLayer[m_nUpperLayerCount++] ��������� �������� pUULayer�� �ȴ�

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
