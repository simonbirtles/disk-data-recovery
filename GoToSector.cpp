// GoToSector.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "GoToSector.h"


// CGoToSector dialog

IMPLEMENT_DYNAMIC(CGoToSector, CDialog)
CGoToSector::CGoToSector(CWnd* pParent /*=NULL*/)
	: CDialog(CGoToSector::IDD, pParent)
{
	m_strTitle = "Go To Sector";
}

CGoToSector::~CGoToSector()
{
}

void CGoToSector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SECTORNO, m_ip);
}


BEGIN_MESSAGE_MAP(CGoToSector, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CGoToSector message handlers

void CGoToSector::OnBnClickedOk()
{
	CString str;
	CEdit* cedit = (CEdit*)this->GetDlgItem(IDC_SECTORNO);
	cedit->GetWindowText(str);

	
	iSectorNo = atoi((LPCSTR)str);



	OnOK();
}





BOOL CGoToSector::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(m_strTitle);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
