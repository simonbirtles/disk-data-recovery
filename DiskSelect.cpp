// DiskSelect.cpp : implementation file
//

// Uses shell32.dll to get the drive icons.
// Written On WindowsXP, so earlier versions of OS may not have some disks and
// therefore may display incorrect icons for newer drives such as CD-RW

#include "stdafx.h"
#include "DiskData.h"
#include "DiskSelect.h"

const char tabAlpha[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};


// CDiskSelect dialog

IMPLEMENT_DYNAMIC(CDiskSelect, CDialog)
CDiskSelect::CDiskSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CDiskSelect::IDD, pParent)
{
//	m_cDisk = NULL;
	m_strSelected.Empty();
	m_iDeviceType = 0;					//Default Logical Disks
	
	m_labelfontI.CreateFont(-11,0,0,0,400, TRUE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "MS Shell Dlg 2");

	m_labelfont.CreateFont(-11,0,0,0,400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "MS Shell Dlg 2");
}

CDiskSelect::~CDiskSelect()
{
	delete m_DiskImageList;
}

void CDiskSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISK, m_ctlListDisk);
	DDX_Control(pDX, IDC_DISK2, m_ctlListPhy);
	DDX_Control(pDX, IDC_FILESEL, m_selectedfile);
	DDX_Control(pDX, IDC_FILE, m_labelfile);
	DDX_Control(pDX, IDC_LOGICAL, m_labellogical);
	DDX_Control(pDX, IDC_PHYSICAL, m_labelphysical);
}


BEGIN_MESSAGE_MAP(CDiskSelect, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_DISK, OnNMDblclkDisk)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_NOTIFY(NM_CLICK, IDC_DISK, OnNMClickDisk)
	ON_NOTIFY(NM_CLICK, IDC_DISK2, OnNMClickDisk2)
	ON_BN_CLICKED(IDFILE, OnBtnClickedOpenFile)
	ON_STN_CLICKED(IDC_FILE, OnStnClickedFile)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDiskSelect message handlers

BOOL CDiskSelect::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->m_labelfile.SetFont(&m_labelfont);
	this->m_labellogical.SetFont(&m_labelfont);
	this->m_labelphysical.SetFont(&m_labelfont);

	LPTSTR lpSysDir;
	TCHAR tchBuf[MAX_PATH+1]; 
	lpSysDir = tchBuf;
	HICON icon;

	HINSTANCE hInst = AfxGetResourceHandle();

	::GetSystemDirectory(lpSysDir, MAX_PATH+1);

	HINSTANCE hLib = LoadLibrary( strcat(lpSysDir, "\\shell32.dll") );    
		
	
	
    m_DiskImageList = new CImageList;		
	m_DiskImageList->Create(48, 48, ILC_COLOR32 , 1, 4);

	if(hLib)			// If we got a handle to shell32.dll
	{
		AfxSetResourceHandle(hLib);

		icon = AfxGetApp()->LoadIcon(7);		// Floppy
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(9);		// HD
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(296);		// CD-RW
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(294);		// CD-ROM
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(295);		// CD-R
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(222);		// DVD
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(298);		// DVD-R
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(29);		// DVD-RAM
		m_DiskImageList->Add(icon);

		icon = AfxGetApp()->LoadIcon(304);		//DVD-ROM
		m_DiskImageList->Add(icon);


		AfxSetResourceHandle(hInst);			// Set Resource Back
	}
	else
	{
        icon = AfxGetApp()->LoadStandardIcon(IDI_WINLOGO);			// Use Standard Logo
		m_DiskImageList->Add(icon);
	}
		
	
	GetLogicalDisks();

	GetPhysicalDrives();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDiskSelect::OnNMDblclkDisk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	OnBnClickedOk()	;		// Call On OK

}

void CDiskSelect::OnBnClickedOk()
{
	CString strLog, strDisk, strPath;
	strLog.Empty();
	
	AfxGetApp()->BeginWaitCursor();
	
	strLog = m_cDisk[m_ctlListDisk.GetSelectionMark()];
	strPath = m_ctlListPhy.GetItemText(m_ctlListPhy.GetSelectionMark(), 0);
	
	switch(m_iDeviceType)
	{
	case 0:				// Logical Disk Support
			m_strSelected.AppendFormat("\\\\.\\%s:",strLog );
			break;

	case 1:				// Physical Disk Support
			m_strSelected.AppendFormat("\\\\.\\%s",strPath );
			break;
	
	case 2:				// File Support
			m_strSelected.AppendFormat("%s",m_fileselect);
			break;
	}
	
	UpdateData(TRUE);

	OnOK();
}

