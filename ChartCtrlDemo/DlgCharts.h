#pragma once

// CDlgCharts dialog

class CDlgCharts : public CDialogEx
{
  enum {CHART_MINX = 450, CHART_MINY = 200, CHART_SPACE = 80};

	DECLARE_DYNAMIC(CDlgCharts)

public:
	CDlgCharts(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCharts();

  void InitDlgCharts(const CChartContainer& chartContainer);

// Dialog Data
	enum { IDD = IDD_DLGCHARTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  virtual void OnCancel();

protected:
  HICON m_hIcon;
public:
  CChartContainer m_chartContainer;
  virtual BOOL OnInitDialog();
};
