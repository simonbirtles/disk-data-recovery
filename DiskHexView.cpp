// DiskHexView.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "DiskHexView.h"
#include "DiskHexCtrl.h"
#include "GoToSector.h"

#include "DiskDataDoc.h"
#include "ChildWnd.h"

// CDiskHexView

IMPLEMENT_DYNCREATE(CDiskHexView, CView)

CDiskHexView::CDiskHexView()
{
	
}

CDiskHexView::~CDiskHexView()
{
}

BEGIN_MESSAGE_MAP(CDiskHexView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_NEXTSECTOR, OnViewNextsector)
	ON_COMMAND(ID_VIEW_PREVIOUSSECTOR, OnViewPrevioussector)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUSSECTOR, OnUpdateViewPrevioussector)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTSECTOR, OnUpdateViewNextsector)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_SECTORDETAILS, OnViewSectordetails)
	ON_COMMAND(ID_VIEW_GOTOSECTOR, OnViewGotosector)
	ON_WM_FONTCHANGE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_GOTOOFFSET, OnViewGotooffset)
	ON_MESSAGE(WM_HEX_DATA_NEXT, OnDataNext)
	ON_MESSAGE(WM_HEX_DATA_PREV, OnDataPrev)
END_MESSAGE_MAP()


LRESULT CDiskHexView::OnDataNext(WPARAM wParam, LPARAM lParam)
{
	OnViewNextsector();
	return 0L;
}

LRESULT CDiskHexView::OnDataPrev(WPARAM wParam, LPARAM lParam)
{
	OnViewPrevioussector();

	return 0L;
}

// CDiskHexView drawing

void CDiskHexView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CDiskHexView diagnostics

#ifdef _DEBUG
void CDiskHexView::AssertValid() const
{
	CView::AssertValid();
}

void CDiskHexView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CDiskHexView message handlers

int CDiskHexView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;



	// Create The HexView Control That This View Displays
	if(!m_cHexCtrl.Create(NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE, 
							CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), 
							this, IDC_HEXEDITBASEVIEW_HEXCONTROL, NULL)) 
	{
		AfxMessageBox("Failed To Create Hex Control", 0, 0);
		return -1;
	}	
	
	
	

	return 0;
}


void CDiskHexView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	if(pDoc->GetData() != NULL)
	{
		m_HexData = (DATA_SET_DATA*)VirtualAlloc(NULL, sizeof(DATA_SET_DATA), MEM_COMMIT, PAGE_READWRITE);
		m_HexData->pData = pDoc->GetData();
		m_HexData->dwLen = pDoc->GetDataSize();
		m_HexData->iOffset = pDoc->GetCurrentOffset();
		::SendMessage(m_cHexCtrl.GetSafeHwnd(), WM_HEX_DATA_UPDATE, (WPARAM)m_HexData, 0);
		//GetHexEditCtrl().SetDataPointer(pDoc->GetData(), pDoc->GetDataSize(), pDoc->GetCurrentOffset());
	}

	// frame
	CChildWnd* cWnd = (CChildWnd*)this->GetParentFrame()   ;
	CString str;
	str.AppendFormat("%s", pDoc->GetPathName());
	cWnd->SetWindowText(str);
	pDoc->SetTitle(str);


	// TODO: Add your specialized code here and/or call the base class
}

void CDiskHexView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_cHexCtrl.MoveWindow(0,0, cx, cy, TRUE);	// Last Param Is The repaint
												// Which Will Call OnPaint In HexCtrl
	

	// TODO: Add your message handler code here
}

void CDiskHexView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	if(pDoc->GetData() != NULL)
	{
		m_HexData = (DATA_SET_DATA*)VirtualAlloc(NULL, sizeof(DATA_SET_DATA), MEM_COMMIT, PAGE_READWRITE);
		m_HexData->pData = pDoc->GetData();
		m_HexData->dwLen = pDoc->GetDataSize();
		m_HexData->iOffset = pDoc->GetCurrentOffset();
		::SendMessage(m_cHexCtrl.GetSafeHwnd(), WM_HEX_DATA_UPDATE, (WPARAM)m_HexData, 0);

	//	GetHexEditCtrl().SetDataPointer(pDoc->GetData(), pDoc->GetDataSize(), pDoc->GetCurrentOffset());
	}

}

void CDiskHexView::OnViewNextsector()
{
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	pDoc->OnViewNextsector();
}

void CDiskHexView::OnViewPrevioussector()
{
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	pDoc->OnViewPrevioussector();
}

void CDiskHexView::OnUpdateViewPrevioussector(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CDiskHexView::OnUpdateViewNextsector(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(true);
}

void CDiskHexView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDiskHexView::OnViewSectordetails()
{
//	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
//	pDoc->GetSectorDetails();
}


void CDiskHexView::OnViewGotosector()
{
	CDiskDataDoc* pDoc = (CDiskDataDoc*)GetDocument();
	pDoc->OnViewGotosector();
}


void CDiskHexView::OnFontChange()
{
	CView::OnFontChange();
	
	::SendMessage(m_cHexCtrl.GetSafeHwnd(), WM_FONTCHANGE, 0,0);

	// TODO: Add your message handler code here
}

void CDiskHexView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	m_cHexCtrl.SetFocus();

	// TODO: Add your message handler code here
}

void CDiskHexView::OnViewGotooffset()
{
	CGoToSector dlg;
	dlg.m_strTitle = "Go To Offset";
	if(dlg.DoModal() == IDOK)
		::SendMessage(m_cHexCtrl, WM_HEX_MOVE_OFFSET, (WPARAM)dlg.iSectorNo , 0);
		

}
