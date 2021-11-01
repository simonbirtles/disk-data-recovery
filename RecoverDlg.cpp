// RecoverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "RecoverDlg.h"


// CRecoverDlg dialog

IMPLEMENT_DYNAMIC(CRecoverDlg, CDialog)
CRecoverDlg::CRecoverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecoverDlg::IDD, pParent)
{
}

CRecoverDlg::~CRecoverDlg()
{
}

void CRecoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRecoverDlg, CDialog)
	ON_BN_CLICKED(IDSELECTFILE, OnBnClickedSelectfile)
END_MESSAGE_MAP()


// CRecoverDlg message handlers

void CRecoverDlg::OnBnClickedSelectfile()
{

	CFileDialog cfile(false, "rec", "Recovered", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 0);

	if(cfile.DoModal() == IDOK)
	{
		m_strFilename = cfile.GetPathName();
		CEdit* cedit = (CEdit*)this->GetDlgItem(IDC_FILESELECTED);
		cedit->SetWindowText(m_strFilename);

	}


}



BOOL CRecoverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	
	CEdit* cedit = (CEdit*)this->GetDlgItem(IDC_FILENAME);
	cedit->SetWindowText(m_strRecoverFile);

	CEdit* cedit1 = (CEdit*)this->GetDlgItem(IDC_FILESELECTED);
	cedit1->SetWindowText(m_strFilename);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
