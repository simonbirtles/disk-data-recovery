// ControlWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "ControlWnd.h"

#include "afxpriv.h"
#include "diskanalysis.h"
#include "MiscDialog.h"
#include "RecoverDlg.h"
#include "DiskDataDoc.h"
#include "IPDisk.h"
#include "DlgIPAddress.h"
#include "SendText.h"

#define DD_ERROR		0x11FF
#define DD_MBR			0x1101
#define DD_BOOT			0x1102
#define DD_FINDFILE		0x1103
#define DD_RECOVERFILE	0x1104
#define DD_MFTRECORD	0x1105


#define WMS_SOCKET_NOTIFY		WM_USER+1
#define WMS_ADD_REMOTE			WM_USER+2
// CControlWnd
IMPLEMENT_DYNAMIC(CControlWnd, CControlBar)
CControlWnd::CControlWnd()
{

	RegisterClass();			// Register this class
	m_ImageList = NULL;

    // initialise the member var that holds the last size of the window
	m_SizeLast.cx = CONTROL_MIN_WIDTH;
	m_SizeLast.cy = CONTROL_MIN_HEIGHT;

}

CControlWnd::~CControlWnd()
{
	if(m_ImageList != NULL)
		delete m_ImageList;
}


BEGIN_MESSAGE_MAP(CControlWnd, CControlBar)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)				// Added By Me Manually 
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SEARCHFORFILE_TEST, OnSearchforfileTest)
	ON_COMMAND(ID_RECOVERTREE_RECOVERTHISFILE, OnRecovertreeRecoverthisfile)
	ON_COMMAND(ID_RECOVERTREE_CLEARFOUNDFILES, OnRecovertreeClearfoundfiles)
	ON_COMMAND(ID_RECOVERTREE_VIEWSECTORS, OnRecovertreeViewsectors)
	ON_COMMAND(ID_RECOVERTREE_VIEWDETAILS, OnRecovertreeViewdetails)
	ON_WM_NCMOUSEMOVE()
	ON_COMMAND(ID_REMOTEMACHINE_CONNECT, OnRemotemachineConnect)
	ON_MESSAGE(WMS_SOCKET_NOTIFY, OnSocketNotify)
	ON_MESSAGE(WMS_ADD_REMOTE, AddRemoteTree)
	ON_COMMAND(ID_REMOTEMACHINE_GETMASTERBOOTRECORD, OnRemotemachineGetMBR)
	ON_COMMAND(ID_REMOTEMACHINE_SENDTEXTMESSAGE, OnRemotemachineSendtextmessage)
END_MESSAGE_MAP()


/*
	NOTE: You need to have this function before any window will appear, once this was
	added the window appeared, see notes below from 
    /////////////////////////////////////////////////////////////////////////////////
	see also :
	ms-help://MS.VSCC/MS.MSDNVS/vclib/html/_MFCNOTES_TN031.htm

	see below:
	ms-help://MS.VSCC/MS.MSDNVS/vclib/html/_MFCNOTES_TN024.htm
	WM_SIZEPARENT
	This message is sent by a frame window to its immediate	children during resizing 
	(CFrameWnd::OnSize calls CFrameWnd::RecalcLayout which calls CWnd::RepositionBars) 
	to reposition the control bars around the side of the frame. 
	The AFX_SIZEPARENTPARAMS structure contains the current available client rectangle 
	of the parent and a HDWP (which may be NULL) with which to call DeferWindowPos to 
	minimize repainting.
    //////////////////////////////////////////////////////////////////////////////////
	wParam Not used 
	lParam Address of an AFX_SIZEPARENTPARAMS structure 
	returns Not used (0) 
	//////////////////////////////////////////////////////////////////////////////////
	Ignoring the message indicates that the window doesn't take part in the layout.
	//////////////////////////////////////////////////////////////////////////////////
    

	1. Create The derived class of CControlBar
	2. Edit the Create Function for when we are called
	3. Add the message map 
			ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
			#include "afxpriv.h"
			add function & header LRESULT CControlWnd::OnSizeParent(WPARAM, LPARAM lparam)
	4. Add the following to the Create/Ex function before calling base create
			m_dwStyle = (dwStyle & CBRS_ALL); - Save the styles in the base class
	5. Override OnCreate and OnPaint as usual and add your code to draw/create objects
	

*/
LRESULT CControlWnd::OnSizeParent(WPARAM wparam, LPARAM lparam)
{     
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lparam;
	lpLayout->bStretch = true;

	return CControlBar::OnSizeParent(wparam, lparam) ;
}

