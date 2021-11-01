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
	2. Call AnalyseDisk with either a handle or device string e.g BOOL = AnalyseDisk("PhysicalDisk0");
	3. Call any other member functions you need.

	CORE FUNCTIONS:
	GetMBR()			-		Fills The MASTER_BOOT_RECORD Structure with the Master Boot Record Details
	GetPartitionTable()	-		Fills the PARTITION_TABLE_NTFS structure with the selected NTFS partition boot sector details
	GetMFTOffsets()     -		Creates an array of MFT_CLUSTERS defining MFT cluster on disk.
	FindFile()			-		Find a file in the MFT.
	


*/
#include "StdAfx.h"
#include "diskanalysis.h"

#include "winioctl.h"				// 
#include "cstringt.h"
#include "math.h"

CDiskAnalysis::CDiskAnalysis(void)
{
	m_HndData = NULL;	
	m_iSize = 0;

	m_ClusterArray = (MFT_CLUSTERS*)malloc(1 * sizeof(MFT_CLUSTERS) ); // Allocate
	m_ClusterArray->iStartCluster = 0;
	m_ClusterArray->iEndCluster = 0;
	m_ClusterArray->iMFTRecSize = 0;

	m_FileDetails = (FILE_CLUSTERS*)malloc(1 * sizeof(FILE_CLUSTERS) ); // Allocate
	m_FileDetails->iIsDeleted = FALSE;
	memset(m_FileDetails->strFolder ,'0', 260)   ;
	m_FileDetails->strFolder[260] = '\0';	
	m_FileDetails->sFileExtents.iFileExtentEndCluster = 0;
	m_FileDetails->sFileExtents.iFileExtentStartCluster = 0;
	m_FileDetails->sFileExtents.iMFTRecordNo = 0;


}

CDiskAnalysis::~CDiskAnalysis(void)
{
	// Cleanup
	if(m_HndData != NULL)
		CloseHandle(m_HndData);

	free(m_ClusterArray);
	free(m_FileDetails);

	

}

/*
	Open Disk/Volume/File
*/
BOOL CDiskAnalysis::OpenHandle(LPCTSTR lpctPath)
{
	m_HndData = CreateFile(lpctPath, GENERIC_READ, 
							  FILE_SHARE_READ | FILE_SHARE_WRITE,
				              NULL, OPEN_EXISTING, 0, 0);

	if(m_HndData == INVALID_HANDLE_VALUE)		// Function Failed
	{
		DWORD dwErr = GetLastError();
		TRACE1("Failed To Open File - DiskAnalysis.cpp-OpenHandle() Error:%i", dwErr);
        return false;
	}

	return true;
}

/*
Get The Master Boot Record (MBR) and Fill In Data Struct
*/
BOOL CDiskAnalysis::GetMBR(void)
{
	DWORD dwErr = 0;
	DWORD dwType;
	BOOL bSuccess;
	DWORD lpdRead;
	BYTE	byData[512];			// Data From ReadFile
	int iLowBits, iHighBits;
	int iOffset = 446;
	char* strText;
	
	if(m_HndData == NULL)
	{
		TRACE0("GetMBR returning FALSE because file handle was NULL.");
		return false;
	}


	// Set File Pointer To Beginning Of Disk
	dwErr = SetFilePointer(m_HndData, 0, 0 , FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Set File Pointer - DiskAnalysis.cpp-GetMBR() Error:%i", dwErr);
		return false;
	}

	// Read In 512 Bytes - (Always the same for all disks for the MBR)
	bSuccess = ReadFile(m_HndData, byData, 512, &lpdRead, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Get File Data - DiskAnalysis.cpp-GetMBR() Error:%i", dwErr);
		return false;
	}


	// Fill In MBR Struct
	// Offset[446] Is the start of the first partition record
	
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
		strText = (char*)malloc(255);
		strText = (char*)memset(strText, '0', 255); 
		GetFormatTypeText(dwType, strText);
		m_HDPartitionTable[i].strFileSystem = strText;			// Description Of Format Type On Disk

		m_HDPartitionTable[i].iEndingHead = byData[iOffset+(i*16)+5];		// Ending Head

		iLowBits = (byData[iOffset+(i*16)+6] & 63);			// AND use '0' in binary to clear opposing bits to 0
		iHighBits =  (byData[iOffset+(i*16)+7] << 2) + (byData[iOffset+(i*16)+6] & 3);

		m_HDPartitionTable[i].iEndingSector = iLowBits;			// End Sector
		m_HDPartitionTable[i].iEndingCylinder = iHighBits;		// End Cylinder

		m_HDPartitionTable[i].dwRelativeSector  =				// This Partition Starting Sector Offset From Start of Phy Disk
			byData[iOffset+(i*16)+8] + (byData[iOffset+(i*16)+9] * 256) 
			+ (byData[iOffset+(i*16)+10] * 65536) + (byData[iOffset+(i*16)+11] * 16777216);

		m_HDPartitionTable[i].dwTotalSectors = 
			byData[iOffset+(i*16)+12] + (byData[iOffset+(i*16)+13] *256 )
			+ (byData[iOffset+(i*16)+14] * 65536 ) + (byData[iOffset+(i*16)+15] * 16777216);

	}

	free(strText);
	return true;
}



