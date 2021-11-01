// DiskData.h : main header file for the DiskData application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CDiskDataApp:
// See DiskData.cpp for the implementation of this class
//

class CDiskDataApp : public CWinApp
{
public:
	CDiskDataApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew( );
	DECLARE_MESSAGE_MAP()
};

extern CDiskDataApp theApp;