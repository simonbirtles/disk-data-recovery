/*
Class CDiskAnalysis()
	
	The basis of this class is to recieve either a handle to an open (disk, volume, file)
	or a string to one of the above to be opened by this class.
	This class performs functions such as determining the type of formatting on a disk 
	getting info from the MasterBootRecord & Partition Table, Sector Sizes, and the 
	PartitionBootTable.
    
    On contruction of this class variables are initlised but no disk access starts until
	the member function AnalyzeDisk is called, then other member functions can be called
	to gather info.
	How To Use This Class
	---------------------
	1. Create an instance of this class in your code e.g CDiskAnalysis myDisk;
	2. Call required function(s)
	

	CORE FUNCTIONS:
	==============================================================================================
	TO COMPLETE:
	===========
	FindSectorMFT()		-		Internal - Find the given sector in the MFT and translate to file details
	FindSector()		-		Public - Called from client to get details of a given sector

	PUBLIC:
	=======
	FindFile()			-		Find a file on the specified logical drive.
	RecoverFile()		-		Recovers file using FILE_CLUSTERS struct info
	FillPartitionInfo() -		Uses the name 'PhysicalDiskX' to open the disk and get the MBR info

	PRIVATE:
	========
	GetMBR()			-		Fills The MASTER_BOOT_RECORD Structure with the Master Boot Record Details
	GetPartitionTable()	-		Fills the PARTITION_TABLE_NTFS structure with the selected NTFS partition boot sector details
	GetMFTOffsets()     -		Creates an array of MFT_CLUSTERS defining MFT cluster on disk.
	FindFileMFT()		-		Internal Function To Get A File From The NTFS MFT into struct FILE_CLUSTERS
	GetFileExtents()	-		Internal - Gets the File Extents from NTFS MFT into struct FILE_EXTENT
	GetFilePathMFT()	-		Internal - Get The DOS path for the seleted file from the NTFS MFT
	GetLogicalDisks()   -       Gets all logical disks assigned in this system into struct PHYSICAL_DISKS
	GetPhysicalDisks()  -		Gets all physical disks installed in this system into struct LOGICAL_DISKS
	
	ON CONSTRUCTION:
	================
	GetPhysicalDisks()  -		Populates struct PHYSICAL_DISKS with each disk name (CdRom1, PhysicalDisk0, etc)
	GetLogicalDisks()	-		Populates struct LOGICAL_DISKS with log disk, phy disk#,start cluster offset (c:\, 0, 63)

	NOTES:
	======
	1. Using the GetMFTOffsets Function:

		diskanalysis.GetMFTOffsets();						// Find All The Sectors That Make The MFT 
		MFT_CLUSTERS *pMFT;
		pMFT = diskanalysis.m_ClusterArray;
		__int64 itest = pMFT->iStartCluster;
		pMFT++;
		__int64 itest2 = pMFT->iStartCluster;

	2. Using the FindFile Function:

		CDiskAnalysis	diskanalysis;
		CString strFile = "shell32.dll"   ;
		diskanalysis.FindFile(strFile, "C:\\", NULL))




*/




#include "StdAfx.h"
#include "diskanalysis.h"
#include "diskioctl.h"

#include "winioctl.h"			
#include "cstringt.h"
#include "math.h"


struct{
	int iIndex;
	DWORD dwType;
	TCHAR* szLabel;
}
FormatTypes[] = 
{
		0,0x00, _T("Unknown Type"),
		1, 0x01, _T("FAT12 primary partition or logical drive (fewer than 32,680 sectors in the volume)"),
		2, 0x04, _T("FAT16 partition or logical drive (32,680–65,535 sectors or 16 MB–33 MB)"),
		3, 0x05, _T("Extended partition"),
		4, 0x06, _T("BIGDOS FAT16 partition or logical drive (33 MB–4 GB)"),
		5, 0x07, _T("Installable File System (NTFS partition or logical drive)"),
		6, 0x0B, _T("FAT32 partition or logical drive"),
		7, 0x0C, _T("FAT32 partition or logical drive using BIOS INT 13h extensions"),
		8, 0x0E, _T("BIGDOS FAT16 partition or logical drive using BIOS INT 13h extensions"),
		9, 0x0F, _T("Extended partition using BIOS INT 13h extensions"),
		10, 0x12, _T("EISA partition or OEM partition"),
		11,0x42, _T("Dynamic volume"),
		12,0x84, _T("Power management hibernation partition"),
		13,0x86, _T("Multidisk FAT16 volume created by using Windows NT 4.0"),
		14,0x87, _T("Multidisk NTFS volume created by using Windows NT 4.0"),
		15,0xA0, _T("Laptop hibernation partition"),
		16,0xDE, _T("Dell OEM partition"),
        17,0xFE, _T("IBM OEM partition"),
		18,0xEE, _T("GPT partition"),
		19,0xEF, _T("EFI System partition on an MBR disk"),

}  ;



const char tabAlpha[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

CDiskAnalysis::CDiskAnalysis(void)
{
	m_HndData = NULL;	
	//m_iSize = 0;
	m_iDiskOpen = -1;			// no open disk

	m_HandleType = OPEN_NONE;

	m_ClusterArray = (MFT_CLUSTERS*)malloc(1 * sizeof(MFT_CLUSTERS) ); // Allocate
	m_ClusterArray->iStartCluster = 0;
	m_ClusterArray->iEndCluster = 0;
	m_ClusterArray->iMFTRecSize = 0;

	// change to use VirtualAlloc
	//m_FileDetails = (FILE_CLUSTERS*)malloc(1 * (sizeof(FILE_CLUSTERS)  )); // Allocate
	//m_FileDetails->sFileExtents = (FILE_EXTENT*)malloc(1 * sizeof(FILE_EXTENT) );

	
	//m_FileDetails = (FILE_CLUSTERS*)VirtualAlloc(NULL, sizeof(FILE_CLUSTERS)*500, MEM_COMMIT, PAGE_READWRITE);

	//m_FileDetails->sFileExtents = (FILE_EXTENT*)VirtualAlloc(NULL, 500 *sizeof(FILE_EXTENT), MEM_COMMIT, PAGE_READWRITE);




//	m_FileDetails->iIsDeleted = FALSE;
//	memset(m_FileDetails->strFolder ,'0', 260)   ;
//	m_FileDetails->strFolder[260] = '\0';	
//	m_FileDetails->sFileExtents->iFileExtentEndCluster = 0;
//	m_FileDetails->sFileExtents->iFileExtentStartCluster = 0;
//	m_FileDetails->sFileExtents->iMFTRecordNo = 0;

	m_physicaldisks = (PHYSICAL_DISKS*)malloc(1 * sizeof(PHYSICAL_DISKS) );	// Allocate
	m_logicaldisks = (LOGICAL_DISKS*)malloc(1 * sizeof(LOGICAL_DISKS) );
	hGlobalPT = NULL;


	// Get The installed disk details, this will call GetLogical Disks As Well
	// TODO : Do we need a modeless box here to tell the user we are gathering data ?

	GetPhysicalDisks();

	

}

CDiskAnalysis::~CDiskAnalysis(void)
{
	// Cleanup
	if(m_HndData != NULL)
		CloseHandle(m_HndData);

	free(m_physicaldisks);
	free(m_logicaldisks);
	free(m_ClusterArray);
	//free(m_FileDetails);

}

/*

			Class Functions


*/

/***************************************************************************************

						Functions Non Specific To OS

						.Getting Master Boot Record

*****************************************************************************************/

/*
	Open Disk/Volume/File

	OS: 95/98/ME cant exceed MAX_PATH for file name string TODO 
*/
BOOL CDiskAnalysis::OpenHandle(LPCTSTR lpctPath)
{
	/* Check to see what type of path has been passed to us
	   or do we do this ourselves, ie we open each phydisk to get the info
	   needed to fill the tree struct.
	   We have two options:
	   1. Make this class specific to the tree control needs
	   2. Make it self supporting and deal with different possible needs

	   Currently we have to open a physical disk

   */
	  

	m_HndData = CreateFile(lpctPath, GENERIC_READ, 
							  FILE_SHARE_READ | FILE_SHARE_WRITE,
				              NULL, OPEN_EXISTING, 0, 0);

	if(m_HndData == INVALID_HANDLE_VALUE)		// Function Failed
	{
		DWORD dwErr = GetLastError();
		//TRACE1("Failed To Open File - DiskAnalysis.cpp-OpenHandle() Error:%i", dwErr);
        return false;
	}

	CString strTemp;

	//GetDriveType("A:");	

	if(strstr(lpctPath, "Volume") != NULL)
		m_HandleType = OPEN_VOLUME   ;


	char *pDest = strstr(lpctPath, ":");
	if(pDest != NULL)
	{
		int result = (int)(pDest - lpctPath + 1);
		int string = (int)strlen(lpctPath);						// for a volume the char : should be the last char in the file open str

		if(result == string)
			m_HandleType = OPEN_LOGICAL   ;
	}

   
   
	if(strstr(lpctPath, "Physical") != NULL || strstr(lpctPath, "CdRom") != NULL)
	{
		m_HandleType = OPEN_PHYSICAL   ;
		CString temp = lpctPath;
		m_iDiskOpen = (int)lpctPath[strlen(lpctPath)];		// get phydisk number
		return (GetMBR() > 0 ? true : false);
	}


	if(m_HandleType == OPEN_NONE)
		m_HandleType = OPEN_FILE;



	return true;
}

/*
	Get The Master Boot Record (MBR) and Fill In Data Struct
	Gets Info About the first four partitions on this disk and 
	puts this details in to a public array of m_HDPartitionTable[x] 
	where x is the partition no
	System Partition is specified by the bBootable data member of the m_HDPartitionTable struct

	OS: Not OS Specific

*/
int CDiskAnalysis::GetMBR(BYTE bParent, DWORD64 dwRelSector, DWORD dwRecno)
{
	DWORD dwErr = 0;
	DWORD dwType;
	BOOL bSuccess;
	DWORD lpdRead;
	BYTE*	byData;			// Data From ReadFile
	DWORD iLowBits, iHighBits;
	int iOffset = 446;
	char* strText = NULL;

	
	// must be a physical disk only 
	ASSERT((m_HandleType & ~OPEN_PHYSICAL) == 0);
	if((m_HandleType & ~OPEN_PHYSICAL) != 0)
	{
		AfxMessageBox(MBR_PHYSICAL_ONLY, MB_ICONSTOP, 0);
		return 0;
	}
   	
	// Save current file pointer
	PLARGE_INTEGER pli;
	pli = (PLARGE_INTEGER)VirtualAlloc(NULL, sizeof(LARGE_INTEGER), MEM_COMMIT, PAGE_READWRITE);
	LARGE_INTEGER li;
	li.HighPart = 0; li.LowPart = 0; li.QuadPart = 0;
	SetFilePointerEx(m_HndData, li, pli, FILE_CURRENT);

	// Set File Pointer To Beginning Of Disk
	li.HighPart = 0; li.LowPart = 0; li.QuadPart = (dwRelSector*512);
	dwErr = SetFilePointerEx(m_HndData, li, NULL , FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		//TRACE1("Failed To Set File Pointer - DiskAnalysis.cpp-GetMBR() Error:%i", dwErr);
		return 0;
	}

	byData = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT, PAGE_READWRITE);
	// Read In 512 Bytes - (Always the same for all disks for the MBR)
	
	bSuccess = ReadFile(m_HndData, byData, 512, &lpdRead, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		//TRACE1("\nFailed To Get File Data - DiskAnalysis.cpp-GetMBR() Error:%i", dwErr);
		return 0;
	}

	if( ( *(byData + 510) != 85) &&   // 55(85) AA(170) is the MBR signature
		( *(byData + 511) != 170))
	{
		// TODO : DiskStatusLog(MBR_MISSING, MB_ICONSTOP, 0);
		// restore file pointer
		li.HighPart = pli->HighPart; li.LowPart = pli->LowPart; li.QuadPart = pli->QuadPart;
		SetFilePointerEx(m_HndData, li, NULL, FILE_BEGIN);
		return 0;				// invalid MBR - does not exist (cdrom) or corrupt/blank (harddisk)
	}


/*********************************************************************************************************************
	NOTE CHANGE TO HEAPS**
*********************************************************************************************************************/
	// strText = (char*)malloc(2);
	
	 
	 BOOL bChild = false;
	 byData += 430;	// offset to 1st partition partition table in the MBR - 16 bytes
	 int iCount = 0, iIndex = 0;
	 char czBuf[2];
	
	 while(true)
	 {
	   iCount++;
	    byData += 16;			// next entry

		if((*(byData+12) + 
			(*(byData+13) << 8) + 
			(*(byData+14) << 16) + 
			(*(byData+15) << 24)) == 0)	// no sectors - invalid record
		{
			// restore file pointer
			li.HighPart = pli->HighPart; li.LowPart = pli->LowPart; li.QuadPart = pli->QuadPart;
			SetFilePointerEx(m_HndData, li, NULL, FILE_BEGIN);
			//free(strText);
			if(bParent == 0)	// back to root or first part table, add a final zero init'd record.
			{
			   dwRecno++;
			   HGLOBAL hPrev = hGlobalPT;
			   hPrev = GlobalReAlloc(hGlobalPT, (dwRecno*sizeof(MASTER_BOOT_RECORD)), GMEM_MOVEABLE|GMEM_ZEROINIT );
			   dwErr = GetLastError();
			   ASSERT(dwErr == 0);
			   if(dwErr == 0)
				   hGlobalPT = hPrev;
			}
            return dwRecno;
		}

		dwRecno++;		// next valid record number

		if(hGlobalPT == NULL)
			hGlobalPT = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, sizeof(MASTER_BOOT_RECORD)); // 1 * MBR Partition Record
		else 
			hGlobalPT = GlobalReAlloc(hGlobalPT, (dwRecno*sizeof(MASTER_BOOT_RECORD)), GMEM_MOVEABLE|GMEM_ZEROINIT );

		ASSERT(hGlobalPT != NULL);

		m_HDPartitionTable = (MASTER_BOOT_RECORD*)GlobalLock(hGlobalPT);
		m_HDPartitionTable += (dwRecno-1);		// move to next blank record

		m_HDPartitionTable->bParent = bParent;
		m_HDPartitionTable->bRecordNo = dwRecno;

		// bootable partition
		m_HDPartitionTable->bBootable = (*(byData) == 0x80 ? true : false);

		m_HDPartitionTable->byStartHead = *(byData+1);
	
		m_HDPartitionTable->iStartSector = *(byData+2) & 63;			// get low 6 bits
		m_HDPartitionTable->iStartCylinder =  (*(byData+3) << 2) + (*(byData+2) & 3);	  // shift offset 3 by two and add

		dwType = *(byData+4);
		m_HDPartitionTable->iFileSystem = dwType;				// Hex of installed file system.
		//delete strText;
		//strText = (char*)malloc(255);
		//strText = (char*)memset(strText, '0', 255); 
		if(strText != NULL)
			VirtualFree(strText, 0, MEM_RELEASE);

		strText = (char*)VirtualAlloc(NULL, 255, MEM_COMMIT, PAGE_READWRITE);

		iIndex = GetFormatTypeText(dwType);		//		  MAX_PATH
		memcpy(m_HDPartitionTable->strFileSystem, FormatTypes[iIndex].szLabel, strlen(FormatTypes[iIndex].szLabel) );			// Description Of Format Type On Disk

		m_HDPartitionTable->iEndingHead = *(byData+5);		// Ending Head

		m_HDPartitionTable->iEndingSector = *(byData+6) & 63;			// AND use '0' in binary to clear opposing bits to 0
		m_HDPartitionTable->iEndingCylinder =  (*(byData+7) << 2) + (*(byData+6) & 3);
	    

		m_HDPartitionTable->dwRelativeSector = *(byData+8) + 
											(*(byData+9) << 8) + 
											(*(byData+10) << 16) +
											(*(byData+11) << 24) ;
	
		m_HDPartitionTable->dwTotalSectors = *(byData+12) + 
											(*(byData+13) << 8) + 
											(*(byData+14) << 16) + 
											(*(byData+15) << 24);

		// valid partition
		m_HDPartitionTable->bValidPartition = (m_HDPartitionTable->dwTotalSectors > 0 ? true : false);

		m_HDPartitionTable->dwFormatType = GetPartitionFormatType(m_HDPartitionTable->dwRelativeSector);

		_itoa(iCount,czBuf, 10)	;
		czBuf[1] = '\0';

		if(*(byData+4) == 0x05 || *(byData+4) == 0x0F)	// this is an extended partition entry, go to this entry and get info
		{
		   strcpy(m_HDPartitionTable->strResourceDesc, "Partition ");
		   strcat(m_HDPartitionTable->strResourceDesc, czBuf);
		   BYTE bRecno =   m_HDPartitionTable->bRecordNo   ;
		   DWORD64 dwSector =   m_HDPartitionTable->dwRelativeSector ;
		   GlobalUnlock(hGlobalPT);
		   VirtualFree(strText, 0, MEM_RELEASE);
		   VirtualFree(byData, 0, MEM_RELEASE);
		   VirtualFree(pli, 0, MEM_RELEASE);
		   dwRecno = GetMBR(bRecno, dwSector , dwRecno);
		   

		}
		else
		{
			strcpy(m_HDPartitionTable->strResourceDesc, "Volume ");
			strcat(m_HDPartitionTable->strResourceDesc, czBuf);
            GlobalUnlock(hGlobalPT);
		}

        //dwRecno++;
	 }


	// should never get here !
	// restore file pointer
	li.HighPart = pli->HighPart; li.LowPart = pli->LowPart; li.QuadPart = pli->QuadPart;
	SetFilePointerEx(m_HndData, li, NULL, FILE_BEGIN);
	//free(strText);
	return true;
}