/*
Formatting For Drive Format Type 
Called From GetMBR()
*/

void CDiskAnalysis::GetFormatTypeText(DWORD dwType, char* strText)
{

	switch(dwType)
	{
		case 0x01:
			memcpy(strText, "FAT12 primary partition or logical drive (fewer than 32,680 sectors in the volume)", 255);
			break;
		case 0x04:
			memcpy(strText, "FAT16 partition or logical drive (32,680–65,535 sectors or 16 MB–33 MB)", 255);
			break;
		case 0x05:
			memcpy(strText, "Extended partition", 255);
			break;
		case 0x06:
			memcpy(strText, "BIGDOS FAT16 partition or logical drive (33 MB–4 GB)" , 255);
			break;
		case 0x07:
			memcpy(strText, "Installable File System (NTFS partition or logical drive)", 255);
			break;
		case 0x0B:
			memcpy(strText, "FAT32 partition or logical drive" , 255);
			break;
		case 0x0C:
			memcpy(strText, "FAT32 partition or logical drive using BIOS INT 13h extensions" , 255);
			break;
		case 0x0E:
			memcpy(strText, "BIGDOS FAT16 partition or logical drive using BIOS INT 13h extensions", 255);
			break;
		case 0x0F:
			memcpy(strText, "Extended partition using BIOS INT 13h extensions", 255);
			break;
		case 0x12:
			memcpy(strText, "EISA partition or OEM partition", 255);
			break;
		case 0x42:
			memcpy(strText, "Dynamic volume", 255);
			break;
		case 0x84:
			memcpy(strText, "Power management hibernation partition" , 255);
			break;
		case 0x86:
			memcpy(strText, "Multidisk FAT16 volume created by using Windows NT 4.0" , 255);
			break;
		case 0x87:
			memcpy(strText, "Multidisk NTFS volume created by using Windows NT 4.0", 255);
			break;
		case 0xA0:
			memcpy(strText, "Laptop hibernation partition" , 255);
			break;
		case 0xDE:
			memcpy(strText, "Dell OEM partition", 255);
			break;
		case 0xFE:
			memcpy(strText, "IBM OEM partition" , 255);
			break;
		case 0xEE:
			memcpy(strText, "GPT partition", 255);
			break;
		case 0xEF:
			memcpy(strText, "EFI System partition on an MBR disk", 255);
			break;
		default:
			memcpy(strText, "Unknown Type", 255);
			break;
	}

	return;
}

/*
 Fills The Partition Boot Sector Table Structure With Selected Partition Info
*/
BOOL CDiskAnalysis::GetPartitionTable(int iPartition)
{
	DWORD dwErr = 0;
	BOOL bSuccess;
	DWORD lpdRead;
	BYTE byData2[512];

	if(m_HndData == NULL)
	{
		TRACE0("GetPartitionTable returning FALSE because file handle was NULL.");
		return false;
	}
	
	// TODO - Need Check That Master Boot Record Has Been Read ( GetMBR() )
																		  // TODO  * 512 (Bytes Per Sector Need Calc not hardset)
	dwErr = SetFilePointer(m_HndData, (LONG)m_HDPartitionTable[0].dwRelativeSector * 512, 0 , FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Set File Pointer - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}

	// Read In 512 Bytes - (Always the same for all disks)
	bSuccess = ReadFile(m_HndData, byData2, 512, &lpdRead, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Get File Data - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}
	

	// We are now at the Start of Partition 1, at the Partition Boot Sector.
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
	   (byData2[55] * 72057594037927936) + m_HDPartitionTable[0].dwRelativeSector; // Needs Tidying when sector + cluster are not both 512k we have this from above

	m_PartitionBootSector.llLogicalClusterNumForMFTMrr =			// $mft mirror start location
		byData2[56] + 
	   (byData2[57] * 256) + 
	   (byData2[58] * 65536) + 
	   (byData2[59] * 16777216)+
	   (byData2[60] * 4294967296)+
	   (byData2[61] * 1099511627776)+
	   (byData2[62] * 281474976710656)+
	   (byData2[63] * 72057594037927936) + m_HDPartitionTable[0].dwRelativeSector; // Needs Tidying when sector + cluster are not both 512k

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

	return true;
}