/*
	Create us
*/
BOOL CControlWnd::CreateEx(DWORD dwStyle, CWnd* pParentWnd, UINT nID)
{
			
	dwStyle = WS_CHILD |					// must have
			  WS_VISIBLE |					// must have
			  WS_DLGFRAME |
			  CBRS_ALIGN_LEFT |				// align left dock
			  CBRS_GRIPPER |				// draw gripper when docked
			  CBRS_SIZE_DYNAMIC |			// allow sizing
			  CBRS_BORDER_ANY |
			  CBRS_ORIENT_VERT |
			  CBRS_HIDE_INPLACE |
			  CBRS_BORDER_3D;
	


	SetBorders(0, 0, 0, 0);					// set the size of the control bar's borders		
	m_cyBottomBorder = -10;


	// save the style (some of these style bits are MFC specific)
	
	m_dwStyle |= CBRS_HIDE_INPLACE |			// Hide ControlBar 
				 CBRS_ORIENT_VERT; 				// Vertical Alignment Only !!

	m_dwStyle |= (dwStyle & CBRS_ALL);

	//m_dwStyle |= dwStyle;
	
	return CWnd::Create(CONTROLWNDCLASS,
										SRB_CLASS_TITLE,
										dwStyle,
										CRect(0,0,0,0),
										pParentWnd,
										SRB_IDW_CONTROLWND);
	
}


int CControlWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	
    
	// Add the controls to it
	if(!m_wndTree.CreateEx(WS_EX_CLIENTEDGE|WS_EX_RIGHTSCROLLBAR|WS_EX_LTRREADING|WS_EX_LEFT, 
					   WS_HSCROLL|WS_VSCROLL|WS_CHILD|WS_VISIBLE|TVS_HASBUTTONS|TVS_HASLINES|TVS_SHOWSELALWAYS,
					   CRect(10,10,100,100),
					   this,
					   0x1108))
	{
		TRACE0("Failed to create tree control\n");
		return -1;
	}
	m_wndTree.SetOwner(this);		// CMainFrame will recieve all message from control
	LoadImageList();
	RefreshTreeControl();				// Size and build tree 
  



	return 0;
}

/*
	On Paint Message
*/

void CControlWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rect, temp;
	GetClientRect(&rect);
	GetWindowRect(&temp);
	ScreenToClient(&temp);
	// Fill Default Background
	dc.FillSolidRect(temp, ::GetSysColor(COLOR_BTNFACE) );
	//DrawBorders(&dc, rect);			// Dont draw ugly borders
	DrawGripper(&dc, rect);
    	
	return;

}
/*
	Register this class (CControlWnd)
*/
void CControlWnd::RegisterClass(void)
{

	WNDCLASS lpWndClass;
	memset(&lpWndClass, 0, sizeof(WNDCLASS));   // start with NULL	
	lpWndClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
	lpWndClass.lpfnWndProc = ::DefWindowProc; 
    lpWndClass.hInstance = AfxGetInstanceHandle();
	lpWndClass.hCursor = AfxGetApp()->LoadCursor( IDC_ARROW );
    lpWndClass.hbrBackground = (HBRUSH) (COLOR_BTNFACE);
    lpWndClass.lpszMenuName = NULL;
    // Specify class name for using FindWindow later
    lpWndClass.lpszClassName = _T(CONTROLWNDCLASS);

	if(!AfxRegisterClass(&lpWndClass))
	{
		TRACE("Failed To Register Control Wnd Class\n");
	}
}
/*
	Calculate non-client area size
*/

void CControlWnd::CalcInsideRect(CRect& rect, BOOL bHorz) const
{
	// subtract standard CControlBar borders
	CControlBar::CalcInsideRect(rect, bHorz);

	// subtract size grip if present
	//if ((GetStyle() & SBARS_SIZEGRIP) && !::IsZoomed(::GetParent(m_hWnd)))
	//{
		// get border metrics from common control
		int rgBorders[3];
		CControlWnd* pBar = (CControlWnd*)this;
		pBar->DefWindowProc(SB_GETBORDERS, 0, (LPARAM)&rgBorders);

		// size grip uses a border + size of scrollbar + cx border
	//	rect.bottom -= rgBorders[0] + ::GetSystemMetrics(SM_CXVSCROLL) +
		//	::GetSystemMetrics(SM_CXBORDER) * 2;
	//}


}
void CControlWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// calculate border space (will add to top/bottom, subtract from right/bottom)
	CRect rect; 
	rect.SetRectEmpty();
	CalcInsideRect(rect, false);				// Get Client Area Of Ctrl Bar
	
	// adjust non-client area for border space
	//lpncsp->rgrc[0].left += rect.left;
	//lpncsp->rgrc[0].top += rect.top;
	//lpncsp->rgrc[0].right -= 30;
	//lpncsp->rgrc[0].bottom -= rect.bottom;
	
	return;
	//CControlBar::OnNcCalcSize(bCalcValidRects, lpncsp);
}

