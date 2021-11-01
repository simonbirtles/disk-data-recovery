#pragma once
#include "afxwin.h"


// CGoToSector dialog

class CGoToSector : public CDialog
{
	DECLARE_DYNAMIC(CGoToSector)

public:
	CGoToSector(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGoToSector();

// Dialog Data
	enum { IDD = IDD_SECTOR };

	__int64		iSectorNo;
	CString		m_strTitle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
private:
	CEdit m_ip;
};
