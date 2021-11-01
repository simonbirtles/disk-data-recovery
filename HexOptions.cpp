// HexOptions.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "HexOptions.h"



// dont really need this but it keeps things tidy and readable !!
struct 
	{
		int iIndex;
		TCHAR* szLabel;
		TCHAR* szRegString;
	}
	generaltypes [] =
	{
		0,	_T("Show Headers"),	_T("Settings\\Hex Editor\\General"),
		1,	_T("Show Offset"),	_T("Settings\\Hex Editor\\General"),
		2,	_T("Show Ascii"),	_T("Settings\\Hex Editor\\General"), 
		3,	_T("Hex Offsets"),	_T("Settings\\Hex Editor\\General"), 
	};



// CHexOptions dialog

IMPLEMENT_DYNAMIC(CHexOptions, CPropertyPage)
CHexOptions::CHexOptions()
	: CPropertyPage(CHexOptions::IDD)
{
}

CHexOptions::~CHexOptions()
{
}

void CHexOptions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHOWHEADERS, m_bShowHeaders);
	DDX_Control(pDX, IDC_SHOWOFFSET, m_bShowOffset);
	DDX_Control(pDX, IDC_SHOWASCII, m_bShowAscii);
	DDX_Control(pDX, IDC_HEXOFFETS, m_bHexOffsets);
}


BEGIN_MESSAGE_MAP(CHexOptions, CPropertyPage)
END_MESSAGE_MAP()


// CHexOptions message handlers

BOOL CHexOptions::OnCommand(WPARAM wParam, LPARAM lParam)
{
	SetModified(true);

	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CHexOptions::OnApply()
{
	SaveHexOptions();

	return CPropertyPage::OnApply();
}

BOOL CHexOptions::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Get Reg Entries
	m_bShowHeaders.SetCheck(
		AfxGetApp()->GetProfileInt(generaltypes[0].szRegString, 
									generaltypes[0].szLabel,
									true));

	m_bShowOffset.SetCheck(
		AfxGetApp()->GetProfileInt(generaltypes[1].szRegString, 
									generaltypes[1].szLabel,
									true));


	m_bShowAscii.SetCheck(
		AfxGetApp()->GetProfileInt(generaltypes[2].szRegString, 
									generaltypes[2].szLabel,
									true));


	m_bHexOffsets.SetCheck(
		AfxGetApp()->GetProfileInt(generaltypes[3].szRegString, 
									generaltypes[3].szLabel,
									true));






	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHexOptions::OnOK()
{
	SaveHexOptions();

	CPropertyPage::OnOK();
}

void CHexOptions::SaveHexOptions(void)
{
	AfxGetApp()->WriteProfileInt(generaltypes[0].szRegString, 
									generaltypes[0].szLabel,
									m_bShowHeaders.GetCheck());

	AfxGetApp()->WriteProfileInt(generaltypes[1].szRegString, 
									generaltypes[1].szLabel,
									m_bShowOffset.GetCheck());


	AfxGetApp()->WriteProfileInt(generaltypes[2].szRegString, 
									generaltypes[2].szLabel,
									m_bShowAscii.GetCheck());


	AfxGetApp()->WriteProfileInt(generaltypes[3].szRegString, 
									generaltypes[3].szLabel,
									m_bHexOffsets.GetCheck());

}
