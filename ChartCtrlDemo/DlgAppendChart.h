#pragma once
#include "afxwin.h"
#include "ChartDef.h"
#include "SliderGDICtrl.h"


// CDlgAppendCharts dialog

class CChartContainer;

class CDlgAppendCharts : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAppendCharts)

public:
  typedef std::map<int, PAIR_DBLS> MAP_POS;

public:
	CDlgAppendCharts(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAppendCharts();

// Dialog Data
	enum { IDD = IDD_DLGAPPEND };

  void InitControls(bool bEnable);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  void GenerateAppendData(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);
  size_t AppendSinWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);  
  size_t AppendSincWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);
  size_t AppendExp(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);
  size_t AppendRectWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);
  size_t AppendRandomNmbs(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData);

  PAIR_DBLS PrepareToAppendCharts(CChartContainer* pContainer, bool bAnimate = false);
  bool ApplyAppend(bool bUpdateX = true);
  void UndoAppend(void);

	DECLARE_MESSAGE_MAP()

  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedAppend();
  afx_msg void OnBnClickedUndoAppend();
  afx_msg void OnBnClickedSave();

  afx_msg void OnTimer(UINT_PTR nIDEvent);

protected:
  CListBox m_lbChartNames;
  SliderGdi::CSliderGdiCtrlT<double> m_slAppendX;
  CButton m_chkAnimate;
  CButton m_btnAppend;
  CButton m_btnUndo;
  CButton m_btnSave;

  bool m_bAppend;
  PAIR_DBLS m_extX;
  double m_animEndX;
  double m_appendDelta;
  V_HIST m_vSavedHist;
  MAP_POS m_mapPosX;
public:
  std::mt19937 m_rndGenNmbs;

};
