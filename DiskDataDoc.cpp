// DiskDataDoc.cpp : implementation of the CDiskDataDoc class
//

#include "stdafx.h"
#include "DiskData.h"

#include "DiskDataDoc.h"
#include "CntrItem.h"

#include "Ntmsapi.h"
#include "winioctl.h"


#include "DiskHexCtrl.h"				// Specific Control Entry
#include "DiskSelect.h"					// Disk/Drive Select OnNewDocument
#include "DiskAnalysis.h"
#include "GoToSector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



//#define DOC_EMPTY_STR	"this is an empty document... open a  file!"
// CDiskDataDoc

IMPLEMENT_DYNCREATE(CDiskDataDoc, COleDocument)

BEGIN_MESSAGE_MAP(CDiskDataDoc, COleDocument)
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_VIEW_NEXTSECTOR, OnViewNextsector)
	ON_COMMAND(ID_VIEW_PREVIOUSSECTOR, OnViewPrevioussector)
	ON_COMMAND(ID_VIEW_GOTOSECTOR, OnViewGotosector)
END_MESSAGE_MAP()


// CDiskDataDoc construction/destruction

CDiskDataDoc::CDiskDataDoc() : m_strData(NULL), m_nSize(512)
{
	// Use OLE compound files
	EnableCompoundFile();
	m_strData = NULL;
	m_bShowSelectDialog = true;
	m_dwLastBytesRead = 0;
	m_dwLastReq = 0;
	m_iOffset = 0;


}

CDiskDataDoc::~CDiskDataDoc()
{
}

// if OpenDocument() is called with a string path name this is called...!!   (see OnNewDocument() )
BOOL CDiskDataDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	//return TRUE;
	//if (!COleDocument::OnOpenDocument(lpszPathName))
	//	return FALSE;

	//CDiskAnalysis disk;
	//BYTE byData[1024];
	//disk.GetMFTRecord('C', 23437, byData, 1024);

	hDisk = CreateFile(lpszPathName, GENERIC_READ, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							NULL, OPEN_EXISTING, 0,0); 

	if( hDisk == INVALID_HANDLE_VALUE )	
	{	
		AfxMessageBox(IDS_INVALID_HANDLE, MB_ICONSTOP,0);	
		return FALSE; 
	}

	CString str;
	str.AppendFormat("'%s'", lpszPathName);
	this->m_strTitle = lpszPathName;
	m_iCurrentOffset = 0;				// Default As Start Of File Here

	//m_nSize = 2048;
	return ReadDiskData(m_nSize, 0);

    //return TRUE;
}

// if OpenDocument is called with a NULL path name this is called (see OnOpenDocument() )
BOOL CDiskDataDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;
	
	ASSERT(false);		// should never be called in this app at least at the moment !! OnOpenDocument !!
	
	//hDisk = CreateFile(m_strOpenPath, GENERIC_READ, 
	//						FILE_SHARE_READ|FILE_SHARE_WRITE, 
	//						NULL, OPEN_EXISTING, 0,0); 
	//if( hDisk == INVALID_HANDLE_VALUE )	
	//{	
	//	AfxMessageBox(IDS_INVALID_HANDLE, MB_ICONSTOP,0);	
	//	return FALSE; 
	//}
	//m_iCurrentOffset = 0;				// Default As Start Of File Here
	//return ReadDiskData(m_nSize, 0);
	

	return true;
	
}




// CDiskDataDoc serialization

void CDiskDataDoc::Serialize(CArchive& ar)
{
	CFile *pFile = ar.GetFile();
	ASSERT(pFile != NULL);

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
	
	}

	// Calling the base class COleDocument enables serialization
	//  of the container document's COleClientItem objects.
	COleDocument::Serialize(ar);
	// activate the first one
	if (!ar.IsStoring())
	{
		POSITION posItem = GetStartPosition();
		if (posItem != NULL)
		{
			CDocItem* pItem = GetNextItem(posItem);
			POSITION posView = GetFirstViewPosition();
			COleDocObjectItem *pDocObjectItem = DYNAMIC_DOWNCAST(COleDocObjectItem, pItem);
			if (posView != NULL && pDocObjectItem != NULL)
			{
				CView* pView = GetNextView(posView);
				pDocObjectItem->DoVerb(OLEIVERB_SHOW, pView);
			}
		}
	}
}


// CDiskDataDoc diagnostics

#ifdef _DEBUG
void CDiskDataDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CDiskDataDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG


// CDiskDataDoc commands

void CDiskDataDoc::OnCloseDocument()
{
	// Close Handle To Disks

	delete []m_strData;

	BOOL bClosed = CloseHandle(hDisk);
	//if (!bClosed) { AfxMessageBox("Failed To Close Disk Handle", 0,0); }

	COleDocument::OnCloseDocument();
}

