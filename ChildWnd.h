#pragma once


// CChildWnd frame

class CChildWnd : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildWnd)

protected:
	CSplitterWnd	m_wndSplitter;

	
protected:
	CChildWnd();           // protected constructor used by dynamic creation
	virtual ~CChildWnd();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
//	afx_msg void OnPaint();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:

	afx_msg void OnFontChange();
};


