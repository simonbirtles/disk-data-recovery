#pragma once


// CMiscDialog dialog

class CMiscDialog : public CDialog
{
	DECLARE_DYNAMIC(CMiscDialog)

public:
	CMiscDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMiscDialog();
	CString m_strFileName;
	CString m_strDisk;

// Dialog Data
	enum { IDD = IDD_MISCDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	BOOL m_DeletedFiles;
	BOOL m_ActiveFiles;
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeFilename();
};
