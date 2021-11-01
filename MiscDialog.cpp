// MiscDialog.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "MiscDialog.h"


// CMiscDialog dialog

IMPLEMENT_DYNAMIC(CMiscDialog, CDialog)
CMiscDialog::CMiscDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMiscDialog::IDD, pParent)
	, m_DeletedFiles(TRUE)
	, m_ActiveFiles(FALSE)
{
}

CMiscDialog::~CMiscDialog()
{
}

void CMiscDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DELETED, m_DeletedFiles);
	DDX_Check(pDX, IDC_ACTIVE, m_ActiveFiles);
}


BEGIN_MESSAGE_MAP(CMiscDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_FILENAME, OnCbnSelchangeFilename)
END_MESSAGE_MAP()


// CMiscDialog message handlers

void CMiscDialog::OnBnClickedOk()
{
	CComboBox* file = (CComboBox*)GetDlgItem(IDC_FILENAME);
	CComboBox* disk = (CComboBox*)GetDlgItem(IDC_DISK);

	file->GetWindowText(m_strFileName);
	disk->GetWindowText(m_strDisk);
    
	UpdateData(TRUE);

	int j = AfxGetApp()->GetProfileInt("File Search MRU", "MaxMRU", 10);
	char buffer[3];
	buffer[2] = '\0';
	CString strMRU;
	for(int i=j;i>0;i--)
	{
		_itoa(i, buffer, 10);
		strMRU.Empty();
		strMRU = AfxGetApp()->GetProfileString("File Search MRU", buffer, NULL);

		if( !strMRU.IsEmpty() )
		{
			_itoa(i+1, buffer, 10);
			AfxGetApp()->WriteProfileString("File Search MRU", buffer, strMRU);
		}
	}
	AfxGetApp()->WriteProfileString("File Search MRU", "1", m_strFileName);

	OnOK();
}

BOOL CMiscDialog::OnInitDialog()
{
	CDialog::OnInitDialog();


	CComboBox* file = (CComboBox*)GetDlgItem(IDC_FILENAME);
	// Open File Ext File and populate IDC_FILENAME

	char buffer[3];
	buffer[2] = '\0';
	CString strMRU;
	int j = AfxGetApp()->GetProfileInt("File Search MRU", "MaxMRU", 10);
	for(int i=0;i<j;i++)
	{
		_itoa(i, buffer, 10);
		strMRU.Empty();
		strMRU = AfxGetApp()->GetProfileString("File Search MRU", buffer, NULL);

		if( !strMRU.IsEmpty() )
			file->AddString(strMRU);
	}
	






	
	file->SetCurSel(0);
    file->SetActiveWindow();

	CComboBox* disk = (CComboBox*)GetDlgItem(IDC_DISK);
	disk->SetCurSel(0);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMiscDialog::OnCbnSelchangeFilename()
{
	// TODO: Add your control notification handler code here
}
