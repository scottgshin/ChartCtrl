#pragma once
#include "afxwin.h"
#include "slidergdictrl.h"

// CDlgTruncate dialog

class CDlgTruncate : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTruncate)

public:
  typedef struct
    {
      V_CHARTDATAD vChartDataD;
      MAP_SELPNTSD mapSelPntsD;
    } STRUCT_STORAGE;

  typedef std::map<int, STRUCT_STORAGE> MAP_STORAGE;

public:
	CDlgTruncate(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTruncate();

  void InitControls(bool bEnable);

// Dialog Data
	enum { IDD = IDD_DLGTRUNCATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()

  afx_msg void OnSlPosChanged(NMHDR* nmhdr, LRESULT* lResult); 
  afx_msg void OnLbnSelchangeTruncateLbchartnames();
  afx_msg void OnBnClickedTruncatApply();
  afx_msg void OnBnClickedTruncateRange();
  afx_msg void OnBnClickedTruncatUndo();


// Data members
protected:
  CListBox m_lbChartNames;

  double m_truncStartX;
  double m_truncEndX;

  double m_currStartX;
  double m_currEndX;

  CButton m_chkRecalc;
  CButton m_chkRange;

  MAP_STORAGE m_mapStorage;

  SliderGdi::CSliderGdiCtrlT<double> m_slEndX;
  SliderGdi::CSliderGdiCtrlT<double> m_slStartX;
  CButton m_btnApply;
  CButton m_btnUndo;
};
