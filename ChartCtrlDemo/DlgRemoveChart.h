#pragma once
#include "afxwin.h"


// CDlgRemoveChart dialog

class CDlgRemoveChart : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgRemoveChart)

public:
	CDlgRemoveChart(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRemoveChart();

// Dialog Data
	enum { IDD = IDD_DLGREMOVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void ApplyRemove(void);

public:
  virtual BOOL OnInitDialog();
  void InitControls(bool bEnabled);



	DECLARE_MESSAGE_MAP()

// Data members
protected:
  CListBox m_lbChartNames;
  CButton m_chkRecalc;
  CButton m_btnApply;
public:
  afx_msg void OnBnClickedRemove();
};