/***************************************************************************************

						Functions Specific To NTFS MFT 

*****************************************************************************************/
/*
  
				Get The File Extents / What Clusters The File Is Held In
				OS : NTFS Supporting OS (NT/2000/XP)

				Params:
					bData		  - Pointer to data BYTE array for this MFT Record
					iExtentOffset - $DATA attribute offset from the start of the MFT record
					sFileExtents  - Pointer to FILE_CLUSTERS array to be filled by this func

*/
int CDiskAnalysis::GetFileExtents(BYTE *bData, int iExtentOffset, FILE_CLUSTERS* sFileExtents)
{

	
	/*************************************************************************************************
	THE DATA RUNS CAN BE AT DIFFERENT OFFSETS FROM THE START OF THE DATA HEADER 0x80
	NO CONSISTENT DOCS ARE AVAILABLE TO SHOW EXACT FORMAT/CALC TO FIGURE THIS OUT...
	BELOW IS THE CALC THAT WORKS. - 32 Bytes from the start of the $DATA attribute
	**************************************************************************************************/

	int iDataRunOffset = bData[32+iExtentOffset];	// iDataRunOffset will now have the offset of the start of the DRuns
													// Offset From Attribute Header 0x80 $DATA.

													// iExtentOffset has the Attribute Header (0x80 $DATA)Offset position
	int iOff = iExtentOffset + iDataRunOffset;		// Take the Start of the Attribute Header (0x80 $ DATA) and add the offset 
													// value of the data runs from the header and we end up with the 
													// position of the start of the extent runs in the MFT file.

	int iCnt = 0, j=0, k=0;
	__int64 iTemp=0;
	int pdwLow = 0, ifcSize = -1;
	DWORD dwHigh=0, dwLow=0;
	DWORD dwTemp=0;
	FILE_EXTENT	*m_fc;
	FILE_EXTENT	*m_de;
	FILE_CLUSTERS *m_fc2;
	FILE_EXTENT *fcTemp;


	

	// do a quick smaler version of the next loop to find how many extents we have
	// and use that to calc mem needed to store them..
	while(bData[iOff] > 0)				// Last Extent will end with 0x00
	{
		dwHigh = (bData[iOff] & 15 );	// Highbits - Byte Length of run
		dwLow = (bData[iOff] & 240);	// Low Bits - Byte Length of cluster
		dwLow = (dwLow >> 4);			// Bitshift 4 to low bits
		iOff += (dwHigh + dwLow+1 );
		iCnt++;
	}

	m_fc2 = sFileExtents;
	// check to see if we have any existing extents in this struct 
	while(m_fc2->iNoExtents > 0)
	{	

		
		// allocate memory 
		fcTemp = (FILE_EXTENT*)VirtualAlloc(
                          NULL,												 // next page to commit
                           m_fc2->iNoExtents*sizeof(FILE_EXTENT),            // page size, in bytes
                           MEM_COMMIT,										 // allocate a committed page
                          PAGE_READWRITE);		

		m_fc = sFileExtents->sFileExtents;	 
		// save current extents
		memcpy(fcTemp, m_fc,  (m_fc2->iNoExtents+1)*sizeof(FILE_EXTENT));

		iCnt += (m_fc2->iNoExtents);

		ifcSize =  (m_fc2->iNoExtents);				   

	    m_fc2->iNoExtents = 0;
 	}

	// Allocate memory 
	sFileExtents->sFileExtents = (FILE_EXTENT*)VirtualAlloc(
                          NULL,								 // next page to commit
                           (iCnt+1)*sizeof(FILE_EXTENT),         // page size, in bytes
                           MEM_COMMIT,							 // allocate a committed page
                          PAGE_READWRITE);						 // read/write access

	m_fc = sFileExtents->sFileExtents;

	if(ifcSize > 0)
		memcpy(m_fc, fcTemp, (ifcSize+1)*sizeof(FILE_EXTENT));
    	
	if(ifcSize > 0)	   
		m_fc += (ifcSize);	    // must point to first empty array member
	
	iOff = iExtentOffset + iDataRunOffset;
	
	iCnt = 0;

	// Do loop to find the FILE_EXTENT and add the details
	while(bData[iOff] > 0)		// Last Extent will end with 0x00
	{
		dwHigh = (bData[iOff] & 15 );	// Highbits - Byte Length of run
		dwLow = (bData[iOff] & 240);	// Low Bits - Byte Length of cluster
		dwLow = (dwLow >> 4);			// Bitshift 4 to low bits
		iOff += (dwHigh + dwLow +1);	// will be on the next extent length byte (ie 0x11, 0x31, etc)
		    
		iCnt++;

		/**********************************************************************************************
	        Get File Extents	 
			dwLow = cluster offset
			MFTReadVar takes the low byte which is the virtual cluster number and 
			calculates the virtual cluster number.
			The bytes are encoded and may be negative, which is why this func is called
		 **********************************************************************************************/
		iTemp = 0;
		pdwLow = dwLow;
		iTemp = MFTReadVar(&bData[iOff-pdwLow], pdwLow);
	//	//TRACE1("\nVar Calc Says : %i", iTemp);
		/**********************************************************************************************
		    save the calculated cluster offset
		 **********************************************************************************************/
		if(iCnt < 2)	// this is the first record so we save the return value as is - its a logical value
		{
			m_fc->iFileExtentStartCluster = iTemp;
			////TRACE0("\nClusters offsets from start of volume");
			////TRACE1("\nStart Cluster : %i", iTemp);
		}
		else				   // 2nd or more record, we need to add the previous start cluster as this is a virtual cluster no 
		{					   // (ie offset from the last starting offset/cluster)
			m_de = m_fc;	   // save pointer to previous array - we need the data from this in a few lines
			m_fc++;                            
			m_fc->iFileExtentStartCluster = (m_de->iFileExtentStartCluster + iTemp);
			////TRACE1("\nStart Cluster : %i", m_fc->iFileExtentStartCluster);
		}

		/*
			Get File/Extent Length (For this Extent Only)
			dwHigh = extent length - (extent length is the no. of contigous cluster which make up an extent)
		*/
		j = dwHigh-1;
		iTemp = 0;
	    for(int i=0;i<(int)dwHigh;i++)						 // bytes are reversed and encoded
		{
			k = iOff - dwLow -1; 							  // On To First Length Byte
			if(bData[k-i] > 0)
				iTemp += bData[k-i] * (DWORD)pow(256, j);	  // Calc Actual Offset
		
			j--;
		}

		if(iCnt < 2)	// First Record	- so add the length of this extent to the starting cluster we saved earlier to get the end cluster
		{				// for this extent
			m_fc->iFileExtentEndCluster = m_fc->iFileExtentStartCluster + (iTemp-1);
			////TRACE1("\nEnd   Cluster : %i", m_fc->iFileExtentEndCluster);
		}
		else
		{   	
            m_fc->iFileExtentEndCluster = m_fc->iFileExtentStartCluster + (iTemp-1);
			////TRACE1("\nEnd   Cluster : %i", m_fc->iFileExtentEndCluster);
		}

		dwTemp = (bData[17] << 8);
		dwTemp = dwTemp + bData[16];
		m_fc->iMFTRecordNo = dwTemp;
		

	} // end while

	// Get The Real Size Of The Complete File - Reset Offsets
    iDataRunOffset = bData[32+iExtentOffset];
	iOff = (iExtentOffset + iDataRunOffset) - 16;			// This calc took us to the start of the data runs
															// if we minus 16 we get the the start of the real file size

	m_fc2->iFileLength += 
			bData[iOff] + 
			(bData[iOff+1] * 256) + 
			(bData[iOff+2] * 65536) + 
			(bData[iOff+3] * 16777216)+
			(bData[iOff+4] * 4294967296)+
			(bData[iOff+5] * 1099511627776)+
			(bData[iOff+6] * 281474976710656)+
			(bData[iOff+7] * 72057594037927936);
	////TRACE1("\nFile size is : %i bytes" , m_fc2->iFileLength);
	
	// add last record with '0' to denote the end of file extents
	m_fc++;
	m_fc->iFileExtentEndCluster = 0;
	m_fc->iFileExtentStartCluster = 0;
	m_fc->iMFTRecordNo = 0;
	
 
	return (iCnt + (ifcSize < 0 ? 0 : ifcSize));
}