/*
	Controls / Allows Resizing
*/
CSize CControlWnd::CalcDynamicLayout(int nLength, DWORD nMode )
{
	CSize	sizeResult;
	POINT	sizePoint;

	sizePoint.x = m_pDockContext->m_ptLast.x;
	sizePoint.y = m_pDockContext->m_ptLast.y;
	ScreenToClient(&sizePoint);
	sizeResult.cx = sizePoint.x;
	sizeResult.cy = sizePoint.y;

	if(sizeResult.cy < 2)
		return m_SizeLast; //CSize(CONTROL_MIN_WIDTH,CONTROL_MIN_HEIGHT);

	if(IsFloating())
	{
		if(sizeResult.cy >= CONTROL_MIN_HEIGHT)
			sizeResult.cy = sizePoint.y;
		else
			sizeResult.cy = CONTROL_MIN_HEIGHT;

   		if(sizeResult.cx >= CONTROL_MIN_WIDTH)
			sizeResult.cx = sizePoint.x;
		else
			sizeResult.cx = CONTROL_MIN_WIDTH;

		CWnd* pFrame = GetParentFrame();
	//	if(pFrame->ModifyStyle(0,  WS_HSCROLL|WS_VSCROLL,   SWP_SHOWWINDOW | SWP_NOZORDER))
	//	{
	//		TRACE0("\nSuccess");
	//	}

	//	if(::IsWindow(m_wndTree.GetSafeHwnd() ))
	//		m_wndTree.ModifyStyleEx( WS_EX_CLIENTEDGE, 0, SWP_SHOWWINDOW | SWP_NOZORDER);

	}
	else		// control bar is docked
	{
		//if(::IsWindow(m_wndTree.GetSafeHwnd() ))
		//	m_wndTree.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_SHOWWINDOW | SWP_NOZORDER);

		CRect rectFrame;
		CFrameWnd* pFrame = GetParentFrame();
		pFrame->GetClientRect(&rectFrame);
		
		CRect wRect;
		GetWindowRect(&wRect);
		ScreenToClient(&wRect);
		//sizeResult.cy = wRect.bottom-wRect.top;
		sizeResult.cy = rectFrame.bottom - rectFrame.top;				// Fill Area We Have Available
		sizeResult.cx = wRect.right-wRect.left;
	}
	//Invalidate(true);
	m_SizeLast = sizeResult;			// save this result
	return sizeResult;
}
/*
	Non client paint
*/
void CControlWnd::OnNcPaint()
{  
	EraseNonClient();
}
/*
	When the Window Position changes and is docked make sure we use the 
	full height of the containing window
*/
void CControlWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if(!IsFloating())
	{
		CRect rectFrame;
		CFrameWnd* pFrame = GetParentFrame();
		pFrame->GetClientRect(&rectFrame);
		// Change the height of the window before calling the base class
		lpwndpos->x = 0;
        lpwndpos->y = 0;
		lpwndpos->cy = rectFrame.bottom - rectFrame.top;				// Fill Area We Have Available
		//lpwndpos->cx = 50;
	}
	else
	{
			// do nothing
	}
	CControlBar::OnWindowPosChanging(lpwndpos);

	CWnd* vp = GetParent();
	CWnd* va;
	if(vp->IsKindOf(RUNTIME_CLASS(CDockBar)))
	{
		va = vp->GetParent();
		BOOL b = vp->ModifyStyle(0, WS_DLGFRAME); 
		BOOL c = vp->ModifyStyleEx(0, WS_EX_OVERLAPPEDWINDOW);
		TRACE0("\n");
	    
	}

}

/*
	Draw The Gripper When Docked
*/
void CControlWnd::DrawGripper(CDC* pDC, const CRect& rect)
{
	// only draw the gripper if not floating and gripper is specified
	if ((m_dwStyle & (CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER)
	{
		{	pDC->Draw3dRect(rect.left +m_cyTopBorder,
							rect.top+CY_BORDER_GRIPPER,
							rect.Width()-m_cyTopBorder-m_cyBottomBorder, 
							CY_GRIPPER,
							::GetSysColor(COLOR_BTNHIGHLIGHT),
							::GetSysColor(COLOR_BTNSHADOW));

		}
	}
}

void CControlWnd::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler)
{
	// TODO: Add your specialized code here and/or call the base class
}


void CControlWnd::EnableDocking(DWORD dwDockStyle)
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);
	// CBRS_SIZE_DYNAMIC toolbar cannot have the CBRS_FLOAT_MULTI style
	ASSERT(((dwDockStyle & CBRS_FLOAT_MULTI) == 0) || ((m_dwStyle & CBRS_SIZE_DYNAMIC) == 0));

	m_dwDockStyle = dwDockStyle;
	if (m_pDockContext == NULL)
		m_pDockContext = new CDockContext(this);

	// permanently wire the bar's owner to its current parent
	if (m_hWndOwner == NULL)
		m_hWndOwner = ::GetParent(m_hWnd);
}

CWnd* CControlWnd::GetCWnd(void)
{
	return this;
}