// Read in data from file starting from iStartOffset of size iReadSize
bool CDiskDataDoc::ReadDiskData(int iReadSize, __int64 iStartOffset, BOOL bAbsolute)
{
   
	
	DWORD lpdBytes = 0;
	BOOL bSuccess;
	CString strText = "";
	CString strDat = "\n";						// Hex String
	LARGE_INTEGER liMoveTo;
	PLARGE_INTEGER p;
	p = (LARGE_INTEGER*)VirtualAlloc(NULL, 1*sizeof(LARGE_INTEGER), MEM_COMMIT, PAGE_READWRITE);



	// we are trying to go past EOF - not allowed so return quietly :-))
	if((( m_dwLastBytesRead != m_dwLastReq ) )		  // || bAbsolute 
		                && ( iStartOffset >= 0 ) )
		return false;

	// we are trying to go before SOF - not allowed so return quietly 
	p->HighPart = 0;
	p->LowPart = 0;
	p->QuadPart = 0;
	liMoveTo.HighPart = 0;
	liMoveTo.LowPart = 0;
	liMoveTo.QuadPart = 0;
	SetFilePointerEx(hDisk, liMoveTo, p, FILE_CURRENT);
	if( (p->QuadPart + (iStartOffset*iReadSize))  < 0)
		return false;



	// Set The File Pointer To iStartOffset
	liMoveTo.HighPart = 0;
	liMoveTo.LowPart = 0;
	// if we read less than requested last time and we are going back
	// only read what we read last time 
	if(( m_dwLastBytesRead != m_dwLastReq ) && ( iStartOffset < 0 ))
	{
		DWORD dwDiff = (m_dwLastReq - m_dwLastBytesRead);
		liMoveTo.QuadPart = (iStartOffset*iReadSize);  // bytes
		liMoveTo.QuadPart += dwDiff;
	}
	else
	{
		liMoveTo.QuadPart = (iStartOffset*iReadSize);
	}


	delete []m_strData;
    m_strData = new BYTE[iReadSize];
	memset(m_strData, ' ', iReadSize);
    
	
	if( (m_iCurrentOffset*m_nSize)+(liMoveTo.QuadPart) < 0 )
		return false;

	if(iStartOffset < 0) // relative move back
	{
		if(!(SetFilePointerEx(hDisk, liMoveTo, NULL, FILE_CURRENT)))
		{
			AfxMessageBox("Failed to set file pointer");
			DWORD dwErr = GetLastError();
			TRACE1("\nSet file pointer failed in ReadDiskData : Error %i", dwErr);
			return false;
		}
	}

	if(bAbsolute)		// absolute move from start of file
	{
		if(!(SetFilePointerEx(hDisk, liMoveTo, NULL, FILE_BEGIN)))
		{
			AfxMessageBox("Failed to set file pointer");
			DWORD dwErr = GetLastError();
			TRACE1("\nSet file pointer failed in ReadDiskData : Error %i", dwErr);
			return false;
		}
    }
	


	

	// Read in 512bytes from the open disk
	bSuccess = ReadFile(hDisk, m_strData, iReadSize, &lpdBytes, NULL);
	if(!bSuccess) { AfxMessageBox("Failed To Read Data", 0,0);	return FALSE;  }
	if((lpdBytes == 0) && bSuccess)	// EOF
	{
		liMoveTo.HighPart = p->HighPart;
		liMoveTo.LowPart = p->LowPart;
		liMoveTo.QuadPart = (p->QuadPart - m_dwLastBytesRead);
		SetFilePointerEx(hDisk, liMoveTo, NULL, FILE_BEGIN);

		bSuccess = ReadFile(hDisk, m_strData, m_dwLastBytesRead, &lpdBytes, NULL);
		// restore org file pointer
		return false;
	}

	//if(lpdBytes < iReadSize)	// EOF possibly - so fill the empty

	// Get Current Position
	
	p->HighPart = 0;
	p->LowPart = 0;
	p->QuadPart = 0;
	liMoveTo.HighPart = 0;
	liMoveTo.LowPart = 0;
	liMoveTo.QuadPart = 0;
	SetFilePointerEx(hDisk, liMoveTo, p, FILE_CURRENT);

	m_iCurrentOffset = (p->QuadPart/m_nSize);
	m_dwLastBytesRead = lpdBytes;			 // save in case we read less than requested
	m_dwLastReq = iReadSize;

	m_iOffset = (((p->QuadPart%m_nSize) != 0) ? 1 : 0);
	m_iOffset += (p->QuadPart/m_nSize);

		
	return true;
}


void CDiskDataDoc::OnViewNextsector()
{
	if(ReadDiskData(m_nSize, 0 ) )
		UpdateAllViews(NULL, -1);
}

void CDiskDataDoc::OnViewPrevioussector()
{
	if(ReadDiskData(m_nSize,-2))
		UpdateAllViews(NULL, -1); 
	
}

void CDiskDataDoc::OnViewGotosector()
{
	
	CGoToSector CSector;

	if(CSector.DoModal() == IDOK)
	{
	
		if(ReadDiskData(m_nSize, CSector.iSectorNo, true))
		{	UpdateAllViews(NULL, -1); }
	}
	

}

