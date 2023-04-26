#pragma once
#include "afxwin.h"
#include "afxcolorbutton.h"
#include "ChartDef.h"
#include "ChartCtrlDemoDlg.h"
#include "slidergdictrl.h"

class CChartContainer;

// CDlgAddChart dialog

class CDlgAddChart : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAddChart)

public:
	CDlgAddChart(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAddChart();

// Dialog Data
	enum { IDD = IDD_DLGADDCHART };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
  STRUCT_FUNCDATA GenerateData(int funcID, double fMinX, double fMaxX, size_t pntsNmb, 
                                                           V_CHARTDATAD& vData, double multY);
  double GenerateSinWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
                                                                                 double multY);  
  double GenerateSincWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
                                                                                 double multY);
  double GenerateExp(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
                                                                                 double multY);
  double GenerateRectWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
                                                                                 double multY);
  double GenerateRandomNmbs(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
                                                                                  double multY);

protected:

	DECLARE_MESSAGE_MAP()

  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedBtnpencolor();
  afx_msg void OnBnClickedBtnadd();

  afx_msg void OnMultYNotification(NMHDR*, LRESULT*); 

protected:
// Controls
  CMFCColorButton m_colBtnPenCol;
  CListBox m_lbDash;
  CListBox m_lbFunc;
  CListBox m_lbPoint;
  SliderGdi::CSliderGdiCtrlT<float> m_slPenWidth;
  SliderGdi::CSliderGdiCtrlT<double> m_slMinX;
  SliderGdi::CSliderGdiCtrlT<double> m_slMaxX;
  SliderGdi::CSliderGdiCtrlT<unsigned long> m_slPntsNmb;
  SliderGdi::CSliderGdiCtrlT<int> m_slMultOrder;
  SliderGdi::CSliderGdiCtrlT<int> m_slPrecisionY;

  CEdit m_edYMult;
  CEdit m_edChartName;
  CEdit m_edValYName;
  CButton m_chkShowPnts;

// Data members
  int m_chartIdx;
  string_t m_strName;
  string_t m_strYValName;
  const string_t m_defaultYName; 
  int m_precisionY;
// Visuals
  int m_tension;
  float m_fPenWidth;  // CChart Pen Width and Color
  Gdiplus::Color m_colChart;
  CString m_strRGB;
 
  int m_showPnts;

// List boxes
  int m_dashStyle;    // Actually it is Gdiplus::DashStyle
  int m_func;         // 0 - sin, 1 sin(x)/x, 2 - exp, 3 - rect, 4 - random, 5 - x = y^2.5
  int m_pointStyle;   // Actually it is a Gdiplus::PointStyle

  double m_multY;
  CString m_strMultY;
// Chart extensions and number of data points
  double m_fChMinX;
  double m_fChMaxX;
  unsigned long m_nPntsNmb;

  CChartContainer* m_pContainer;
public:
  std::mt19937 m_rndGenNmbs;

};
