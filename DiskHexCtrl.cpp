// DiskHexCtrl.cpp : implementation file
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "stdafx.h"
#include "DiskData.h"
#include "DiskHexCtrl.h"

#include "Shlwapi.h"
#include "cmath"


// windows class-name
#define DISKHEXCTRL_CLASSNAME	_T("CDiskHexCtrl")

//#define __DEBUG__PAINT

const char tabHexCharacters[16] = {
	'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' }; 
const char tabAscCharacters[16] = {
	'0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5' }; 

const char tabHexValid[64] = {
	'1','1','0','0','1','1','0','0','1','1','0','0','1','1','0','0',
	'1','1','0','0','1','1','0','0','1','1','0','0','1','1','0','0',
	'1','1','0','0','1','1','0','0','1','1','0','0','1','1','0','0',
	'1','1','0','0','1','1','0','0','1','1','0','0','1','1','0','0'}; 


// CDiskHexCtrl
IMPLEMENT_DYNAMIC(CDiskHexCtrl, CWnd)
CDiskHexCtrl::CDiskHexCtrl()
: m_ptrData(NULL)
{
	
	RegisterClass();

	m_ptrData = NULL;
	memset(&m_tMetricDetails, 0, sizeof(METRIC_DETAILS));
	m_tMetricDetails.nLastFP = 1;

	m_bMoveForward = false;
	m_BeginHighSelected = CPoint(0,0);
	m_EndHighSelected = CPoint(0,0);
	m_AreWeDragging = false;
	m_Highlighted = false;
	m_iOffset = 0;
	m_nDrawHeaders = true;
	m_tMetricDetails.nVScrollStart = 1;
	m_tMetricDetails.nHScrollStart = 1;
	m_tMetricDetails.nByteCount = 1;


	// Define Default Colors
	m_headerclr = RGB(0,0,255);
	m_offsetclr = RGB(0,0,0);
	m_hexclr	= RGB(0,0,0);
	m_ascclr	= RGB(0,0,0);
	m_bkgclr	= RGB(255,255,255);
	m_sepclr	= GetSysColor(COLOR_SCROLLBAR);
	m_gray		= RGB(192,192,192);
	m_ascselectbkg = RGB(0,20,0);
	m_ascselecttxt = RGB(0,255,0);
	m_hlightbkg = GetSysColor(COLOR_HIGHLIGHT);
	m_hlighttxt = RGB(255,255,255);
	m_crMenu = GetSysColor(COLOR_3DFACE);		// Face Of Object
	m_crLight = GetSysColor(COLOR_3DHIGHLIGHT);		// Edge Facing Light Source (Top, Right)
	m_crShadow = GetSysColor(COLOR_3DSHADOW);	// Edges Facing Away From Light (Btm, Left)

	m_tMetricDetails.nLastFP = 0;	
	m_tMetricDetails.nCharPerLine = 16;
	m_tMetricDetails.nRowsPerPage = 32;
	m_tMetricDetails.nLineSpace = 0;		// Changed in CalcMetricDetails
	m_tMetricDetails.nCharSpace = 2;		// Changed in CalcMetricDetails
	m_tMetricDetails.bSectorHex = true;
	m_tMetricDetails.pLastCaretPoint.x = -1;
	m_tMetricDetails.pLastCaretPoint.y = -1;

	


	
}

CDiskHexCtrl::~CDiskHexCtrl()
{
	//_CrtDumpMemoryLeaks();
}



BEGIN_MESSAGE_MAP(CDiskHexCtrl, CWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_HEXPOPUPMENU_SELECTALL, OnHexpopupmenuSelectall)
	ON_COMMAND(ID_HEXPOPUPMENU_COPY, OnHexpopupmenuCopy)
	ON_COMMAND(ID_COPY_COPYASCIIVALUES, OnCopyCopyasciivalues)
	ON_COMMAND(ID_COPY_COPYHEXVALUES, OnCopyCopyhexvalues)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_FONTCHANGE()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_HEX_MOVE_OFFSET, OnOffsetMove)
	ON_MESSAGE(WM_HEX_DATA_UPDATE, UpdateData) 
END_MESSAGE_MAP()


// CDiskHexCtrl message handlers
BOOL CDiskHexCtrl::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	return CWnd::Create(DISKHEXCTRL_CLASSNAME, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CDiskHexCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_tMetricDetails.rSector.PtInRect(point))
	{
		m_tMetricDetails.bSectorHex = (!m_tMetricDetails.bSectorHex);
		AfxGetApp()->WriteProfileInt(
						_T("Settings\\Hex Editor\\General"), 
						_T("Hex Offsets"),
						m_tMetricDetails.bSectorHex);

		InvalidateRect(m_tMetricDetails.rSector, true);
		InvalidateRect(m_tMetricDetails.rHexH, true);
	}

	
	CWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CDiskHexCtrl::OnEraseBkgnd(CDC* pDC)
{
	DrawLayout(pDC);

	return 1;
    
	//return CWnd::OnEraseBkgnd(pDC);
}
void CDiskHexCtrl::OnPaint()
{
   PAINTSTRUCT ps;
   CDC* cPaintDC = BeginPaint(&ps);
      
   DestroyCaret();
   
   // Check we have data to display before rushing in to painting
   if(m_ptrData == NULL)
	   return;
   
   if(m_Highlighted)
   {
		CBrush cbrush ;
		cbrush.CreateSolidBrush(m_hlightbkg);
		CBrush* oldbrush = cPaintDC->SelectObject(&cbrush);

		// draw with a thick blue pen
		CPen penBlue(PS_SOLID, 1, m_hlightbkg);
		CPen* pOldPen = cPaintDC->SelectObject(&penBlue);

		//HighlightSelected();
		cPaintDC->Polygon(m_ptV, m_iVertices);

		cPaintDC->SelectObject(&oldbrush);
		cPaintDC->SelectObject(&pOldPen);
   }

    // Show The Hex Data
   cPaintDC->SetBkMode(TRANSPARENT);

   OnEraseBkgnd(cPaintDC);

   if(m_nDrawHeaders)	  // if users wants headers
		PaintHeaders(cPaintDC);

   PaintHex(cPaintDC);

   if(m_nPaintOffset)
	   PaintOffset(cPaintDC);

   if(m_nPaintAscii)
	     PaintAsc(cPaintDC);

  
   if(GetFocus() == FromHandle(this->GetSafeHwnd()) )
   {
		CreateSolidCaretLocal();
		SetEditCaretPos(CPoint(m_tMetricDetails.pLastCaretPoint.x,m_tMetricDetails.pLastCaretPoint.y) );
   }
  
   EndPaint(&ps);

   ReleaseDC(cPaintDC);

 
   // Do not call CWnd::OnPaint() for painting messages
}


// Need To Register This Class With Windows.
void CDiskHexCtrl::RegisterClass()
{

	WNDCLASS wndcls;

	memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL Defaults

	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW ;
    wndcls.lpfnWndProc = ::DefWindowProc; 
    wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.lpszClassName = DISKHEXCTRL_CLASSNAME;
    wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndcls.hbrBackground = NULL; //(HBRUSH) (COLOR_WINDOW + 1);
	wndcls.hIcon = NULL;

	// Register the new class and exit if it fails
    if(!AfxRegisterClass(&wndcls))
    {
       TRACE("Class Registration Failed\n");
       return;
    }
}

// Create The Screen Layout
void CDiskHexCtrl::DrawLayout(CDC* pDC)
{
	 CBrush cbrush;
	//CBrush cBack;

	 UnrealizeObject(cbrush);

	// select pen 
	CPen cPen(PS_SOLID, 1, m_sepclr);			
	CPen* pOldPen = pDC->SelectObject(&cPen);		// Select New Pen Type

	// select background brush
    cbrush.CreateSolidBrush(m_bkgclr);
	CBrush* oldbrush = pDC->SelectObject(&cbrush);
    	
	// Get The Client Rect Area We Have To Work With And Paint 
	CRect rBtm;
	GetClientRect(rBtm);
	pDC->Rectangle(rBtm);

	
#ifdef __DEBUG__PAINT
	pDC->Rectangle(m_tMetricDetails.rHexH);	
	pDC->Rectangle(m_tMetricDetails.rHex);

	if(m_nPaintAscii)
	{
		pDC->Rectangle(m_tMetricDetails.rAsc);
		pDC->Rectangle(m_tMetricDetails.rAscH);
	}

	if(m_nPaintOffset)
	{
		pDC->Rectangle(m_tMetricDetails.rSectorH);
		pDC->Rectangle(m_tMetricDetails.rSector);
	}
#endif

    // Restore Old DC Values 
	pDC->SelectObject(&pOldPen);
	pDC->SetBkColor(RGB(255,255,255));
}


void CDiskHexCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	
	POINT point;

	SetFocus();

	switch(nChar) {

	case VK_NEXT:
		::SendMessage(this->GetParent()->GetSafeHwnd(), WM_HEX_DATA_NEXT, 0, 0);
		break;


	case VK_PRIOR:
		::SendMessage(this->GetParent()->GetSafeHwnd(), WM_HEX_DATA_PREV, 0, 0);
		break;

	case VK_ESCAPE:
		break;

	case VK_DOWN:
		point.x = m_tMetricDetails.pLastCaretPoint.x;
		point.y = m_tMetricDetails.pLastCaretPoint.y + (m_tMetricDetails.nCharHeightTtl+1) ;
		m_tMetricDetails.sGridPos.cy += 1;			// Vertical
		if(!SetEditCaretPos(point))
		{
			m_tMetricDetails.sGridPos.cy -= 1;			// Vertical
		}
		
		
		break;

	case VK_UP:
		point.x = m_tMetricDetails.pLastCaretPoint.x;
		point.y = m_tMetricDetails.pLastCaretPoint.y - (m_tMetricDetails.nCharHeightTtl-1) ;
		m_tMetricDetails.sGridPos.cy -= 1;			// Vertical
		if(!SetEditCaretPos(point))
		{
			// Set The Vitual Grid Position
			m_tMetricDetails.sGridPos.cy += 1;			// Vertical
		}
		break;

	case VK_RIGHT:
		point.x = m_tMetricDetails.pLastCaretPoint.x + (m_tMetricDetails.nCharWidth+2);
		point.y = m_tMetricDetails.pLastCaretPoint.y ;
		m_tMetricDetails.sGridPos.cx += 1;			// Horizon
		m_bMoveForward = true;
		if(!SetEditCaretPos(point))
		{	// Set The Vitual Grid Position
			m_tMetricDetails.sGridPos.cx -= 1;			// Horizon
		}
		break;

	case VK_LEFT:
		point.x = m_tMetricDetails.pLastCaretPoint.x - (m_tMetricDetails.nCharWidth-2);
		point.y = m_tMetricDetails.pLastCaretPoint.y ;
		m_tMetricDetails.sGridPos.cx -= 1;			// Horizon
		m_bMoveForward = false;
		if(!SetEditCaretPos(point))
		{	// Set The Vitual Grid Position
			m_tMetricDetails.sGridPos.cx += 1;			// Horizon
		}
		break;
	default:
		//GetParent()->OnKeyDown(nChar, nRepCnt, nFlags);			// Pass Other Keystrokes to parent
		//SendMessage(WM_KEYDOWN, (WPARAM)GetParent(), nChar & nRepCnt & nFlags);
		return;
		break;
	}
	
		
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


// This function is called by UpdataData after receiving WM
//
void CDiskHexCtrl::SetDataPointer(BYTE* pData, DWORD iLen, __int64 iOffset)
{

	// Actual Data
	if(m_pData == NULL)
		return;

	ASSERT(pData != NULL);
	m_pData = pData;
	// Copy Pointer For Data Access
	m_ptrData = m_pData;
	UpdateDataPointer(0, true);

	ASSERT(iLen > 0);
	this->m_iSize = (int)iLen;
	m_iOffset = iOffset;
	m_Highlighted = false;
	CalcMetricDetails();
	Invalidate(false);
	return;
}

void CDiskHexCtrl::PaintHex(CDC* pDC)
{
	int iBeginHigh=0, iEndHigh=0;
	int iTop, iLeft;
	CString strHex;

	// Select Drawing Font and Color
	CFont* dcOldFont = pDC->SelectObject(&m_hexfont);
	pDC->SetTextColor(m_hexclr);		
	pDC->SetBkMode(TRANSPARENT);

	// starting point
	iTop = m_tMetricDetails.rHex.top; 
	iLeft = m_tMetricDetails.rHex.left;
	//iLeft -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);
   
	// highlight calculation 
	iBeginHigh = (int)(GetGridPositionEx(m_BeginHighSelected.x+1)/2)  + ( 16 * (m_BeginHighSelected.y-1 ));
	iEndHigh =   (int)(GetGridPositionEx(m_EndHighSelected.x+1)/2)  + ( 16 * (m_EndHighSelected.y-1 ));
   	
	int i = 1 + (((int)m_tMetricDetails.nVScrollStart-1) * (int)m_tMetricDetails.nCharPerLine);
	for(i;i<(int)m_iSize+1;i++)
	{
		strHex.Empty();

		strHex.AppendFormat("%02X", m_pData[i-1]);
	
		if(m_Highlighted)
		{   					
		    if((i >= min(iBeginHigh, iEndHigh)) && (i <= max(iEndHigh, iBeginHigh)))
				pDC->SetTextColor(m_hlighttxt);
			else
				pDC->SetTextColor(m_hexclr);
		}
		else
			pDC->SetTextColor(m_hexclr);


		pDC->TextOut(iLeft, iTop, strHex.Trim());
	
		// Now Determine Where We Should Put The Next Char
		iLeft += (m_tMetricDetails.nCharWidth * 2);		// 2 because hex has 2 chars per byte
		iLeft += (m_tMetricDetails.nCharWidth * m_tMetricDetails.nCharSpace);		// now jump the space we leave

		if( ((i%16) == 0) & (i > 0) )
		{ 	
			iLeft = this->m_tMetricDetails.rHex.left; 		// Reset The Line
		//	iLeft -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);

			iTop +=  m_tMetricDetails.nCharHeightTtl;						// Next Line
		}
	}

	pDC->SelectObject(&dcOldFont);

   return;

}


void CDiskHexCtrl::PaintOffset(CDC* pDC)
{
   // Select Drawing Font and Color
	CFont* dcOldFont = pDC->SelectObject(&m_offsetfont);
	pDC->SetTextColor(m_offsetclr);

	
	// start position
	int iTop = m_tMetricDetails.rSector.top;  

	int iLeft = m_tMetricDetails.rSector.left + m_tMetricDetails.nCharWidth;
	//iLeft -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);

	__int64 iOffset = ((m_iOffset-1)*(m_tMetricDetails.nRowsPerPage*m_tMetricDetails.nCharPerLine));
	iOffset += ((m_tMetricDetails.nVScrollStart-1) * m_tMetricDetails.nCharPerLine);


	CString strOffset;			 

	int iLoop = 0;
	iLoop = ( ((m_iSize%m_tMetricDetails.nCharPerLine) != 0) ? 1 : 0) ;
    iLoop +=   abs(m_iSize/m_tMetricDetails.nCharPerLine);
	for(int i=0;i<iLoop;i++)
	{
		strOffset.Empty();
		if(m_tMetricDetails.bSectorHex)	// show hex format
			strOffset.AppendFormat("00000000000%X", iOffset);
		else							// show dec
			strOffset.AppendFormat("00000000000%d", iOffset);

		strOffset = strOffset.Right(8);
		pDC->TextOut(iLeft, iTop, strOffset, strOffset.GetLength() );
		iTop += m_tMetricDetails.nCharHeightTtl;
		iOffset += 16;
    }
    
	pDC->SelectObject(dcOldFont);

 return;
}


void CDiskHexCtrl::PaintAsc(CDC* pDC)
{
	int iTop, iLeft;
	CString strAsc;

	// Select Drawing Font and Color
	CFont* dcOldFont = pDC->SelectObject(&m_ascfont);
	pDC->SetTextColor(m_ascclr);
	
	// Get the starting point for data
	iTop = m_tMetricDetails.rAsc.top; 
	iLeft = this->m_tMetricDetails.rAsc.left + m_tMetricDetails.nCharWidth ;
		
	// Append and Create String to put displayed on screen 
	LPCSTR lpcstr = "00";
	int i = 1 +  ((m_tMetricDetails.nVScrollStart-1) * m_tMetricDetails.nCharPerLine);
	for(i;i<(int)m_iSize;i++)
	{
		strAsc.Empty();
		if(m_pData[i-1] < 30)		// non display char
			strAsc = ".";
		else
			strAsc.AppendFormat("%00c",m_pData[i-1]);

		pDC->TextOut(iLeft, iTop, strAsc.Trim());
        		
		iLeft += m_tMetricDetails.nCharWidth;		// We Should Now Be At The Edge Of The Last Char

		if( ((i%16) == 0) & (i > 0) )
		{ 	iLeft = m_tMetricDetails.rAsc.left + m_tMetricDetails.nCharWidth ;		// Reset The Line
			iTop +=  m_tMetricDetails.nCharHeightTtl;								// Next Line
		}
	}

   SetAscCaretPos();
   pDC->SelectObject(&dcOldFont);

   return;
}

