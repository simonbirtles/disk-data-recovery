// DiskInfo.cpp : implementation file
//
// Function of this class is to gather physical disk info and other details about
// disks installed on the local PC. This class is based on CObject so is the lowest 
// class used.

#include "stdafx.h"
#include "DiskData.h"
#include "DiskInfo.h"

#include "DiskDataDoc.h"
#include "DiskEditView.h"

#include "Ntmsapi.h"
#include "winioctl.h"


// CDiskInfo

CDiskInfo::CDiskInfo()
{
	GetPhyInfo();
	//GetMBRInfo();				// Get The Master Boot Record Info
	//GetBootSectorInfo();		// Get The Boot Sector and Backup Sector Info
	//GetMFTInfo();				// Get The Master File Table Info and Backup - NTFS Only

}

CDiskInfo::~CDiskInfo()
{
}


// CDiskInfo member functions



void CDiskInfo::GetPhyInfo()
{
/*
  // Removed as GUID Problems...

	// Open an RMS (Removable Media Storage) Session
	HANDLE hNtms = OpenNtmsSession(NULL, NULL, 0) ;
	if (hNtms == INVALID_HANDLE_VALUE)
	{ AfxMessageBox("Error Opening NTMS Session", 0, 0); }
	
	// 
	LPDWORD lpListSize = (LPDWORD)100; // Ptr to max no. of ID's Returned
	GUID* m_guid;	// Buffer For Array Of Devices

	m_guid = NTMS_NULLGUID;
	//m_guid = GUID_NULL;



DWORD dwEnum = EnumerateNtmsObject(hNtms, NULL, m_guid, lpListSize, NTMS_PHYSICAL_MEDIA, 0);
	if (dwEnum != ERROR_SUCCESS)
		{ AfxMessageBox("Error Enumerating NTMS Objects", 0, 0); }

	

	CloseNtmsSession(hNtms);
*/

// Using the Physical Disk Info / Numbers Get The
// Sector / Clusters / Etc info for the Disk

// Open A File For The Disk We Need To Examine - TODO : Remove Static C: Drive
// \\\\.\\PHYSICALDRIVE0
	HANDLE hDisk = CreateFile("\\\\.\\C:", GENERIC_READ, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							NULL, OPEN_EXISTING, 0,0); 


  if( hDisk == INVALID_HANDLE_VALUE )	
  {		AfxMessageBox("Failed To Open Disk", 0,0);	return;  }

// Using the File We Just Opened, Get Phy Disk Details
  DISK_GEOMETRY geom;
  DWORD lpdSize;

  BOOL bSuccess = DeviceIoControl(hDisk,IOCTL_DISK_GET_DRIVE_GEOMETRY, 
					NULL, 0, &geom, sizeof(geom), &lpdSize , NULL);

  if (!bSuccess) 
   { AfxMessageBox("Failed To Get Disk Geometry", 0,0); return;}

   // Now We have the disk details (Cylinders, MediaType, TracksPerCylinder,
   //								SectorsPerTrack, BytesPerSector)
   // For Now Lets Just Print The Details To The Screen
   
   // Get A Pointer to the Main Frame Window
   CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
   // Get the active MDI child window.
   CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    // Get the active view attached to the active MDI child window.
   CDiskEditView *pView = (CDiskEditView *) pChild->GetActiveView();
   // Get The Drawing Context Pointer For The Active View 
   //CDC* pDC  = pView->GetDC();
  
   // Lets Format The Disk Data Into A Common String
   CString strText = "";
   CString strTemp;
   strTemp.Format("Cylinders: %i ", geom.Cylinders.QuadPart) ;
   strText = strText + "\n" + strTemp;
   strTemp.Format("Bytes Per Sector: %i ", geom.BytesPerSector);
   strText = strText + "\n" + strTemp;
   strTemp.Format("Sectors Per Track: %i ", geom.SectorsPerTrack);
   strText = strText + "\n" + strTemp;
   strTemp.Format("Tracks Per Cylinder: %i ", geom.TracksPerCylinder);
   strText = strText + "\n" + strTemp;
   // Enum, Only Shows Integer - Not Actual Desc.
   strTemp.Format("Media Type: %i ", geom.MediaType) ;
   strText = strText + "\n" + strTemp;
   // Calculate The Total Disk Capacity In Bytes
   ULONGLONG lDiskSize = (((geom.Cylinders.QuadPart * (ULONGLONG)geom.TracksPerCylinder) 
						* (ULONGLONG)geom.SectorsPerTrack) * (ULONGLONG)geom.BytesPerSector);
   strTemp.Format("Total Disk Capacity : %I64d Bytes", lDiskSize);
   strText = strText + "\n" + strTemp;

   // Done Formatting Disk Data

   // This is just a test of ReadFile() to display the first 512 bytes of the PhyDisk
   SetFilePointer(hDisk, 0, 0, FILE_BEGIN); 

   unsigned char strData[512];
   DWORD lpdBytes = 0;
   // Read in 512bytes from the open disk
   bSuccess = ReadFile(hDisk, strData, 512, &lpdBytes, NULL);
   if(!bSuccess) { AfxMessageBox("Failed To Read Data", 0,0);	return;  }

   // Append and Create String to put displayed on screen 
   strText = "";								// Remove Drive Details
   CString strDat = "\n";						// Hex String
   for(int i=1; i<513; i++)					   // TODO: Change To Dynamic Sector Size
   {   
	  strDat.AppendFormat("%02X ", strData[i-1]);		// Hex Format
	  
	  if( ((i%16) == 0) & (i > 0) )
      { strText.AppendFormat("%s\n", strDat);
	    strDat = "";
	  }
   }
   
  // Show The Collected Data
  //pView->SetWindowText(strText);		
	
		
    
   
 

  // Close Handle To Disks
  BOOL bClosed = CloseHandle(hDisk);
  if (!bClosed) { AfxMessageBox("Failed To Close Disk Handle", 0,0); }

}





