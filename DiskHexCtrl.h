#pragma once
/*
  Notes on communication with parent.

  Messages Received Explictly From Parent:
  ========================================
  WM_HEX_OFFSET_MOVE		- User has selected to move to offset (WPARAM) in the current data 'set'
  WM_HEX_DATA_UPDATE		- Data has been updates and a pointer to a DATA_SET_DATA struct in (WPARAM) give new data info

  // standard windows messages
  WM_FONTCHANGE				- Font has been changed, reread registry to get new font settings etc

  Message Sent Explictly To Parent:
  ========================================
  WM_HEX_DATA_NEXT			 - Get the next 'Set' of data, the parent should reply with	 WM_HEX_DATA_UPDATE ('Page Down')
  WM_HEX_DATA_PREV			 - Get previous set of data (ie from a 'Page Up' key command)
  




*/
// custom messages
#define WM_HEX_DATA_UPDATE  0x8100
#define WM_HEX_DATA_NEXT	0x8101
#define WM_HEX_DATA_PREV	0x8102

#define WM_HEX_MOVE_OFFSET	0x8200



	typedef struct DATA{
		BYTE* pData;
		DWORD dwLen;
		__int64 iOffset;
	}DATA_SET_DATA;


// CDiskHexCtrl
class CDiskHexCtrl : public CWnd
{
	DECLARE_DYNAMIC(CDiskHexCtrl)

public:
	CDiskHexCtrl();
	virtual ~CDiskHexCtrl();
	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

protected:
	DECLARE_MESSAGE_MAP()

private:
	// defines
	#define MAX_VERTICES 8					   // for polygon highlights

	// Attributes
	COLORREF	m_headerclr;
	COLORREF	m_offsetclr;
	COLORREF	m_hexclr;
	COLORREF	m_ascclr;
	COLORREF	m_bkgclr;
	COLORREF	m_sepclr;
	COLORREF	m_gray;
	COLORREF	m_ascselectbkg;
	COLORREF	m_ascselecttxt;
	COLORREF	m_hlightbkg;
	COLORREF	m_hlighttxt;
	COLORREF	m_crMenu;					// Face Of Object
	COLORREF	m_crLight;					// Edge Facing Light Source (Top, Right)
	COLORREF	m_crShadow;					// Edges Facing Away From Light (Btm, Left)
   
	CFont		m_headerfont;
	CFont		m_offsetfont;
	CFont		m_hexfont;
	CFont		m_ascfont;

	BYTE*		m_pData;					// File Data Pointer
	BYTE*		m_ptrData;					// This is a pointer to the current data passed
	UINT		m_iSize;					// Length Of Data in m_pData
 __int64		m_iOffset;					// Current Offset (we could calc this but better for the doc to do it)
    	
	POINT		m_BeginHighSaved;			// Last highlight start
	POINT		m_EndHighSaved;				// last highlight end
	POINT		m_BeginHighSelected;		// current highlight start (while dragging)
	POINT		m_EndHighSelected;			// current highlight end (while dragging)
	BOOL		m_AreWeDragging;			// are we currently dragging
	BOOL		m_Highlighted;				// do we have a current highlight to draw
    CRect		m_lastrecthigh;				// last rectangle that was highlighted 
	POINT		m_ptV[MAX_VERTICES];		// used to calculate the highlight polygon
	UINT		m_iVertices;				// used to store the qty of points in the highlight polygon
	CRect		m_lastsmallrect;			// last smallest rect when highlighting

	BOOL		m_nDrawHeaders;				// BOOL ? Draw Headers
	BOOL		m_nPaintOffset;				// BOOL ? Draw Offsets
	BOOL		m_nPaintAscii;				// BOOL ? Draw Ascii
	BOOL		m_bMoveForward;				// used to track cursor movements
	BOOL		m_bHaveCaret;

    // private structures
	struct METRIC_DETAILS {
		UINT  nHeaderX;				// Header Line X Pos For Draw
		UINT  nCharWidth;			// Char Width (AVG)
		UINT  nCharHeight;			// Char Height
		UINT  nCharHeightTtl;		// Total Char Height
		UINT  nLineSpace;			// Additional Line spacing   (in points)
		UINT  nCharSpace;			// Char spacing (# of nCharWidth)ie charspacing = nCharSpace*nCharWidth
		UINT  nCharPerLine;			// Char per line for ASC and HEX (0=Auto)
		UINT  nRowsPerPage;			// Rows per page
		BOOL  bCaretBlock;			// is Caret Type a block ?
		POINT pLastCaretPoint;		// Last (current) Caret Pos x,y
		CRect rSectorH;				// Sector Header Rect
		CRect rHexH;				// Hex Header Rect
		CRect rAscH;				// Asc Header Rect
		CRect rSector;				// Current Window Size For Drawing Sector Details
		CRect rHex;					// Current Window Size For Drawing Hex Details
		CRect rAsc;					// Current Window Size For Drawing ASCII Details
		CSize sGridPos;				// Grid Position Hex Editor Caret
        CSize sLastAscPos;			// Point Position Of The Last ASC Pos Highlight
		UINT  nByteCount;			// saved by SetAscCaretPos = if -1 os passed to func
		BOOL  bSectorHex;			// Show Hex or Decimal in the offset column
		UINT  nLastFP;				// Last Position of the file pointer - used for ASC Disp
		UINT  nLastByteCount;		// Last Byte Count
		UINT  nVScrollStart;		// Has the position of the vertical scroll bar, used for repainting sector,hex,asc data
		UINT  nHScrollStart;		// Has the position of the horz scroll bar, used for repainting sector,hex,asc data
	};
	METRIC_DETAILS	m_tMetricDetails;


	
									 

	// windows messages
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHexpopupmenuSelectall();
	afx_msg void OnHexpopupmenuCopy();
	afx_msg void OnCopyCopyasciivalues();
	afx_msg void OnCopyCopyhexvalues();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFontChange();
	// Custom Messages
	afx_msg LRESULT OnOffsetMove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT UpdateData(WPARAM wParam, LPARAM lParam);


	// custom functions
	static void RegisterClass();
	CRect CalcSmallRect(CRect iRect, CRect lRect) ;
	POINT CheckCaretPosSpecial(POINT point);
	POINT CalculateGridPosition(POINT pt, BOOL bSaveStruct);
	POINT CalculatePointPosition(POINT pt);
	BOOL SetEditCaretPos(POINT point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void SetDataPointer(BYTE* pData, DWORD iLen, __int64 iOffset);
	void DrawLayout(CDC *pDC);
	void PaintHex(CDC *pDC);
	void PaintAsc(CDC *pDC);
	void PaintOffset(CDC *pDC);
	void PaintHeaders(CDC* pDC);
	void CalcScrollDetails();
	void UpdateDataPointer(int iBytes, bool bForward);
	void SetAscCaretPos(int iByteCount = -1);
	void HighlightSelected();
    void Paint3DHeaders(void);
	void Paint3DLines(CRect rt);
    void CalcMetricDetails(void);
	void GetFontSettings(CFont *pFont, CString strRegKey);
	void CreateSolidCaretLocal(void);
	void SetCaretPosLocal(CPoint point);
	void GetFontColors(DWORD *crColorFore, DWORD *crColorBack, CString strRegKey);
	void EnumFontSettings(void);	
	void GetRegistrySettings(void);
	int GetGridPositionEx(int pt);
	int  CalcGridOffset(int pt);
	

};