BOOL CDiskHexCtrl::SetEditCaretPos(POINT point)
{

	// Default 
	if(point.x < 0 || point.y < 0)
	{	
		
		// Set The Vitual Grid Position for the first displayed char
		CRect rect;
		GetClientRect(&rect);
		if(m_tMetricDetails.rHex.left < rect.left)		// we have scrolled hex rect off screen to left
		{
			// use the client rect
			CPoint gp = CalculateGridPosition(CPoint(rect.left, point.y), true);	  // set to absolute left, current vert (y) pos
			CPoint pp = CalculatePointPosition(gp);
			m_tMetricDetails.sGridPos.cx = gp.x;			// Horizon
			m_tMetricDetails.sGridPos.cy = gp.y;			// Vertical
			m_tMetricDetails.pLastCaretPoint.x = pp.x; 
			m_tMetricDetails.pLastCaretPoint.y = pp.y;
		}
		else	// default to 0,0
		{
			CPoint pt = CalculatePointPosition(CPoint(1,1));
			m_tMetricDetails.pLastCaretPoint.x = pt.x; 
			m_tMetricDetails.pLastCaretPoint.y = pt.y;
			m_tMetricDetails.sGridPos.cx = 1;			// Horizon
			m_tMetricDetails.sGridPos.cy = 1;			// Vertical
			SetAscCaretPos(1);				// 1 = Start Of Sector
		}

        		
		CreateSolidCaretLocal();
		SetCaretPosLocal(m_tMetricDetails.pLastCaretPoint);
		ShowCaret();
		
		return false;
	}
	
		
	// Check to see if we are attemting to go beyond a line etc and 
	// change point to reflect this.
	point = this->CheckCaretPosSpecial(point);
    if(!(m_tMetricDetails.rHex.PtInRect(point)))
		return false;
	else
	{	
		// Now We need to make sure that only the Hex Chars can be selected
		//int iFirstPos = m_tMetricDetails.rHex.left; // First Hex Char Position on vertical
		//int iCharWidth;
		//if(iFirstPos < 0)
		//	iCharWidth = -m_tMetricDetails.nCharWidth;
		//else
		//	iCharWidth = m_tMetricDetails.nCharWidth;

		//int iCount =  1;
		//// Find The Position The User Asked For
		//do
		//{
		//	iFirstPos +=  iCharWidth; //m_tMetricDetails.nCharWidth;		// Move One Char Across
		//	iCount++;
		//				
		//}while(point.x >= (iFirstPos +  iCharWidth));		 //m_tMetricDetails.nCharWidth

		CPoint ptGrid = CalculateGridPosition(point, false);
		int iCount = ptGrid.x;

		// Find Out File Position
		int iFilePos=0;
		for(int j=0;j<iCount;j++)
		{
			if(tabHexValid[j] == '1' && tabHexValid[j+1] == '1')		// Vaild Char (ie not space)
				iFilePos++;
		}
		
		if(tabHexValid[iCount+1] == '0')   // Must Be A Char So Allow the move
		{
			
			// We need to recalc or do a sanity check here in case font / size has changed
			CPoint ptAdjust = CalculateGridPosition(point, false);
			point = CalculatePointPosition(ptAdjust);

			// Find the width of the last char we were on and move back to the 
			// start of that char and move forward that char width only.
			m_tMetricDetails.sGridPos.cx;			// Horizon
			m_tMetricDetails.sGridPos.cy;			// Vertical

			m_tMetricDetails.pLastCaretPoint.x = point.x;
			m_tMetricDetails.pLastCaretPoint.y = point.y;

            SetCaretPosLocal(point); 
			ShowCaret();
			POINT pt;
			pt = CalculateGridPosition(point, true);
			SetAscCaretPos(iFilePos);
			ShowCaret();
			UpdateDataPointer(iFilePos, true);
		}
		else	// Its A Space So Jump To The Next Char
		{	
			if(point.x > m_tMetricDetails.pLastCaretPoint.x)	// MOving Right to Left
				point.x += (m_tMetricDetails.nCharWidth * 2);
			else
				point.x -= (m_tMetricDetails.nCharWidth * 2);
			
			SetEditCaretPos(point);
			SetFocus();
			return true;
		}
	}

 return true;
}


// Updates The Temporary Pointer To The Data Passed To The CTRL

void CDiskHexCtrl::UpdateDataPointer(int iBytes, bool bForward)
{
	m_ptrData = m_pData;
	if(iBytes > 0)
	{
		m_ptrData = m_ptrData + (iBytes-1) + ( 16 * (m_tMetricDetails.sGridPos.cy-1));

	}

	
	
}



void CDiskHexCtrl::SetAscCaretPos(int iByteCount)
{
	CString strAsc;
	CDC* dc = this->GetDC();
	dc->SelectObject(m_ascfont);
	
	// Start Positions
	if(iByteCount < 0)
		iByteCount = m_tMetricDetails.nByteCount;		// restore
	else
		m_tMetricDetails.nByteCount = iByteCount;		// Save
	
	int iy = m_tMetricDetails.pLastCaretPoint.y; //m_tMetricDetails.nCharHeightTtl * (1+ m_tMetricDetails.sGridPos.cy);
	int ix = m_tMetricDetails.rAsc.left + (m_tMetricDetails.nCharWidth * iByteCount);
	
	if(iByteCount > 0)
		iByteCount--;

	iByteCount += ((m_tMetricDetails.nVScrollStart-1) * m_tMetricDetails.nCharPerLine);

	strAsc.Empty();
	if(m_pData[(iByteCount) + ( 16 * (m_tMetricDetails.sGridPos.cy-1))] < 30)
		strAsc = ".";
	else
        strAsc.AppendFormat("%00c",m_pData[(iByteCount) + ( 16 * (m_tMetricDetails.sGridPos.cy-1))]);

		
	int iPrev = dc->SetROP2(R2_NOT)	;		
	// Draw Asc Caret
	if(m_tMetricDetails.rAsc.PtInRect(CPoint(ix, iy))  )
		dc->TextOut(ix, iy, strAsc.Trim());	

	dc->SetROP2(iPrev);
		
	
	// Now Change Last Selected Back To Normal
	if(m_tMetricDetails.nLastFP == (iByteCount) + ( 16 * (m_tMetricDetails.sGridPos.cy-1)) )
	{
		// Save Current Details For Next Time
		m_tMetricDetails.nLastFP = (iByteCount) + ( 16 * (m_tMetricDetails.sGridPos.cy-1));
		m_tMetricDetails.sLastAscPos.cy = iy;				// Last y pos in ASC
		m_tMetricDetails.sLastAscPos.cx = ix;				// Last x pos in ASC
	}// Do Nothing
	else
	{
		dc->SetBkColor(m_bkgclr);
		dc->SetTextColor(m_ascclr);

		strAsc.Empty();
		if(m_pData[m_tMetricDetails.nLastFP] < 30)
			strAsc = ".";
		else
            strAsc.AppendFormat("%00c",m_pData[m_tMetricDetails.nLastFP]);

		dc->TextOut(m_tMetricDetails.sLastAscPos.cx, m_tMetricDetails.sLastAscPos.cy, strAsc.Trim());	// Temp Pointer To Data

		// Save Current Details For Next Time
		m_tMetricDetails.nLastFP = (iByteCount) + ( 16 * (m_tMetricDetails.sGridPos.cy-1));
		m_tMetricDetails.sLastAscPos.cy = iy;				// Last y pos in ASC
		m_tMetricDetails.sLastAscPos.cx = ix;				// Last x pos in ASC
	}

	m_tMetricDetails.nLastByteCount = iByteCount;
}

void CDiskHexCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if((m_Highlighted) && (m_tMetricDetails.rHex.PtInRect(point)) )
	{   		
		m_Highlighted = false;					// dont try to paint selection if we have no selection !!
		//InvalidateRect(m_lastrecthigh, true);	// clean and repaint old selection area 
		InvalidateRect(m_tMetricDetails.rHex);
	}

	CPoint cp, pp;
	if(m_tMetricDetails.rHex.PtInRect(point))
	{
		cp = CalculateGridPosition(point, true);
		pp = CalculatePointPosition(cp);
		SetFocus();
		SetEditCaretPos(pp);
		m_AreWeDragging = true;
		m_BeginHighSelected = cp; 
		m_EndHighSelected = CPoint(0,0);
		SetCapture();
	}

	
	CWnd::OnLButtonDown(nFlags, point);
}


void CDiskHexCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_AreWeDragging)
	{
		CPoint cp;
		cp = CalculateGridPosition(point, false);
		if(cp == m_BeginHighSelected)				// not moved anywhere !!
		{
			//if(m_Highlighted)		
				//InvalidateRect(m_tMetricDetails.rHex, true);
				//InvalidateRect(m_lastrecthigh, true);
				
			m_Highlighted = false;
		}

//		m_AreWeDragging = false;
		//if(m_Highlighted)								     // removed all this cos we dont need to invalidate if we get the
			//	InvalidateRect(m_tMetricDetails.rHex, true); // selection highlight correct when dragging, saves flashing...
				//InvalidateRect(m_lastrecthigh, true);

		
		
	}
	
	ReleaseCapture();
	m_AreWeDragging = false;
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CDiskHexCtrl::OnMouseMove(UINT nFlags, CPoint point)
{	  	
	CPoint cp, pp;
	
	if(m_tMetricDetails.rHex.PtInRect(point))
        SetCursor(::LoadCursor(NULL, IDC_IBEAM)) ;
	else
		SetCursor(::LoadCursor(NULL, IDC_ARROW) );


	if(m_AreWeDragging)
	{
		cp = CalculateGridPosition(point, false);
		pp = CalculatePointPosition(cp);
		
		m_EndHighSelected = cp; 
    	CRect rectA = m_lastrecthigh;
	    HighlightSelected();
		if((rectA == m_lastrecthigh) && (cp.x == m_EndHighSelected.x))
		{	
			// do nothing
		}
		else
		{
			InvalidateRect(CalcSmallRect(rectA,m_lastrecthigh), false);
			SetEditCaretPos(pp);
			TRACE0("\nIn Calc");
		}
			
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CDiskHexCtrl::HighlightSelected()
{
	CBrush cbrush;
	CRect rHigh;
	CPoint pt, pt2;
	POINT b,e;
	//POINT m_ptV[MAX_VERTICES]; 
	POINT l_BeginHighSelected, l_EndHighSelected; 
	//UINT iVertices = 0;

	m_Highlighted = true;

	CDC* pDC = GetDC();
    	
	cbrush.CreateSolidBrush(m_hlightbkg);
	CBrush* oldbrush = pDC->SelectObject(&cbrush);

	// draw with a thick blue pen
    CPen penBlue(PS_SOLID, 1, m_hlightbkg);
    CPen* pOldPen = pDC->SelectObject(&penBlue);

	// if we are going down start at col 1
	// if we are going up start at col 2
	if(m_BeginHighSelected.y > m_EndHighSelected.y)		// selecting up
	{
		if(m_BeginHighSelected.x <= 1)
			  m_BeginHighSelected.x = 2;

		if(m_BeginHighSelected.x >= 62)
			   m_BeginHighSelected.x = 61;

		if(m_EndHighSelected.x >= 63)
			   m_EndHighSelected.x = 62;



		if((GetGridPositionEx(m_BeginHighSelected.x) % 2 != 0) && m_BeginHighSelected.x > 1)
			   m_BeginHighSelected.x++;

		if((GetGridPositionEx(m_EndHighSelected.x) % 2 == 0) && m_EndHighSelected.x > 1)
			   m_EndHighSelected.x--;


    }
	else	// dragging down
	{

		if(m_EndHighSelected.x <= 1)
			  m_EndHighSelected.x = 2;

		if(m_BeginHighSelected.x >= 62)
			   m_BeginHighSelected.x = 61;

		if(m_EndHighSelected.x >= 63)
			   m_EndHighSelected.x = 62;

		if((GetGridPositionEx(m_BeginHighSelected.x) % 2 == 0) && m_BeginHighSelected.x > 1)
			   m_BeginHighSelected.x--;

		if((GetGridPositionEx(m_EndHighSelected.x) % 2 != 0) && m_EndHighSelected.x > 1)
			   m_EndHighSelected.x++;


	}
	
	// dont change the original variables
	l_BeginHighSelected = m_BeginHighSelected;
	l_EndHighSelected  = m_EndHighSelected;

 
	if(l_BeginHighSelected.y > l_EndHighSelected.y)		// were going up so swap values
	{ 
		b.x =	l_BeginHighSelected.x;
		b.y =	l_BeginHighSelected.y;
        e.x =	l_EndHighSelected.x;
		e.y =	l_EndHighSelected.y;

		l_BeginHighSelected.x = e.x;
		l_BeginHighSelected.y = e.y;

		l_EndHighSelected.x = b.x;
		l_EndHighSelected.y = b.y;
		
	}

	// starts in the middle of a line and ends in the middle of a line
	if((l_BeginHighSelected.x != 1 &&	l_EndHighSelected.x != 63) &&
		(l_BeginHighSelected.y != l_EndHighSelected.y))		// different rows !!
		 m_iVertices = 8;

	// starts in the middle of a line and ends in the middle of a line
	if((l_BeginHighSelected.x != 63 &&	l_EndHighSelected.x != 1) &&
		(l_BeginHighSelected.y != l_EndHighSelected.y))		// different rows !!
		 m_iVertices = 8;
 
	// at least one of the start or end points in on the first or last char of a line
	if((l_BeginHighSelected.x == 1) || (l_EndHighSelected.x == 63))
		 m_iVertices = 6;

	if((l_BeginHighSelected.x == 63) || (l_EndHighSelected.x == 1))
		 m_iVertices = 6;
 
	// starts in the middle of a line and ends in the middle of a line
	// and is on the same line - basic rectangle
	if(l_BeginHighSelected.y == l_EndHighSelected.y)		// same row !!
		 m_iVertices = 4;
		 
	// basic rectangle
	if((l_BeginHighSelected.x == 1 && l_EndHighSelected.x == 63)||
		(l_BeginHighSelected.x == 63 && l_EndHighSelected.x == 1))
			m_iVertices = 4;
     		
	// convert from grid pos to client points
	l_BeginHighSelected = CalculatePointPosition(l_BeginHighSelected);
	l_EndHighSelected = CalculatePointPosition(l_EndHighSelected);
	// store get the starting point
	m_ptV[0].x = l_BeginHighSelected.x;
	m_ptV[0].y = l_BeginHighSelected.y;
	

   	
	switch (m_iVertices)
	{
	case 4:
		m_ptV[1].y = l_BeginHighSelected.y;
		m_ptV[1].x = l_EndHighSelected.x;

		m_ptV[2].y = l_EndHighSelected.y + m_tMetricDetails.nCharHeightTtl;
		m_ptV[2].x = l_EndHighSelected.x;
		
		m_ptV[3].y = l_EndHighSelected.y + m_tMetricDetails.nCharHeightTtl;
		m_ptV[3].x = l_BeginHighSelected.x;
		//pDC->Polygon(m_ptV, iVertices);

		m_lastrecthigh.top =  min(l_EndHighSelected.y,l_BeginHighSelected.y) - (m_tMetricDetails.nCharHeightTtl*2);
		m_lastrecthigh.left = min(l_EndHighSelected.x,l_BeginHighSelected.x) - (m_tMetricDetails.nCharWidth*2); 

		m_lastrecthigh.bottom = max(l_EndHighSelected.y,l_BeginHighSelected.y) + (m_tMetricDetails.nCharHeightTtl*2);
		m_lastrecthigh.right = max(l_EndHighSelected.x,l_BeginHighSelected.x) + (m_tMetricDetails.nCharWidth*2); 

		break;

	case 6:	// starts or ends on the start or end of a line

		// Go to the end of the line
		pt = CalculatePointPosition(CPoint(63,1))	 ;
		m_ptV[1].y = l_BeginHighSelected.y;
		m_ptV[1].x = pt.x;						// column
	
		// go down to the bottom right of the full rect
		if(min(m_BeginHighSelected.x, m_EndHighSelected.x) == 1)	// starts at col 1
		{
			// take the end drag figure and minus to get prvious row and end col
			pt =CalculatePointPosition(CPoint(63, max(m_EndHighSelected.y, m_BeginHighSelected.y) - 1));
			m_ptV[2].y = pt.y + m_tMetricDetails.nCharHeightTtl;		// row
			m_ptV[2].x = pt.x;			// column

			m_ptV[3].y = m_ptV[2].y;										// row
			m_ptV[3].x = l_EndHighSelected.x + m_tMetricDetails.nCharWidth;		// col

			////rect

			pt =CalculatePointPosition(CPoint(63, max(m_EndHighSelected.y, m_BeginHighSelected.y)));
			m_ptV[4].y = pt.y + m_tMetricDetails.nCharHeightTtl;	 //row
			m_ptV[4].x = l_EndHighSelected.x + m_tMetricDetails.nCharWidth;		//col

			
			m_ptV[5].y = m_ptV[4].y;			// row
			pt =CalculatePointPosition(CPoint(1,1));
			m_ptV[5].x = pt.x;							// col


			m_lastrecthigh.top =  m_ptV[0].y-2;
			m_lastrecthigh.left = m_ptV[0].x-2;
       		m_lastrecthigh.bottom = m_ptV[4].y+2;
			m_lastrecthigh.right = m_ptV[2].x+2;

		}
		else	   // end at end column where drag stopped
		{	
			m_ptV[2].y = l_EndHighSelected.y + m_tMetricDetails.nCharHeightTtl;		// row
			m_ptV[2].x = l_EndHighSelected.x;											// col

			pt =CalculatePointPosition(CPoint(1, max(m_EndHighSelected.y, m_BeginHighSelected.y)));
			m_ptV[3].y = l_EndHighSelected.y + m_tMetricDetails.nCharHeightTtl;				// row
			m_ptV[3].x = pt.x;																// col
			
			pt =CalculatePointPosition(CPoint(1, min(m_BeginHighSelected.y, m_EndHighSelected.y)+1));
			m_ptV[4].y = l_BeginHighSelected.y + m_tMetricDetails.nCharHeightTtl;
			m_ptV[4].x = pt.x;

			pt =CalculatePointPosition(CPoint(min(m_EndHighSelected.x, m_BeginHighSelected.x)
						, min(m_EndHighSelected.y, m_BeginHighSelected.y)));


			m_ptV[5].y = l_BeginHighSelected.y + m_tMetricDetails.nCharHeightTtl;  // row
			m_ptV[5].x = l_BeginHighSelected.x;			// col
		
			pt = CalculatePointPosition(CPoint(1,1) );
			m_lastrecthigh.top =  l_BeginHighSelected.y - (m_tMetricDetails.nCharHeightTtl*2);
			m_lastrecthigh.left = pt.x;
			m_lastrecthigh.bottom = l_EndHighSelected.y + (m_tMetricDetails.nCharHeightTtl*2);
			pt = CalculatePointPosition(CPoint(62,1) );
			m_lastrecthigh.right = pt.x;
		}

		//pDC->Polygon(m_ptV, iVertices);
		break;

	case 8:

			// Go to the end of the line
			pt = CalculatePointPosition(CPoint(63,1))	 ;
			m_ptV[1].y = l_BeginHighSelected.y;
			m_ptV[1].x = pt.x;					

			// take the end drag figure and minus to get prvious row and end col
			pt =CalculatePointPosition(CPoint(63, max(m_EndHighSelected.y, m_BeginHighSelected.y) - 1));
			m_ptV[2].y = pt.y + m_tMetricDetails.nCharHeightTtl;		// row
			m_ptV[2].x = pt.x;			// column

			m_ptV[3].y = m_ptV[2].y;										// row
			m_ptV[3].x = l_EndHighSelected.x + m_tMetricDetails.nCharWidth;		// col

			pt =CalculatePointPosition(CPoint(63, max(m_EndHighSelected.y, m_BeginHighSelected.y)));
			m_ptV[4].y = pt.y + m_tMetricDetails.nCharHeightTtl;	 //row
			m_ptV[4].x = l_EndHighSelected.x + m_tMetricDetails.nCharWidth;		//col

			
			m_ptV[5].y = m_ptV[4].y;			// row
			pt =CalculatePointPosition(CPoint(1,1));
			m_ptV[5].x = pt.x;							// col

		    pt =CalculatePointPosition(CPoint(1, min(m_EndHighSelected.y, m_BeginHighSelected.y)+1));

			m_ptV[6].y = l_BeginHighSelected.y + m_tMetricDetails.nCharHeightTtl;
			m_ptV[6].x = pt.x;

			pt =CalculatePointPosition(CPoint( max(m_EndHighSelected.x, m_BeginHighSelected.x), 
									min(m_EndHighSelected.y, m_BeginHighSelected.y)));



			m_ptV[7].y = l_BeginHighSelected.y + m_tMetricDetails.nCharHeightTtl;  // row
			m_ptV[7].x = l_BeginHighSelected.x;			// col

			// Calculate Invalid Rect Size
			pt =CalculatePointPosition(CPoint( 1, m_BeginHighSelected.y));			// first col, top row
			pt2 =CalculatePointPosition(CPoint( 63, m_EndHighSelected.y+2));		// lat col, btm row

			m_lastrecthigh.top = min(pt.y,pt2.y) - (m_tMetricDetails.nCharHeightTtl*4); 
			m_lastrecthigh.left = min(pt.x-1, pt2.x+2);
             			
			m_lastrecthigh.bottom = max(pt.y, pt2.y) + (m_tMetricDetails.nCharHeightTtl*2);
			m_lastrecthigh.right = max(pt.x, pt2.x);
		    			

			//pDC->Polygon(m_ptV, iVertices);
		break;

	default:
		m_Highlighted = false;
		break;
	}

	if(m_BeginHighSelected.y > m_EndHighSelected.y)	
	{
	
	CRect tr = m_lastrecthigh;
	m_lastrecthigh.top = (tr.bottom - m_tMetricDetails.nCharHeightTtl);
	m_lastrecthigh.bottom = (tr.top + m_tMetricDetails.nCharHeightTtl)+5;
	m_lastrecthigh.left = tr.left-13;
	m_lastrecthigh.right = tr.right + m_tMetricDetails.nCharWidth+13 ;
	
	}

	pDC->SelectObject(&oldbrush);
    pDC->SelectObject(&pOldPen);

	
	/*
	// Need To Find The First Char Selected In The Char Buffer
	// We could use the vgrid to calculate first pos
	int iByteCount = m_tMetricDetails.sGridPos.cx  + ( 16 * (m_tMetricDetails.sGridPos.cy-1));
	  */
	this->ReleaseDC(pDC);
}


POINT CDiskHexCtrl::CalculateGridPosition(POINT pt, BOOL bSaveStruct)
{    
	// We recieve a pt structure, from this we calculate the actual vgrid position  (this position returned may also be a space)
	// and store it in the metric details struct

	// Do Horizon First 
	int iRelX ;
	//if(m_tMetricDetails.rHex.left >= 0)
        iRelX = pt.x -  m_tMetricDetails.rHex.left;	 			// Get Relative Pos From Left Grid Sep
	//else
	//	iRelX = pt.x +  m_tMetricDetails.rHex.left;	 			// Get Relative Pos From Left Grid Sep

	int ixPos = 1 + ((int)iRelX / m_tMetricDetails.nCharWidth);		// calc Actual vgrid Pos

	// Do Vertical
	int iRelY = pt.y - m_tMetricDetails.rHex.top;				// Get Relative Pos From Top Grid Sep
	int iyPos = 1 + ((int)iRelY / m_tMetricDetails.nCharHeightTtl);		// calc Actual vgrid Pos

	// a fix in case the mouse goes out of the hex area
	if(ixPos == 0) {ixPos = 1;}
	if(ixPos > 63 && ixPos < 1000){ ixPos = 63;}
	if(ixPos > 1000){ ixPos = 1;}
	if(iyPos > 32 && iyPos < 1000){ iyPos = 32;}
	if(iyPos > 1000){ iyPos = 1;}

	if(bSaveStruct)
	{
		m_tMetricDetails.sGridPos.cx = ixPos;
		m_tMetricDetails.sGridPos.cy = iyPos;
	}
	
	POINT pr;
	pr.x = ixPos;
	pr.y = iyPos;

	return(pr);
}


POINT CDiskHexCtrl::CalculatePointPosition(POINT pt)
{

	// We recieve a POINT structure that tells us the grid position in the hex editor
	// We need to return the screen x/y coords 

	// we have the char width & height
	// we will return the upper top corner & left
	
	// Horizon First
	int ihx = (pt.x * m_tMetricDetails.nCharWidth) -  m_tMetricDetails.nCharWidth ;
	int ix = m_tMetricDetails.rHex.left + ihx;

	// Vertical
	int ihy = (pt.y * m_tMetricDetails.nCharHeightTtl) - m_tMetricDetails.nCharHeightTtl;
	int iy = (m_tMetricDetails.rHex.top + ihy); // - (m_tMetricDetails.nCharHeightTtl -1) ;

	CPoint cp;
	cp.x = ix;
	cp.y = iy; 

    return (cp);
}

void CDiskHexCtrl::Paint3DHeaders(void)
{
	// We should have been called with the textmetrics struct filled in
	CRect rect;
	CDC* pDC = GetDC();
	    
	CBrush cbrush;

	cbrush.CreateSolidBrush(m_crMenu);
	CBrush* oldBrush = pDC->SelectObject(&cbrush);

	// Paint Offset Headers
	rect.left = 0;
	rect.right = m_tMetricDetails.rSectorH.Width();
	rect.top = 0;
	rect.bottom = m_tMetricDetails.nHeaderX-4;
    pDC->Rectangle(rect);
	pDC->Draw3dRect(rect, m_crLight, m_crShadow);

	// Paint Hex Headers
	rect.left = m_tMetricDetails.rSectorH.Width()+1;
	rect.right = m_tMetricDetails.rSectorH.Width() + m_tMetricDetails.rHexH.Width();
	rect.top = 0;
	rect.bottom = m_tMetricDetails.nHeaderX-4;
    pDC->Rectangle(rect);
	pDC->Draw3dRect(rect, m_crLight, m_crShadow);

	// Paint Asc Headers
	rect.left = m_tMetricDetails.rSectorH.Width() + m_tMetricDetails.rHexH.Width()+1;
	rect.right = m_tMetricDetails.rAsc.right;
	rect.top = 0;
	rect.bottom = m_tMetricDetails.nHeaderX-4;
    pDC->Rectangle(rect);
    pDC->Draw3dRect(rect, m_crLight, m_crShadow);
	

	// We also want two boxes to the left and bottom of the screen
    GetClientRect(&rect);

	rect.left = m_tMetricDetails.rAsc.right;
	rect.bottom = m_tMetricDetails.rAsc.bottom;
	pDC->Rectangle(rect);
	pDC->Draw3dRect(rect, m_crLight, m_crShadow);

	GetClientRect(&rect);
	rect.top = m_tMetricDetails.rAsc.bottom;
	pDC->Draw3dRect(rect, m_crLight, m_crShadow);
	pDC->Rectangle(rect);
	Paint3DLines(&rect);

	pDC->SelectObject(&oldBrush);
	ReleaseDC(pDC);
	return;
}


void CDiskHexCtrl::Paint3DLines(CRect rt)
{
	CDC* pDC = GetDC();
	
	CPen cpen;
	
	cpen.CreatePen(PS_SOLID | PS_INSIDEFRAME, 1, m_crLight);
	CPen* oPen = pDC->SelectObject(&cpen);

	CPen cpens;
	cpens.CreatePen(PS_SOLID | PS_INSIDEFRAME, 1, RGB(0,0,0) );
	
	// Do Left Side Of Rect
	pDC->MoveTo(rt.left, rt.top);
	pDC->LineTo(rt.left ,rt.bottom );
	pDC->MoveTo(rt.left+1, rt.top);
	pDC->LineTo(rt.left+1 ,rt.bottom );

	// Do Top Side Of Rect (x,y)
	pDC->MoveTo(rt.left, rt.top);
	pDC->LineTo(rt.right ,rt.top);
	pDC->MoveTo(rt.left, rt.top+1);
	pDC->LineTo(rt.right ,rt.top+1);


	// Change Color 
	pDC->SelectObject(cpens);
	// Do Right Side Of Rect
	pDC->MoveTo(rt.right, rt.top);
	pDC->LineTo(rt.right ,rt.bottom);
	// Do BTM Side Of Rect
	pDC->MoveTo(rt.left, rt.bottom);
	pDC->LineTo(rt.right ,rt.bottom);


	CPen cpenx;
	cpenx.CreatePen(PS_SOLID | PS_INSIDEFRAME, 1, m_crShadow );
	pDC->SelectObject(cpens);	

	pDC->MoveTo(rt.right-1, rt.top);
	pDC->LineTo(rt.right-1 ,rt.bottom);

	pDC->SelectObject(cpenx);
	pDC->MoveTo(rt.left, rt.bottom-1);
	pDC->LineTo(rt.right ,rt.bottom-1);

	pDC->SelectObject(oPen);
	ReleaseDC(pDC);

}

BOOL CDiskHexCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (CWnd::OnCommand(wParam, lParam))
		return TRUE;

	return !GetParentFrame()->SendMessage(WM_COMMAND, wParam, lParam);
	//return CWnd::OnCommand(wParam, lParam);
}


BOOL CDiskHexCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	if (CWnd::OnNotify(wParam, lParam, pResult))
		return TRUE;

	// route commands to the splitter to the parent frame window
	*pResult = GetParentFrame()->SendMessage(WM_NOTIFY, wParam, lParam);
	return TRUE;
	//return CWnd::OnNotify(wParam, lParam, pResult);
}

