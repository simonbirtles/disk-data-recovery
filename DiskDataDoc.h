// DiskDataDoc.h : interface of the CDiskDataDoc class
//

#include "diskinfo.h"
#pragma once

class CDiskDataDoc : public COleDocument
{
protected: // create from serialization only
	CDiskDataDoc();
	DECLARE_DYNCREATE(CDiskDataDoc)

// Attributes
public:
	//CDiskInfo*	m_DiskInfo;
	BOOL	m_bShowSelectDialog;
	CString m_strOpenPath;
	
protected:
	HANDLE hDisk;			// File Handle To Open Disk
	BYTE *m_strData;
	UINT m_nSize;
	__int64	 m_iCurrentOffset;
	__int64  m_iOffset;
	DWORD m_dwLastBytesRead;
	DWORD m_dwLastReq;


public:
	BYTE* GetData() { return m_strData; }
	DWORD GetDataSize() const { return m_dwLastBytesRead; }
	__int64 GetCurrentOffset() { return m_iOffset; }

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CDiskDataDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnCloseDocument();
	// Read in data from file starting from iStartOffset of size iReadSize
	bool ReadDiskData(int iReadSize, __int64 iStartOffset,  BOOL bAbsolute = false);
//	afx_msg void OnViewNextsector();
//	afx_msg void OnViewPrevioussector();
	afx_msg void OnViewNextsector();
	afx_msg void OnViewPrevioussector();
private:
	
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnViewGotosector();

};