void CControlWnd::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);
	
	// first size the tree control correctly
	CRect rect;
	GetClientRect(&rect);
	CalcInsideRect(rect, false);
	if(::IsWindow(m_wndTree) )
		m_wndTree.SetWindowPos(NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW|SWP_NOZORDER);
	
}

/***********************************************************************************************
								Tree Control Functions
***********************************************************************************************/
void CControlWnd::RefreshTreeControl()
{
	CDisk = new CDiskAnalysis;
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvInsert.item.iImage = 4;
	tvInsert.item.iSelectedImage = 4;
	
	CString str;
	str.LoadString(IDS_TREEROOT); 
	// add the root of all evil - 'My Computer'
	tvInsert.item.pszText = _T("My Computer");
	m_hTreeRoot = m_wndTree.InsertItem(&tvInsert);

	// add the leaf for found file search
	str.LoadString(IDS_SEARCHROOT); 
	tvInsert.item.pszText = _T("Search Results");
	tvInsert.hParent = m_hTreeRoot;
	tvInsert.item.iImage = 3;
	tvInsert.item.iSelectedImage = 3;
	m_hTreeDeleted = m_wndTree.InsertItem(&tvInsert);


	PHYSICAL_DISKS*	phyDisk;
	phyDisk = CDisk->m_physicaldisks;

	LOGICAL_DISKS*	logDisk;
	logDisk = CDisk->m_logicaldisks;
	//DA_DISK_EX_INT13_INFO* sDiskInfo;

	CString tmpStr;
	HTREEITEM m_hTree2nd, htreeLastParent;
	//char cTemp[10];
	//cTemp[9] = '\0';
	BYTE bLastRecNo = 0;
//	char* strText = NULL;
	//	char czBuf[2];
	TCHAR *czTemp = NULL;
	//strText = (char*)malloc(2);
	//strText = (char*)malloc(2);
    while(phyDisk->cName[0] != '\0')
	{
		tvInsert.item.iImage = 0;
		tvInsert.item.iSelectedImage = 0;
		tvInsert.hParent = m_hTreeRoot;
		tvInsert.item.pszText = _T(phyDisk->cName);
		htreeLastParent = m_hTree2nd = m_wndTree.InsertItem(&tvInsert);

		//sDiskInfo = (DA_DISK_EX_INT13_INFO*)VirtualAlloc(NULL, sizeof(DA_DISK_EX_INT13_INFO), MEM_COMMIT, PAGE_READWRITE);
		//CDisk->GetINT13Info(phyDisk->cName, sDiskInfo);


		//sDiskInfo.iTotalBytes
		/////////////////////////////////////////////////////////////////
		// Does this disk have any listed volumes in the MBR ?
		TRACE1("Opening disk %s\n", phyDisk->cName);
		MASTER_BOOT_RECORD* sMBR;
		if(CDisk->FillPartitionInfo(phyDisk->cName))		// fill MBR with info about partitions/volumes
		{
			 int i=0;

			/*


			*/
			
			while(true)	 // Start sector will be 0 on a non exist partition
			{
				sMBR = (MASTER_BOOT_RECORD*)GlobalLock(CDisk->hGlobalPT);
				sMBR += i;

				if(sMBR->dwRelativeSector == 0)
				{
					GlobalUnlock(CDisk->hGlobalPT);
					break;
				}
		
				
				if(czTemp != NULL)
					VirtualFree(czTemp, 0, MEM_RELEASE);

				czTemp = (TCHAR*)VirtualAlloc(NULL, 
											(strlen(sMBR->strResourceDesc) + strlen(sMBR->strFileSystem) + 3 ), 
											MEM_COMMIT, 
											PAGE_READWRITE);

				strcpy(czTemp, sMBR->strResourceDesc);
				strcat(czTemp, " - ");
				strcat(czTemp, sMBR->strFileSystem);
		

				if(sMBR->bParent == bLastRecNo)	// child of last 
					tvInsert.hParent = htreeLastParent;
				else
                    tvInsert.hParent = m_hTree2nd;


				tvInsert.item.pszText = czTemp;
				htreeLastParent = m_wndTree.InsertItem(&tvInsert);


				bLastRecNo = sMBR->bRecordNo;			// save this recno in case the next record is a child
                
				GlobalUnlock(CDisk->hGlobalPT);
            

				i++;
			}
		}



		/////////////////////////////////////////////////////////////////


		// Next record	/ disk
		phyDisk++;
	}

    // add the leaf for found file search
	tvInsert.item.pszText = _T("Remote Machines");
	tvInsert.hParent = m_hTreeRoot;
	tvInsert.item.iImage = 4;
	tvInsert.item.iSelectedImage = 4;
	m_hTreeRemote = m_wndTree.InsertItem(&tvInsert);

	m_wndTree.Expand(m_hTreeRoot, TVE_EXPAND);


	VirtualFree(czTemp, 0, MEM_RELEASE);
//	VirtualFree(strText, 0, MEM_RELEASE);
	delete CDisk;
	CDisk = NULL;
	czTemp = NULL;
	//strText = NULL;
	phyDisk = NULL;
	logDisk = NULL;


	return;
}



