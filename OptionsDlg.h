#pragma once
#include "afxcmn.h"
#include "afxwin.h"





// COptionsDlg dialog

class COptionsDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg();   // standard constructor
	virtual ~COptionsDlg();

// Dialog Data
	enum { IDD = IDD_GLOBALOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CStringList fontlist;

	

	

public:

private:
	CComboBox	m_TextAreaList;
	CComboBox	m_cboFont;
	CComboBox	m_cboFontSize;
	CButton		m_bForeground;
	CButton		m_bBackground;
	CButton		m_cBold;
	CRect		m_rSample;
	CFont		m_font;
	int			m_iCboTextSel;


	CString		m_strFontName;
	int			m_iFontSize;
	BOOL		m_bBold;
	COLORREF	m_crForeGround;
	COLORREF	m_crBackGround;
	BOOL		m_bFontDirty;
	BOOL		m_bFontChanged;			// flag to call WM_???CHANGE
	BOOL		m_bCaretBlock;


	void ProcessFonts(void);
	static int CALLBACK EnumFontFamExProc(
    ENUMLOGFONTEX *lpelfe,	// pointer to logical-font data
    NEWTEXTMETRICEX *lpntme,	// pointer to physical-font data
    int FontType,	// type of font
    LPARAM lParam	// application-defined data
	 );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCbofonttype();
private:
	void SetFontSample(void);
	void ProcessFontSize(void);
public:
	afx_msg void OnCbnSelchangeFontsize();

	afx_msg void OnBnClickedBold();
	void ProcessText(void);
private:
	void ProcessSystem(void);		//misc font functions
public:
	afx_msg void OnBnClickedBforegnd();
	afx_msg void OnBnClickedBbackgnd();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
private:


public:
	afx_msg void OnCbnSelchangeFontitem();
private:
	void SaveFontSettings(void);
	void GetFontSettings(void);
	void ProcessBold(void);
public:
	afx_msg void OnBnClickedCaretline();
	afx_msg void OnBnClickedCaretblock();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnApply();
	virtual void OnOK();
};
