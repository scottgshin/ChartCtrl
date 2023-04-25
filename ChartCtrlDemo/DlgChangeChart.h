#pragma once
#include "afxwin.h"
#include "afxcolorbutton.h"
#include "slidergdictrl.h"

///////////////////////////////////////////////////////////////////////////////
// CDlgChange dialog

class CChartContainer;

class CDlgChange : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgChange)

  enum {CH_CHARTNAME, CH_NAMEY, CH_PRECY, CH_CHARTCOL, CH_DASH, CH_PENW, CH_TENSION};
  typedef std::tuple<string_t, string_t, int, Gdiplus::Color, int, float,float> TUPLE_CHANGE;
  typedef std::map<int, TUPLE_CHANGE> MAP_CHANGE;

public:
	CDlgChange(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgChange();
  void SetControls(int selIdx, CChartContainer* pContainer);
  void OnChartVisibilityChanged(int chartIdx, bool bVisible, CChartContainer* pContainer);

// Dialog Data
	enum { IDD = IDD_DLGCHANGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

public:
  void InitControls(bool bEnable, bool bClear);

	DECLARE_MESSAGE_MAP()

  afx_msg void OnBnClickedChangeBtnapply();
  afx_msg void OnBnClickedChangeBtnundo();
  afx_msg void OnCbnSelchangeChangeCbnames();
  afx_msg void OnLbnSelchangeChangeLbnames();

public:
  CListBox m_lbNames;
protected:
  CEdit m_edName;
  CMFCColorButton m_btnColor;
  CEdit m_edCols;
  CEdit m_edNameY;
  SliderGdi::CSliderGdiCtrlT<int> m_slPrecY;
  CListBox m_lbDash;
  SliderGdi::CSliderGdiCtrlT<float> m_slPenWidth;
  SliderGdi::CSliderGdiCtrlT<float> m_slTension;
 
  int m_chartIdx;
  string_t m_chartName;
  string_t m_nameY;
  Gdiplus::Color m_chartCol;
  int m_dashStyle;
  int m_precisionY;
  float m_penWidth;
  float m_tension;

  MAP_CHANGE m_mapChange;
};
