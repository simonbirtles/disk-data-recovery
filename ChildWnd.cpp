// ChildWnd.cpp : implementation file
//

// Includes
#include "stdafx.h"
#include "DiskData.h"
#include "ChildWnd.h"

#include "DiskHexView.h"
#include "DiskTree.h"


// CChildWnd

IMPLEMENT_DYNCREATE(CChildWnd, CMDIChildWnd)

CChildWnd::CChildWnd()
{
}

CChildWnd::~CChildWnd()
{
}


BEGIN_MESSAGE_MAP(CChildWnd, CMDIChildWnd)
//	ON_WM_PAINT()
ON_WM_FONTCHANGE()
END_MESSAGE_MAP()


// CChildWnd message handlers



BOOL CChildWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{     	
  /*
	if(!m_wndSplitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE | WS_SIZEBOX, AFX_IDW_PANE_FIRST ))
	    {TRACE0("\nFailed To CreateStatic");}
                           
	if(!m_wndSplitter.CreateView(0,1, RUNTIME_CLASS(CDiskHexView), CSize(750,0), pContext))
	{TRACE0("\nFailed To CreateView1");}

	if(!m_wndSplitter.CreateView(0,0, RUNTIME_CLASS(CDiskTree), CSize(0,0), pContext))
	{TRACE0("\nFailed To CreateView2");}

	SetActiveView((CView*) m_wndSplitter.GetPane(0,1) );
	*/

	this->EnableDocking(CBRS_ALIGN_RIGHT);

	//return TRUE;

    return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}

BOOL CChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style = WS_MAXIMIZE | WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	//cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW;

	return CMDIChildWnd::PreCreateWindow(cs);
}





void CChildWnd::OnFontChange()
{
	CMDIChildWnd::OnFontChange();

	CView* pView = this->GetActiveView();
	::SendMessage(pView->GetSafeHwnd(), WM_FONTCHANGE,0,0);

	// TODO: Add your message handler code here
}
