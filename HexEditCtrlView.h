/////////////////////////////////////////////////////////////////////////////
// HexEditCtrlView.h : interface of the CHexEditCtrlView class
/////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_HEXEDITCTRLVIEW_H__5DDBE23B_CA69_4C02_8643_5D03A1CB690C__INCLUDED_)
#define AFX_HEXEDITCTRLVIEW_H__5DDBE23B_CA69_4C02_8643_5D03A1CB690C__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// includes
/////////////////////////////////////////////////////////////////////////////
#include "HexEditBase.h"
#include "diskdataDoc.h"



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CHexEditCtrlView
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CHexEditCtrlView : public CHexEditBaseView
{
public:
	CDiskDataDoc* GetDocument();

protected: 
	CHexEditCtrlView();
	virtual ~CHexEditCtrlView();	

	//{{AFX_VIRTUAL(CHexEditCtrlView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CHexEditCtrlView)
	afx_msg void OnHexEditChanged();
	//}}AFX_MSG

	DECLARE_DYNCREATE(CHexEditCtrlView)
	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#ifndef _DEBUG  // debug version in HexEditCtrlView.cpp
inline CHexEditCtrlDoc* CHexEditCtrlView::GetDocument()
   { return (CHexEditCtrlDoc*)m_pDocument; }
#endif


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_HEXEDITCTRLVIEW_H__5DDBE23B_CA69_4C02_8643_5D03A1CB690C__INCLUDED_)