POINT CDiskHexCtrl::CheckCaretPosSpecial(POINT point)
{
	  //return point;
	CPoint pta, pt;

	// Bottom Of Grid Go Top Of Grid -
	//if(((UINT)point.x > (UINT)m_tMetricDetails.rHex.left && (UINT)point.x < (UINT)m_tMetricDetails.rHex.right  )
	//	&& (UINT)point.y == ((UINT)m_tMetricDetails.rHex.bottom - (m_tMetricDetails.nCharHeightTtl *1)))
	//{

	//	pt.x = point.x;
	//	pt.y = point.y;
	//	pta = CalculateGridPosition(pt, false);
	//	pta.y = 1;
	//	point = CalculatePointPosition(pta);
	//	return point;
	//	
	//}

	//// End Of Sector Go Top Of Sector - TODO - GO NEXT SECTOR
	//if(((UINT)point.x >= m_tMetricDetails.rHex.left + (63 * m_tMetricDetails.nCharWidth))
	//	&& (UINT)point.y == ((UINT)m_tMetricDetails.rHex.bottom - (m_tMetricDetails.nCharHeightTtl *2)))
	//{
	//	pt.x = 1;
	//	pt.y = 1;
	//	point = CalculatePointPosition(pt);
	//	return point;
	//	
	//}

	//// Begining Of Sector Go Bottom Of Sector - TODO - GO Previous SECTOR
	//if(((UINT)point.x < m_tMetricDetails.rHex.left + ( m_tMetricDetails.nCharWidth))
	//	&& (UINT)point.y == ((UINT)m_tMetricDetails.rHex.top ))
	//{
	//	pt.x = 62;		// right
	//	pt.y = 32;		//Bottom
	//	point = CalculatePointPosition(pt);
	//	return point;
	//	
	//}


	//// Top Of Sector Go Bottom Of Sector - Same Offset Though
	//if( (UINT)point.x >= m_tMetricDetails.rHex.left // + (1 * m_tMetricDetails.nCharWidth))
	//	&& (UINT)point.y < (UINT) m_tMetricDetails.rHex.top)  //+ m_tMetricDetails.nCharHeightTtl)            
	//{
	//	pt.x = point.x;
	//	pt.y = point.y;
	//	pta = CalculateGridPosition(pt, false);
	//	//pta.x = ;		// right
	//	pta.y = 32;		//Bottom
	//	point = CalculatePointPosition(pta);
	//	return point;
	//	
	//}

												 
	// Begin Of Current Line Go Next Line
	if(point.x < m_tMetricDetails.rHex.left &&
		point.y <= (m_tMetricDetails.nRowsPerPage * m_tMetricDetails.nCharHeightTtl))               //m_tMetricDetails.rHex.bottom - (m_tMetricDetails.nCharHeightTtl *2)) )
	{
		pt.x = point.x;
		pt.y = point.y;
		pta = CalculateGridPosition(pt, false);
		pta.y--;
		pta.x = 62;									// TODO
		point = CalculatePointPosition(pta);
		return point;
	}

	// End Of Current Line Go Next Line
	if( (point.x > m_tMetricDetails.rHex.right) && 
	   (point.y <= ((LONG)m_tMetricDetails.nRowsPerPage * (LONG)m_tMetricDetails.nCharHeightTtl)) )  //(m_tMetricDetails.rHex.bottom - (m_tMetricDetails.nCharHeightTtl *2)) )
	{
		pt.x = point.x;
		pt.y = point.y;
		pta = CalculateGridPosition(pt, false);
		pta.y++;
		pta.x = 1;
		point = CalculatePointPosition(pta);
		return point;
	}

	return point;
}


