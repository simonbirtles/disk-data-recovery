#pragma once


// CFileOptions dialog

class CFileOptions : public CPropertyPage
{
	DECLARE_DYNAMIC(CFileOptions)

public:
	CFileOptions();
	virtual ~CFileOptions();

// Dialog Data
	enum { IDD = IDD_FILEOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
