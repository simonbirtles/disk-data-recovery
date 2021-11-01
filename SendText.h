#pragma once
#include "afxwin.h"


// CSendText dialog

class CSendText : public CDialog
{
	DECLARE_DYNAMIC(CSendText)

public:
	CSendText(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendText();

// Dialog Data
	enum { IDD = IDD_TEXTMESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit;
	afx_msg void OnBnClickedOk();
	CString m_strEdit;
};