/*
	Fills The Partition Boot Sector Table Structure With Selected Partition Info
	NFTS Partition is the record at the start of the volume/partition that holds
	info such as sector/cluster size where the 1st MFT is located

	OS: Specific To NTFS

	Notes: Always a phy disk must be open, the function should be sent the offset 
	of the partition from the start of the phy disk

*/
BOOL CDiskAnalysis::GetNTFSPartitionTable(int iPartition)
{
	DWORD dwErr = 0;
	BOOL bSuccess;
	DWORD lpdRead;
	BYTE *byData2;

	byData2 = (BYTE*)VirtualAlloc(
                           NULL, // next page to commit
                           512,         // page size, in bytes
                           MEM_COMMIT,         // allocate a committed page
                           PAGE_READWRITE);    // read/write access
  if (byData2 == NULL )
    {
      //TRACE0("\nVirtualAlloc failed");
      return FALSE;
    } else 



	if(m_HndData == NULL)
	{
		//TRACE0("GetPartitionTable returning FALSE because file handle was NULL.");
		return false;
	}
	
	// TODO - Need Check That Master Boot Record Has Been Read ( GetMBR() )
							
	// TODO  * 512 (Bytes Per Sector Need Calc not hardset)

	LARGE_INTEGER liDist;
	liDist.HighPart = 0;
	liDist.LowPart = 0;
	// if iPartition is <0 (-1) then must be a logical disk open
	//__int64 iLge = (iPartition < 0 ? 0 : m_HDPartitionTable[iPartition].dwRelativeSector);
	__int64 iLge = (iPartition < 0 ? 0 : 63);		// BUG: temp fix while redoing GetMBR
	__int64 iRelSector = iLge;  //TODO : Needs Tidying ?? maybe
	if(iPartition < 0)
		ASSERT( (iPartition < 0) &&	 (m_HandleType == OPEN_LOGICAL) );

	iLge *= 512;
	
	liDist.QuadPart = iLge;
	dwErr = SetFilePointerEx(m_HndData, liDist, NULL,  FILE_BEGIN);
	if(dwErr == 0)
	{
		dwErr = GetLastError();
		//TRACE1("Failed To Set File Pointer - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}

	// Read In 512 Bytes - (Always the same for all disks)
	bSuccess = ReadFile(m_HndData, byData2, 512, &lpdRead, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		//TRACE1("Failed To Get File Data - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}
	

	// We are now at the Start of Partition i, at the Partition Boot Sector.
	// Working From Relative Offsets As We have read in another 512
	m_PartitionBootSector.iBytesPerSector =						// Offset 0x0B
		byData2[11] + 
	   (byData2[12] * 256);

	m_PartitionBootSector.iSectorsPerCluster = 	byData2[13];	// Offset 0x0D

	m_PartitionBootSector.iReservedSectors =					// Offset 0x0E
		byData2[14] + 
	   (byData2[15] * 256);

	//m_PartitionBootSector.chReserved = 0;						// OffSet 0x10

	//															// offset 0x13

	m_PartitionBootSector.iMediaDesc = byData2[21] ;			// offset 0x15

	//	always 0												// offset 0x16

	m_PartitionBootSector.iSectorsPerTrack =       		// 0x18
		byData2[24] + 
	   (byData2[25] * 256);

	m_PartitionBootSector.iNumOfHeads =           				// 0x1a
		byData2[26] + 
	   (byData2[27] * 256);

	m_PartitionBootSector.iHiddenSectors =						// 0x1C
		byData2[28] + 
	   (byData2[29] * 256) + 
	   (byData2[30] * 65536) + 
	   (byData2[31] * 16777216);


   // 0x20 - Not Used By NTFS 32,33, 34, 35
   // 0x24 - Not Used By NTFS 36,37, 38, 39

   m_PartitionBootSector.llTotalSectors =						// 0x28
		byData2[40] + 
	   (byData2[41] * 256) + 
	   (byData2[42] * 65536) + 
	   (byData2[43] * 16777216)+
	   (byData2[44] * 4294967296)+
	   (byData2[45] * 1099511627776)+
	   (byData2[46] * 281474976710656)+
	   (byData2[47] * 72057594037927936);
    

	m_PartitionBootSector.llLogicalClusterNumForMFT =			// $mft sector start
		byData2[48] + 
	   (byData2[49] * 256) + 
	   (byData2[50] * 65536) + 
	   (byData2[51] * 16777216)+
	   (byData2[52] * 4294967296)+
	   (byData2[53] * 1099511627776)+
	   (byData2[54] * 281474976710656)+
	   (byData2[55] * 72057594037927936) + iRelSector; 

	m_PartitionBootSector.llLogicalClusterNumForMFTMrr =			// $mft mirror start location
		byData2[56] + 
	   (byData2[57] * 256) + 
	   (byData2[58] * 65536) + 
	   (byData2[59] * 16777216)+
	   (byData2[60] * 4294967296)+
	   (byData2[61] * 1099511627776)+
	   (byData2[62] * 281474976710656)+
	   (byData2[63] * 72057594037927936) + iRelSector; 

	m_PartitionBootSector.iClustersPerMFTFileRecord =			// 0x40
		byData2[64] + 
	   (byData2[65] * 256) + 
	   (byData2[66] * 65536) + 
	   (byData2[67] * 16777216);

	m_PartitionBootSector.iClustersPerIndexBlock =				// 0x44
		byData2[68] + 
	   (byData2[69] * 256) + 
	   (byData2[70] * 65536) + 
	   (byData2[71] * 16777216);
	
	m_PartitionBootSector.llVolumeSerialNumber = (LONGLONG)				// 0x48
		byData2[72] + 
	   (byData2[73] * 256) + 
	   (byData2[74] * 65536) + 
	   (byData2[75] * 16777216)+
	   (byData2[76] * 4294967296)+
	   (byData2[77] * 1099511627776)+
	   (byData2[78] * 281474976710656)+
	   (byData2[79] * 72057594037927936);

	m_PartitionBootSector.iChecksum =						// 0x50
		byData2[80] + 
	   (byData2[81] * 256) + 
	   (byData2[82] * 65536) + 
	   (byData2[83] * 16777216);

	return GetMFTOffsets();
}


/*
	Walks The MFT To Find All Clusters That The MFT Uses
	
	The MFT_CLUSTERS structure will be added to inthe array until no more records need to be added
	the last record in this struct will have zeros and will mean that no more data exists.

	OS: NT/2000/XP
																								
*/
BOOL CDiskAnalysis::GetMFTOffsets(void)
{

	// TODO - Error Checking For NULL FILE
    __int64 lMFTSectorBytes = 0;
	DWORD dwErr;
	DWORD lpdRead;
	CString 	strTest, str;
	int	iMFTRecSize = 1;
	BOOL bSuccess;

    
	// TODO Could have opened another disk/vol/file
	// Using the Logical cluster number convert this to bytes for SetFilePointerEx
	lMFTSectorBytes = m_PartitionBootSector.llLogicalClusterNumForMFT *
					  m_PartitionBootSector.iBytesPerSector;			// Convert To Bytes
		
	LARGE_INTEGER li;
	li.HighPart =0;
	li.LowPart = 0;
	li.QuadPart = lMFTSectorBytes;
	dwErr = SetFilePointerEx(m_HndData, li, NULL , FILE_BEGIN);
	if(dwErr == 0)
	{
		dwErr = GetLastError();
		//TRACE1("\nFailed To Set File PointerEX - DiskAnalysis.cpp-GetMFTOffsets() Error:%i", dwErr);
		return false;
	}

	// Calculate Size Of one MFT Record
	iMFTRecSize = 
	(   (  this->m_PartitionBootSector.iClustersPerMFTFileRecord 
		 * this->m_PartitionBootSector.iSectorsPerCluster  )
	     * this->m_PartitionBootSector.iBytesPerSector );
	

	BYTE byData2[1007616];		// Temp Storage Of The Record
	// Read in one MFT Record (This Being The MFT record itself)
	bSuccess = ReadFile(m_HndData, byData2, iMFTRecSize, &lpdRead, NULL); //used to be iMFTRecSize not 1024
	if(!bSuccess)
	{
		dwErr = GetLastError();
		TRACE1("\nFailed To Get File Data - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}
	
	// As we need the value of Attribute ($DATA) we need to open the $AttrDef File to get
	// the value of $DATA - currently 0x80
	// CString strDataDef = "$DATA";
	// DWORD dwAttrDef = GetAttrDef(strDataDef);

	// Get The First Attribute Offset
	DWORD iOff = byData2[20];

	// We want to find attibute 0x80 which is the data attribute which has
	// the extents.

	DWORD dwAttrib;
	DWORD dwLength;
	DWORD dwOffsetSave, pdwLow = 0;;
	  
	for(;;)						
	{
		// Go To First Attribute
		dwAttrib = byData2[iOff];

		// What Attribute Have We Found.
		if(dwAttrib == 0x80)			// If data attribute
		{
			dwOffsetSave = iOff;							// Save Offset for later use

			int iDataRunOffset = byData2[32+iOff];			// iDataRunOffset will now have the offset of the start of the DRuns
															// Offset From Attribute Header 0x80 $DATA.

															// iOff has the Attribute Header (0x80 $DATA)Offset position
			int dwData = iOff + iDataRunOffset;				// Take the Start of the Attribute Header (0x80 $ DATA) and add the offset 
															// value of the data runs from the header and we end up with the 
															// position of the start of the extent runs in the MFT file.

		
			// Offset to the start of the data runs from the attribute header

			__int64 dwCalc =0, dwCalc2 =0, dwOffset =0;
			DWORD dwHigh =0, dwLow =0;
			int j=0,i=0, ks=0;
			MFT_CLUSTERS	*m_Ca, *m_Cab;
						
			while(byData2[dwData] > 0)
			{
					dwHigh = (byData2[dwData] & 15 );	// Highbits - Byte Length of run
					dwLow = (byData2[dwData] & 240);		// Low Bits - Byte Length of cluster
					dwLow = (dwLow >> 4);
					dwData +=  (dwHigh + dwLow + 1);

	//*******************************************************************************************************************************//
	//								EXTENT LENGTH
	// Get Length Of Run For This Extent we need to get the next dwHigh bytes
	// dwHigh = Length Of Extent Run for length of this extent data
	//*******************************************************************************************************************************//
 					
					//dwCalc2 = 0;
					//dwOffset = iOff + dwData + dwHigh;		// On Last Byte Now
					
					j = dwHigh-1;
					dwCalc2 = 0;
					for(i=0;i<=(int)dwHigh;i++)
					{
					    dwOffset = dwData - dwLow -1;
						if(byData2[dwOffset-i] > 0)
							dwCalc2 += byData2[dwOffset-i] * (DWORD)pow(256, j);
						
						j--;	
					}
					////TRACE1("\nLength Of Extent Run (In This Case Length Of This Section Of MFT) : 0x%X", dwCalc2 );

	//*******************************************************************************************************************************//
	//								EXTENT STARTING OFFSET
	// Get Cluster Offset Value
	// dwLow = Length Of Extent Run for cluster value offset
	//*******************************************************************************************************************************//
					
					pdwLow = dwLow;
					dwCalc = MFTReadVar(&byData2[dwData-pdwLow], pdwLow);

					// Add To Struct Array
					// Reallocate Storage to fit new record
					ks++;
					m_ClusterArray = (MFT_CLUSTERS*)realloc(m_ClusterArray, ks * sizeof(MFT_CLUSTERS) );
					
					
					if(ks > 1)  // Second or more record
					{	
						m_Ca = m_ClusterArray;	// Get New Starting Address For Bigger Array
						m_Ca = m_Ca + (ks-1);	// Move Pointer To Next New Record (0 Based Index (ks-1), ks start at 1 )
						m_Cab = m_ClusterArray; // Get New Address For Bigger Array	New Record
						m_Cab = (m_Ca-1);		// Set To last added record to get last details
                        m_Ca->iStartCluster = m_Cab->iStartCluster + (__int64)dwCalc;  // Fill In New Record
						m_Ca->iEndCluster = m_Ca->iStartCluster + (__int64)dwCalc2;	   // Fill In New Record
						m_Ca->iMFTRecSize =	 ((m_Ca->iEndCluster - m_Ca->iStartCluster) 
							/ this->m_PartitionBootSector.iClustersPerMFTFileRecord );
						
					}
					else	// Must Be First Record (Just Add The Start and len to get end cluster)
					{	
						m_Ca = m_ClusterArray;						// Point To First Record
						m_Ca->iStartCluster = (__int64)dwCalc;
						m_Ca->iEndCluster = (__int64)dwCalc2 + dwCalc;
						m_Ca->iMFTRecSize =	( (m_Ca->iEndCluster - m_Ca->iStartCluster )
							/ this->m_PartitionBootSector.iClustersPerMFTFileRecord );
					}
                    
					
			}// end while(dwData > 0)

			// Allocate Storage - Create One More Record With 0 to show end of data
			ks++;
			m_ClusterArray = (MFT_CLUSTERS*)realloc(m_ClusterArray, ks * sizeof(MFT_CLUSTERS) );
			m_Ca = m_ClusterArray;
			m_Ca += (ks-1);
			m_Ca->iEndCluster =0;
			m_Ca->iStartCluster =0;

			return true;
			break;				
		}	// end if Attribute = 0x80 ($DATA)
	
		// Not This One So Get The Length of this attribute ie to find start of next attrib
		dwLength = byData2[iOff+4];
		iOff += dwLength;
	}  // end for(;;)

	////TRACE0("\n");

	return TRUE;
}

/*
	Opens the $AttrDef file and returns the value of the requested attribute
	TODO : Unable To Open file with CreateFile() Error #5 - Access Denied.

	OS: NT/2000/XP

*/
DWORD CDiskAnalysis::GetAttrDef(CString strAttribute)
{
	HANDLE fhnd = NULL;
	DWORD dwErr = 0;
	DWORD dwFileSize = 0;

	fhnd = CreateFile("C:\\$AttrDef", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0,  0);

	if(fhnd == INVALID_HANDLE_VALUE)
	{
		dwErr = GetLastError();
		//TRACE1("\nFailed to open $AttrDef, Error Number : %i", dwErr);
		return 0;
	}

	dwFileSize = SetFilePointer(fhnd, 0, 0, FILE_END);

	dwErr = SetFilePointer(fhnd, 0, 0, FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		//TRACE1("Failed to set $AttrDef file pointer, Error Number : %i", dwErr);
		CloseHandle(fhnd);
		return 0;
	}
	
	DWORD lpbytes;
	BYTE lpByte[160];
	BOOL bSuccess;
	bSuccess = ReadFile(fhnd, lpByte, 160, &lpbytes, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		//TRACE1("Failed to read $AttrDef file , Error Number : %i", dwErr);
		CloseHandle(fhnd);
		return 0;
	}


	CString strLabel;
	int i = 0;
	for(;;)
	{
		for(int j=0;j<128;j++)	// 128 is the label size
		{
			if( isprint(lpByte[j]) )
				strLabel.AppendFormat( "%c", lpByte[j] );
		}
		
		strLabel.Trim();

		if( strLabel.Compare("$DATA") == 0 )		// Found It
		{
			return (DWORD)lpByte[0x80];
			break;
		}

		// Read in another record
		bSuccess = ReadFile(fhnd, lpByte, 160, &lpbytes, NULL);
		if(lpbytes == 0)  // EOF !!
		{
			CloseHandle(fhnd);
			//TRACE0("EOF in $AttrDef, before attribute found !!");
			return 0;
		}

		if(!bSuccess)
		{
			dwErr = GetLastError();
			//TRACE1("Failed to read $AttrDef file , Error Number : %i", dwErr);
			break;
		}
	}
	return 0;
}


/*
	
	Public called function, really a helper function to decide what
	to when the file and logical disk are passed to us
	We also need to check the type of FS installed on the volume
	it could be FAT/FAT32/NTFS..etc so we would need to call different functions.

	For NTFS - We want to setup
	OpenHandle(xXX) - using correct phydisk
	GetMBR() - Using correct disk
	GetNTFSPartitionTable - Using Correct Partition #
	GetMFTOffsets - This will use the partition table we open above

*/
int CDiskAnalysis::FindFile(FF_PARAMS* pParam, FILE_CLUSTERS* pFile)
{
	CString strFile;
	LPCTSTR strDisk;
	int iCnt = 0;

	strFile = pParam->strFileName;
	strDisk = pParam->strLogicalDisk;
    	
		

	m_FileDetails = pFile; // save passed mem space for FindFileMFT etc...
	int iDisk = (int)LogicalToPhysical(strDisk[0], CONVERT_TO_PHYSICALDISK);
	if(iDisk == -1)
	{
		AfxMessageBox(FINDFILE_UNKNOWNHANDLE,MB_ICONSTOP,0 );
		return 0;
	}

	char cString[] = "\\\\.\\PhysicalDrive ";
    char czBuf[2];
	_itoa(iDisk,czBuf, 10);
	czBuf[1] = '\0';
	cString[17] = czBuf[0];

	// open the disk 
	if(!OpenHandle(cString))
	{
		AfxMessageBox(FINDFILE_UNKNOWNHANDLE,MB_ICONSTOP,0 );
		return 0;
	}

	if( isalpha( strDisk[0] ) )		// Its a Logical Drive
	{			
		int iPart = LogicalToPhysical(strDisk[0], CONVERT_TO_PARTITION);	// return partition no from logical letter	
		int iOff = LogicalToPhysical(strDisk[0], CONVERT_TO_OFFSET);		// return start offset in bytes	fom logical
	

		DWORD dw = GetPartitionFormatType((iOff/512));		// convert to sector # and get the format type
  		switch (dw)				
		{
		case FORMAT_NTFS:
			GetNTFSPartitionTable(iPart);		// Get NTFS Partition\Volume Boot Sector Info
			iCnt = FindFileMFT(strFile, iPart, pParam);
			CloseDiskHandle();				    // dont need to keep this open !!
			return iCnt;
			

			return false;
			break;	

		case FORMAT_FAT32:
		case FORMAT_FAT16:
		case FORMAT_FAT12:
		case FORMAT_CDFS:
		default:
			break;
			} 
   
	}

	

	return 0;
}
/*
	 Find the specfied file in the MFT - Full path currently required in strFileName

	 OS : SetFilePointerEx is not supported in 95/98/Me

		 Windows NT/2000/XP: Included in Windows 2000 and later.
		 Windows 95/98/Me: Unsupported.


*/
int CDiskAnalysis::FindFileMFT(CString strFileName, int iPartition, FF_PARAMS* pParam)
{

	
	// Start at the first MFT extent and work through this until 
	// either the end of the MFT extent comes or we find the file,
	// when the end of the MFT extent comes, move on to the next one.
	// m_HndData - HANDLE to open disk/volume - Access to MFT
	// m_ClusterArray - MFT File Clusters\Extents (Last Record Has 0,0)
	// MFT Attributes - 0x30 $DATA and 0x80 $FILE_NAME
	DWORD dwErr =0, dwbyte, iRecSize =0, iOff = 0;
	BYTE *lpByte, *ptrByte;
	BYTE lpData[1024];
	BOOL bDeleted = false, bData = false;
	CString strLabel;
	LARGE_INTEGER itmp;
	CFileTime myFT1, myFT2;
	BOOL bMatch = false;

	int	iFoundCount=0;
	__int32	iRec=0;
	__int64 dwSeq, dwFRec, iClc =0, iOld=0, i=0;
	char *strFilePath;
	
	MFT_CLUSTERS	*m_Mc;			// Array defined that holds the clusters of the MFT
	m_Mc = m_ClusterArray;
	
	FILE_CLUSTERS	*m_Fc;			// Array this function will fill based on the file search for
	m_Fc = m_FileDetails;

    strFilePath = (char*)VirtualAlloc(
                           NULL, // next page to commit
                           MAX_PATH,         // page size, in bytes
                           MEM_COMMIT,         // allocate a committed page
                           PAGE_READWRITE);    // read/write access

    iRecSize = ( ( m_PartitionBootSector.iBytesPerSector * m_PartitionBootSector.iSectorsPerCluster )
					* m_PartitionBootSector.iClustersPerMFTFileRecord );

	dwErr = SetFilePointer(m_HndData, 0, 0, FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		//TRACE1("\nSet File Pointer Failed In FindFileMFT, Error %i", dwErr);
		return false;

	}
	
	while(m_Mc->iStartCluster > 0)			 // If zero we have the last data record  - m_Mc is the MFT cluster array created by GetMFTOffsets();
	{ 	
		// TODO : ?? Add 63 Which is the partition offset from begin of phy disk
		// assuming here that we opened a physical disk 
		//itmp.QuadPart = ((m_Mc->iStartCluster + m_HDPartitionTable[iPartition].dwRelativeSector) * 512);								
		itmp.QuadPart = ((m_Mc->iStartCluster +63) * 512);								// BUG: temp fix while redoing GetMBR Func
		// Move to the start of this MFT cluster extent
		dwErr = SetFilePointerEx(m_HndData, itmp, NULL , FILE_BEGIN);
		if(!dwErr)
		{ 
			dwErr = GetLastError();
			TRACE1("\nFailed to set file pointer, error: %i", dwErr);
			return false;
		}
		 
		for(i=0;i<m_Mc->iMFTRecSize;i++)		// loop through all MFT records in this extent
		{ 			
             	// Read in Next MFT record
				
				if(! ReadFile(m_HndData, lpData, iRecSize , &dwbyte, NULL) )
				{
					dwErr = GetLastError();
					//TRACE1("\nError in FindFile, Could not read file, Error :%i", dwErr);
					return false;
				}

				lpByte = lpData;

				if( *lpByte     != 'F' ||
					*(lpByte+1) != 'I' ||
					*(lpByte+2) != 'L' ||
					*(lpByte+3) != 'E')		// BAD Record
				{
					TRACE0("\nBad Record Found");
                    continue;
				}
				lpByte = lpData;		 	 
//*****************************************************************************************************************************
//				FIXUP's - The last two bytes of the sector are stored at 0x32, 0x33 form the start of the record
//						  and the replacement values are stored at 0x30, 0x31... 
//						  Basically we just replace 0x200, 0x199 with 0x32, 0x33 respectivly
//
//				TODO : This is a short and dirty fix which needs doing properly
//
//				@ offset 5 the value is the offset to where the fixup data is
//				@ offset 7 is the length of the fixup which seems to be 3 but including the first is 4
//
//
				*(lpByte+510) = *(lpByte+50);
				*(lpByte+511) = *(lpByte+51);
//****************************************************************************************************************************				
				iOff = GetMFTAttrOffset(lpByte, 0x30);		// $FILE_NAME
				if(iOff == 0)
					continue;		// bad record or doesnt contain FILE_NAME attribute
				
				lpByte += iOff;
				strLabel.Empty();
				for(int k=0;k<*(lpByte+88)*2 ;k++)	// UNICODE : *2, offset 0x40 has the label size
				{
					if( isprint(*(lpByte+k+90)) )   // UNICODE : Need to sort this as we are skipping unicode chars
						strLabel.AppendFormat( "%c", *(lpByte+k+90) );
				}

				if(IsStringMatchedW(strFileName, strLabel))
				{   					 
					if(*(lpData+21) == 0 && *(lpData+22) == 0)
						bDeleted = true;
					else
						bDeleted = false;

					if((bDeleted && pParam->bDeleted) || (!bDeleted && pParam->bActive))
					{   						  
	                  //  ASSERT((*(lpData+21) == 0 && *(lpData+22) == 0));

						m_Fc->iIsDeleted = bDeleted;
						m_Fc->iRecordNo = *(lpData+44) | (*(lpData+45) << 8) | (*(lpData+46) << 16);		// save mft recno	
						m_Fc->iPhyDiskNumber = m_iDiskOpen;
						m_Fc->iPartition = iPartition;

						iFoundCount++;  
						ASSERT((!strLabel.IsEmpty()));
						memcpy(m_Fc->strFileName, strLabel, MAX_PATH);
									   
						// parent record no	
						dwFRec = *(lpByte+24) | (*(lpByte+25) << 8) | (*(lpByte+26) << 16);
						// parent seq no
						dwSeq = *(lpByte+30) | (*(lpByte+31) << 8);
                        						
						GetFilePathMFT(dwFRec, dwSeq, strFilePath, iPartition);
						memcpy(m_Fc->strFolder, strFilePath, MAX_PATH);
						memset(strFilePath, '\0', MAX_PATH);

						lpByte = lpData; 

						// get the $DATA info
//****************************************************************************************************************************
						m_Fc->iFileLength = 0;
						iOff = GetMFTAttrOffset(lpByte, 0x80);		// $DATA
						// BUG: We only get the first $DATA offset here, could be more
						//      in the case of ADS !

						// if we get 0 back means that no attribute with 0x80 was found, therefore
						// we need to check for attribute 0x20 and see if any 0x80 attributes occur in there 
						// if they do we need to read them and find out where they are to open those MFT records
						// and read the data.
						
						if(iOff == 0)
						{	 
							iOff = GetMFTAttrOffset(lpByte, 0x20);		// $ATTRIBUTE_LIST
							if(iOff > 0)
							{
								BYTE* tByte;
								tByte = lpByte;
								tByte += iOff;
								DWORD dwMFTRecNo;
								__int64 iSector;
								
								int i=-1;
								while(iOff > 0)  // loop through and get all $DATA offsets in the $ATTRIBUTE_LIST attribute
								{   									
									iOff = GetMFTAttrOffset(tByte, 0x80,++i);	 // get $DATA from $ATTRIBUTE_LIST
									TRACE2("\nFile %s $80 is offset %i in $20",strLabel,  iOff);

									if(iOff > 0)
									{	
										// now do something with the offset  (ie get the MFT no)
									    // from offset 18 & 17 & 16  and read that MFT record to get extents from $DATA 		  
										dwMFTRecNo = *(tByte+iOff+16) | (*(tByte+iOff+17)<<8) | (*(tByte+iOff+18) << 16);
										iSector = CalcMFTRecord(dwMFTRecNo);
										TRACE2("   MFT Rec No 0x%X, Sector(i64) %i", dwMFTRecNo, iSector);
										PLARGE_INTEGER pLi;
										LARGE_INTEGER	Li;
										BYTE lpBuf[1024];
										DWORD lpBytes = 0;
										BYTE *ptByte;

										pLi = (LARGE_INTEGER*)VirtualAlloc(NULL, 1*sizeof(LARGE_INTEGER), MEM_COMMIT, PAGE_READWRITE);
										pLi->HighPart = 0; pLi->LowPart = 0; pLi->QuadPart = 0;
										Li.HighPart = 0; Li.LowPart = 0; Li.QuadPart = 0;


										//save current file pointer
									    SetFilePointerEx(m_HndData, Li, pLi, FILE_CURRENT);

										// set file pointer to new MFT record
										Li.QuadPart = iSector + (63 * 512);	    // TODO : 63 !!
										SetFilePointerEx(m_HndData, Li, NULL, FILE_BEGIN);
										ReadFile(m_HndData, lpBuf, 1024, &lpBytes, NULL);
										ptByte = lpBuf;
										/*                  FIXUP's						 */
										/*************************************************/
										*(ptByte+510) = *(ptByte+50);
										*(ptByte+511) = *(ptByte+51);
										/*************************************************/
										DWORD dwOffset = GetMFTAttrOffset(ptByte, 0x80);

										m_Fc->iNoExtents = GetFileExtents(lpBuf, dwOffset, m_Fc);

										// restore file pointer
										Li.QuadPart = pLi->QuadPart;
										SetFilePointerEx(m_HndData, Li, NULL, FILE_BEGIN);

										
									}
								}
							}
						 }
						
//****************************************************************************************************************************
					
						if(iOff > 0)
						{
							lpByte += iOff;
							m_Fc->iNoExtents = 0;
							if(*(lpByte+8) == 1)	   // Is The File Data Resident In The MFT Rec ?
							{	
								ptrByte = lpData;
								m_Fc->iNoExtents = GetFileExtents(ptrByte, iOff, m_Fc);
							}
							//else
								// TODO : need to return the MFT sector number with the offset start+end
							/*
								* If more than one $DATA record exists AND offset 0x09 > 0 
											 then we have a ADS $DATA record.
											 then we need to get offset 0x09 which gives us the location of
											 the ADS stream name and use (0x09)*2 for the length of the name
                                             
								* Then check offset 0x08, if this is 0 the data is resident and we then use
								  the value at offset 0x14 to get the offset to the start of the ADS resident data

							      If 0x08 is 1 (non resident) then get the Extent offset start from 0x20 and procees
								  the file extents as usual.

										

							*/

							/* at offset 
										'the name' described here can be the 
										name of the ADS stream or the start of the data for 
										a normal resident file, for example a text file with 
										data 'this is the file data' (incl quotes) in a normal 
										resident file will begin at this offset (0x10), for a ADS
										file at this offset you will find the name of the data
										stream. The length of the name (0x09) states the length
										of the name BUT the name has spaces so if the length was
										4 and the name was xray, this is stored as
										x r a y where the spaces are 0x00 (NULL).

								   {$DATA Header}
								        0x04 - This attribute length, length from 0x80 to one past last byte
										0x08 - Resident flag 1=Non Resident 0=Resident
										0x09 - This is the length of the name  (0 for normal resident files)
										0x0A - this is the offset to the name from 0x80
										       1. Start of the data for normal resident file
											   2. Start of the name of the ADS stream for ADS files     
									    0x0E - Instance Number

								   {Resident Form/Data}
										0x14 - For ADS / Normal Resident This gives offset to start of data
										       if this is a normal resident file this value will be the same as 0x10

								   {Non Resident Form/Data}
								        For ADS Data Extents ( the same as normal extents)
										0x20 has the offset from the start of the $DATA attribute 0x80

										
									   


									    

							*/
						}

						if(strlen(pParam->strLogicalDisk) > 0)
						memcpy(m_Fc->strLogicalDisk, pParam->strLogicalDisk, sizeof(m_Fc->strLogicalDisk));
							else
						m_Fc->strLogicalDisk[0] = '\0';	   //todo

						m_Fc ++;

					}
				}
 //****************************************************************************************************************************							
		} 
 		m_Mc++;		   // Move to next record
	}	
	return iFoundCount; 
}

/***************************************************************************************

							Misc/Helper Functions 

*****************************************************************************************/
/*
	Formatting For Drive Format Type 
	Called From GetMBR()

	OS: Not OS Specific

*/
DWORD CDiskAnalysis::GetFormatTypeText(DWORD dwType)
{

	for(int i=0; i<sizeof(FormatTypes); i++)
	{
		if(FormatTypes[i].dwType == dwType)
			return FormatTypes[i].iIndex;

	}

	return 0;		// default 'Unknown'
}

/*
				Get Physical Disks Installed On This System
*/
BOOL CDiskAnalysis::GetPhysicalDisks(void)
{
    char cTemp[255];
	memset(cTemp, '0', 255);				

	char mydata[65535];
	mydata[65534] = '\n';

	PHYSICAL_DISKS	*m_phy;


	DWORD dw = ::QueryDosDevice(NULL, mydata, sizeof(mydata)) ;
	if(dw == 0)
	{
		dw = GetLastError();
		//TRACE1(L"Error %lu querying \n", dw);
	}

	//if(dw >= sizeof(mydata)-1)
	//{ //TRACE0("Buffer Too Small");}
	
	int j=0;
	int p=0;
	for(int k=0;k<65535;k++)
	{
		if(mydata[k] == NULL && mydata[k+1] == NULL)
			break;												// End Of File

		cTemp[j] = mydata[k];
		j++;
			if(mydata[k] == NULL)
			{ 
				cTemp[j+1] = '\0';
				if((strnicmp("PhysicalDrive", cTemp, 13) == 0|| 
				strnicmp("CdRom", cTemp, 5) == 0) )					// Found Physical Media
				{
					p++;
					m_physicaldisks = (PHYSICAL_DISKS*)realloc(m_physicaldisks, p * sizeof(PHYSICAL_DISKS) );
					m_phy =	 m_physicaldisks;
					m_phy += p-1;
					memcpy(m_phy->cName, cTemp, sizeof(cTemp) );
				}
				
				j=0;
				memset(cTemp, '0', 255);				
			}
	}
	
	p++;
	m_physicaldisks = (PHYSICAL_DISKS*)realloc(m_physicaldisks, p * sizeof(PHYSICAL_DISKS) );
	m_phy =	 m_physicaldisks;
	m_phy += p-1;
	memset(m_phy->cName, '\0', 1);
    
	GetLogicalDisks();						// This Function Requires Us First, So Call It Now Anyway !!!

	return 0;
}

/*

			Get Logical Disks On This System

			** GetPhysicalDisks() must be called first !!!

*/

BOOL CDiskAnalysis::GetLogicalDisks(void)
{
	
	char temp[] = "  Drive";
	char cDrive[] = " :\\";
	DWORD dw2 = GetLogicalDrives();				// call function to pass back an array of log disks
	DWORD dwDisk = 1; 
	int p=0;
	LOGICAL_DISKS	*m_log;

	if(dw2 == 0) // Failed To Get Logical Disks
		{return false;}

	int j=0;
	for(int i=0; i<26;i++)
	{
		if(dw2 & dwDisk)
		{
			// Add To The Selection
 			p++;
			m_logicaldisks = (LOGICAL_DISKS*)realloc(m_logicaldisks, p * sizeof(LOGICAL_DISKS) );
			m_log =	 m_logicaldisks;
			m_log += p-1;
			cDrive[0] = tabAlpha[i];
			memcpy(m_log->cName, cDrive, sizeof(cDrive) );
		}
		dwDisk = (dwDisk * 2);
	}


	p++;
	m_logicaldisks = (LOGICAL_DISKS*)realloc(m_logicaldisks, p * sizeof(LOGICAL_DISKS) );
	m_log =	 m_logicaldisks;
	m_log += p-1;
	memset(m_log->cName, NULL, 1);	

	// Now we have the logical disks, we need to fill in the phy disk details in the array
	m_log = m_logicaldisks;			// Reset To The Start
    
	HANDLE	hFile;
	ULONG	bytes;
	UCHAR	bdExtents[0x400];
	PVOLUME_DISK_EXTENTS dExtents = (PVOLUME_DISK_EXTENTS)bdExtents;
	char tDrive[] = "\\\\.\\ :";


	while(strlen(m_log->cName) > 0)
	{
			tDrive[4] = m_log->cName[0];					
			hFile = CreateFile( tDrive,
							GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
							NULL, OPEN_EXISTING, 0, NULL );

			int iPart = 0;
			if( hFile != INVALID_HANDLE_VALUE )
			{
				// get extents
				BOOL bSuccess = DeviceIoControl( hFile, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0,
												dExtents, sizeof(bdExtents), &bytes, NULL ); 
				if(bSuccess)
				{
					m_log->iDisk = dExtents->Extents[0].DiskNumber;
					m_log->StartingOffset = dExtents->Extents[0].StartingOffset;
					m_log->iPartitionNo = iPart;
					iPart++;
				}
				else
				{
				   m_log->iDisk = -1;
				   m_log->StartingOffset.HighPart =0;
				   m_log->StartingOffset.LowPart =0;
				   m_log->StartingOffset.QuadPart =0;
				   m_log->iPartitionNo = -1;
				}

			}
			//else
			//{//TRACE1("\nError getting extents for %C: drive.", m_log->cName );}
			CloseHandle(hFile);

		m_log++;
		
	}


	return 0;
}






/*
	Find the full path of a file using the files Parent Directory Attribute
	which points to the MFT record # (sequence no)
*/
void CDiskAnalysis::GetFilePathMFT(__int64 dwParentRecord, __int64 dwSeq, char* ptrFilePath, int iPartition)
{

	PLARGE_INTEGER li;
	LARGE_INTEGER liFP;
	DWORD iOff =0, lpdBytes = 0;
	__int64  dwTemp = 0, dwlSeq=0;;
	int iCnt=0, iLen=0;
	BYTE *lpByte;
	BYTE lpBytes[1024];
	CString strStr, strTmp;
	

	ASSERT(m_HndData != NULL);		// Make sure we have a valid File Handle

	strStr.Empty();
	strTmp.Empty();
	
	li = (LARGE_INTEGER*)VirtualAlloc(NULL, 1*sizeof(LARGE_INTEGER), MEM_COMMIT, PAGE_READWRITE);
	li->QuadPart =0;
	li->LowPart = 0;
	li->HighPart = 0;
	liFP.QuadPart = 0;
	liFP.HighPart = 0;
	liFP.LowPart = 0;
	BOOL bSuccess = SetFilePointerEx(m_HndData, liFP, li, FILE_CURRENT);	// Save Current File Pointer
	if(!bSuccess)
	{
		AfxMessageBox(FILE_POINTER_SAVE_FAILED, 0,0);
		return;
	}

	while(true)	
	{
		liFP.QuadPart = CalcMFTRecord(dwParentRecord);	  // find location of record on disk
		//liFP.QuadPart += (m_HDPartitionTable[iPartition].dwRelativeSector * 512);
		liFP.QuadPart += (63 * 512); // BUG : whle fixing GetMBR
		
		bSuccess = SetFilePointerEx(m_HndData, liFP, NULL, FILE_BEGIN);
		bSuccess = ReadFile(m_HndData, lpBytes, 1024 , &lpdBytes, NULL);			// TODO Error Checking	& NTFS MFT Record Size(1024)
		
		lpByte = lpBytes;
		// Get The Record No.
		dwTemp = *(lpByte + 44) | (*(lpByte + 45) << 8) | (*(lpByte + 46) << 16);
		// Get The Sequence No
		dwlSeq = *(lpByte + 16) | (*(lpByte + 17) << 8) ;

		ASSERT(dwTemp == dwParentRecord);
		//if(dwTemp == dwParentRecord)  	// make sure it is the parent record
		//{
		if((dwlSeq > dwSeq) || (!( (*(lpByte + 22) & 3) == 3)))	 // if seq is greater then the parent record has been reused
		{													 // or record does not describe a directory
			strStr = "\\<orphan>";							 // for deleted files any wierd combo could happen
			break;								
		}

		iOff = GetMFTAttrOffset(lpByte, 0x30);			// $FILE_NAME
		lpByte += iOff;

		// Get The Name Of The Dir
		iLen = (*(lpByte+88)*2);			// Len of Dir/FileName
 		if(*(lpByte+90) ==  '.')			// Root Dir
			break;
		
		strTmp = strStr;
		strStr.Empty();
		strStr.AppendFormat("\\");
		for(int j=0;j<iLen;j++)
		{
			if(isprint( *(lpByte+90+j) ))
				strStr.AppendFormat("%c", *(lpByte+90+j) );	// Get File/Dir Name Into a string
		}
  		strStr.AppendFormat("%s", strTmp);
		// parent record no	
		dwParentRecord = *(lpByte+24) | (*(lpByte+25) << 8) | (*(lpByte+26) << 16);
		// parent seq no
		dwSeq = *(lpByte+30) | (*(lpByte+31) << 8);
		//}
	} // end while(true)	

	
	LPCSTR tmp = (LPCSTR)strStr;
	memcpy(ptrFilePath, tmp, strStr.GetLength());

	liFP.QuadPart = li->QuadPart;
	liFP.HighPart = li->HighPart;
	liFP.LowPart = li->LowPart ;
	SetFilePointerEx(m_HndData, liFP, NULL, FILE_BEGIN);		// Restore Saved File Pointer

	return;
}


/*

	Recover file from a partition to czRecoverPath

*/

BOOL CDiskAnalysis::RecoverFile(FILE_CLUSTERS *pFile)
{
	BOOL bSuccess = false;

	// Open phy disk that the file is on
	// Recover file doesnt use the MFT as it is told what extents the file is in 
	// from when find file was called
	char cString[] = "\\\\.\\PhysicalDrive ";
	char czBuf[2];
	_itoa(pFile->iPhyDiskNumber ,czBuf, 10);
	czBuf[1] = '\0';
	cString[17] = czBuf[0];

	// open the disk 
	if(!OpenHandle(cString))
	{
		AfxMessageBox(FINDFILE_UNKNOWNHANDLE,MB_ICONSTOP,0 );
		return false;
	}
	

  
//	if(pFile->iFileLength < 1)
//		pFile->iFileLength =  (pFile->sFileExtents->iFileExtentEndCluster - pFile->sFileExtents->iFileExtentStartCluster);

	//bSuccess = RecoverFileMFT(
	//				pFile->sFileExtents->iFileExtentStartCluster, 
	//				pFile->iFileLength, 
	//				pFile->strRecoverPath
	//				);

	bSuccess = RecoverFileMFT(pFile);

	return bSuccess;
}

//BOOL CDiskAnalysis::RecoverFileMFT(__int64 iStartSector, __int64 iFileSize, char* czRecoverPath)
BOOL CDiskAnalysis::RecoverFileMFT(FILE_CLUSTERS *pFile)
{
  
	ASSERT(m_HndData != NULL);		// Use the currently open handle for the volume/partition

	BOOL bSuccess = false, bBreak = false;
	LARGE_INTEGER	liOffset;
	DWORD			dwErr=0, dwRead=0, dwWrite=0;
	BYTE			lpByte[512];
	HANDLE			hndNewFile = NULL;
	int				iBytesToRead=0;
  __int64			iTtlBytesRead=0;
	int				iBWritten = 0;
	

  if(pFile->iFileLength == 0)
	  return false;

  FILE_EXTENT*	pExtents;
  pExtents = pFile->sFileExtents;
	
	liOffset.HighPart = 0;
	liOffset.LowPart = 0;
	//liOffset.QuadPart =	 (iStartSector+63)  * 512;		// TODO : Bytes per sector 512 and 63
	liOffset.QuadPart = (pExtents->iFileExtentStartCluster+63) * 512; 

	bSuccess = SetFilePointerEx(m_HndData, liOffset, NULL, FILE_BEGIN);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		//TRACE1("\nFailed to set file pointer in RecoverFileMFT Error : %i", dwErr);
		return false;
	}

	hndNewFile = CreateFile(pFile->strRecoverPath, 
							GENERIC_WRITE, 
							0,
							NULL,
							CREATE_NEW,
							FILE_ATTRIBUTE_ARCHIVE,
							NULL);

	if(hndNewFile == NULL)
	{
		dwErr = GetLastError();
		//TRACE1("\nError creating new file in RecoverFile : Error %i", dwErr);
		return false;
	}

	liOffset.HighPart = 0;
	liOffset.LowPart = 0;
	// Loop up to iFileSize
	//while(pExtents->iFileExtentStartCluster>0)
	for(int x=0;x<pFile->iNoExtents;x++)
	{
		liOffset.QuadPart = (pExtents->iFileExtentStartCluster+63) * 512; 
		bSuccess = SetFilePointerEx(m_HndData, liOffset, NULL, FILE_BEGIN);
		if(!bSuccess)
		{
			TRACE0("\nFailed to set filepointer");
			ASSERT(false);
		}

			// do loop for # of clusters in this extent
			for(int i=-1;i<pExtents->iFileExtentEndCluster-pExtents->iFileExtentStartCluster;i++)
			{
				bSuccess = ReadFile(m_HndData, lpByte, 512, &dwRead, NULL);
				if(!bSuccess)
				{
					dwErr = GetLastError();
					//TRACE1("\nFailed to read data in Recover File : Error %i", dwErr);
					return false;
				}


				if( *(lpByte+3) == 'N' &&			  //3
					*(lpByte+4) == 'T' &&
					*(lpByte+5) == 'F' &&
					*(lpByte+6) == 'S')		
				{
					TRACE0("\nBad Record Found");
					continue;
				}


				if( (pFile->iFileLength - iTtlBytesRead) >= 512)
				{
					//TRACE1("\nTotal File Length : %i", pFile );
					//TRACE1("\nTotal Bytes Read  : %i", iTtlBytesRead);
					dwWrite = 512;
					//TRACE1("\nWriting Bytes     : %i", dwWrite);
				}
				else
				{
					//TRACE1("\nTotal File Length : %i", pFile->iFileLength );
					//TRACE1("\nTotal Bytes Read  : %i", iTtlBytesRead);

					dwWrite = (DWORD)(pFile->iFileLength - iTtlBytesRead);

					//TRACE1("\nWriting Bytes     : %i", dwWrite);
					bBreak = true;
				}

				iTtlBytesRead += dwRead;			// TTL Bytes Read
				iBWritten += dwWrite;
				//TRACE1("\nTotal Bytes Written  : %i",iBWritten );

				// Add to new file
				bSuccess = WriteFile(hndNewFile, lpByte, dwWrite, &dwRead, NULL); 
				if(!bSuccess)
				{
					dwErr = GetLastError();		  
					TRACE1("\nFailed to write data in Recover File : Error %i", dwErr);
					::FlushFileBuffers(hndNewFile);
					CloseHandle(hndNewFile);
					return false;
				}

			}// end if 
		//if(bBreak)
			//x = pFile->iNoExtents+2;

		pExtents++   ;
	}	  
	::FlushFileBuffers(hndNewFile);
    CloseHandle(hndNewFile);
    
	return true;
}

/*
	use the starting offset from the start of the disk, go to that offset and 
	check the boot partition format against some knowns
	Bytes 3-10    OEM name/version (E.g. "IBM  3.3", "MSDOS5.0", "MSWIN4.0".
				  Various format utilities leave their own name, like "CH-FOR18".
				  Sometimes just garbage. Microsoft recommends "MSWIN4.1".)
*/

DWORD CDiskAnalysis::GetPartitionFormatType(__int64 iPartitionStartSector)
{
	PLARGE_INTEGER li;
	LARGE_INTEGER liFP, liSector;
	DWORD dwRetVal = FORMAT_NONE;

	li = (LARGE_INTEGER*)VirtualAlloc(NULL, 1*sizeof(LARGE_INTEGER), MEM_COMMIT, PAGE_READWRITE);
	li->QuadPart =0;
	li->LowPart = 0;
	li->HighPart = 0;
	liFP.QuadPart = 0;
	liFP.HighPart = 0;
	liFP.LowPart = 0;

	BOOL bSuccess = SetFilePointerEx(m_HndData, liFP, li, FILE_CURRENT);	// Save Current File Pointer
	if(!bSuccess)
		return FORMAT_ERROR;

	
	liSector.HighPart = 0;
	liSector.LowPart = 0;
	liSector.QuadPart = (iPartitionStartSector * 512);
	bSuccess = SetFilePointerEx(m_HndData,liSector , NULL, FILE_BEGIN);
	if(!bSuccess)
		return FORMAT_ERROR;


    // we are now at the part boot sector - read in 512b
	DWORD lpd = 0;
	BYTE lpBuf[512];
	bSuccess = ReadFile(m_HndData, lpBuf, 512, &lpd, NULL);
	if(!bSuccess)
	{
		// restore saved file pointer
		liFP.QuadPart = li->QuadPart;
		liFP.HighPart = li->HighPart;
		liFP.LowPart = li->LowPart ;
		SetFilePointerEx(m_HndData, liFP, NULL, FILE_BEGIN);		// Restore Saved File Pointer
		return FORMAT_ERROR;
	}

	// Now check for known format types
	// Currently :
	/*
	NTFS			
		0x00 = "EB"
		0x02 = "90"
		0x03 = "4E 54 46 53"	 =	NTFS
		0x01FE = " 55 AA"
	
	FAT16
		0x00 = "EB"		JMP instruction
		0x02 = "90"		JMP instruction
		0x01FE "55 AA"	Signature
		
	FAT32
		0x02 = "90"
		0x52 = "46 41 54 33 32"  = FAT32 at offset 52
		0x01FE = "55 AA"

	CDFS
	  ISO 9660
	  JOLIET
	  Micro-UDF

	*/
	
	if(lpBuf[0x00] == 0xeb && 
	   lpBuf[0x02] == 0x90 &&		
	   lpBuf[0x01fe] == 0x55 &&
	   lpBuf[0x01ff] == 0xaa)
		dwRetVal =  FORMAT_FAT16;

	if(lpBuf[0x00] == 0xeb && 
	   lpBuf[0x02] == 0x90 &&	
  	   lpBuf[0x36] == 0x46 &&		// F
	   lpBuf[0x37] == 0x41 &&		// A
	   lpBuf[0x38] == 0x33 &&		// T
	   lpBuf[0x39] == 0x31 &&		// 1
	   lpBuf[0x3a] == 0x32 &&		// 2
  	   lpBuf[0x01fe] == 0x55 &&
	   lpBuf[0x01ff] == 0xaa)
		dwRetVal =  FORMAT_FAT12;


	if(lpBuf[0x0] == 0xeb && 
	   lpBuf[0x2] == 0x90 &&
	   lpBuf[0x03] == 0x4e &&		// N
	   lpBuf[0x04] == 0x54 &&		// T
	   lpBuf[0x05] == 0x46 &&		// F
	   lpBuf[0x06] == 0x53 &&		// S
	   lpBuf[0x01fe] == 0x55 &&
	   lpBuf[0x01ff] == 0xaa)
		dwRetVal = FORMAT_NTFS;

	if(lpBuf[0x2] == 0x90 && 
	   lpBuf[0x52] == 0x46 &&		// F
	   lpBuf[0x53] == 0x41 &&		// A
	   lpBuf[0x54] == 0x54 &&		// T
	   lpBuf[0x55] == 0x33 &&		// 3
	   lpBuf[0x06] == 0x52 &&		// 2
	   lpBuf[0x01fe] == 0x55 &&
	   lpBuf[0x01ff] == 0xaa)
		dwRetVal = FORMAT_FAT32;




	// restore saved file pointer
	liFP.QuadPart = li->QuadPart;
	liFP.HighPart = li->HighPart;
	liFP.LowPart = li->LowPart ;
	SetFilePointerEx(m_HndData, liFP, NULL, FILE_BEGIN);		// Restore Saved File Pointer

	return dwRetVal;
}

/*
  Public called function to get the Partition info filled in in the structs 
  for the selcted parition no.
  NTFS -  GetNTFSPartitionInfo(x) - PARTITION_TABLE_NTFS
  FAT12
  FAT16
  FAT32

*/
BOOL CDiskAnalysis::GetPartitionBootSectorInfo(int iPartitionNumber)
{
	DWORD dwType = FORMAT_NONE;

	// use currently open logical disk
	if(iPartitionNumber == -1)
	{
		// check that we have a logical disk open
		ASSERT((m_HandleType & ~OPEN_LOGICAL|OPEN_VOLUME) == 0);
		ASSERT(m_HndData == NULL);

		// get the format type - use offset 0 as we have a open volume
		// and the offset will be 0 ie beginning of the volume
		dwType = GetPartitionFormatType(0);
		switch (dwType)
		{
		case FORMAT_NTFS:
			// GetNTFSPartitionInfo	 ? 
			break;

		case FORMAT_FAT12:
			// GetFATInfo ?
			break;

		case FORMAT_FAT16:
			break;

		case FORMAT_FAT32:
			break;
        
		default:
			 break;
		}


	}







	




	return 0;
}

__int64 CDiskAnalysis::LogicalToPhysical(char cDriveLetter, DWORD dwRetType)
{               		
		LOGICAL_DISKS *m_log;
		m_log = m_logicaldisks;
		while(strlen(m_log->cName) > 0)
		{
			if(m_log->cName[0] == cDriveLetter)				// Found the array member with the letter spec'd
			{
				if(dwRetType == CONVERT_TO_PARTITION)
					return (__int64)m_log->iPartitionNo;
				
				if(dwRetType == CONVERT_TO_OFFSET)
					return (__int64)m_log->StartingOffset.QuadPart;

				if(dwRetType == CONVERT_TO_PHYSICALDISK)
					return (__int64)m_log->iDisk;
			}
					
			
			m_log++;
        }


	return -1;			// Not found
}

/*
  Public function called, just reformats the disk name and 
  calls openhandle() and returns result
  structs LOGICALDISK, PHYSICALDISK, HD_PartitionTable will be filled after success
*/
BOOL CDiskAnalysis::FillPartitionInfo(LPCTSTR lpPhysicalDiskName)
{

	CString tmpStr = "\\\\.\\";
	tmpStr.AppendFormat("%s", lpPhysicalDiskName);	

	BOOL bSuccess = OpenHandle(tmpStr);
	CloseDiskHandle();			


	return bSuccess;
}


/*
  close file handle and clean up member variables.

*/
void CDiskAnalysis::CloseDiskHandle(void)
{
	CloseHandle(m_HndData);

	m_HndData = NULL;			// disk/file handle
	//m_iSize = 0;				// not currently used
	m_iDiskOpen = -1;			// no open disk
	m_HandleType = OPEN_NONE;	// no open type
}



/****************************************************************************************************
   mailto:yuantuh@techsim.com.au
****************************************************************************************************/
BOOL CDiskAnalysis::IsStringMatchedW(CString czWild, CString str)
{
	LPCSTR wc = czWild.MakeLower();
	LPCSTR st = str.MakeLower();

	BOOL bSuccess = IsStringMatched((char*)wc, (char*)st);

	return bSuccess;
}


BOOL CDiskAnalysis::IsStringMatched(char* Wildcards, char* str)
{

	bool Yes = 1;

	//iterate and delete '?' and '*' one by one
	while(*Wildcards != '\0' && Yes && *str != '\0')
	{
		if (*Wildcards == '?') str ++;
		else if (*Wildcards == '*')
		{
			Yes = Scan(Wildcards, str);
			Wildcards --;
		}
		else
		{
			Yes = (*Wildcards == *str);
			str ++;
		}
		Wildcards ++;
	}
	while (*Wildcards == '*' && Yes)  Wildcards ++;

	return Yes && *str == '\0' && *Wildcards == '\0';

}

BOOL CDiskAnalysis::Scan(char*& Wildcards, char*& str)
{
		// remove the '?' and '*'
	for(Wildcards ++; *str != '\0' && (*Wildcards == '?' || *Wildcards == '*'); Wildcards ++)
		if (*Wildcards == '?') str ++;
	while ( *Wildcards == '*') Wildcards ++;
	
	// if str is empty and Wildcards has more characters or,
	// Wildcards is empty, return 
	if (*str == '\0' && *Wildcards != '\0') return false;
	if (*str == '\0' && *Wildcards == '\0')	return true; 
	// else search substring
	else
	{
		char* wdsCopy = Wildcards;
		char* strCopy = str;
		bool  Yes     = 1;
		do 
		{
			if (!Match(Wildcards, str))	strCopy ++;
			Wildcards = wdsCopy;
			str		  = strCopy;
			while ((*Wildcards != *str) && (*str != '\0')) str ++;
			wdsCopy = Wildcards;
			strCopy = str;
		}while ((*str != '\0') ? !Match(Wildcards, str) : (Yes = false) != false);

		if (*str == '\0' && *Wildcards == '\0')	return true;

		return Yes;
	}
}

bool CDiskAnalysis::Match(char* Wildcards, char* str)
{
	bool Yes = 1;

	//iterate and delete '?' and '*' one by one
	while(*Wildcards != '\0' && Yes && *str != '\0')
	{
		if (*Wildcards == '?') str ++;
		else if (*Wildcards == '*')
		{
			Yes = Scan(Wildcards, str);
			Wildcards --;
		}
		else
		{
			Yes = (*Wildcards == *str);
			str ++;
		}
		Wildcards ++;
	}
	while (*Wildcards == '*' && Yes)  Wildcards ++;

	return Yes && *str == '\0' && *Wildcards == '\0';
}


/*
  Thanks to Matthew A. Taylor for the following code.
  mailto:para@cfl.rr.com 
  http://my.fit.edu/~mtaylor/
*/
__int64 CDiskAnalysis::MFTReadVar(BYTE *pSrc, DWORD nLen)		   //(0x11 0x04 0x93), length of the offset bytes (1) 
{
	__int64 x;							 // *pSrc points to the first byte in the cluster start offset 
										// in this example it will point to 0x93 if a seconf byte was afer 0x93 it would still point to 0x93
	if (nLen == 0)
		return 0;

	if (pSrc[nLen - 1] & 0x80)			// if last byte of the data run for
		x = -1;							// the sector offset is  &  0x80=128 (ie less than 128d) x=-1
	else
		x = 0;

	pSrc += nLen;						// pSrc == 0x11, + nLen(1), will now point to 0x93
	while(nLen-- > 0)
	{								    // x = -1 = 0xFFFFFFFFFFFFFFFF (16F's)
		x <<= 8;						// x = 0xFFFFFFFFFFFFFF (14F's)
		x += *--pSrc;					// x = x + (--pSrc)=0x93   - (0xFFFFFFFFFFFFFF + 0x93)
	}

	return x;
}



/*
   returns the requested MFT record no data in byData
   TODO : cDrive - hardcoded c: at present
*/
BOOL CDiskAnalysis::GetMFTRecord(char cDrive, int iRecordNo, BYTE* byData, int iDataSize)
{

	ASSERT(m_HndData == NULL);		// must have no open file on this handle 

	if(!OpenHandle("\\\\.\\c:"))
		return false;


	if(!GetNTFSPartitionTable(-1))		// Also calls GetMFTOffsets()
		return false;

	// now we have the struct MFT_CLUSTERS filles with a member variable : m_ClusterArray

	// calculate the offset of the record (all recnos are in sequence thankfully !!)
	DWORD dwBytesPerMFTRec = ((m_PartitionBootSector.iClustersPerMFTFileRecord * 
							  m_PartitionBootSector.iSectorsPerCluster) *
							  m_PartitionBootSector.iBytesPerSector) ;

	MFT_CLUSTERS* m_Ca = m_ClusterArray;
	DWORD dwCount = 0, dwRecCount = 0;
	DWORD dwExtentNo=0;
	__int64 iSector = 0;
	while(m_Ca->iStartCluster > 0)
	{
		dwCount += m_Ca->iMFTRecSize;

		if(iRecordNo <= dwCount)
		{
			dwRecCount = (dwCount - m_Ca->iMFTRecSize);		// total recs before this extent
			iSector = (iRecordNo - dwRecCount);				// number of records offset from start of this extent
			iSector = ((iSector * m_PartitionBootSector.iClustersPerMFTFileRecord) * m_PartitionBootSector.iSectorsPerCluster);
			iSector += m_Ca->iStartCluster;
         	break;
		}
		else
		{
			m_Ca++;
			dwExtentNo++;
		}
	}

	if(iSector == 0)
		return false;			// didnt find the requested record.


	LARGE_INTEGER li;
	li.HighPart = 0;
	li.LowPart = 0;
	li.QuadPart = (iSector * m_PartitionBootSector.iBytesPerSector);
	SetFilePointerEx(m_HndData, li, 0, FILE_BEGIN);
	DWORD lpRead=0;
	ReadFile(m_HndData, byData, dwBytesPerMFTRec, &lpRead, NULL);

	//DWORD dwDataOffset = GetMFTAttrOffset(byData, 0x80);


					    
	return 0;
}

DWORD CDiskAnalysis::GetMFTAttrOffset(BYTE* byMFTRecord, DWORD dwAttribute, int iCount)
{
	BYTE* lp = byMFTRecord;
	int iOff = 0, iAttCount = -1;
	int	iCnt = *(lp+20);		 // offset 20 from the start of the MFT record states where the first Attribute is
	lp += *(lp+20);				 // move to the first attribute

	while( *lp != 0xFF )	  
	{   						
		if(*lp == dwAttribute)		 	// what  is this attribute ?, is it what we want ?
		{	
			if((++iAttCount) == iCount)
				return iCnt;
		}

		iOff = ( *(lp+4) + (*(lp+5) << 8) );	 // uses two bytes swapped so bit shift the last byte
		iCnt += iOff;				// offset 4 from the start of each attribute is the length of this attribute
		lp +=  iOff;					// so we move that distance to the start of the next attribute.
	}


	return 0;
}

__int64 CDiskAnalysis::CalcMFTRecord(DWORD dwRecNo)
{
	// calculate the offset of the record (all recnos are in sequence thankfully !!)
	MFT_CLUSTERS* m_Ca = m_ClusterArray;
	DWORD dwCount = 0, dwRecCount = 0;
	DWORD dwExtentNo=0;
	__int64 iSector = 0;

	while(m_Ca->iStartCluster > 0)
	{
		dwCount += m_Ca->iMFTRecSize;

		if(dwRecNo <= dwCount)
		{
			dwRecCount = (dwCount - m_Ca->iMFTRecSize);		// total recs before this extent
			iSector = (dwRecNo - dwRecCount);				// number of records offset from start of this extent
			iSector = ((iSector * m_PartitionBootSector.iClustersPerMFTFileRecord) * m_PartitionBootSector.iSectorsPerCluster);
			iSector += m_Ca->iStartCluster;
         	break;
		}
		else
		{
			m_Ca++;
			dwExtentNo++;
		}
	}

	if(iSector == 0)
		return 0;			// didnt find the requested record.


	return (iSector * m_PartitionBootSector.iBytesPerSector);			
}



/*

int CDiskAnalysis::FindFileMFT(CString strFileName, int iPartition, FF_PARAMS* pParam)
{

	
	// Start at the first MFT extent and work through this until 
	// either the end of the MFT extent comes or we find the file,
	// when the end of the MFT extent comes, move on to the next one.
	// m_HndData - HANDLE to open disk/volume - Access to MFT
	// m_ClusterArray - MFT File Clusters\Extents (Last Record Has 0,0)
	// MFT Attributes - 0x30 $DATA and 0x80 $FILE_NAME
	DWORD dwErr =0, dwbyte, dwAttrib, iRecSize =0, iOff = 0;
	BYTE lpByte[1024];
	BYTE *ptrByte =lpByte;
	BOOL bDeleted = false, bData = false, bFound = false;
	CString strLabel;
	LARGE_INTEGER itmp;
	PLONG ihigh;
	CFileTime myFT1, myFT2;
	BOOL bMatch = false;

	int	iFoundCount=0;
	__int32	iRec=0;
	__int64 dwSeq, dwFRec, iClc =0, iOld=0, i=0;
	char *strFilePath;
	
	MFT_CLUSTERS	*m_Mc;			// Array defined that holds the clusters of the MFT
	m_Mc = m_ClusterArray;
	
	FILE_CLUSTERS	*m_Fc;			// Array this function will fill based on the file search for
	m_Fc = m_FileDetails;


    strFilePath = (char*)VirtualAlloc(
                           NULL, // next page to commit
                           MAX_PATH,         // page size, in bytes
                           MEM_COMMIT,         // allocate a committed page
                           PAGE_READWRITE);    // read/write access


	myFT1 = CFileTime::GetCurrentTime();
    ihigh  = (LONG*)malloc(sizeof(LONG) );
    //TRACE1("\n\nSearching for %s", strFileName);
    iRecSize = ( ( m_PartitionBootSector.iBytesPerSector * m_PartitionBootSector.iSectorsPerCluster )
					* m_PartitionBootSector.iClustersPerMFTFileRecord );

	dwErr = SetFilePointer(m_HndData, 0, 0, FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		//TRACE1("\nSet File Pointer Failed In FindFileMFT, Error %i", dwErr);
		return false;

	}
    		
	// m_Mc is the MFT cluster array created by GetMFTOffsets();
	while(m_Mc->iStartCluster > 0)			 // If zero we have the last data record
	{ 								
		itmp.QuadPart = ((m_Mc->iStartCluster + m_HDPartitionTable[iPartition].dwRelativeSector) * 512);				// TODO : ?? Add 63 Which is the partition offset from begin of phy disk
																		//        * 512 as SetFilePointerEx works in bytes not clusters
		// Move to the start of this MFT cluster extent
		dwErr = SetFilePointerEx(m_HndData, itmp, NULL , FILE_BEGIN);
		if(!dwErr)
		{ 
			dwErr = GetLastError();
			//TRACE1("\nFailed to set file pointer, error: %i", dwErr);
			return false;
		}

		int iRecNo=0;
		for(i=0;i<m_Mc->iMFTRecSize;i++)		// loop through all MFT records in this extent
		{ 			
             	// Read in Next MFT record
				if(! ReadFile(m_HndData, lpByte, iRecSize , &dwbyte, NULL) )
				{
					dwErr = GetLastError();
					//TRACE1("\nError in FindFile, Could not read file, Error :%i", dwErr);
					return false;
				}

				iRecNo = lpByte[44];
				iRecNo += lpByte[45] << 8;
				iRecNo += lpByte[46] << 16;
				////TRACE1("\nCurrent MFT Record Number :0x%X ", iRecNo+1); 

				iOff = lpByte[20];			// at offset 20 the record states where the first attribute starts
				dwAttrib = lpByte[iOff];	// read this attribute header 0x10,0x30,0x80 etc

				// Will loop through the entire record to get all attribs
				// 0=Corrupt record, FF=end of attributes in this MFT record
				while( lpByte[iOff + 4] != 0  &&   lpByte[iOff] != 0xFF ) // In find correct attribute loop
				{   						
						// Find The File Attribute in this record, this is the attribute header
			           	dwAttrib = lpByte[iOff];	

						iOff = GetMFTAttrOffset(lpByte, 0x30);


						switch(dwAttrib) // TODO : we are assuming a certain order of attributes in 
						{				 // the record by setting the bfound flag - could be dangerous !!

						case 0x80:			// $DATA		
							// Check for Resident Only In That Case We Dont Need GetFileExtents
							if(bData)	// if we have already done this for this record dont do it again
								break;	// reason: there can be more than one 0x80 record

							 if(bFound)
                             {	
								bData = true;
								m_Fc->iNoExtents = 0;
								if(lpByte[iOff + 8] == 1)		// Is The File Data Resident In The MFT Rec ?
								{
								//	//TRACE1("\nFile is not resident in MFT: Flag %i", lpByte[iOff + 8] );
									m_Fc->iNoExtents = GetFileExtents(ptrByte, iOff, m_Fc);
								}
								else
								{
									////TRACE1("\nFile is resident in MFT record: Flag %i", lpByte[iOff + 8] );
									// TODO : need to return the MFT sector number with the offset start+end
								}

							 }

							iOff += lpByte[iOff + 4];		// Set Offset To Next Attribute ie $DATA,$FILE_NAME etc
							break;

						case 0x10:			// $STANDARD_INFORMATION : Byte 44 & 45 = Sequence No		   (2c & 2d)
							iOff += lpByte[iOff + 4];
							break;

						case 0x30:		    // $FILE_NAME
							strLabel.Empty();
							for(int k=0;k<lpByte[iOff+88]*2 ;k++)	// offset 0x40 has the label size
							{
								if( isprint(lpByte[iOff+k+90]) ) // UNICODE:Need to sort this as we are skipping unicode chars
									strLabel.AppendFormat( "%c", lpByte[iOff+k+90] );
							}

							bMatch = false;
							bMatch = IsStringMatchedW(strFileName, strLabel);
							if(bMatch)
							{
								if(lpByte[21] == 0 && lpByte[22] == 0)
									bDeleted = true;
								else
									bDeleted = false;

							  // //TRACE1("\nMFT Record Number :0x%X ", iRecNo);
									  
								if((bDeleted && pParam->bDeleted) || (!bDeleted && pParam->bActive))
								{
									m_Fc->iIsDeleted = bDeleted;
									m_Fc->iRecordNo = iRecNo;		// save mft recno							
									bFound = true;  // Found file, Set Flag To Get File Data
									iFoundCount++;  // TODO : Could be more than one file of the same name
									memcpy(m_Fc->strFileName, strLabel, MAX_PATH);
									
									// parent record no		
									dwFRec = lpByte[iOff+24];
									dwFRec += lpByte[iOff+25] << 8;
									dwFRec += lpByte[iOff+26] << 16;

									// parent seq no
									dwSeq = (lpByte[iOff+31] << 8);
									dwSeq = dwSeq + lpByte[iOff+30];

									GetFilePathMFT(dwFRec, dwSeq, strFilePath, iPartition);
									memcpy(m_Fc->strFolder, strFilePath, MAX_PATH);
									memset(strFilePath, '\0', MAX_PATH);
									m_Fc->iPartition = iPartition;
									m_Fc->iPhyDiskNumber = m_iDiskOpen;

									strLabel.FormatMessage("%1\\%2", strFilePath, strLabel);
									////TRACE1("\nComplete File Path : %s", strLabel);
								}
							}
							
							iOff += lpByte[iOff + 4];			// MOve to the next attribute
							break;
							
                        											        				
						default:			// Move To Next Record
							iOff += lpByte[iOff + 4];	// this is the record length so add to the attrib offset to get to the start pos of the next attrib
							break;
						}			

				} //END: while() - Loop Through This Individual MFT Record
	
				if(bFound)
				{
					__int64 ia = ( m_Mc->iStartCluster + m_HDPartitionTable[iPartition].dwRelativeSector + (i*2))   ;
					////TRACE2("\nFound \"%s\" MFT record at sector offset %i from start of phy disk", strFileName, ia);
					////TRACE1("\nRecord is %s", (bDeleted ? "Deleted" : "Active")  );
					////TRACE0("\n\n\n");
					bFound = false; // reset
					bData = false;
					if(strlen(pParam->strLogicalDisk) > 0)
						memcpy(m_Fc->strLogicalDisk, pParam->strLogicalDisk, sizeof(m_Fc->strLogicalDisk));
					else
						m_Fc->strLogicalDisk[0] = '\0';

					// TODO : should increase array pointer here for next record found
					// and reallocate memory 
					//m_FileDetails = (FILE_CLUSTERS*)realloc(m_FileDetails, sizeof(FILE_CLUSTERS) * (iFoundCount+1));

					//m_FileDetails->sFileExtents = (FILE_EXTENT*)realloc(m_FileDetails->sFileExtents , (iFoundCount + 1) *sizeof(FILE_EXTENT));
					//m_Fc = m_FileDetails;
					m_Fc ++; //=	iFoundCount;		// new\blank record
				}

		} // END:  for(__int64 i=0;i<m_Mc->iMFTRecSize;i++)	 - Loop Through this MFT Extent (All MFT Records)

 		m_Mc++;		   // Move to next record

	}	//END: while(m_Mc->iStartCluster > 0)

	
    // TODO : current record pointer will be pointing to a 'blank' array need to null this
	
	free(ihigh);
   // //TRACE1("\n\nRecords found : %i", iFoundCount);
	bFound = (iFoundCount > 0 ? true : false);
//	if(iFoundCount > 0)  
//		bFound = true;
//	else
//		bFound = false;

	myFT2=CFileTime::GetCurrentTime();
	CFileTimeSpan myFTS;
	myFTS = myFT2 - myFT1;
	//TRACE1("\nTime To Search %i", myFTS.GetTimeSpan()/CFileTime::Second );

	return iFoundCount; //bFound;	 // Did we find it ?



	
=======================================================							
						//dwFRec = *(lpByte+24);
						//dwFRec += *(lpByte+25) << 8;
						//dwFRec += *(lpByte+26) << 16;

						// parent seq no 
						//dwSeq = *(lpByte+30);
						//dwSeq += *(lpByte+31) << 8;



									//if(*(lpByte) == 0xFF)		    // End Of Record
			//	break; 

			*/

/*


for(int i=0;i<4;i++)
	{
		if( byData[iOffset+(i*16)] == 0x80 )							// Bootable Partition (0x80 | 0x00)
			m_HDPartitionTable[i].bBootable = true;
		else
			m_HDPartitionTable[i].bBootable = false;

		m_HDPartitionTable[i].byStartHead = byData[iOffset+(i*16)+1];		// Starting Head

		iLowBits = (byData[iOffset+(i*16)+2] & 63);			// AND use '0' in binary to clear opposing bits to 0 //450
		iHighBits =  (byData[iOffset+(i*16)+3] << 2) + (byData[iOffset+(i*16)+2] & 3);
		m_HDPartitionTable[i].iStartSector = iLowBits;			// Start Sector
		m_HDPartitionTable[i].iStartCylinder = iHighBits;		// Start Cylinder
		
		dwType = byData[iOffset+(i*16)+4];
		m_HDPartitionTable[i].iFileSystem = dwType;				// Hex of installed file system.
		delete strText;
		strText = (char*)malloc(255);
		strText = (char*)memset(strText, '0', 255); 
		GetFormatTypeText(dwType, strText);
		m_HDPartitionTable[i].strFileSystem = strText;			// Description Of Format Type On Disk

		m_HDPartitionTable[i].iEndingHead = byData[iOffset+(i*16)+5];		// Ending Head

		iLowBits = (byData[iOffset+(i*16)+6] & 63);			// AND use '0' in binary to clear opposing bits to 0
		iHighBits =  (byData[iOffset+(i*16)+7] << 2) + (byData[iOffset+(i*16)+6] & 3);

		m_HDPartitionTable[i].iEndingSector = iLowBits;			// End Sector
		m_HDPartitionTable[i].iEndingCylinder = iHighBits;		// End Cylinder

		if(i==0)
		{m_HDPartitionTable[i].dwRelativeSector  =				// This Partition Starting Sector Offset From Start of Phy Disk
			(byData[iOffset+(i*16)+8] + (byData[iOffset+(i*16)+9] * 256) 
			+ (byData[iOffset+(i*16)+10] * 65536) + (byData[iOffset+(i*16)+11] * 16777216));
		}
		else
		{m_HDPartitionTable[i].dwRelativeSector  =				// This Partition Starting Sector Offset From Start of Phy Disk
			((byData[iOffset+(i*16)+8] + (byData[iOffset+(i*16)+9] * 256) 
			+ (byData[iOffset+(i*16)+10] * 65536) + (byData[iOffset+(i*16)+11] * 16777216)) +63);
		}

		m_HDPartitionTable[i].dwTotalSectors = 
			byData[iOffset+(i*16)+12] + (byData[iOffset+(i*16)+13] *256 )
			+ (byData[iOffset+(i*16)+14] * 65536 ) + (byData[iOffset+(i*16)+15] * 16777216);


		// get the format type for the partition
		if(m_HDPartitionTable[i].dwTotalSectors > 0)
			m_HDPartitionTable[i].dwFormatType = GetPartitionFormatType(m_HDPartitionTable[i].dwRelativeSector);
		else
			m_HDPartitionTable[i].dwFormatType = 0;
        		
	}


	*/
BOOL CDiskAnalysis::GetINT13Info(LPCSTR lpPhysicalDisk, DA_DISK_EX_INT13_INFO* DiskInt13Info)
{

	CString tmpStr = "\\\\.\\";
	tmpStr.AppendFormat("%s", lpPhysicalDisk);	

	BOOL bSuccess = OpenHandle(tmpStr);
		
	if(!bSuccess)
		return false;

	DWORD lpByte;
	DISK_GEOMETRY_EX pDiskInfo;	   

	memset(&pDiskInfo, '0', sizeof(DISK_GEOMETRY_EX));

	bSuccess = DeviceIoControl(m_HndData, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
								NULL, 0, &pDiskInfo, sizeof(DISK_GEOMETRY_EX), &lpByte, NULL);

	DWORD dwErr = GetLastError();

	// fill in user struct
	DiskInt13Info->iTotalBytes = pDiskInfo.DiskSize.QuadPart;
	DiskInt13Info->iTotalSectors = (pDiskInfo.DiskSize.QuadPart / pDiskInfo.Geometry.BytesPerSector);


	//PDISK_DETECTION_INFO pDetectInfo = DiskGeometryGetDetect(&pDiskInfo);
	//PDISK_PARTITION_INFO pPartInfo = DiskGeometryGetPartition(&pDiskInfo);
	
   		


	CloseDiskHandle();	

	return 0;
}
