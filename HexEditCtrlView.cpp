/////////////////////////////////////////////////////////////////////////////
// HexEditCtrlView.cpp : implementation of the CHexEditCtrlView class
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// includes
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Diskdata.h"
#include "DiskDataDoc.h"
#include "HexEditCtrlView.h"


/////////////////////////////////////////////////////////////////////////////
// defines
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CHexEditCtrlView
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CHexEditCtrlView, CHexEditBaseView)

BEGIN_MESSAGE_MAP(CHexEditCtrlView, CHexEditBaseView)
	//{{AFX_MSG_MAP(CHexEditCtrlView)
	ON_EN_CHANGE(CHexEditBaseView::IDC_HEXEDITBASEVIEW_HEXCONTROL, OnHexEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CHexEditCtrlView::CHexEditCtrlView()
{
	GetHexEditCtrl().SetAddressSize(4, false);
	GetHexEditCtrl().SetShowAddress(true, false);
	GetHexEditCtrl().SetShowAscii(true, false);
	GetHexEditCtrl().SetBytesPerRow(16, true, true);
}

CHexEditCtrlView::~CHexEditCtrlView()
{
}

BOOL CHexEditCtrlView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CHexEditBaseView::PreCreateWindow(cs);
}

void CHexEditCtrlView::OnInitialUpdate() 
{
	CHexEditBaseView::OnInitialUpdate();
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	GetHexEditCtrl().SetDirectDataPtr(pDoc->GetData(), pDoc->GetDataSize());	
}

void CHexEditCtrlView::OnHexEditChanged()
{
	GetDocument()->SetModifiedFlag();
}

#ifdef _DEBUG
void CHexEditCtrlView::AssertValid() const
{
	CHexEditBaseView::AssertValid();
}

void CHexEditCtrlView::Dump(CDumpContext& dc) const
{
	CHexEditBaseView::Dump(dc);
}

CDiskDataDoc* CHexEditCtrlView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDiskDataDoc)));
	return (CDiskDataDoc*)m_pDocument;
}
#endif //_DEBUG

