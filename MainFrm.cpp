// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DiskData.h"
#include "DiskDataDoc.h"

#include "MainFrm.h"
#include "Controlwnd.h"
#include "AppOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_RESOURCEEXPLORER, OnViewResourceexplorer)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_WM_FONTCHANGE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
		
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle());

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_GRIPPER | CBRS_FLOAT_MULTI);


	if (!m_wndPosBar.CreateEx(this) ||
		!m_wndPosBar.LoadToolBar(IDR_POSITIONBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
		
	m_wndPosBar.SetBarStyle(m_wndPosBar.GetBarStyle() & ~CBRS_HIDE_INPLACE);
	m_wndPosBar.SetBarStyle(m_wndPosBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_GRIPPER | CBRS_FLOAT_MULTI);


	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Do after all commmon controls but before custom controls
	// Enable Docking For This MainFrame
		EnableDocking(CBRS_ALIGN_ANY);




	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndPosBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndPosBar);


	if(!m_wndControl.CreateEx(WS_CHILD, this, 0x1105))
	{
		TRACE0("Failed to create m_wndControl bar\n");
		return -1;      // fail to create
	}

	

    m_wndControl.EnableDocking(CBRS_ALIGN_LEFT);
	DockControlBar(&m_wndControl, AFX_IDW_DOCKBAR_LEFT);

	
    
	this->MDIMaximize(this);
	

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


void CMainFrame::OnViewResourceexplorer()
{	
	BOOL bVisible = ((m_wndControl.IsWindowVisible()) != 0);
   	ShowControlBar(&m_wndControl, !bVisible, FALSE);
	RecalcLayout();
}


void CMainFrame::OnToolsOptions()
{
	CAppOptions dlg("Application Options");
	dlg.DoModal();
	
}

void CMainFrame::OnFontChange()
{
	CMDIFrameWnd::OnFontChange();

	// As Child Windows dont get this we have to enumerate all our child windows and let them
	// get a WM_FONTCHANGE  message;

	//CWnd* pWnd	;

	//pWnd = GetWindow(GW_CHILD);
	//while(pWnd != NULL)
	//{
	//	::PostMessage(pWnd->GetSafeHwnd(), WM_FONTCHANGE, 0 ,0);
	//	//::SendMessage(pWnd->GetSafeHwnd(), WM_FONTCHANGE, 0 ,0); 
 //       pWnd = GetWindow(GW_HWNDNEXT);
	//}

	 HWND hwndT;
	hwndT=::GetWindow(m_hWndMDIClient, GW_CHILD);
	while (hwndT != NULL)
	{
		::PostMessage(hwndT, WM_FONTCHANGE, 0 ,0);
		hwndT=::GetWindow(hwndT,GW_HWNDNEXT);
	}








	// TODO: Add your message handler code here
}