void CDiskHexCtrl::CalcMetricDetails(void)
{   
	CRect rect;
	TEXTMETRIC	lpMetrics;
	CDC *pDC;
	pDC = GetDC();

	// Select The Font For headers
	CFont* m_oldfont = pDC->SelectObject(&m_headerfont);
    	    
	// Get Text Metrics
	if (!pDC->GetTextMetrics(&lpMetrics))
			ASSERT(false);
	

	// Fill in our struct with details we will need throughout	
	m_tMetricDetails.nCharHeightTtl = (UINT)lpMetrics.tmHeight + (UINT)lpMetrics.tmExternalLeading ;
	m_tMetricDetails.nCharHeight = (UINT)lpMetrics.tmHeight;
	m_tMetricDetails.nCharWidth =  (lpMetrics.tmPitchAndFamily & 1 ? 3 : 2) * lpMetrics.tmAveCharWidth / 2 ;     
	m_tMetricDetails.nCharSpace = 2;	 // we want 2 spaces in between each hex char
	if(m_nDrawHeaders)
		m_tMetricDetails.nHeaderX = (UINT)m_tMetricDetails.nCharHeightTtl * 2;			// Header Bottom
	else
		m_tMetricDetails.nHeaderX = 2;			// Header Bottom

    
	m_tMetricDetails.nRowsPerPage = (m_iSize / m_tMetricDetails.nCharPerLine)  ;

	// Setup The Client Area Defaults
	rect.top = 0;
	rect.left = 0;
	rect.bottom = (m_tMetricDetails.nRowsPerPage * m_tMetricDetails.nCharHeightTtl) + m_tMetricDetails.nHeaderX; 
      
	// calc Sectors header rect
	if(m_nPaintOffset)
	{
		m_tMetricDetails.rSectorH.left = rect.left;
		m_tMetricDetails.rSectorH.left -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);
		m_tMetricDetails.rSectorH.right = (11 * m_tMetricDetails.nCharWidth) - ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth); 
		m_tMetricDetails.rSectorH.bottom = m_tMetricDetails.nHeaderX;
		m_tMetricDetails.rSectorH.top = rect.top;	
		m_tMetricDetails.rHexH.left = m_tMetricDetails.rSectorH.right; 
	}
	else
	{
	  m_tMetricDetails.rSectorH.left = 0;
	  m_tMetricDetails.rSectorH.left = 0;
	  m_tMetricDetails.rSectorH.right = 0;
	  m_tMetricDetails.rSectorH.top = 0;
	  m_tMetricDetails.rSectorH.bottom = 0;
	  m_tMetricDetails.rHexH.left = rect.left;
	  m_tMetricDetails.rHexH.left -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth); 

	}

	// calc hex header rect
	
	m_tMetricDetails.rHexH.right =  m_tMetricDetails.rHexH.left + (62 * m_tMetricDetails.nCharWidth); 
	m_tMetricDetails.rHexH.bottom = m_tMetricDetails.nHeaderX;
	m_tMetricDetails.rHexH.top = 0;
	
	if(m_nPaintAscii)
	{
		// calc Asc header rect
		m_tMetricDetails.rAscH.left = m_tMetricDetails.rHexH.right; 
		m_tMetricDetails.rAscH.right = m_tMetricDetails.rAscH.left + 17 * m_tMetricDetails.nCharWidth; 
		m_tMetricDetails.rAscH.bottom = m_tMetricDetails.nHeaderX;
		m_tMetricDetails.rAscH.top = 0;
	}


	// data rectangles
						 
	// Calc Sector Data Rect   
	
	if(m_nPaintOffset)
	{
		m_tMetricDetails.rSector.left = rect.left;
		m_tMetricDetails.rSector.left -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);
		m_tMetricDetails.rSector.right = (11 * m_tMetricDetails.nCharWidth) - ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth); 
		m_tMetricDetails.rSector.bottom = rect.bottom;		  
		m_tMetricDetails.rSector.top = m_tMetricDetails.nHeaderX;
		
		m_tMetricDetails.rHex.left = m_tMetricDetails.rSector.right; 
	}
	else
	{
	  m_tMetricDetails.rSector.left = 0;
	  m_tMetricDetails.rSector.left = 0;
	  m_tMetricDetails.rSector.right = 0;
	  m_tMetricDetails.rSector.top = 0;
	  m_tMetricDetails.rSector.bottom = 0;

	  m_tMetricDetails.rHex.left = rect.left;
	  m_tMetricDetails.rHex.left -= ((m_tMetricDetails.nHScrollStart-1) * m_tMetricDetails.nCharWidth);

	}
	
	// Calc Hex data rect
  	m_tMetricDetails.rHex.right = m_tMetricDetails.rHex.left + (62 * m_tMetricDetails.nCharWidth);
  	m_tMetricDetails.rHex.bottom = rect.bottom;
	m_tMetricDetails.rHex.top = m_tMetricDetails.nHeaderX;

	if(m_nPaintAscii)
	{
		// Calc ASCII data rect
		m_tMetricDetails.rAsc.left = m_tMetricDetails.rHex.right;
		m_tMetricDetails.rAsc.right = m_tMetricDetails.rAsc.left + 17 * m_tMetricDetails.nCharWidth; 
		m_tMetricDetails.rAsc.bottom = rect.bottom;
		m_tMetricDetails.rAsc.top = m_tMetricDetails.nHeaderX;
	}

	ASSERT(m_tMetricDetails.rHexH.left == m_tMetricDetails.rHex.left);

	
	// calculate the metrics for the scroll bars
	CalcScrollDetails();

	// Reset font back
	pDC->SelectObject(&m_oldfont);
}

int CDiskHexCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnumFontSettings();
	GetRegistrySettings();
		
	
	return 0;
}

// must be passed vgrid positions
// returns char #
int CDiskHexCtrl::GetGridPositionEx(int pt)
{
	// columns only (x) for every xx chars there are xx spaces
	// xxooxxoo(x)xoox xooxxooxxooxxoo 9

	 int j=0;
	for(int i=0;i<pt;i++)
	{
		if(tabHexValid[i] == '1')	  // 1 == char
			j++;
	}
   
	return j;
}
// an offset required is passed to this func which passes back the actual screen grid pos
// xxooxxooxxooxxooxx ooxxooxxoo 13
int CDiskHexCtrl::CalcGridOffset(int pt)
{										//pt = 5

    int j=0;
	int i=0;
	while(j<pt)
	{
		if((tabHexValid[i] == '1') && (tabHexValid[i+1] == '1'))	  // 1 == char
		{
			j++;
		}
		i++;

	}
   
	return i;


}
/*
  Find the smallest rect from iRect and lRect

*/
CRect CDiskHexCtrl::CalcSmallRect(CRect iRect, CRect lRect)
{
   CRect fRect;
   BOOL bBtmChange, bTopChange;
	
   if(lRect.bottom == iRect.bottom)
   {
		// btm has not changed
	   bBtmChange = false;
	   TRACE0("\nBTM NOT Changed");
   }
   else					    
   {
	    lRect.bottom = max(max(iRect.bottom, lRect.bottom), max(iRect.top, lRect.top));   
		TRACE1("\nBTM Changed %i ", lRect.bottom);							  
	    TRACE1("\nBTM Changed %i ", iRect.bottom);	
		lRect.bottom += m_tMetricDetails.nCharHeightTtl;	// Remove Last Line
		bBtmChange = true;
   }						    
   	   
					 					  
   if(lRect.top == iRect.top)			  
   {
	  // top has not changed
	  lRect.top = lRect.bottom  - (lRect.bottom - iRect.bottom);
	  if(lRect.bottom == iRect.bottom)
		lRect.top -= (m_tMetricDetails.nCharHeightTtl+2);
	  else
		lRect.top -= (m_tMetricDetails.nCharHeightTtl*2);

      TRACE0("\nTOP NOT Changed");
	  bTopChange = false;
   }									  				   
   else
   {
	   lRect.top = min( min(iRect.top, lRect.top) , min(iRect.bottom, lRect.bottom));	      
	   TRACE2("\nTOP Changed %i %i", lRect.top, m_lastsmallrect.top);							  
	   TRACE1("\nTOP Changed %i ", iRect.top);	
	   bTopChange = true;
   }
   
  

  

   CPoint cp = CalculatePointPosition(CPoint(63, 1));
   CPoint cp2 = CalculatePointPosition(CPoint(1, 1));


   if((!bTopChange) && (!bBtmChange))		// change only on a single line 
   {									  
	   TRACE0("\nNo Row Change");				 
	   // find out what has changed
	   TRACE2("%i %i", iRect.right, lRect.right);
	   if(iRect.right != lRect.right)		// change on the right of the line
	   {
		   lRect.left = iRect.right;
		   lRect.right = lRect.right;
	   }

   }
   else
   {

	   lRect.right  = max(cp.x, cp2.x); //max(max(iRect.right, lRect.right), max(iRect.left, lRect.left));
  
	   lRect.left   = min(cp.x, cp2.x); //min(min(iRect.left, lRect.left), min(iRect.right, lRect.right));
   }


  
   fRect = lRect;
   return fRect;
    // lRect.bottom = max(min(iRect.bottom, lRect.bottom), min(iRect.top, lRect.top));   
 //  lRect.top = min( min(iRect.top, lRect.top) , min(iRect.bottom, lRect.bottom));	  
}

void CDiskHexCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMenu cmenu;
	CMenu *pPopup, *pSub;
	CPoint pt = point;
	if(cmenu.LoadMenu(IDR_HEXPOPUP))
	{
		pPopup = cmenu.GetSubMenu(0);
		pSub = pPopup->GetSubMenu(0);
		ClientToScreen(&pt);

		pSub->EnableMenuItem(ID_HEXPOPUPMENU_COPY, (!m_Highlighted));
		pSub->EnableMenuItem(ID_COPY_COPYHEXVALUES, (!m_Highlighted));
		pSub->EnableMenuItem(ID_COPY_COPYASCIIVALUES, (!m_Highlighted));

		pPopup->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
	}
	CWnd::OnRButtonUp(nFlags, point);
}



void CDiskHexCtrl::OnHexpopupmenuSelectall()
{
	m_BeginHighSelected = CPoint(1,1); 
	m_EndHighSelected = CPoint(62,32);	
	m_Highlighted = true;
	HighlightSelected();
	InvalidateRect(m_tMetricDetails.rHex, false);
}

void CDiskHexCtrl::OnHexpopupmenuCopy()
{
	BYTE* pData;
	pData = m_pData;
	HANDLE hGlobal;
    LPSTR pGlobal;
	int iBeginHigh, iEndHigh;
   	
	iBeginHigh = (int)(GetGridPositionEx(m_BeginHighSelected.x+1)/2)  + ( 16 * (m_BeginHighSelected.y-1 ));
	iEndHigh =   (int)(GetGridPositionEx(m_EndHighSelected.x+1)/2)  + ( 16 * (m_EndHighSelected.y-1 ));

	iBeginHigh--;

	hGlobal = GlobalAlloc( GHND|GMEM_SHARE, 1024 );
	pGlobal = (LPSTR)GlobalLock(hGlobal);
	CString str;

	for(int i=iBeginHigh;i<iEndHigh;i++)
	{
		if(m_pData[i] != 0x00)
			*pGlobal = m_pData[i];
		else
			*pGlobal = ' ';
		
	
		*pGlobal++;
		*pData++; 
	}

	GlobalUnlock(hGlobal);
	OpenClipboard();
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();
    GlobalFree(hGlobal);


}

void CDiskHexCtrl::OnCopyCopyasciivalues()
{
	// TODO: Add your command handler code here
}

void CDiskHexCtrl::OnCopyCopyhexvalues()
{
	// TODO: Add your command handler code here
}

void CDiskHexCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	DestroyCaret();
}

void CDiskHexCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	CreateSolidCaretLocal();
	SetEditCaretPos(m_tMetricDetails.pLastCaretPoint);
	ShowCaret();
}


void CDiskHexCtrl::GetFontSettings(CFont* pFont, CString strRegKey)
{
	  CString m_strFontName;
	  int	m_iFontSize;
	  BOOL	m_bBold;

	 
	// Now get the cur selections details and process
			m_strFontName = AfxGetApp()->GetProfileString(strRegKey, 
											"Font Name",
											"Courier");
		
			// Save Font Size
			m_iFontSize = AfxGetApp()->GetProfileInt(strRegKey, 
											"Font Size",
											10);

			// Save Font Bold
			m_bBold = AfxGetApp()->GetProfileInt(strRegKey, 
											"Font Bold",
											0);

			pFont->DeleteObject();
			pFont->CreateFont(10,0,0,0, (0 ? 700:400), FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier");

}


void CDiskHexCtrl::OnFontChange()
{
	CWnd::OnFontChange();

	EnumFontSettings();

	GetRegistrySettings();

	CalcMetricDetails();

	Invalidate(true);
}

void CDiskHexCtrl::CreateSolidCaretLocal(void)
{

	if(m_tMetricDetails.bCaretBlock) // create a block
		CreateSolidCaret(m_tMetricDetails.nCharWidth, m_tMetricDetails.nCharHeightTtl);
   else
	   CreateSolidCaret(-2, m_tMetricDetails.nCharHeightTtl);
		//CreateSolidCaret(m_tMetricDetails.nCharWidth, -2);
}
void CDiskHexCtrl::SetCaretPosLocal(CPoint point)
{
	//if(m_tMetricDetails.bCaretBlock)
		SetCaretPos(point);
//	else
//	{
//		SetCaretPos(point);
//	}
}

void CDiskHexCtrl::GetFontColors(DWORD *crColorFore, DWORD *crColorBack, CString strRegKey)
	{    		
	// Get Font ForeGround Color
	*crColorFore = AfxGetApp()->GetProfileInt(strRegKey, 
									"Font ForeGround",
									RGB(255,255,255));

	// Get Font BackGround Color
	*crColorBack = AfxGetApp()->GetProfileInt(strRegKey, 
									"Font BackGround",
									RGB(0,0,0));

	// Get caret type
	m_tMetricDetails.bCaretBlock = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\Header Details"), 
									"Caret Type",
									true);
}

