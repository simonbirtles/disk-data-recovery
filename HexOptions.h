#pragma once
#include "afxwin.h"


// CHexOptions dialog

class CHexOptions : public CPropertyPage
{
	DECLARE_DYNAMIC(CHexOptions)

public:
	CHexOptions();
	virtual ~CHexOptions();

// Dialog Data
	enum { IDD = IDD_HEXOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CButton m_bShowHeaders;
	CButton m_bShowOffset;
	CButton m_bShowAscii;
	CButton m_bHexOffsets;
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
private:
	void SaveHexOptions(void);
};
