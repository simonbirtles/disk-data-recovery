// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "OptionsDlg.h"

#define NUMLINES ((int) (sizeof(texttypes) / sizeof(texttypes[0])))

struct 
	{
		int iIndex;
		TCHAR* szLabel;
		TCHAR* szRegString;
		BOOL	bFont;			// to tell the code if these selctions should be enabled for this type of text
		BOOL	bColor;			// ie highlight text will only be colors not font types/sizes etc
	}
	texttypes [] =
	{
		0,	_T("{Hex Editor}Font"),			_T("Settings\\Hex Editor\\Font Details"), true, false,
		1,	_T("{Hex Editor}Header"),		_T("Settings\\Hex Editor\\Header Details"), false, true,
		2,	_T("{Hex Editor}Hex"),			_T("Settings\\Hex Editor\\Hex Details"), false, true,
		3,	_T("{Hex Editor}Ascii"),		_T("Settings\\Hex Editor\\Ascii Details"), false, true,
		4,	_T("{Hex Editor}Sector"),		_T("Settings\\Hex Editor\\Sector Details"), false, true,
		5,	_T("{Hex Editor}Highlight"),	_T("Settings\\Hex Editor\\Highlight Text"), false, true,
	};

// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertyPage)
COptionsDlg::COptionsDlg()
	: CPropertyPage(COptionsDlg::IDD)
{
	// Default Settings - real ones will come from reg
	m_iFontSize = 10;
	m_strFontName = "Courier";
	m_bBold = false;
	m_crForeGround = RGB(0,0,0);
	m_crBackGround = RGB(255,0,0);
	//        l   t    r    b
	 m_rSample = CRect(17, 220, 235, 245);
	 m_bFontDirty = false;
	 m_bFontChanged = false;
	 m_iCboTextSel =0;
	 m_bCaretBlock = true;
	

}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FONTITEM, m_TextAreaList);
	DDX_Control(pDX, IDC_CBOFONTTYPE, m_cboFont);
	DDX_Control(pDX, IDC_FONTSIZE, m_cboFontSize);
	DDX_Control(pDX, IDC_BFOREGND, m_bForeground);
	DDX_Control(pDX, IDC_BBACKGND, m_bBackground);
	DDX_Control(pDX, IDC_BOLD, m_cBold);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_CBOFONTTYPE, OnCbnSelchangeCbofonttype)
	ON_CBN_SELCHANGE(IDC_FONTSIZE, OnCbnSelchangeFontsize)
	ON_BN_CLICKED(IDC_BOLD, OnBnClickedBold)
	ON_BN_CLICKED(IDC_BFOREGND, OnBnClickedBforegnd)
	ON_BN_CLICKED(IDC_BBACKGND, OnBnClickedBbackgnd)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_FONTITEM, OnCbnSelchangeFontitem)
	ON_BN_CLICKED(IDC_CARETLINE, OnBnClickedCaretline)
	ON_BN_CLICKED(IDC_CARETBLOCK, OnBnClickedCaretblock)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ProcessText();						// Add the text types to the dropdown   - only do once

	OnCbnSelchangeFontitem();			// get the selection recognized and get registry data into vars
    	
	


	//m_cboFont.SetCurSel(1);
//	m_cboFontSize.SetCurSel(14);

	//m_sSample.ModifyStyle(0, SS_BLACKFRAME, 0);


	//SetFontSample();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsDlg::ProcessFonts(void)
{
	LOGFONT lf;
	POSITION pos;

	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0]='\0';
	lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
	
	int iPos = 0;

	m_cboFont.ResetContent();
	ASSERT(m_cboFont.GetCount() == 0);
	fontlist.RemoveAll();
	
	HDC pDC = ::GetDC(this->GetSafeHwnd());
	CString strFont;
	

	::EnumFontFamiliesEx(pDC, 
						&lf, 
						(FONTENUMPROC) COptionsDlg::EnumFontFamExProc, 
						(LPARAM) &fontlist, 0);

	for(pos = fontlist.GetHeadPosition(); pos != NULL;)
	{   
		strFont = fontlist.GetNext(pos);
		iPos = m_cboFont.AddString(strFont);

		strFont.Trim();
		m_strFontName.Trim() ;

		if(m_strFontName.CompareNoCase(strFont) == 0 )
			m_cboFont.SetCurSel(iPos);
	}




}

int CALLBACK COptionsDlg::EnumFontFamExProc(
    ENUMLOGFONTEX *lpelfe,	// pointer to logical-font data
    NEWTEXTMETRICEX *lpntme,	// pointer to physical-font data
    int FontType,	// type of font
    LPARAM lParam)	// application-defined data)
{
	CStringList* m_temp = (CStringList*) lParam;
	m_temp->AddTail((char*)lpelfe->elfFullName);
	return 1; 
}