void CDiskSelect::GetPhysicalDrives(void)
{

	char mydata[65535];
	mydata[65534] = '\n';
	LV_ITEM lvitem;
	char cText[] = "PhysicalDrive ";
	char cCdRom[] = "CdRom ";

	CString ctemp;
	ctemp.Empty();
	
	m_ctlListPhy.SetImageList(m_DiskImageList, LVSIL_NORMAL);
	
	DWORD dw = ::QueryDosDevice(NULL, mydata, sizeof(mydata)) ;
	if(dw == 0)
	{
		dw = GetLastError();
		TRACE1(L"Error %lu querying \n", dw);
	}

	//if(dw >= sizeof(mydata)-1)
	//{ TRACE0("Buffer Too Small");}
	
	int j=0;
    for(int k=0;k<65535;k++)
	{
		ctemp.AppendFormat("%c", mydata[k]);
		
		if(mydata[k] == NULL)
		{	//TRACE1("\nDevice : %s", ctemp);
			if(ctemp.Left(13) == "PhysicalDrive" || ctemp.Left(5) == "CdRom")	// Found Physical Disk
			{
				if(ctemp.Left(13) == "PhysicalDrive")
				{
					cText[13] = mydata[k-1];	
					lvitem.pszText = cText;			
					
				}

				if(ctemp.Left(5) == "CdRom")
				{
					cCdRom[5] = mydata[k-1];	
					lvitem.pszText = cCdRom;			
				}

				lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
				lvitem.iItem = k;
				lvitem.iSubItem = 0;
				lvitem.iImage = 1;
				lvitem.stateMask = LVIS_STATEIMAGEMASK;
				lvitem.state = INDEXTOSTATEIMAGEMASK(1);
				

				m_ctlListPhy.InsertItem(&lvitem) ;
				//if(m_ctlListPhy.InsertItem(&lvitem) == -1)
				//	TRACE0("\nFailed To Insert Item");

				j++;		
			}
			ctemp.Empty();
		}
	}

	
	return;
}
////////////////////////////////////////////////////////////////////////////////////////
void CDiskSelect::GetLogicalDisks(void)
{

	LV_ITEM lvitem;
	char temp[] = "  Drive";
	char cDrive[] = " :\\";
	UINT uiDriveType = 0;
	HINSTANCE hInst = AfxGetResourceHandle();
	HINSTANCE hLib = LoadLibrary("c:\\windows\\system32\\shell32.dll");      // TODO : c:\windows\system32
	
	m_ctlListDisk.SetImageList(m_DiskImageList, LVSIL_NORMAL);
	DWORD dw2 = GetLogicalDrives();
	DWORD dwDisk = 1; 

	if(dw2 == 0) // Failed To Get Logical Disks
		{return;}

	int j=0;
	for(int i=0; i<26;i++)
	{
		if(dw2 & dwDisk)
		{
			// Add To The Selection
			cDrive[0] = tabAlpha[i];
			m_cDisk[j] = tabAlpha[i];
			uiDriveType = ::GetDriveType( cDrive );
			lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvitem.iItem = i;
			lvitem.iSubItem = 0;
			lvitem.iImage = GetIconIndex(uiDriveType);
			lvitem.stateMask = LVIS_STATEIMAGEMASK;
			lvitem.state = INDEXTOSTATEIMAGEMASK(1);
			// Conversion.
			temp[0] = tabAlpha[i];			// pszText cant handle single char so we copy the single
			lvitem.pszText = temp;			// char to a null term string, then pass this to pszText.
			j++;
		
			m_ctlListDisk.InsertItem(&lvitem)  ;

			//if(m_ctlListDisk.InsertItem(&lvitem) == -1)
				//TRACE0("Failed To Insert Item");	
		}
		dwDisk = (dwDisk * 2);
	}
	
}


void CDiskSelect::OnNMClickDisk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	// User Has Clicked The Control So make sure that the other list view has nothing selected
	// This Code if for the Logical Disk Click

	m_iDeviceType = 0;
	m_selectedfile.SetWindowText("");
	this->m_labelfile.SetFont(&m_labelfont);
	this->m_labellogical.SetFont(&m_labelfontI);
	this->m_labelphysical.SetFont(&m_labelfont);
	

	*pResult = 0;
}

void CDiskSelect::OnNMClickDisk2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	// Code For Physical; Disk

	m_iDeviceType = 1;
	m_selectedfile.SetWindowText("");
	this->m_labelfile.SetFont(&m_labelfont);
	this->m_labellogical.SetFont(&m_labelfont);
	this->m_labelphysical.SetFont(&m_labelfontI);

	*pResult = 0;
}



void CDiskSelect::OnBtnClickedOpenFile()
{
	// TODO: Add your control notification handler code here
	// m_fileselect	

	CFileDialog cfile(true);
	if(cfile.DoModal() == IDOK)
	{
		m_fileselect = cfile.GetPathName();
		m_selectedfile.SetWindowText(m_fileselect);
		m_iDeviceType = 2;
		this->m_labelfile.SetFont(&m_labelfontI);
		this->m_labellogical.SetFont(&m_labelfont);
		this->m_labelphysical.SetFont(&m_labelfont);

	}
    
}

void CDiskSelect::OnStnClickedFile()
{
	// TODO: Add your control notification handler code here
}

int CDiskSelect::GetIconIndex(UINT uiDriveType)
{
	int iIndex = 0;

	switch(uiDriveType)
	{
	case DRIVE_UNKNOWN:
		iIndex = 0;
		break;

	case DRIVE_NO_ROOT_DIR:
		iIndex = 0;
		break;

	case DRIVE_REMOVABLE:
		iIndex = 0;
		break;

	case DRIVE_FIXED:
		iIndex = 1;
		break;

	case DRIVE_REMOTE:
		iIndex = 0;
		break;

	case DRIVE_CDROM:
		iIndex = 3;
		break;

	case DRIVE_RAMDISK:
		iIndex = 7;
		break;
    
	default:
		iIndex = 1;

	}


	return iIndex;

}








void CDiskSelect::OnBnClickedCancel()
{
	m_strSelected.Empty();
	OnCancel();
}
