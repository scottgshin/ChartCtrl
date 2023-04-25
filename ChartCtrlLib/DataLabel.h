///////////////////////////////////////////////////////////////////////////////
//
// DataLabel.h - header for data label wnd
//
//
///////////////////////////////////////////////////////////////////////////////

#include "ChartDef.h"

#pragma once

using namespace Gdiplus;

// Data Wnd ID
#define IDW_LABELS    99

// Data label window layout
#define LB_FONTSIZE  7
#define LB_BULLETSZ  20
#define LB_BORDERSPACE  3
#define LB_STROFFSX 2*LB_BORDERSPACE + LB_BULLETSZ
#define LB_WNDDST 10;

#define LABEL_OFFS     6



///////////////////////////////////////////////////////////////////////////////
// CDataWnd

class CDataWnd : public CWnd
{
	DECLARE_DYNAMIC(CDataWnd)

public:
	CDataWnd();
  CDataWnd(Color bkCol, Color borderCol);
	virtual ~CDataWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnPaint();
public:
  bool IsDataLeg(void) const {return m_bData;}

  string_t GetFontFamilyName() const {return m_strFontFamilyName;}
  void SetFontFamilyName(string_t fontFamName) {m_strFontFamilyName = fontFamName;}

// Font size in points (1/72")
  int GetFontSize(void) const {return m_fontSize;}
  void SetFontSize(int fontSize) { m_fontSize = fontSize;}

  CRect GetDataWndRect(void) const {return m_labelWndRect;}
  CRect GetDataWndStrRect(void) const {return m_labelStrRect;}

  Color GetBkColor(void) const {return m_colBkgnd;}
  void SetBkColor(Color bkCol, bool bRedraw = false);

  Color GetBorderColor(void) const {return m_colBorder;}
  void SetBorderColor(Color borderCol, bool bRedraw = false);

// Get labelWnd rectangle (in client coordinates of the ChartContainer wnd
  CRect GetLabWndRect(void) const {return m_labelWndRect;}
  void DrawDataWnd(int OffsX, int offY, Graphics* grPtr);

protected:
  CRect CalcLabelStrRect(void);    // Bounding rectangle for legend strings
// Child windows rectangles
  CRect CalcNameWndRect(CRect labStrRect, CWnd* pParent);
  CRect CalcDataWndRect(CRect& labStrRect, CWnd* pParent, CPoint origPnt);

  float DrawLabel(const TUPLE_LABEL& tupleLabel, RectF labRect, Gdiplus::Font* pFont, 
                                            Graphics* grPtr);
  void DrawLabels(Graphics* grPtr, float offsXF = 0.0f, float offsYF = 0.0f);

  void CDataWnd::DrawLabelLine(RectF labelRectF, Color labCol, 
                            DashStyle dashStyle, float penWidth, Graphics* grPtr);

public:
  bool CreateLegend(CWnd* pOwner);
  bool CreateLegend(CWnd* pOwner, const CPoint origPnt, bool bData);
  bool ShowLegendWnd(void);
  bool UpdateDataLegend(MAP_LABSTR& mapLabStr, CWnd*pParent, CPoint origPnt);

// Data members
public:
  MAP_LABSTR m_mapLabs; 

protected:
  bool m_bData;
  string_t m_strFontFamilyName;
  int m_fontSize;               // Font size in points
// Next members are for calculating leg wnd pos and size
  float m_fOrigOffsX;           // Strings offset in the data/names widows
  float m_fBorderSpace;         // Is #defined in ChartDef.h
  float m_fBulletSize;          // Same
// Layout's rectangles
  RectF m_strRF;                // Bounding rect for string max len
  RectF m_maxNameRF;            // Bounding rect for name
  RectF m_maxNameXRF;           // Bounding rect for the nameX
  RectF m_maxXRF;               // Bounding rect for X value
  RectF m_maxNameYRF;           // Bounding rect for the name Y
  RectF m_maxYRF;               // Bounding rect for Y value
  CRect m_labelStrRect;         // LabelStr rect
  CRect m_labelWndRect;         // Total window

  Color m_colBkgnd;
  Color m_colBorder;

};
