#pragma once
#include "afxcmn.h"


// CDlgIPAddress dialog

class CDlgIPAddress : public CDialog
{
	DECLARE_DYNAMIC(CDlgIPAddress)

public:
	CDlgIPAddress(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgIPAddress();

// Dialog Data
	enum { IDD = IDD_SELECTIP };

	LPTSTR m_ipaddr;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
private:
	CIPAddressCtrl m_ipaddr32;
};