void CControlWnd::OnContextMenu(CWnd* pWnd, CPoint ptMousePos)
{
	
	// if Shift-F10
	if (ptMousePos.x == -1 && ptMousePos.y == -1)
		ptMousePos = (CPoint) GetMessagePos();

	m_wndTree.ScreenToClient(&ptMousePos);	   // get tree mouse points

	UINT uFlags;
	HTREEITEM htItem;
	
	htItem = m_wndTree.HitTest( ptMousePos, &uFlags );

	if( htItem == NULL )
		return;
	
	m_hActiveItem = htItem;
	m_wndTree.SelectItem(m_hActiveItem);

    CMenu menu;
	CMenu* pPopup;

	// the popup is stored in a resource
	 if(m_hTreeDeleted == htItem)
	 {
		if( menu.LoadMenu(IDR_TREEITEM_CONTEXTMENU) > 0 )
		{
			pPopup = menu.GetSubMenu(0);
			ClientToScreen(&ptMousePos);
			pPopup->TrackPopupMenu( TPM_LEFTALIGN, ptMousePos.x, ptMousePos.y, this );
		}
		return;
	 }
	
	// the connect to remote machine
	 if(m_hTreeRemote == htItem)
	 {
		if( menu.LoadMenu(IDR_REMOTEITEM_CONTEXTMENU) > 0 )
		{
			pPopup = menu.GetSubMenu(0);
			ClientToScreen(&ptMousePos);
			pPopup->TrackPopupMenu( TPM_LEFTALIGN, ptMousePos.x, ptMousePos.y, this );
		}
		return;
	 }
	



	 TV_ITEM tvItem;
	 TCHAR szText[1024];
	 tvItem.hItem = htItem;
	 tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
	 tvItem.pszText = szText;
	 tvItem.cchTextMax = 1024;

	 if(m_wndTree.GetItem(&tvItem))
	 {
		if(tvItem.lParam == 0x0100)		// Deleted File Record
		{
			menu.LoadMenu(IDR_FILETASKS);	
			pPopup = menu.GetSubMenu(0);
			ClientToScreen(&ptMousePos);
			pPopup->TrackPopupMenu( TPM_LEFTALIGN, ptMousePos.x, ptMousePos.y, this );
		
		}
	 }




}

void CControlWnd::OnSearchforfileTest()
{

	CMiscDialog dlg;
	if(dlg.DoModal() == IDOK)
	{
		

		FF_PARAMS	m_params;
		m_params.bActive = dlg.m_ActiveFiles;
		m_params.bDeleted = dlg.m_DeletedFiles;
		m_params.strFileName = dlg.m_strFileName;
		m_params.strLogicalDisk = dlg.m_strDisk;

		// TODO : check string format here !!!
		CString strFileName = dlg.m_strFileName;
		CString strLogicalDisk = dlg.m_strDisk;


		CDiskAnalysis CDisk;

		// Alloc memory
		m_FileClusters = (FILE_CLUSTERS*)VirtualAlloc(NULL,
													 5000000,				// TODO :
													 MEM_COMMIT,
													 PAGE_READWRITE);


		//if(CDisk.FindFile(strFileName, strLogicalDisk , 0, m_FileClusters))
		int iCnt = CDisk.FindFile(&m_params, m_FileClusters);
		if(iCnt>0)
		{
			CString str;
			CString s; 	s.AppendFormat("%i", iCnt);
			AfxFormatString1(str, IDS_FOUNDFILE, s);
			AfxMessageBox(str, MB_ICONINFORMATION);
			FILE_CLUSTERS* pFile;
			pFile = m_FileClusters;
			DeleteTreeFoundFiles();
			int iCount = AddTreeFoundFiles(pFile);

			str.LoadString(IDS_SEARCHROOT); 
			str.AppendFormat(" (%i)", iCount);
			m_wndTree.SetItemText(m_hTreeDeleted, str);

		}
		else
		{
			AfxMessageBox(IDS_SFILENOTFOUND, MB_ICONINFORMATION);
		}
	


	}
}

