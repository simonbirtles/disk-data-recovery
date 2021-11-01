// AppOptions.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "AppOptions.h"




// CAppOptions

IMPLEMENT_DYNAMIC(CAppOptions, CPropertySheet)
CAppOptions::CAppOptions(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CAppOptions::CAppOptions(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_ppGlbOptions);
	AddPage(&m_ppFileOptions);
	AddPage(&m_ppHexOptions);
}

CAppOptions::~CAppOptions()
{
}


BEGIN_MESSAGE_MAP(CAppOptions, CPropertySheet)
END_MESSAGE_MAP()


// CAppOptions message handlers
