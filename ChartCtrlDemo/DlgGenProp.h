#pragma once
#include "slidergdictrl.h"
#include "afxwin.h"
#include "afxcolorbutton.h"
#include "ChartDef.h"

///////////////////////////////////////////////////////////////////////////////
// GetLabelValStrFn

string_t __stdcall GetLabNamedValStr(double val, int precision);

///////////////////////////////////////////////////////////////////////////////


enum FLAGS {
             F_LABELX = 0x0001,       F_PRECISION = 0x0002,   F_EXTX = 0x0004,         F_COLBKGND = 0x0008,   
             F_COLBORDER = 0x0010,   F_COLAXES = 0x0020,     F_COLZOOM = 0x0040,      F_COLLEGPNTS = 0x0080, 
             F_COLDATAPNTS = 0x0100, F_COLLEGBKGND = 0x0200, F_COLLEGBORDER = 0x0400, F_COLGRID = 0x0800,     
             F_ALLVALS = 0x0007,    
             F_ALLCOLS = 0xFF8
           };

enum FLAGS_TU {FTU_LABELX = 0, FTU_PREC = 1, FTU_STARTX = 2, FTU_ENDX = 3, 
               FTU_NEW = 4, FTU_UNDO = 5, FTU_DEFAULT = 6};

typedef std::map<short, Gdiplus::Color> MAP_COLORS;
typedef std::vector<MAP_COLORS> V_MAPCOLORS;
typedef std::tuple<string_t, int, double, double> TUPLE_VALS;
typedef std::vector<TUPLE_VALS> V_TUPLEVALS;

class CChartContainer;

///////////////////////////////////////////////////////////////////////////////
// CStColors

class CStColors : public CStatic
{
	DECLARE_DYNAMIC(CStColors)

public:
	CStColors();
	virtual ~CStColors();

  void RefreshCtrl(MAP_COLORS& mapCols, bool bRedraw = true);

protected:
  void DrawBkgnd(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);
  void DrawAxes(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);
  void DrawZoomBorder(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);
  void DrawLegPnts(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);
  void DrawDataPnt(Gdiplus::Graphics* grPtr);
  void DrawLegWnd(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);
  void DrawGrid(Gdiplus::RectF rClF, Gdiplus::Graphics* grPtr);

	DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();

// Data members
  MAP_COLORS m_mapCols;
  V_CHARTDATAF  m_vPntsF;

  Gdiplus::PointF m_legPntF;
  Gdiplus::PointF m_dataPntF;
};

// CDlgGenProp dialog

class CDlgGenProp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGenProp)

public:
	CDlgGenProp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGenProp();

// Dialog Data
	enum { IDD = IDD_DLGGENPROP };

public:
  void InitControls(bool bEnable);
  virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  void OnExtTypeChanged(void);
  void ApplyVals(CChartContainer* pContainer, short flags, TUPLE_VALS& tuple_vals);
  void ApplyVals(CChartContainer* pContainer, short flags, int action);
  void ApplyColors(CChartContainer* pContainer, short flags, MAP_COLORS& mapCols);

	DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnSlPosChanged(NMHDR* nmhdr, LRESULT* lResult); 
  afx_msg void OnBnClickedGenpropExtX();
  afx_msg void OnBnClickedGenpropExtY();
  afx_msg void OnBnClickedGenpropBtnapply();
  afx_msg void OnColRbtnClicked();
  afx_msg void OnBnClickedGenpropCols();
  afx_msg void OnBnClickedGenpropUndo();
  afx_msg void OnBnClickedGenpropDefault();
  afx_msg void OnEnChangeGenpropEdnamex();
  afx_msg void OnBnClickedGenpropBtndone();

// Data members
protected:
  CEdit m_edNameX;
  SliderGdi::CSliderGdiCtrlT<int> m_slPrecision;
  SliderGdi::CSliderGdiCtrlT<double> m_slExtLT;
  SliderGdi::CSliderGdiCtrlT<double> m_slExtRB;
  CStColors m_stColors;
  CMFCColorButton m_btnColors;
  CStatic m_stColorTxt;

  CButton m_rbtnColBkgnd;
  CButton m_rbtnColBorder;
  CButton m_rbtnColAxes;
  CButton m_rbtnColZoom;
  CButton m_rbtnColLegPnts;
  CButton m_rbtnColDataPnts;
  CButton m_rbtnColLegBkgnd;
  CButton m_rbtnColLegBorder;
  CButton m_rbtnColGrid;
  CButton m_btnUndo;
  CButton m_btnDefault;

public:
  bool m_bValsChanged;
  bool m_bColsChanged;
  unsigned short m_flags;
  int m_colorType;

  TUPLE_VALS m_tupleValsNew;
  TUPLE_VALS m_tupleValsOld;
  TUPLE_VALS m_tupleValsDefault;
  V_TUPLEVALS m_vTupleVals;
  V_MAPCOLORS m_vMapColors;

  MAP_COLORS m_mapColsNew;
  MAP_COLORS m_mapColsOld;
  MAP_COLORS m_mapColsDefault;
  
};



