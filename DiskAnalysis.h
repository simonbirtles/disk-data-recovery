#pragma once
#include "diskioctl.h"
/*****************************************************************************************************

						Header & Class Def for CDiskAnalysis

******************************************************************************************************/
class CDiskAnalysis
{
private:
	HANDLE m_HndData;			// Pointer To Data either passed calling 
	//int m_iSize;				// Integer for size of m_pData
	//__int64	*iMFTClusters;		// Array of MFT Clusters

public:
	MFT_CLUSTERS			*m_ClusterArray;			// MFT Clusters Array
	FILE_CLUSTERS			*m_FileDetails;				// Find File Structure Details.
	PHYSICAL_DISKS			*m_physicaldisks;
	LOGICAL_DISKS			*m_logicaldisks;
	MASTER_BOOT_RECORD		*m_HDPartitionTable;		// Master Boot Record Struct (Contains Partition Table)
    BOOT_SECTOR_NTFS		 m_PartitionBootSector;		// NTFS Boot Sector
	BOOT_SECTOR_FAT32		 m_FAT32PartitionBootSector;// FAT32 Boot Sector
	DWORD					 m_HandleType;				// Indicated what type of handle is open (ie Logical, Phy)
	int						 m_iDiskOpen;				// Stores the Physical Disk # that is currently open

	HGLOBAL hGlobalPT;



public:
	CDiskAnalysis(void);
	virtual ~CDiskAnalysis(void);

protected:
	
	int GetFileExtents(BYTE* bData, int iExtentOffset, FILE_CLUSTERS* sFileExtents);
	void GetFilePathMFT(__int64 dwParentRecord, __int64 dwSeq, char* ptrFilePath, int iPartition);
	DWORD GetAttrDef(CString strAttribute);
	BOOL GetPhysicalDisks(void);
	BOOL GetLogicalDisks(void);
	BOOL GetNTFSPartitionTable(int iPartition);
	BOOL GetMFTOffsets(void);
	//BOOL GetMBR(void);

	int GetMBR(BYTE bParent = 0x00 , DWORD64 dwRelSector = 0, DWORD dwRecno = 0);
	
    int FindFileMFT(CString strFileName, int iPartition, FF_PARAMS* pParam);
	DWORD GetPartitionFormatType(__int64 iPartitionStartSector);
	BOOL OpenHandle(LPCTSTR lpctPath);
	__int64 LogicalToPhysical(char cDriveLetter, DWORD dwRetType);
	void CloseDiskHandle(void);
	//BOOL RecoverFileMFT(__int64 iStartSector, __int64 iFileSize, char* czRecoverPath);
	BOOL CDiskAnalysis::RecoverFileMFT(FILE_CLUSTERS *pFile);
	BOOL IsStringMatched(char* Wildcards, char* str);
	BOOL Scan(char*& Wildcards, char*& str);
	bool Match(char* Wildcards, char* str)   ;
	BOOL IsStringMatchedW(CString czWild, CString str);
	__int64 MFTReadVar(BYTE *pSrc, DWORD nLen);
	DWORD GetMFTAttrOffset(BYTE* byMFTRecord, DWORD dwAttribute, int iCount = 0);
	// Fills the selected partition struct with the relevent data based on format type
	BOOL GetPartitionBootSectorInfo(int iPartitionNumber = -1);
	// we may not even need this func
	BOOL GetINT13Info(LPCSTR lpPhysicalDisk, DA_DISK_EX_INT13_INFO* DiskInt13Info);
	DWORD CDiskAnalysis::GetFormatTypeText(DWORD dwType);
	__int64 CalcMFTRecord(DWORD dwRecNo);

public:	
    // structs HD_PartitionTable,LOGICALDISK, PHYSICALDISK,will be filled after success
	BOOL FillPartitionInfo(LPCTSTR lpPhysicalDiskName);
    // Call FindFile to find a file on any disk that has a logical no assigned
	int CDiskAnalysis::FindFile(FF_PARAMS* pParam, FILE_CLUSTERS* pFile);
	// Call recover file after a successful find file with the pFile pointer at the vaild record
	BOOL RecoverFile(FILE_CLUSTERS *pFile);
	// Get MFT Record Details, returns record in raw form in byData
	BOOL GetMFTRecord(char cDrive, int iRecordNo, BYTE* byData, int iDataSize);
	
	

	

};
