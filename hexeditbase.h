///////////////////////////////////////////////////////////////////////////
// Definition
//-------------------------------------------------------------------------
// File........................hexeditbase.h
// Version.....................1.1.0.0
// Autor(s)....................Ch. Kuendig / kuendigc@spectraweb.ch
// Operating system(s).........Windows NT/2000/[95?/98?]
// Compiler(s).................MS VC++, SP3, SP5
//-------------------------------------------------------------------------
// CHexEditBase is a Hex-Edit-Control based on MFC and the CWnd class.
// It implements basic behavior to edit/view Data in hexadecimal view 
// (binary). It's been a fast implementation and it's not that carefully
// designed and as a result of that it doesn't support all the features
// one can dream of. Scrolling is a little slow with slow computers because
// basiacally the whole control is redrawn instead of bitblittering parts
// of the existing control.
//
// Features:
// - multi- / singleline (depending on the windows-style)
// - automatical scrolling when multiline
// - horizontal scrolling without scrollbars when singleline
// - show Address (on/off)
// - variable Address-Size 
// - show Binary (on/off)
// - cursor navigation (arrows, home, end, page up/down)
// - copy/paste (ctrl + c / ctrl + v)
// - context menue (copy / paste)
//   --> strings from resources, when defined: IDS_CONTROL_COPY, IDS_CONTROL_PASTE
// - edit (only hex-data, not binary)
// - selection (only hex-data)
// - special highlighting of a section (not selection)
// - show selection always (only multiline mode)
// - set how many bytes per row or let it calculate (automatic)
// - set colours (every colour can be set)
//   - address: text & background
//   - hexdata: 
//	   - normal: text, background
//     - selected & focus: text & background
//     - selected & no focus: text & background
//	   - highlighted section: text, background and frame (border)
//   - bindata: text & background
//   - unused area (window-background)
// - set readonly (enabled/disabled: no colour-difference) 
//   (derive from CHexEditBase and override SetReadonly: change colours there)
//
// Basic Instructions:
// - Use folowing Code in the InitInstance:
//	 AfxOleInit();
//	 CHexEditBase::RegisterClass();	 (when using the CHexEditBase_SC windows-class)
// - Use the view or/and the control (depending on project)
// - Using the view is bloody simple (check the demo-project: HexEditCtrl)
// - Using the control is easy as well (easiest way:)
//   - Edit the dialogresource and insert edit-controls where you want 
//     to see the hex-control later. Set multiline-flag when you want to 
//     use the control as a multiline.
//   - Use the ClassWizard to connect a member-variable (control (NOT value))
//     with the previously inserted edit-control. (Give class CEdit first)
//   - When this is done, use the code editor and replace the CEdit (int the h-file)
//     with CHexEditBase. Don't forget to include "hexeditbase.h" there.
//	 - Go to the InitDialog (or insert it) and use the m_HexEdit (or how ever your
//     member is called) to set data: m_HexEdit.SetData((BYTE*)"dummydata", 9);
//   - Use other members to set other attributes (set bUbdate (usually last parameter for set-methodes)
//     only with the last SetXXXXX-Methode:
//	   (example for use in a view-class derived from CHexEditBaseView)
//	   GetHexEditCtrl().SetAddressSize(4, false);
//	   GetHexEditCtrl().SetShowAddress(true, false);
//     GetHexEditCtrl().SetShowAscii(true, false);
//     GetHexEditCtrl().SetBytesPerRow(16, true, true);
//
// Legal Notices:
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
// 
// How much time I'll put into maintaining this control depends highly
// on the feedback/help/support I get from people using this control.
// People modifiying and improving this component are asked to send me
// the new source-code. 
// Coordination for extending the control would be nice, so we don't
// land up with 20 differnt hex-controls where none is really working.
//
//
// 
// greetings goes to
// - Maarten Hoeben for his great CReportCtrl & legal notices
//-------------------------------------------------------------------------
// dependencies on libraries and frameworks:
//
// Name            | Version | Description
//-----------------+---------+---------------------------------------------
// MFC             | 4.2     | Microsoft Foundation Classes
//-----------------+---------+---------------------------------------------
//                 |         | 
//-----------------+---------+---------------------------------------------
//                 |         | 
//-----------------+---------+---------------------------------------------
//                 |         | 
//-----------------+---------+---------------------------------------------
//
//
///////////////////////////////////////////////////////////////////////////
// history:
//
// date	     | signature | descritpion of modification
//-----------+-----------+-------------------------------------------------
// 11.01.01  | kuendig   | version 0.0.0.1 
//           |           | - first test version
//-----------+-----------+-------------------------------------------------
// 13.01.01  | kuendig   | version 0.0.0.2 
//           |           | - context menue 
//           |           |   use OnExtendContextMenu to extend the 
//           |           |   context menue in a derived class
//           |           | - paste methode
//           |           | - Windows-Class registering
//           |           |   - CHexEditBase: for use with DDX / Edit-Control
//           |           |   - CHexEditBase_SC: when not using DDX
//           |           | - several small changes
//-----------+-----------+-------------------------------------------------
// 19.01.01  | kuendig   | version 0.0.0.3
//           |           | - bug in CreateHighlightingPolygons
//           |           |   (when scrolling highlighting out of window on 
//           |           |   top, sometimes the address got overpainted
//           |           |   by some parts of the highlighting section)
//-----------+-----------+-------------------------------------------------
// 04.02.01  | kuendig   | version 1.0.0.0 (official release)
//           |           | - MakeVisible is now smarter
//           |           | - SetFont, GetFont WM_SETFONT, WM_GETFONT works now
//-----------+-----------+-------------------------------------------------
// 24.05.01  | kuendig   | version 1.1.0.0
//           |           | - Fixed the 16Bit Scrollrange limitation when
//           |           |   thumbtracking (see OnVScroll)
//           |           | - Modified SetFont to only accept fixed-pitched
//           |           |   fonts
//           |           | - Replaced some GetSafeHwnd() with 
//           |           |   ::IsWindow(m_hWnd), since it's rather what's
//           |           |   beeing checked. (Even when GetSafeHwnd worked
//           |           |   in most of the cases)
//           |           | - Call DestroyWnd from the Destructor, to get
//           |           |   rid of the TRACE from "CWnd::~CWnd ..."
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
// --.--.--  |           | 
//-----------+-----------+-------------------------------------------------
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
#ifndef __hexeditbase_h
#define __hexeditbase_h
#if _MSC_VER > 1000
#pragma once
#endif


