#pragma once
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 
/**********************************************************************************************************
 These following structs are init'd outside of this class by the calling function and pointers passed to
 public functions within this class, NOTE: they are no longer directly allocated within this class
**********************************************************************************************************/
/* Find File Results */ 
	 typedef struct FILE_EXTENT{  				    // Will be arrays of this type in next struct
			__int64	iFileExtentStartCluster;		// Starting Cluster For File
			__int64 iFileExtentEndCluster;			// Ending Cluster For File
			__int32	iMFTRecordNo;					// Extent Record Number
	}FILE_EXTENT;

	 typedef struct	FILECLUSTERS{
		  LPARAM lParam;							// user var to use as wish
		  BOOL	iIsDeleted;							// Is File Deleted 
	    __int64 iFileLength;						// Actual File Length  (Bytes)
		  int	iPartition;							// Partition Number	   
		  int	iPhyDiskNumber;						// physical disk no ie 0,1,2, etc	
		  char  strLogicalDisk[3];					// logical disk C:
		  int	iNoExtents;							// no of extent arrays
		  char  strFolder[MAX_PATH+1];				// File Folder
		  char  strRecoverPath[MAX_PATH+1];			// user filled full recover path
		  char	strFileName[MAX_PATH+1];			// file name searched for
		  DWORD	iRecordNo;							// record no (MFT entry, FAT entry etc)
		  struct FILE_EXTENT* sFileExtents;			// File Cluster Start/Stop
	 }FILE_CLUSTERS;

	 typedef struct FINDFILEPARAMS{
		 CString strFileName;
		 CString strLogicalDisk;
		 BOOL bDeleted;
		 BOOL bActive;
	 }FF_PARAMS;

 
 /*	 End Find File Results */
 typedef struct DA_DISK_EX_INT13_INFO {
    WORD   ExBufferSize;
    WORD   ExFlags;
    DWORD  ExCylinders;
    DWORD  ExHeads;
    DWORD  ExSectorsPerTrack;
   DWORD64 ExSectorsPerDrive;
    WORD   ExSectorSize;
    WORD   ExReserved;
 DWORD64   iTotalSectors;
 DWORD64   iTotalBytes;
} DA_DISK_EX_INT13_INFO;

/* GetMBR() Function Fills This Struct */
typedef struct M_B_R{// Byte Offset
	BOOL    bValidPartition;
	char	strResourceDesc[20];
	BYTE	bRecordNo;
	BYTE	bParent;
	BOOL	bBootable;			// Is Partition Bootable ie system	// 00
	int		byStartHead;		// Fill in							// 01
	DWORD64	iStartSector;		// Fill in							// 02
	int		iStartCylinder;		//									// 03
	char    strFileSystem[MAX_PATH];		// File System Installed (string)	// 04
	int		iFileSystem;		// File System Installed (hex)				// Added (NS)
	DWORD	dwFormatType;		// Added - Use Format Define Above			// Added (NS)
	int		iEndingHead;		//									// 05
	DWORD64	iEndingSector;		//									// 06
	int		iEndingCylinder;	//									// 07
	DWORD64	dwRelativeSector;	// Sectors from the start of the disk preceding this partition			// 08
	DWORD64	dwTotalSectors;		// Total Sectors in this partition  // 12
}MASTER_BOOT_RECORD;

/* NTFS Partition Table Entry */	  /* GetNTFSPartitionTable() fills this struct */
typedef struct NTFS_BOOT_SECTOR{
	char cDriveLetter;								// 1 Byte   - New Addition(Not Std)
	int	iBytesPerSector;							// 2 Bytes
	int	iSectorsPerCluster;							// 1 Bytes
	int iReservedSectors;							// 2 Bytes
	char chReserved[3];								// 3 Bytes Always 000 (3 Bytes)
	int	iUnknown;									// 2 Bytes Not Used By NTFS 
	int	iMediaDesc;									// 1 Bytes Media Descriptor 
	int	iUnknown2;									// 2 Bytes Always 00 (2 Bytes)
	int	iSectorsPerTrack;							// 2 Bytes
	int iNumOfHeads;								// 2 Bytes
	int iHiddenSectors;								// 4 Bytes
	int iUnknown3;									// 4 Bytes
	int iUnknown4;									// 4 Bytes
	LONGLONG	llTotalSectors;						// 8 Bytes
	LONGLONG	llLogicalClusterNumForMFT;			// 8 Bytes
	LONGLONG	llLogicalClusterNumForMFTMrr;		// 8 Bytes
	int	iClustersPerMFTFileRecord;					// 4 Bytes
	int	iClustersPerIndexBlock;						// 4 Bytes
	LONGLONG	llVolumeSerialNumber;				// 8 Bytes
	int	iChecksum;									// 2 Bytes
}BOOT_SECTOR_NTFS;


