#pragma once


// CControlWnd
#include "DiskAnalysis.h"
#include "IPDisk.h"

#define CONTROLWNDCLASS	"ControlWnd32"			// registered class name
#define SRB_IDW_CONTROLWND              0xE815  // CDialogBar
#define	CONTROL_MIN_WIDTH	250					// Minimum width of control
#define CONTROL_MIN_HEIGHT	200					// Minimum hright of control
#define CAPTION_HEIGHT	18						// Not currently used
#define CX_GRIPPER  3							
#define CY_GRIPPER  3							
#define CX_BORDER_GRIPPER 2
#define CY_BORDER_GRIPPER 2
#define SRB_CLASS_TITLE	"Resource Explorer"


class CControlWnd : public CControlBar
{
	DECLARE_DYNAMIC(CControlWnd)

public:
	CControlWnd();
	virtual ~CControlWnd();
	LRESULT OnSizeParent(WPARAM wparam, LPARAM lparam);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode );

 	CSize		m_SizeLast;				// Save the last window size TODO - Save Registry !!
	CRect		m_RectFinalClient;		// Client area with the borders and gripper added

	CTreeCtrl	m_wndTree;

	FILE_CLUSTERS*	m_FileClusters;
	


private:		// Tree Control members
	HTREEITEM m_hActiveItem;
	HTREEITEM m_hTreeRoot;				// Root of the tree
	HTREEITEM m_hTreeDeleted;			// Root for deleted files found from last search
	HTREEITEM m_hTreeRemote;

	CImageList*	m_ImageList;			// Tree image list
	CIPDisk*    m_ipdisk;

private:
	CDiskAnalysis*	CDisk;
	void RefreshTreeControl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL CreateEx(DWORD dwStyle, CWnd* pParentWnd, UINT nID);
	void RegisterClass(void);
	void DrawGripper(CDC* pDC, const CRect& rect);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	//BOOL CreateControls(void);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler);
	void EnableDocking(DWORD dwDockStyle);
	CWnd* GetCWnd(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSearchforfileTest();
private:
	int AddTreeFoundFiles(FILE_CLUSTERS* pFile);
	void LoadImageList(void);
	void CalcInsideRect(CRect& rect, BOOL bHorz) const;
public:
	afx_msg void OnRecovertreeRecoverthisfile();
private:
	void DeleteTreeFoundFiles(void);
public:
	afx_msg void OnRecovertreeClearfoundfiles();
	afx_msg void OnRecovertreeViewsectors();
private:
	FILE_CLUSTERS* EnumFileClusters(void);
public:

	afx_msg void OnRecovertreeViewdetails();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnRemotemachineConnect();	
	afx_msg LRESULT OnSocketNotify(WPARAM wparam, LPARAM lparam);
private:
	
public:
	afx_msg void OnRemotemachineGetMBR();
	afx_msg LRESULT CControlWnd::AddRemoteTree(WPARAM wparam, LPARAM lparam);
	afx_msg void OnRemotemachineSendtextmessage();
};