/////////////////////////////////////////////////////////////////////////////
// defines
/////////////////////////////////////////////////////////////////////////////
#define NOSECTION_VAL				0xffffffff

// notification codes
#define HEN_CHANGE					EN_CHANGE	//the same as the EDIT (CEdit)


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CHexEditBase
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CHexEditBase : public CWnd
{
public:
	CHexEditBase();
	virtual ~CHexEditBase();
	void SetShowAddress(bool bShow, bool bUpdate = true);
	void SetShowAscii(bool bShow, bool bUpdate = true);
	void SetData(const BYTE *pData, UINT nLen, bool bUpdate = true);
	void SetDirectDataPtr(BYTE *pData, UINT nLen, bool bUpdate = true); // won't copy data and won't free memory
	void SetHighlighted(UINT nBegin, UINT nEnd, bool bMakeVisible = true, bool bUpdate = true);
	void SetSelection(UINT nBegin, UINT nEnd, bool bMakeVisible = true, bool bUpdate = true);
	void MakeVisible(UINT nBegin, UINT nEnd, bool bUpdate=true);
	UINT GetData(BYTE *pByte, UINT nLength);
	void SetBytesPerRow(UINT nBytesPerRow, bool bAuto = false, bool bUpdate = true);
	void SetAddressSize(BYTE nAdrSize, bool bUpdate = true);
	void SetAdrCol(COLORREF tAdrBkgCol, COLORREF tAdrTxtCol, bool bUpdate = true);
	void SetAsciiCol(COLORREF tAsciiBkgCol, COLORREF tAsciiTxtCol, bool bUpdate = true);
	void SetHighlightCol(COLORREF tHighlightFrameCol, COLORREF tHighlightBkgCol, COLORREF tHighlightTxtCol, bool bUpdate = true);
	void SetHexCol(COLORREF tHexBkgCol, COLORREF tHexTxtCol, bool bUpdate = true);
	void SetSelectedNoFocusCol(COLORREF tSelectedNoFocusBkgCol, COLORREF tSelectedNoFocusTxtCol, bool bUpdate = true);
	void SetSelectedFocusCol(COLORREF tSelectedFousTxtCol, COLORREF tSelectedFousBkgCol, bool bUpdate = true);
	void SetNotUsedCol(COLORREF tNotUsedBkCol, bool bUpdate = true);
	bool GetSelection(UINT& nBegin, UINT& nEnd) const;
	bool GetHighlighted(UINT& nBegin, UINT& nEnd) const;
	bool IsSelection() const;
	bool IsHighlighted() const;
	UINT GetDataSize() const { return m_nLength; }
	virtual void SetReadonly(bool bReadOnly, bool bUpdate = true);
	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hwndParent, HMENU nIDorHMenu, LPVOID lpParam = NULL);
	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam = NULL);	
	static void RegisterClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
	void SetContextCopyStr(const CString& cStr) { m_cContextCopy = cStr; }
	void SetContextPasteStr(const CString& cStr) { m_cContextPaste = cStr; }

