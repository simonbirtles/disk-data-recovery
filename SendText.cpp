// SendText.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "SendText.h"


// CSendText dialog

IMPLEMENT_DYNAMIC(CSendText, CDialog)
CSendText::CSendText(CWnd* pParent /*=NULL*/)
	: CDialog(CSendText::IDD, pParent)
{
}

CSendText::~CSendText()
{
}

void CSendText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT, m_edit);
}


BEGIN_MESSAGE_MAP(CSendText, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CSendText message handlers

void CSendText::OnBnClickedOk()
{   
	UpdateData(TRUE);
	m_edit.GetWindowText(m_strEdit);
	OnOK();
}
