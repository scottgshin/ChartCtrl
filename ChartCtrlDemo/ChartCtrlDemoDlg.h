
// ChartCtrlDemoDlg.h : header file
//

#include "ChartDef.h"
#include "ChartContainer.h"

#pragma once

class CDlgGenProp;
class CDlgAddChart;
class CDlgChange;
class CDlgAppendCharts;
class CDlgTruncate;
class CDlgRemoveChart;
class CDlgMisc;

typedef struct
{
  int funcID;
  double lastX;
  double deltaX;
  double alpha;
  double multY;
  size_t pntsNmb;
  size_t period;
} STRUCT_FUNCDATA;

typedef std::map<int, STRUCT_FUNCDATA > MAP_FUNCTIONS;

int GetRangePrecision(double minX, double maxX, int stepsNmb);

///////////////////////////////////////////////////////////////////////////////
// CChartCtrlDemoDlg dialog

class CChartCtrlDemoDlg : public CDialogEx
{
// Construction
public:
	CChartCtrlDemoDlg(CWnd* pParent = NULL);	// standard constructor
  virtual ~CChartCtrlDemoDlg();

// Dialog Data
	enum { IDD = IDD_CHARTDEMO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation

public:
  void UpdateFuncMap(void);

protected:
	HICON m_hIcon;

	DECLARE_MESSAGE_MAP()

// Generated message map functions
	virtual BOOL OnInitDialog();
  virtual void OnCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
//public:
  afx_msg LRESULT OnTabChanged(WPARAM tabID , LPARAM);
  afx_msg void OnChartVisibilityChanged(NMHDR*, LRESULT*); 


public:
  CChartContainer m_chartContainer;
  MAP_FUNCTIONS m_mapFunctions;

  CStatic m_tabLoc;
  CMFCTabCtrl m_tabCtrl;

  CDlgGenProp* m_pDlgGenProp;
  CDlgAddChart* m_pDlgAddChart;
  CDlgChange* m_pDlgChange;
  CDlgAppendCharts* m_pDlgAppendChart;
  CDlgTruncate* m_pDlgTruncate;
  CDlgRemoveChart* m_pDlgRemoveChart;
  CDlgMisc* m_pDlgMisc;

};