protected:
	struct PAINTINGDETAILS {
		UINT nFullVisibleLines;
		UINT nLastLineHeight;
		UINT nVisibleLines;
		UINT nLineHeight;
		UINT nCharacterWidth;
		UINT nBytesPerRow;
		UINT nHexPos;
		UINT nHexLen;
		UINT nAsciiPos;
		UINT nAsciiLen;
		UINT nAddressPos;
		UINT nAddressLen;
		CRect cPaintingRect;
	};

	bool m_bSelfCleanup;
	bool m_bDeleteData;
	PAINTINGDETAILS m_tPaintDetails;
	BYTE *m_pData;
	UINT m_nBytesPerRow;
	UINT m_nAdrSize;
	UINT m_nHighlightedEnd;
	UINT m_nHighlightedBegin;
	UINT m_nSelectionBegin;
	UINT m_nSelectionEnd;
	UINT m_nCurrentAddress;
	UINT m_nCurCaretHeight; 
	UINT m_nLength;
	UINT m_nScrollPostionY;	
	UINT m_nScrollRangeY;
	UINT m_nScrollPostionX;	
	UINT m_nScrollRangeX;
	UINT m_nCurCaretWidth;
	UINT m_nSelectingBeg;
	UINT m_nSelectingEnd;
	UINT m_nBinDataClipboardFormat;	
	bool m_bRecalc;
	bool m_bHasCaret;
	bool m_bHighBits;
	bool m_bAutoBytesPerRow;
	bool m_bShowAddress;
	bool m_bShowAscii;
	bool m_bAddressIsWide;
	bool m_bShowCategory;
	bool m_bReadOnly;
	COLORREF m_tAdrBkgCol;
	COLORREF m_tAdrTxtCol;
	COLORREF m_tAsciiBkgCol;
	COLORREF m_tAsciiTxtCol;
	COLORREF m_tHighlightBkgCol;
	COLORREF m_tHighlightTxtCol;
	COLORREF m_tHighlightFrameCol;
	COLORREF m_tHexTxtCol;
	COLORREF m_tHexBkgCol;
	COLORREF m_tNotUsedBkCol;
	COLORREF m_tSelectedNoFocusTxtCol;
	COLORREF m_tSelectedNoFocusBkgCol;
	COLORREF m_tSelectedFousTxtCol;
	COLORREF m_tSelectedFousBkgCol;	
	CString m_cContextCopy;
	CString m_cContextPaste;	
	CFont m_cFont;	
	CRect m_cDragRect;
	CPoint m_cMouseRepPoint;
	int m_iMouseRepDelta;
	WORD m_nMouseRepSpeed;
	WORD m_nMouseRepCounter;
	bool m_bIsMouseRepActive;

	// overrideables
	virtual void OnExtendContextMenu(CMenu&) {} // override this to add your own context-menue-items

	void NotifyParent(WORD wNBotifictionCode);
	void CalculatePaintingDetails(CDC& cDC);
	void PaintAddresses(CDC& cDC);
	void PaintHexData(CDC& cDC);
	void PaintAsciiData(CDC& cDC);	
	void CreateEditCaret(UINT nCaretHeight, UINT nCaretWidth);
	void DestoyEditCaret();
	void SetEditCaretPos(UINT nOffset, bool bHighBits);
	bool OnEditInput(WORD nInput);
	void MoveCurrentAddress(int iDeltaAdr, bool bHighBits);
	void SetScrollPositionY(UINT nPosition, bool bUpdate=false);
	void SetScrollPositionX(UINT nPosition, bool bUpdate=false);
	void SetScrollbarRanges();
	void MoveScrollPostionY(int iDelta, bool bUpdate=false);
	void MoveScrollPostionX(int iDelta, bool bUpdate=false);
	void StartMouseRepeat(const CPoint& cPoint, int iDelta, WORD nSpeed);
	void StopMouseRepeat();
	void GetAddressFromPoint(const CPoint& cPt, UINT& nAddress, bool& bHighByte);
	UINT CreateHighlightingPolygons(const CRect& cHexRect, 
		UINT nBegin, UINT nEnd, POINT *pPoints);

	//{{AFX_VIRTUAL(CHexEditBase)
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CHexEditBase)
	afx_msg void OnDestroy(); 
	afx_msg void OnTimer(UINT nTimerID);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint cPoint);
	afx_msg LRESULT OnWMChar(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnWMSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUmSetScrollRange(WPARAM, LPARAM);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd*);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar*);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar*);
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg void OnLButtonDown(UINT, CPoint point);
	afx_msg void OnLButtonDblClk(UINT, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnKeyDown(UINT nChar, UINT, UINT);
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditSelectAll();
	//}}AFX_MSG
	DECLARE_DYNCREATE(CHexEditBase)
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CHexEditBaseView
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CHexEditBaseView : public CView
{
	DECLARE_DYNCREATE(CHexEditBaseView)

public:
	enum { IDC_HEXEDITBASEVIEW_HEXCONTROL = 0x100 };

	CHexEditBaseView();
	virtual ~CHexEditBaseView();
	CHexEditBase& GetHexEditCtrl() { return m_cHexEdit; }

protected:
	CHexEditBase m_cHexEdit;

	//{{AFX_VIRTUAL(CHexEditBaseView)
	virtual void OnDraw(CDC*);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CHexEditBaseView)
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg int OnCreate(LPCREATESTRUCT pCreateStruc);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



#endif
