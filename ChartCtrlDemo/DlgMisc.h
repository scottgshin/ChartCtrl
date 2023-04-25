#pragma once
#include "afxwin.h"
#include "DlgCharts.h"
#include "afxcolorbutton.h"


// CDlgMisc dialog

class CDlgMisc : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMisc)

public:
	CDlgMisc(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMisc();

  string_t ColorToString(Gdiplus::Color color);

  void InitControls(bool bEnable);
  void InitChartNamesListBox(const MAP_CHARTCOLS& mapContent);
  

// Dialog Data
	enum { IDD = IDD_DLGMISC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

  afx_msg void OnBnClickedClone();

  afx_msg void OnBnClickedLoad();
  afx_msg void OnBnClickedApply();

  afx_msg void OnLbColsSelChange();
  afx_msg void OnBnClickedCols();
  afx_msg void OnLbSelChangeCharts();

protected:
  string_t m_pathName;

  CStatic m_stFileName;
  CButton m_btnClone;
  CListBox m_lbChartNames;
  CListBox m_lbCols;
  CMFCColorButton m_btnCols;
  CStatic m_stCols;

  CDlgCharts m_dlgCharts;
};