int CControlWnd::AddTreeFoundFiles(FILE_CLUSTERS* pFile)
{
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	int iCount = 0;
	
	tvInsert.item.mask = TVIF_TEXT |  TVIF_IMAGE | TVIF_SELECTEDIMAGE|TVIF_PARAM;
	FILE_EXTENT*	m_Extents;
	m_Extents = pFile->sFileExtents;

	HTREEITEM htTemp;
	while(strlen(pFile->strFileName) >0)
	{
		char buffer[2];
		buffer[1] = '\0';
		_itoa(pFile->iPhyDiskNumber, buffer, 10);

		char buffer2[2];
		buffer2[1] = '\0';
		_itoa(pFile->iPartition, buffer2, 10);
	    	
		char lpString[MAX_PATH];
		sprintf(lpString, "\\\\PhysicalDisk%c\\Volume%c%s\\%s", buffer[0], buffer2[0], pFile->strFolder, pFile->strFileName);
		tvInsert.item.iImage = 0;
		tvInsert.item.iSelectedImage = 0;
		tvInsert.item.pszText = lpString;
	    tvInsert.hParent = m_hTreeDeleted;
		tvInsert.item.lParam = 0x0100;			// Deleted File = 0x0100
		htTemp = m_wndTree.InsertItem(&tvInsert);
		pFile->lParam = (LPARAM)htTemp;					// store for later use when item selected
		
		m_Extents = pFile->sFileExtents;
		for(int i=0;i<pFile->iNoExtents;i++)
		{
			char cString[100];
			int j = sprintf(cString, "Cluster# %i", m_Extents->iFileExtentStartCluster);
			sprintf(cString + j, "-%i", m_Extents->iFileExtentEndCluster);
			tvInsert.item.pszText = cString;
			tvInsert.item.lParam = 0;
			tvInsert.hParent = htTemp;
			tvInsert.item.iImage = 2;
			tvInsert.item.iSelectedImage = 2;
			m_wndTree.InsertItem(&tvInsert);
			m_Extents++;
		}


		pFile++;
		iCount++;
	}

	return iCount;	    
}

/*
	Populate image list for tree ctrl.
*/
void CControlWnd::LoadImageList(void)
{
	m_ImageList = new CImageList;

	LPTSTR lpSysDir;
	TCHAR tchBuf[MAX_PATH+1]; 
	lpSysDir = tchBuf;
	HICON icon;

	::GetSystemDirectory(lpSysDir, MAX_PATH);
	HINSTANCE hLib = ::LoadLibrary(strcat(lpSysDir, "\\shell32.dll") );
	HINSTANCE hInst = AfxGetResourceHandle();

	m_ImageList->Create(16, 16, ILC_COLOR32, 1 , 10);

	if(hLib)
	{
        AfxSetResourceHandle(hLib);

		icon = AfxGetApp()->LoadIcon(9);		// HD  index 0
		m_ImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(12);		// CD index 1
		m_ImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(274);		// RAM index 2
		m_ImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(254);		// Deleted Files index 3
		m_ImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(16);		// My Computer	 index 4
		m_ImageList->Add(icon);



		m_wndTree.SetImageList(m_ImageList , LVSIL_NORMAL);
		AfxSetResourceHandle(hInst);


	}



}
/*

	Recover selected file.

*/
void CControlWnd::OnRecovertreeRecoverthisfile()
{
	FILE_CLUSTERS* pFile;
	pFile = EnumFileClusters();

	if(pFile->iNoExtents == 0)
	{
		AfxMessageBox("File has zero extents\nCannot recover this file.", MB_ICONINFORMATION);
		return;
	}

	if(pFile != NULL)
	{
		CDiskAnalysis CDisk;
		CRecoverDlg	dlg;
		dlg.m_strRecoverFile = pFile->strFileName;		// Found File Name
		dlg.m_strFilename = "F:\\";		  
		dlg.m_strFilename.AppendFormat("%s", pFile->strFileName);


		if(dlg.DoModal() == IDOK)
		{ 
			memcpy(pFile->strRecoverPath, dlg.m_strFilename, strlen(dlg.m_strFilename));
			if(CDisk.RecoverFile(pFile))
			{
				AfxMessageBox("File recovered..");
				return;
			}
		}
	}

	AfxMessageBox("File recovery failed.", MB_ICONSTOP);
	return;
}

void CControlWnd::DeleteTreeFoundFiles(void)
{

	TVITEM	tvitem;
	tvitem.hItem = m_hTreeDeleted;
	HTREEITEM hChild;


	hChild = m_wndTree.GetNextItem(m_hTreeDeleted, TVGN_CHILD);
	while( hChild != NULL)	
	{
		m_wndTree.DeleteItem(hChild);
		hChild =m_wndTree.GetNextItem(m_hTreeDeleted, TVGN_CHILD);
	}

	 
}

void CControlWnd::OnRecovertreeClearfoundfiles()
{
	DeleteTreeFoundFiles();
	// TODO: Add your command handler code here
}

FILE_CLUSTERS* CControlWnd::EnumFileClusters(void)
{
	FILE_CLUSTERS* pFile;
	pFile =	m_FileClusters;

	while(strlen(pFile->strFileName) > 0)
	{
		if(pFile->lParam == (LPARAM)m_hActiveItem)
			return pFile;

		pFile++	;
	}

	return NULL;
}