/*
 Walks The MFT To Find All Clusters That The MFT Uses
	
 The MFT_CLUSTERS structure will be added to inthe array until no more records need to be added
 the last record in this struct will have zeros and will mean that no more data exists.

*/
BOOL CDiskAnalysis::GetMFTOffsets(void)
{

	// TODO - Error Checking For NULL FILE
    LONG lMFTSectorBytes = 0;
	DWORD dwErr;
	DWORD lpdRead;
	CString 	strTest, str;
	int	iMFTRecSize = 1;
	BOOL bSuccess;

    
	//  Because We Have Opened The PhysicalDisk we also need to add				// TODO Could have opened another disk/vol/file
	// the offset of the PartitionBootSector from the start of the disk
	lMFTSectorBytes = (LONG)this->m_PartitionBootSector.llLogicalClusterNumForMFT 
				* this->m_PartitionBootSector.iBytesPerSector;			// Convert To Bytes

		//
		
	dwErr = SetFilePointer(m_HndData, lMFTSectorBytes, 0 , FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Set File Pointer - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
		return false;
	}

	// Calculate Size Of one MFT Record
	iMFTRecSize = 
	(   (  this->m_PartitionBootSector.iClustersPerMFTFileRecord 
		 * this->m_PartitionBootSector.iSectorsPerCluster  )
	     * this->m_PartitionBootSector.iBytesPerSector );
	

	BYTE byData2[1024];		// Temp Storage Of The Record
	// Read in one MFT Record (This Being The MFT record itself)
	bSuccess = ReadFile(m_HndData, byData2, iMFTRecSize, &lpdRead, NULL);
	if(!bSuccess)
	{
		dwErr = GetLastError();
		TRACE1("Failed To Get File Data - DiskAnalysis.cpp-GetPartitionTable() Error:%i", dwErr);
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
	DWORD dwOffsetSave;
	  
	for(;;)						
	{
		// Go To First Attribute
		dwAttrib = byData2[iOff];

		// What Attribute Have We Found.
		if(dwAttrib == 0x80)			// If data attribute
		{
			TRACE0("\nFound attrib 0x80 ");
			TRACE1("at offset 0x%x", iOff);
			dwOffsetSave = iOff;					// Save Offset for later use
			// TRACE Output Attribute Details
			TRACE1("\nAttribute Record Length :0x%X", 
				byData2[iOff + 4] + 
				(byData2[iOff + 5] * 256) + 
				(byData2[iOff + 6] * 65536) + 
				(byData2[iOff + 7] * 16777216)  );
			TRACE1("\nNon Resident Flag : %i", byData2[iOff + 8] );
			TRACE1("\nName Length : %i", byData2[iOff +9] );
			TRACE1("\nOffset To The Name (ie Offset To Start Of Data Runs) : 0x%X", 
				byData2[iOff + 10] + 
				(byData2[iOff + 11] * 256) );

			// Offset to the start of the data runs from the attribute header
			DWORD dwData = (byData2[iOff + 10] + 
							(byData2[iOff + 11] * 256) );

			// If dwData == 0x00 at this point, it means there are no more extents.

			TRACE0("\n\nData Runs:");
			DWORD dwHigh =0, dwLow =0, dwCalc =0, dwCalc2 =0, dwOffset =0;
			int j=0,i=0, ks=0;
				// iOff = Start Of Attribute Header
				// dwData = Offset from iOff for the start of the current data runs
// in constructor	m_ClusterArray = (MFT_CLUSTERS*)malloc(1 * sizeof(MFT_CLUSTERS) ); // Allocate
			MFT_CLUSTERS	*m_Ca, *m_Cab;
						
			while(byData2[iOff + dwData] > 0)
			{
					                 

					dwHigh = (byData2[iOff + dwData] & 15 );	// Highbits - Byte Length of run
					TRACE1("\nLength of run : %i", dwHigh );

					dwLow = (byData2[iOff + dwData] & 240);		// Low Bits - Byte Length of cluster
					TRACE1("\nLength of cluster offset: %i", dwLow >> 4  );

					// Get Length Of Run For This Extent we need to get the next dwHigh bytes
					// dwHigh = Length Of Extent Run for length of this extent data
					dwCalc2 = 0;
					dwOffset = iOff + dwData + dwHigh;		// On Last Byte Now
					j = (int)dwHigh-1;
					for(i=0;i<=(int)dwHigh-1;i++)
					{
						if(byData2[dwOffset-i] > 0)
						{
							dwCalc2 += byData2[dwOffset-i] * (DWORD)pow(256, j);
						}
						j--;	
					}
					TRACE1("\nLength Of Extent Run (In This Case Length Of This Section Of MFT) : 0x%X", dwCalc2 );

						

					// Get Cluster Offset Value
					// dwLow = Length Of Extent Run for cluster value offset
					dwLow = (dwLow >> 4);
					j = dwLow-1;
					dwCalc = 0;
					dwOffset = iOff + dwData + dwHigh + dwLow;		// On Last Byte Now For ClusterVal
					for(i=0;i<=(int)dwLow-1;i++)
					{
						if(byData2[dwOffset-i] > 0)
						{
							dwCalc += byData2[dwOffset-i] * (DWORD)pow(256, j);
						}
						j--;	
					}
					TRACE1("\nStarting Cluster Number For This Extent (In This Case Start Of MFT)  : 0x%X", dwCalc );
					
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
						m_Ca->iEndCluster = m_Ca->iStartCluster + (__int64)dwCalc2-1;	   // Fill In New Record
						m_Ca->iMFTRecSize =	 ((m_Ca->iEndCluster - m_Ca->iStartCluster) 
							/ this->m_PartitionBootSector.iClustersPerMFTFileRecord );
						
					}
					else	// Must Be First Record (Just Add The Start and len to get end cluster)
					{	
						m_Ca = m_ClusterArray;						// Point To First Record
						m_Ca->iStartCluster = (__int64)dwCalc;
						m_Ca->iEndCluster = (__int64)dwCalc2 + dwCalc-1;
						m_Ca->iMFTRecSize =	( (m_Ca->iEndCluster - m_Ca->iStartCluster )
							/ this->m_PartitionBootSector.iClustersPerMFTFileRecord );
					}
                    




					// Now Find The Next Data Run (Could be 0x00 which means no more data runs !!)
					dwData +=  (dwHigh + dwLow + 1);
					TRACE1("\n\nNext Run 0x%X\n\n", dwData);

			}// end while(dwData > 0)

			// Allocate Storage - Create One More Record With 0 to show end of data
			ks++;
			m_ClusterArray = (MFT_CLUSTERS*)realloc(m_ClusterArray, ks * sizeof(MFT_CLUSTERS) );
			//m_Ca = m_Ca + (ks-1);	
			m_Ca = m_ClusterArray;
			m_Ca += (ks-1);
			m_Ca->iEndCluster =0;
			m_Ca->iStartCluster =0;

			break;				
		}	// end if Attribute = 0x80 ($DATA)
	
		// Not This One So Get The Length of this attribute ie to find start of next attrib
		dwLength = byData2[iOff+4];
		iOff += dwLength;
	}  // end for(;;)

	TRACE0("\n");

	return TRUE;
}

