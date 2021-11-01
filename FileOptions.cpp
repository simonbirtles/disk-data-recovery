// FileOptions.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "FileOptions.h"


// CFileOptions dialog

IMPLEMENT_DYNAMIC(CFileOptions, CPropertyPage)
CFileOptions::CFileOptions()
	: CPropertyPage(CFileOptions::IDD)
{
}

CFileOptions::~CFileOptions()
{
}

void CFileOptions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileOptions, CPropertyPage)
END_MESSAGE_MAP()


// CFileOptions message handlers
