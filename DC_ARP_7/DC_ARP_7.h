
// DC_ARP_7.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CDC_ARP_7App:
// �� Ŭ������ ������ ���ؼ��� DC_ARP_7.cpp�� �����Ͻʽÿ�.
//

class CDC_ARP_7App : public CWinApp
{
public:
	CDC_ARP_7App();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CDC_ARP_7App theApp;