void COptionsDlg::OnCbnSelchangeCbofonttype()
{
	m_cboFont.GetWindowText(m_strFontName);
	m_bFontDirty = true;
	SetModified(true);
	SetFontSample();	
}

void COptionsDlg::SetFontSample(void)
{

	
	int iWeight = 400;

	if(m_bBold)
		iWeight = 700;
	else
		iWeight = 400;

	
	m_font.DeleteObject();
	m_font.CreateFont(m_iFontSize,0,0,0,iWeight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, m_strFontName);
						  
	InvalidateRect(m_rSample);  
}

void COptionsDlg::ProcessFontSize(void)
{

	CString strSize;
	m_cboFontSize.ResetContent();
	ASSERT(m_cboFontSize.GetCount() == 0);
	int iSel = 0;
	for(int i=6;i<25;i++)
	{
		strSize.Empty()	;
		strSize.AppendFormat("%i", i);
		iSel = m_cboFontSize.AddString(strSize)	;
		if(i == m_iFontSize)
			m_cboFontSize.SetCurSel(iSel);

	}
}

void COptionsDlg::OnCbnSelchangeFontsize()
{
	CString str;
	m_cboFontSize.GetWindowText(str);
	m_iFontSize = atoi(str);
	SetModified(true);
	m_bFontDirty = true;
	//m_BtnApply.EnableWindow(true);
	SetFontSample();
}



void COptionsDlg::OnBnClickedBold()
{
	if(m_cBold.GetCheck() == BST_CHECKED)
		m_bBold = true;
	else
		m_bBold = false;

	m_bFontDirty = true;
	SetModified(true);
	//m_BtnApply.EnableWindow(true);
	SetFontSample();
}

void COptionsDlg::ProcessText(void)
{
	// Call Functions to fill controls.
	for(int i=0;i<NUMLINES;i++)
	{
		m_TextAreaList.AddString(texttypes[i].szLabel);	

	}
	//m_TextAreaList.AddString("{Hex Editor} - Sector Details");
	//m_TextAreaList.AddString("{Hex Editor} - Hex Details");
	//m_TextAreaList.AddString("{Hex Editor} - Ascii Details");

	m_TextAreaList.SetCurSel(0);
	m_iCboTextSel =	 m_TextAreaList.GetCurSel();

	
}

void COptionsDlg::ProcessSystem(void)
{
	CButton* cBlock = (CButton*)GetDlgItem(IDC_CARETBLOCK);
	CButton* cLine = (CButton*)GetDlgItem(IDC_CARETLINE);

	if(m_bCaretBlock)
		cBlock->SetCheck(BST_CHECKED);
	else
		cLine->SetCheck(BST_CHECKED);

}

void COptionsDlg::OnBnClickedBforegnd()
{
	CColorDialog cColor;
	if(cColor.DoModal() == IDOK)
	{
		m_crForeGround = cColor.GetColor();
		m_bFontDirty = true;
		SetModified(true);
	}

	
	SetFontSample();
}
  

void COptionsDlg::OnBnClickedBbackgnd()
{
	CColorDialog cColor;
	if(cColor.DoModal() == IDOK)
	{
		m_crBackGround = cColor.GetColor();
		m_bFontDirty = true;
		SetModified(true);
	}

	
	SetFontSample();
}