#define CTRL_CLASSNAME	_T("DCtrl")
void CControlWnd::OnRecovertreeViewdetails()
{
	// Get a struct pointer to the currently selected item in the tree control
	FILE_CLUSTERS* pFile = EnumFileClusters();

	HWND hwnd = CreateWindow("EDIT", "", WS_OVERLAPPEDWINDOW | ES_MULTILINE | ES_READONLY, 
		100,
		100, 
		500, 
		200, this->GetSafeHwnd(), NULL,  NULL, NULL);

	if(hwnd == NULL)
    {
       TRACE("Create Window Failed\n");
	   DWORD dwErr = GetLastError();
       return;
    }

	CString str;
	::ShowWindow(hwnd, SW_SHOW);	  
	::UpdateWindow(hwnd);
	HDC hDC = ::GetDC(hwnd);									   
			  
						  
	str.AppendFormat("File Name      : %s\r\n", pFile->strFileName);
    str.AppendFormat("Physical Disk  : %i\r\n", pFile->iPhyDiskNumber);
	str.AppendFormat("Volume		 : %i\r\n", pFile->iPartition);
	str.AppendFormat("File Folder    : %s\r\n", pFile->strFolder);
	str.AppendFormat("Logical Disk   : %s\r\n", pFile->strLogicalDisk);
    str.AppendFormat("File Length    : %i\r\n", pFile->iFileLength);
	str.AppendFormat("No of Extents  : %i\r\n", pFile->iNoExtents);
	str.AppendFormat("MFT Record No  : %i\r\n", pFile->iRecordNo);
	str.AppendFormat("Is Deleted ?   : %i\r\n", pFile->iIsDeleted);

	LPCSTR str2 = str;
    
	::SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)str2);




	::UpdateWindow(hwnd);
	
													  
	 




}


void CControlWnd::OnRecovertreeViewsectors()
{
	// Get a struct pointer to the currently selected item in the tree control
	FILE_CLUSTERS* pFile = EnumFileClusters();
	
	if(pFile == NULL)
	{
		AfxMessageBox("Error Enumerating File Clusters", MB_ICONSTOP);
		return;
	}

	// get pointer to extents struct in file clusters struct
	FILE_EXTENT* pExtent = pFile->sFileExtents;
													   

	CString strClusters;
	strClusters.Empty();

	for(int i=0;i<pFile->iNoExtents;i++)
	{
		strClusters.AppendFormat("%i,", pExtent->iFileExtentStartCluster);
		strClusters.AppendFormat("%i,", pExtent->iFileExtentEndCluster);
		pExtent++;
	}

	char lpString[MAX_PATH];
	if(  !pFile->iIsDeleted &&  strlen(pFile->strLogicalDisk) > 0)		// if not deleted then view file using MFT/FAT entry
		sprintf(lpString, "%s%s\\%s", pFile->strLogicalDisk, pFile->strFolder, pFile->strFileName);
	else
	{
		TRACE0("\n\n***needs to be completed***\n\n");
		
		ASSERT(false);		// if you have asserted here, you have tried to view a deleted files sectors
							// you havent done this func yet !!! :-(
		return;
	}
	

	CString strDocName;
    CDocTemplate* pSelectedTemplate;
    POSITION pos = AfxGetApp()->m_pDocManager->GetFirstDocTemplatePosition();
    while (pos != NULL)
	{
        pSelectedTemplate = (CDocTemplate*) AfxGetApp()->m_pDocManager->GetNextDocTemplate(pos);
        pSelectedTemplate->GetDocString(strDocName, CDocTemplate::docName);
        if (strDocName == "DiskData")
		{ 
			// create new funtion similiar to OpenDocument but one to cope with this situation
			// of opening clusters
            CDiskDataDoc* pDoc = (CDiskDataDoc*)pSelectedTemplate->OpenDocumentFile(lpString, true);		// Dont Show until we update local vars
			//pDoc->m_bShowSelectDialog = false;
        }
    }


	
	return;
}









void CControlWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	ASSERT(false);

	CControlBar::OnNcMouseMove(nHitTest, point);
}

BOOL CControlWnd::PreCreateWindow(CREATESTRUCT& cs)
{

   	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	return true;

	return CControlBar::PreCreateWindow(cs);
}

LRESULT CControlWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

		return CWnd::WindowProc(message, wParam, lParam);
	//return lResult;
}

/*
  connect to remote machine
*/
void CControlWnd::OnRemotemachineConnect()
{
	m_ipdisk = new CIPDisk(this->GetSafeHwnd());

	

	CDlgIPAddress dlg;
	if(dlg.DoModal() == IDOK)
	{
		
		if(m_ipdisk->InitSockets(CR_CLIENT, dlg.m_ipaddr) == CR_STATUS_FAILED)
		{
			TRACE0("\nFailed to open server socket");
			AfxMessageBox("Failed to start Winsock", MB_ICONSTOP);
		}

	}

}