/*
	Opens the $AttrDef file and returns the value of the requested attribute
	TODO : Unable To Open file with CreateFile() Error #5 - Access Denied.
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
		TRACE1("\nFailed to open $AttrDef, Error Number : %i", dwErr);
		return 0;
	}

	dwFileSize = SetFilePointer(fhnd, 0, 0, FILE_END);

	dwErr = SetFilePointer(fhnd, 0, 0, FILE_BEGIN);
	if(dwErr == INVALID_SET_FILE_POINTER)
	{
		dwErr = GetLastError();
		TRACE1("Failed to set $AttrDef file pointer, Error Number : %i", dwErr);
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
		TRACE1("Failed to read $AttrDef file , Error Number : %i", dwErr);
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
			TRACE0("EOF in $AttrDef, before attribute found !!");
			return 0;
		}

		if(!bSuccess)
		{
			dwErr = GetLastError();
			TRACE1("Failed to read $AttrDef file , Error Number : %i", dwErr);
			break;
		}


	}
	

	

	return 0;
}

 
/*
	 Find the specfied file in the MFT - Full path currently required in strFileName
*/
BOOL CDiskAnalysis::FindFile(CString strFileName)
{

	
	// Start at the first MFT extent and work through this until 
	// either the end of the extent comes or we find the file,
	// when the end of the extent comes, move on to the next one.
	// m_HndData - HANDLE to open disk/volume - Access to MFT
	// m_ClusterArray - MFT Clusters (Last Record Has 0,0)
	// MFT Attributes - 0x30 $DATA and 0x80 $FILE_NAME
	MFT_CLUSTERS	*m_Mc;	
	m_Mc = m_ClusterArray;
	DWORD dwErr =0;
	BYTE lpByte[1024];
    DWORD dwbyte, dwAttrib;
	DWORD iRecSize =0;
	DWORD iOff = 0;				
	CString strLabel;
	__int64 iClc =0, iOld=0;
	__int32	iRec=0;
	LARGE_INTEGER itmp;
	PLONG ihigh;
	CFileTime myFT1, myFT2;

	myFT1 = CFileTime::GetCurrentTime();

	ihigh  = (LONG*)malloc(sizeof(LONG) );

	iRecSize = ( ( m_PartitionBootSector.iBytesPerSector * m_PartitionBootSector.iSectorsPerCluster )
					* m_PartitionBootSector.iClustersPerMFTFileRecord );

	dwErr = SetFilePointer(m_HndData, 0, 0, FILE_BEGIN);
	
	while(m_Mc->iStartCluster > 0)			 // If zero we have already seen last data record
	{ 	
		itmp.QuadPart = ((m_Mc->iStartCluster + 63) * 512);
                 
		// Move to the start of this MFT cluster extent
		dwErr = SetFilePointerEx(m_HndData, itmp, NULL , FILE_BEGIN);
		if(!dwErr)
		{ 
			dwErr = GetLastError();
			TRACE1("\nFailed to set file pointer, error: %i", dwErr);
			return false;
		}


		for(__int64 i=0;i<m_Mc->iMFTRecSize;i++)
		{ 			
             	// Read in Next MFT record
				if(! ReadFile(m_HndData, lpByte, iRecSize , &dwbyte, NULL) )
				{
					dwErr = GetLastError();
					TRACE1("\nError in FindFile, Could not read file, Error :%i", dwErr);
					return false;
				}

				iOff = lpByte[20];	// at offset 20 the record states where the first attribute starts
				dwAttrib = lpByte[iOff];

					for(;;) // In find correct attribute loop
					{   						
						// Find The File Attribute in this record
						dwAttrib = lpByte[iOff];						// this is the first attribute
						if(dwAttrib == 0x30)							// Found file attrib !!
						{
							strLabel.Empty();
							for(int k=0;k<lpByte[iOff+88]*2 ;k++)	// offset 0x40 has the label size
							{
								if( isprint(lpByte[iOff+k+90]) )
								strLabel.AppendFormat( "%c", lpByte[iOff+k+90] );
							}
							if(!strLabel.CompareNoCase(strFileName) == 0)
							{	// Filenames are not the same - so break out to next record
                                break; 
							}	
							else
							{
								__int64 ia = ( m_Mc->iStartCluster + 63 + (i*2))   ;
								TRACE1("\nFound record at offset %i", ia);

								// TODO : Now Do What With It !!
								// We need to find the 0x10, 0x80 attribs to fill this data

								// Is it deleted ?
								// Where is it ?
								// What MFT Rec no


								free(ihigh);
								return TRUE;
							}

						} // END: if(dwAttrib == 0x30)

						// Not correct one so find the offset of the next attribute
						if(lpByte[iOff + 4] == 0)		// Corrupt Record
							break;

						iOff += lpByte[iOff + 4];	// this is the record length so add to the attrib offset to get to the start of the next attrib

						if(lpByte[iOff] == 0xFF)	// no more attributes
							break; // let next record be read in

					} //END: for(;;)

		} // END:  for(__int64 i=0;i<m_Mc->iMFTRecSize;i++)

		
		TRACE0("\nChanging Array !!");
		m_Mc++;		   // Move to next record

	}	//END: while(m_Mc->iStartCluster > 0)

	// Retrieve the current time - Calc length of search
	myFT2=CFileTime::GetCurrentTime();
	CFileTimeSpan myFTS;
	myFTS = myFT2 - myFT1;
    TRACE1("\nTime To Search %i", myFTS.GetTimeSpan()/CFileTime::Second );

	free(ihigh);

	return false;	  // File Does Not Exist In The MFT
}