HBRUSH COptionsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    	
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void COptionsDlg::OnPaint()
{
	CPaintDC pDC(this); // device context for painting
							  
	CBrush pBrush;				 	  
	pBrush.CreateSolidBrush(m_crBackGround); 
	CBrush* oldBrush  = pDC.SelectObject(&pBrush); 

	// Paint the Sample Box
	pDC.SetTextColor(m_crForeGround);
	pDC.SetBkColor(m_crBackGround);
	pDC.Rectangle(m_rSample);
	pDC.SelectObject(&m_font);
	pDC.SelectObject(m_font);
	CString str = "EB AC 1D FF AA BC";
	CRect r = m_rSample;
	pDC.DrawText(str,m_rSample , DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	



	pDC.SelectObject(oldBrush);
    	
}












void COptionsDlg::OnCbnSelchangeFontitem()
{
	if(m_bFontDirty)
	{
		m_bFontChanged = true;
		DWORD dwRet = AfxMessageBox(IDS_FONTDIRTY, MB_ICONINFORMATION|MB_YESNOCANCEL , 0);
		int iSel = 0;
		switch (dwRet)
		{
		case IDYES:		// save font details for this texttype
			//SaveFontSettings();
			OnApply();
           	break;
		case IDNO:			// dont do anything
			break;
		case IDCANCEL:		// dont do anything
			return;			// stay as we are
			break;
            
		}
	}

	m_iCboTextSel = m_TextAreaList.GetCurSel();	// Save this current selection
	GetFontSettings();

		

	m_cboFont.EnableWindow(texttypes[m_iCboTextSel].bFont);
	m_cboFontSize.EnableWindow(texttypes[m_iCboTextSel].bFont);
	m_cBold.EnableWindow(texttypes[m_iCboTextSel].bFont);
	m_bForeground.EnableWindow(texttypes[m_iCboTextSel].bColor);
	m_bBackground.EnableWindow(texttypes[m_iCboTextSel].bColor);


	ProcessFonts();						// Fill the fonts dropdown and select the correct font for this text selection

	ProcessFontSize();	

	ProcessBold();

	ProcessSystem();

	SetFontSample();
}

void COptionsDlg::SaveFontSettings(void)
{

	if(m_bFontDirty)
	{
		if(texttypes[m_iCboTextSel].bFont)
		{
			// Save Font Name
			AfxGetApp()->WriteProfileString(texttypes[m_iCboTextSel].szRegString, 
											"Font Name",
											m_strFontName);

			// Save Font Size
			AfxGetApp()->WriteProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font Size",
											m_iFontSize);

			// Save Font Bold
			AfxGetApp()->WriteProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font Bold",
											m_bBold);

			
		} // end if bFont

		if(texttypes[m_iCboTextSel].bColor)
		{

		 // Save Font ForeGround Color
			AfxGetApp()->WriteProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font ForeGround",
											m_crForeGround);

			// Save Font BackGround Color
			AfxGetApp()->WriteProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font BackGround",
											m_crBackGround);

		}




			// save caret type
			 AfxGetApp()->WriteProfileInt(_T("Settings\\Hex Editor\\Header Details"), 
											"Caret Type",
											m_bCaretBlock);
	
			m_bFontDirty = false;
			SetModified(false);
			m_bFontChanged = true;
	}
}

void COptionsDlg::GetFontSettings(void)
{
	// Now get the cur selections details and process
	// Save Font Name
			m_strFontName = AfxGetApp()->GetProfileString(texttypes[m_iCboTextSel].szRegString, 
											"Font Name",
											"Courier");
			//ProcessFonts();

			// Get Font Size
			m_iFontSize = AfxGetApp()->GetProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font Size",
											10);

			// Get Font Bold
			m_bBold = AfxGetApp()->GetProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font Bold",
											0);

			// Get Font ForeGround Color
			m_crForeGround = AfxGetApp()->GetProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font ForeGround",
											RGB(255,255,255));

			// Get Font BackGround Color
			m_crBackGround = AfxGetApp()->GetProfileInt(texttypes[m_iCboTextSel].szRegString, 
											"Font BackGround",
											RGB(0,0,0));


			// Get caret type
			m_bCaretBlock = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\Header Details"), 
											"Caret Type",
											true);
}

void COptionsDlg::ProcessBold(void)
{
	m_cBold.SetCheck(m_bBold);
}

void COptionsDlg::OnBnClickedCaretline()
{
	m_bCaretBlock = false;
	m_bFontDirty = true;
	SetModified(true);
	m_bFontChanged = true;
}

void COptionsDlg::OnBnClickedCaretblock()
{
	m_bCaretBlock = true;
	m_bFontDirty = true;
	SetModified(true);
	m_bFontChanged = true;
}


void COptionsDlg::OnBnClickedCancel()
{
	if(m_bFontChanged)
	{
		SaveFontSettings();

		CWnd *pWnd = AfxGetApp()->m_pMainWnd;
		//	 HWND_BROADCAST
	    ::SendMessage(pWnd->GetSafeHwnd() , WM_FONTCHANGE, 0 ,0); 
	}

	OnCancel();
}

BOOL COptionsDlg::OnApply()
{
	// TODO: Add your control notification handler code here
	//if(m_bFontDirty)
	//{
		SaveFontSettings();

		CWnd *pWnd = AfxGetApp()->m_pMainWnd;
		//	 HWND_BROADCAST
	    ::SendMessage(pWnd->GetSafeHwnd() , WM_FONTCHANGE, 0 ,0); 
	//}

	SetModified(false);

	return CPropertyPage::OnApply();
}

void COptionsDlg::OnOK()
{
	SaveFontSettings();

	if(m_bFontChanged)
	{
		CWnd *pWnd = AfxGetApp()->m_pMainWnd;
		//	 HWND_BROADCAST
	    ::SendMessage(pWnd->GetSafeHwnd() , WM_FONTCHANGE, 0 ,0); 
	}


	CPropertyPage::OnOK();
}