LRESULT CControlWnd::OnSocketNotify(WPARAM wparam, LPARAM lparam)
{
	WORD wEvent, wError;

	wEvent = WSAGETSELECTEVENT(lparam);
	wError = WSAGETSELECTERROR(lparam);

	switch(wEvent)
	{
	case FD_CONNECT:			// connect to remote host
		TRACE0("\nConnect-DiskData");
		if(!wError)
		{
            TRACE0("\nSocket connected to server");
			AfxMessageBox("Socket connected to server");
			m_ipdisk->m_bConnected = true;
		}
		else
		{
			switch(wError)
			{
			case WSAEHOSTUNREACH:
				AfxMessageBox("A socket operation was attempted to an unreachable host.", MB_ICONINFORMATION);
				break  ;
			case WSAECONNREFUSED:
				AfxMessageBox("The attempt to connect was forcefully rejected.", MB_ICONINFORMATION);
				break;
			case WSAEAFNOSUPPORT: 
				AfxMessageBox("Addresses in the specified family cannot be used with this socket.", MB_ICONINFORMATION); 
				break;
			case WSAENETUNREACH: 
				AfxMessageBox("The network cannot be reached from this host at this time.", MB_ICONINFORMATION); 
				break;
			case WSAEFAULT: 
				AfxMessageBox("The namelen parameter is incorrect", MB_ICONINFORMATION); 
				break;
			case WSAEINVAL: 
				AfxMessageBox("The socket is already bound to an address.", MB_ICONINFORMATION); 
				break;
			case WSAEISCONN: 
				AfxMessageBox("The socket is already connected.", MB_ICONINFORMATION); 
				break;
			case WSAEMFILE: 
				AfxMessageBox("No more file descriptors are available.", MB_ICONINFORMATION); 
				break;
			case WSAENOBUFS: 
				AfxMessageBox("No buffer space is available. The socket cannot be connected.", MB_ICONINFORMATION); 
				break;
			case WSAENOTCONN: 
				AfxMessageBox("The socket is not connected.", MB_ICONINFORMATION); 
				break;
			case WSAETIMEDOUT: 
				AfxMessageBox("Attempt to connect timed out without establishing a connection.", MB_ICONINFORMATION);
				break;
			default:
				AfxMessageBox("Connect failed", MB_ICONINFORMATION);
				break;
			}

		}
		break;

	case FD_ACCEPT:				// attempt to connect to us, ignore as we dont accept calls we make them
		TRACE0("\nConnect Attempt-DiskData");
		break;

	case FD_WRITE:				// write data to remote 	
		TRACE0("\nWrite-DiskData");
		break;

	case FD_READ:				// recieved data from remote
		m_ipdisk->IPRead();
		TRACE0("\nRead-DiskData");
		break;

	default:
		break;

	}

	return 0L;
}

void CControlWnd::OnRemotemachineGetMBR()
{
	// request remote machines MBR
	m_ipdisk->IPSend(DD_MBR);
}

LRESULT CControlWnd::AddRemoteTree(WPARAM wparam, LPARAM lparam)
{
	MASTER_BOOT_RECORD* mbr = (MASTER_BOOT_RECORD*)wparam;
	int iSize = (int)lparam;
	char* czTemp = NULL;
	int bLastRecNo = -1;
	HTREEITEM htreeLastParent = NULL;
	TVINSERTSTRUCT tvInsert;

	tvInsert.item.iImage = 0;
	tvInsert.item.iSelectedImage = 0;
	tvInsert.hParent = m_hTreeRoot;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	htreeLastParent = m_hTreeRemote;

	for(int j=0;j<iSize;j++)	 // Start sector will be 0 on a non exist partition
	{
		
				
		if(czTemp != NULL)
			VirtualFree(czTemp, 0, MEM_RELEASE);

		czTemp = (TCHAR*)VirtualAlloc(NULL, 
									(strlen(mbr->strResourceDesc) + strlen(mbr->strFileSystem) + 3 ), 
									MEM_COMMIT, 
									PAGE_READWRITE);

		strcpy(czTemp, mbr->strResourceDesc);
		strcat(czTemp, " - ");
		strcat(czTemp, mbr->strFileSystem);


		if(mbr->bParent == bLastRecNo)	// child of last 
			tvInsert.hParent = htreeLastParent;
		else
            tvInsert.hParent = m_hTreeRemote;
    
		tvInsert.item.pszText = czTemp;
		htreeLastParent = m_wndTree.InsertItem(&tvInsert);

		bLastRecNo = mbr->bRecordNo;			// save this recno in case the next record is a child
        
		mbr++;
	}

	VirtualFree(czTemp, 0, MEM_RELEASE);

	return 0L;
}

void CControlWnd::OnRemotemachineSendtextmessage()
{
	CSendText dlg;
	if(dlg.DoModal() == IDOK)
		m_ipdisk->SendMessage(dlg.m_strEdit);
	
}