/* FAT32 Boot Partition Entry */
// NOT FINISHED !!!!!!!!!!
typedef struct FAT32_BOOT_SECTOR{									// Field Bytes	NTFS Cmt			  FAT32
													// --------------------------------------------------------------------
	int	iBytesPerSector;							// 2 Bytes		Bytes Per Sector	  "
	int	iSectorsPerCluster;							// 1 Bytes		Sectors Per Cluster	  "
	int iReservedSectors;							// 2 Bytes		Reserved Sectors	  "
	int iNoFATs;									// 1 Bytes		Always 0  			  #Fats(2)
	int iRootEntries;								// 2 Bytes		Not Used 00			  0
	int	iSmallSectors;								// 2 Bytes		N/A NTFS			  0
	int	iMediaDesc;									// 1 Bytes		Media Descriptor	  N/A
	int	iSectorsPerFAT;								// 2 Bytes		Always 00			  0
	int	iSectorsPerTrack;							// 2 Bytes		Sectors Pertrack
	int iNumOfHeads;								// 2 Bytes		NumOfDiskHeads
	int iHiddenSectors;								// 4 Bytes		Hidden Sectors
	int iLargeSectors;								// 4 Bytes		N/A NTFS			  #SectorsInFAT32Vol
	int iSectorsPerFat;								// 4 Bytes		N/A NTFS			  #SectorsThatMakeUpAFAT
	unsigned btNumActiveFAT	:4;								// 4 Bits							  # Of Active Fat (mrr dis)
	unsigned btReserved		:3;								// 3 Bits		MFT Start			  Reserved
	unsigned btMrrRuntime	:1;								// 1 Bit		MFT Mrr Start		  0=FAT Mrr At Runtime
	unsigned btReserved1		:8;								// 8 Bits		Cluster /MFT Rec	  Reserved
	unsigned int	uiMajorRev;						// 1 Bytes		Cluster /Idx Block    Major Revision # -HighByte DWORD
	unsigned int	uiMinorRev;						// 1 Bytes							  Minor Revision # -LowByte DWORD
	
}BOOT_SECTOR_FAT32;
/**********************************************************************************************************
 End outside class
**********************************************************************************************************/


/**********************************************************************************************************
 These following structs are always init'd within functions of this class and never init'd outside
 of this class although they may be accessed from outside of this class
**********************************************************************************************************/
typedef struct PHYDISK{
		char cName[255];						// Physical Disk Name (PhysicalDisk0, CdRom0..etc)
	}PHYSICAL_DISKS;
	PHYSICAL_DISKS;			


	typedef struct LOGDISK{
		char cName[5];						// Logical Disk Name stored as (c:\ , d:\, e:\ ..etc)
		int iDisk;							// Physical Disk This Volume Is On
		LARGE_INTEGER StartingOffset;		// IN BYTES ** Starting Offset From Start Of Phy Disk;
		int	iPartitionNo;
	}LOGICAL_DISKS;
	LOGICAL_DISKS;


/* MFT Cluster Offsets */ /* Must Be Public as external uses by calling class */
	 typedef struct MFTCLUSTERS{
		__int64	iStartCluster;			// Starting Cluster For MFT
		__int64 iEndCluster;			// Ending Cluster For MFT
		__int64 iMFTRecSize;			// Total Qty Of Current MFT Records 
	 }MFT_CLUSTERS;


/**********************************************************************************************************
 End 
**********************************************************************************************************/

// FindFile Flags
#define FILE_ALLDELETED		0x0100L			// Find all deleted files with name
#define FILE_DELETED		0x0200L			// Find only deleted files with name
#define	FILE_ACTIVE			0x0400L			// Find only active files with name
#define	FILE_RESTORE		0x0600L			// Restore File
#define	FILE_DESTROY		0x0800L			// Destory File
#define FILE_MFTSEARCH		0x1600L			// Search MFT Only
#define FILE_SECTORSEARCH	0x3200L			// Search Each Sector
#define	FILE_ANY			0x0FFFL			// Any file with other flag such as FILE_DELETED (means find all deleted files)
// Open handle types
#define	OPEN_NONE			0x0000L
#define	OPEN_PHYSICAL		0x0100L			// Physical Disk Open
#define	OPEN_VOLUME			0x0200L			// Volume/Logical Open
#define	OPEN_LOGICAL		0x0200L			// Volume/Logical Open
#define	OPEN_FILE			0x0400L			// File Open
#define OPEN_REMOTE			0x0F00L			// Remote machine + one of the above
// Open Format Type
#define FORMAT_NONE			0x0000L			// No format or unknown
#define	FORMAT_NTFS			0x0100L			// NTFS 4/5
#define FORMAT_FAT12		0x0200L			// FAT 12
#define FORMAT_FAT16		0x0400L			// FAT 16
#define FORMAT_FAT32		0x0800L			// FAT 32
#define FORMAT_CDFS			0x1600L			// CDFS - CD File System
#define FORMAT_ERROR		0x0FFFL			// error occured getting format
#define	FORMAT_UNSUPPORT	0xFFFFL			// Not currently supported
// Logical To Physical Conversions
#define CONVERT_TO_PARTITION	0xE100L
#define CONVERT_TO_OFFSET	    0xE200L
#define CONVERT_TO_PHYSICALDISK 0xE300L


// AFX Message Box 
#define MBR_MISSING					"Master Boot Record missing or corrupt !"
#define MBR_PHYSICAL_ONLY			"Master boot record can only be obtained from a physical disk !"
#define FINDFILE_UNKNOWNHANDLE		"Invalid drive specification !"
#define FILE_POINTER_SAVE_FAILED	"Failed To Save File Pointer !"

