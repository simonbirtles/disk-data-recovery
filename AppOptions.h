#pragma once


// Property Pages
#include "OptionsDlg.h"
#include "HexOptions.h"
#include "FileOptions.h"


// CAppOptions

class CAppOptions : public CPropertySheet
{
	DECLARE_DYNAMIC(CAppOptions)

public:
	CAppOptions(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CAppOptions(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CAppOptions();

	COptionsDlg		m_ppGlbOptions;
	CHexOptions		m_ppHexOptions;
	CFileOptions	m_ppFileOptions;


protected:
	DECLARE_MESSAGE_MAP()
};


