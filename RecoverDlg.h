#pragma once


// CRecoverDlg dialog

class CRecoverDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecoverDlg)

public:
	CRecoverDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRecoverDlg();

	CString m_strFilename;				// New File Name
	CString m_strRecoverFile;			// Original File Name

// Dialog Data
	enum { IDD = IDD_RECOVERFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectfile();
	virtual BOOL OnInitDialog();
};
