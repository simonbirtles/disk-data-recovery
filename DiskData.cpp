// DiskData.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DiskData.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "DiskDataDoc.h"
#include "DiskHexCtrl.h"
#include "DiskHexView.h"

#include "ChildWnd.h"
#include "DiskEditView.h"
#include "DiskInfo.h"
#include "DiskSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDiskDataApp

BEGIN_MESSAGE_MAP(CDiskDataApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

void CDiskDataApp::OnFileNew()
{

	CDiskSelect cdlg;
	cdlg.DoModal();					// Returns and leaves disk select in cdlg.m_strSelected 
	//m_strOpenPath = cdlg.m_strSelected;

	if(!(cdlg.m_strSelected.IsEmpty()))
	{
	   CString strDocName;
		CDocTemplate* pSelectedTemplate;
		POSITION pos = AfxGetApp()->m_pDocManager->GetFirstDocTemplatePosition();
		while (pos != NULL)
		{
			pSelectedTemplate = (CDocTemplate*) AfxGetApp()->m_pDocManager->GetNextDocTemplate(pos);
			pSelectedTemplate->GetDocString(strDocName, CDocTemplate::docName);
			if (strDocName == "DiskData")
			{ 
				CDiskDataDoc* pDoc = (CDiskDataDoc*)pSelectedTemplate->OpenDocumentFile(cdlg.m_strSelected, true);		// Dont Show until we update local vars
				//pDoc->m_bShowSelectDialog = false;
				return;
			}
		}
     
	}

	//CWinApp::OnFileNew();

	return;
}

// CDiskDataApp construction

CDiskDataApp::CDiskDataApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDiskDataApp object

CDiskDataApp theApp;

// CDiskDataApp initialization

BOOL CDiskDataApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();
  
 
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox("IDP_SOCKETS_INIT_FAILED");
		return FALSE;
	}



	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	//CDiskHexCtrl::RegisterClass();			// Custom Hex Control - diskhexctrl.h
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Active Disk"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_DiskDataTYPE,
		RUNTIME_CLASS(CDiskDataDoc),
		RUNTIME_CLASS(CChildWnd), // custom MDI child frame - changed from CChildFrm Class
		RUNTIME_CLASS(CDiskHexView));  // CDiskDataView / richedit
	pDocTemplate->SetContainerInfo(IDR_DiskDataTYPE_CNTR_IP);
	AddDocTemplate(pDocTemplate);
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Disable this line to reactivate  the dialog selection 
   // document on start also the dialog new document choice 
   // appears.
	//cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	



	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CDiskDataApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CDiskDataApp message handlers


