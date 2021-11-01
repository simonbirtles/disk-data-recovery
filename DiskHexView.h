#pragma once


#include "DiskHexCtrl.h"

// CDiskHexView view

class CDiskHexView : public CView
{
	DECLARE_DYNCREATE(CDiskHexView)
public:
	CDiskHexCtrl& GetHexEditCtrl() { return m_cHexCtrl; }

protected:
	CDiskHexView();           // protected constructor used by dynamic creation
	virtual ~CDiskHexView();
	CDiskHexCtrl	m_cHexCtrl;		// Hex Control Class

	DATA_SET_DATA*	m_HexData;
	

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	enum { IDC_HEXEDITBASEVIEW_HEXCONTROL = 0x100 };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnViewNextsector();
	afx_msg void OnViewPrevioussector();
	afx_msg void OnUpdateViewPrevioussector(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewNextsector(CCmdUI *pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewSectordetails();
	afx_msg void OnViewGotosector();
	afx_msg void OnFontChange();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnViewGotooffset();
	afx_msg LRESULT OnDataNext(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDataPrev(WPARAM wParam, LPARAM lParam);
};