void CDiskHexCtrl::EnumFontSettings(void)
{
	// All font are now the same in the hex editor
	GetFontSettings(&m_headerfont, _T("Settings\\Hex Editor\\Font Details"));
	GetFontSettings(&m_offsetfont, _T("Settings\\Hex Editor\\Font Details"));
	GetFontSettings(&m_hexfont, _T("Settings\\Hex Editor\\Font Details"));
	GetFontSettings(&m_ascfont, _T("Settings\\Hex Editor\\Font Details"));

	// Different colors though !! :-)
	GetFontColors(&m_headerclr	, &m_bkgclr		, _T("Settings\\Hex Editor\\Header Details"));
	GetFontColors(&m_hexclr		, &m_bkgclr		, _T("Settings\\Hex Editor\\Hex Details"));
	GetFontColors(&m_offsetclr	, &m_bkgclr		, _T("Settings\\Hex Editor\\Sector Details"));
	GetFontColors(&m_ascclr		, &m_bkgclr		, _T("Settings\\Hex Editor\\Ascii Details"));
	GetFontColors(&m_hlighttxt	, &m_hlightbkg	, _T("Settings\\Hex Editor\\Highlight Text"));

}

void CDiskHexCtrl::CalcScrollDetails()
{  
	
    SCROLLINFO si;

	CRect rect;
	GetClientRect(&rect);

	// set up for vertical	
	si.fMask = SIF_RANGE | SIF_PAGE;	  
	si.nMin = 1;

	si.nMax = m_tMetricDetails.nRowsPerPage;
	si.nPage = ( ( rect.bottom - max(rect.top,  m_tMetricDetails.rHex.top) ) 
				/ m_tMetricDetails.nCharHeightTtl) ;
	si.cbSize = sizeof(si);
	SetScrollInfo(SB_VERT, &si, true);


	// readjust for horizon
	si.nMax = (m_tMetricDetails.rSector.Width() + 
			   m_tMetricDetails.rHex.Width() +
			   m_tMetricDetails.rAsc.Width()) / m_tMetricDetails.nCharWidth;

	//si.nMax = (m_tMetricDetails.rAsc.right + m_tMetricDetails.rAsc.left) / m_tMetricDetails.nCharWidth; 
	si.nPage = ((rect.right - rect.left)	/ m_tMetricDetails.nCharWidth ) ;
	si.cbSize = sizeof(si);
	SetScrollInfo(SB_HORZ, &si, true);	

	return;
}
void CDiskHexCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;
	GetScrollInfo(SB_VERT, &si);
	int iOrgPos = si.nPos;

	 si.fMask = SIF_POS;
	 si.cbSize = sizeof(si);

	 switch(nSBCode)
	 {
	 case SB_LINEDOWN :
		 si.nPos += 1;
		 SetScrollInfo(SB_VERT, &si, true);
		 GetScrollInfo(SB_VERT, &si);
		 if(si.nPos != iOrgPos)		// new pos != old pos
		 {
			  int i = -(int)m_tMetricDetails.nCharHeightTtl;
              ScrollWindowEx(0, i, NULL, NULL, NULL, NULL, 0);
			  m_tMetricDetails.nVScrollStart = si.nPos;
			  InvalidateRect(m_tMetricDetails.rHex, false);
			  InvalidateRect(m_tMetricDetails.rSector, false);
			  InvalidateRect(m_tMetricDetails.rAsc, false);
		 }
		 break;

	 case SB_LINEUP:
		 si.nPos -= 1;
		 SetScrollInfo(SB_VERT, &si, true);
		 GetScrollInfo(SB_VERT, &si);
		 if(si.nPos != iOrgPos)		// new pos != old pos
		 {	
             ScrollWindowEx(0, m_tMetricDetails.nCharHeightTtl, NULL, NULL, NULL, NULL, 0);
			 m_tMetricDetails.nVScrollStart = si.nPos;
			 InvalidateRect(m_tMetricDetails.rHex, false);
			 InvalidateRect(m_tMetricDetails.rSector, false);
			 InvalidateRect(m_tMetricDetails.rAsc, false);
		 }
		 break;

		 

	 default:
		 break;
	 }

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDiskHexCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	 SCROLLINFO si;
	GetScrollInfo(SB_HORZ, &si);
	int iOrgPos = si.nPos;

	 si.fMask = SIF_POS;
	 si.cbSize = sizeof(si);

	 switch(nSBCode)
	 {
	 case SB_LINERIGHT :
		 si.nPos += 1;
		 SetScrollInfo(SB_HORZ, &si, true);
		 GetScrollInfo(SB_HORZ, &si);
		 if(si.nPos != iOrgPos)		// new pos != old pos
		 {
			 int i = -(int)m_tMetricDetails.nCharWidth;
             ScrollWindowEx(i, 0, NULL, NULL, NULL, NULL, 0);
			 m_tMetricDetails.nHScrollStart = si.nPos;
			 CalcMetricDetails();
			 InvalidateRect(m_tMetricDetails.rHex, false);
			 InvalidateRect(m_tMetricDetails.rSector, false);
			 InvalidateRect(m_tMetricDetails.rAsc, false);
			 InvalidateRect(m_tMetricDetails.rSectorH + m_tMetricDetails.rHexH + m_tMetricDetails.rAscH);
		 }
		 break;

	 case SB_LINELEFT:
		 si.nPos -= 1;
		 SetScrollInfo(SB_HORZ, &si, true);
		 GetScrollInfo(SB_HORZ, &si);
		 if(si.nPos != iOrgPos)		// new pos != old pos
		 {	
			 ScrollWindowEx(m_tMetricDetails.nCharWidth, 0, NULL, NULL, NULL, NULL, 0);
			 m_tMetricDetails.nHScrollStart = si.nPos;
			 CalcMetricDetails();
			 InvalidateRect(m_tMetricDetails.rHex, false);
			 InvalidateRect(m_tMetricDetails.rSector, false);
			 InvalidateRect(m_tMetricDetails.rAsc, false);
			 InvalidateRect(m_tMetricDetails.rSectorH + m_tMetricDetails.rHexH + m_tMetricDetails.rAscH);
		 }
		 break;

		 

	 default:
		 break;
	 }


	 

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDiskHexCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_tMetricDetails.nVScrollStart = 1;
	m_tMetricDetails.nHScrollStart = 1;
	CalcMetricDetails();		// Also Calls CalcScrollDetails


}
void CDiskHexCtrl::PaintHeaders(CDC* pDC)
{
	CBrush cBack;
    CFont* m_oldfont = pDC->SelectObject(&m_headerfont);
	CString strHexHeaders;	

	pDC->SetBkMode(TRANSPARENT);						// no need for setbkcolor
	pDC->SetTextColor(m_headerclr);					// user selected header text color

	int iTop = m_tMetricDetails.rHexH.top + m_tMetricDetails.nCharHeightTtl;
	int iLeft = m_tMetricDetails.rHexH.left  ;
	for(int i = 0;i<sizeof(tabHexCharacters);i++)
	{
		strHexHeaders.Empty();
		if(m_tMetricDetails.bSectorHex)
			strHexHeaders.AppendFormat("%c", tabHexCharacters[i]);
		else
			strHexHeaders.AppendFormat("%c", tabAscCharacters[i]);

		pDC->TextOut(iLeft, iTop, strHexHeaders.Trim());
		iLeft += (m_tMetricDetails.nCharWidth * 2);		// 2 because hex has 2 chars per byte
		iLeft += (m_tMetricDetails.nCharWidth * m_tMetricDetails.nCharSpace);		// now jump the space we leave
	}

	pDC->DrawText("Offset", 6, m_tMetricDetails.rSectorH, DT_SINGLELINE |  DT_BOTTOM  | DT_CENTER);
	pDC->DrawText("Ascii", 5, m_tMetricDetails.rAscH, DT_SINGLELINE| DT_BOTTOM | DT_CENTER);
	pDC->SelectObject(&m_oldfont);
	

}

void CDiskHexCtrl::GetRegistrySettings(void)
{
	m_nDrawHeaders = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\General"), 
									_T("Show Headers"),
									true);

	m_nPaintOffset = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\General"), 
									_T("Show Offset"),
									true);


	m_nPaintAscii = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\General"), 
									_T("Show Ascii"),
									true);


	m_tMetricDetails.bSectorHex = AfxGetApp()->GetProfileInt(_T("Settings\\Hex Editor\\General"), 
									_T("Hex Offsets"),
									true);

 
}


// custom window message functions

// WM_HEX_DATA_UPDATE
LRESULT CDiskHexCtrl::UpdateData(WPARAM wParam, LPARAM lParam)
{
   DATA_SET_DATA *data;
   data = (DATA_SET_DATA*)wParam;
   SetDataPointer(data->pData, 
				  data->dwLen,
				  data->iOffset);
  return 0;
}

// WM_HEX_MOVE_OFFSET
LRESULT CDiskHexCtrl::OnOffsetMove(WPARAM wParam, LPARAM lParam)
{
    CPoint cp, pp;
	// Calc Grid Pos
	cp.y = ((int)wParam / m_tMetricDetails.nCharPerLine)+1;
	cp.x = ((int)wParam % m_tMetricDetails.nCharPerLine)+1;
	cp.x = CalcGridOffset(cp.x);
	pp = CalculatePointPosition(cp);
	SetFocus();
	SetEditCaretPos(pp);

   return 0;
}