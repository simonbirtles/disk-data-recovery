// DlgIPAddress.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "DlgIPAddress.h"


// CDlgIPAddress dialog

IMPLEMENT_DYNAMIC(CDlgIPAddress, CDialog)
CDlgIPAddress::CDlgIPAddress(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIPAddress::IDD, pParent)
{
}

CDlgIPAddress::~CDlgIPAddress()
{
	
}

void CDlgIPAddress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipaddr32);
}


BEGIN_MESSAGE_MAP(CDlgIPAddress, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgIPAddress message handlers

void CDlgIPAddress::OnBnClickedOk()
{
	BYTE b0,b1,b2,b3;

	m_ipaddr32.GetAddress(b0, b1, b2, b3);

	m_ipaddr = (LPTSTR)VirtualAlloc(NULL, 16, MEM_COMMIT, PAGE_READWRITE);

	wsprintf(m_ipaddr, "%i.%i.%i.%i", b0,b1,b2,b3);


	OnOK();
}
