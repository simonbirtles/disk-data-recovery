#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDiskSelect dialog

class CDiskSelect : public CDialog
{
	DECLARE_DYNAMIC(CDiskSelect)

public:
	CDiskSelect(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiskSelect();
	char	m_cDisk[26];					// Pointer to disks passed
	int		iDiskSize;					// Size Of Char Array - m_cDisk
	CString	m_strSelected;				// Selected Disk/Logical Drive
	CString	m_fileselect;				// Selected File.
	int	m_iDeviceType;					// 0=Logical,1=Physical,2=File
	CFont m_labelfontI;					// Italic
	CFont m_labelfont;					// Not Italis



// Dialog Data
	enum { IDD = IDD_DISKSELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	CListCtrl m_ctlListDisk;
	CListCtrl m_ctlListPhy;
	CImageList* m_DiskImageList;
	int GetIconIndex(UINT uiDriveType);
public:
	afx_msg void OnNMDblclkDisk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
private:
	void GetPhysicalDrives(void);
	void GetLogicalDisks(void);
public:
	afx_msg void OnNMClickDisk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickDisk2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBtnClickedOpenFile();
private:
	CEdit m_selectedfile;
	CStatic m_labelfile;
	CStatic m_labellogical;
	CStatic m_labelphysical;
public:
	afx_msg void OnStnClickedFile();
private:

public:
	afx_msg void OnBnClickedCancel();
};
