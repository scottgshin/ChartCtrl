///////////////////////////////////////////////////////////////////////////////
//
// DataLabel.cpp - implementation of the data label
//
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "resource.h"
#include "ChartDef.h"
#include "Chart.h"
#include "ChartContainer.h"
#include "DataLabel.h"
#include "util.h"

using namespace std;
using namespace Gdiplus;


// CDataWnd

IMPLEMENT_DYNAMIC(CDataWnd, CWnd)

CDataWnd::CDataWnd() : m_fontSize(LB_FONTSIZE), m_fOrigOffsX(LB_STROFFSX),
                       m_fBorderSpace(LB_BORDERSPACE), m_fBulletSize(LB_BULLETSZ),
                       m_colBkgnd(Color(0xFFFFFFE0)), m_colBorder((ARGB)Color::Black)
{
  m_strFontFamilyName = string_t(_T("Verdana"));
}

CDataWnd::CDataWnd(Color bkCol, Color borderCol) :
                          m_fontSize(LB_FONTSIZE), m_fOrigOffsX(LB_STROFFSX),
                          m_fBorderSpace(LB_BORDERSPACE), m_fBulletSize(LB_BULLETSZ),
                          m_colBkgnd(bkCol), m_colBorder(borderCol)
{
  m_strFontFamilyName = string_t(_T("Verdana"));
}

CDataWnd::~CDataWnd()
{
}

void CDataWnd::SetBkColor(Gdiplus::Color bkCol, bool bRedraw)
{
  m_colBkgnd = bkCol;
  if (bRedraw)
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

void CDataWnd::SetBorderColor(Gdiplus::Color borderCol, bool bRedraw)
{
  m_colBorder = borderCol;
  if (bRedraw)
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

CRect CDataWnd::CalcLabelStrRect(void)
{
  size_t labStrNmb = m_mapLabs.size();
  if (labStrNmb == 0)
    return CRect();

// Extract typles from the map into a vector
  V_LABSTR vLabStr(m_mapLabs.size()); 
  transform(m_mapLabs.begin(), m_mapLabs.end(), vLabStr.begin(), 
                                                   get_map_value<int, TUPLE_LABEL>());
// Find selStr max length
// Measure strings: Create font
  FontFamily fontFamily(m_strFontFamilyName.c_str());
  Gdiplus::Font labelFont(&fontFamily, static_cast<float>(m_fontSize), FontStyleBold);
// Measure the max strings
  CDC* pDC = GetDC();
  Graphics gr(pDC->m_hDC);
  gr.SetTextRenderingHint( TextRenderingHintAntiAliasGridFit);

  PointF pntF(m_fOrigOffsX, 0.0f);

  m_maxNameRF = for_each(vLabStr.begin(), vLabStr.end(), 
    get_max_str<TUPLE_LABEL, IDX_LNAME>(&labelFont, &gr))._maxRF; 
  m_maxNameRF.Width += m_fBorderSpace;

  if (m_bData)
  {
    string_t colonStr(_T(": "));
    RectF colonRF;
    gr.MeasureString(colonStr.c_str(), -1, &labelFont, PointF(0.0f, 0.0f), &colonRF);
    m_maxNameRF.Width += colonRF.Width;
    m_maxNameRF.Offset(pntF);

    pntF.X = m_maxNameRF.GetRight();
    m_maxNameXRF = for_each(vLabStr.begin(), vLabStr.end(), 
                      get_max_str<TUPLE_LABEL, IDX_LNAMEX>(&labelFont, &gr))._maxRF; 
    if (!m_maxNameXRF.IsEmptyArea())
      m_maxNameXRF.Width += m_fBorderSpace;
    m_maxNameXRF.Offset(pntF);

    pntF.X = m_maxNameXRF.GetRight();
    m_maxXRF = for_each(vLabStr.begin(), vLabStr.end(), 
                      get_max_str<TUPLE_LABEL, IDX_LX>(&labelFont, &gr))._maxRF; 
    m_maxXRF.Width += m_fBorderSpace;
    m_maxXRF.Offset(pntF);

    pntF.X = m_maxXRF.GetRight();
    m_maxNameYRF = for_each(vLabStr.begin(), vLabStr.end(), 
                      get_max_str<TUPLE_LABEL, IDX_LNAMEY>(&labelFont, &gr))._maxRF; 
    if (!m_maxNameYRF.IsEmptyArea())
      m_maxNameYRF.Width += m_fBorderSpace;
    m_maxNameYRF.Offset(pntF);

    pntF.X = m_maxNameYRF.GetRight();
    m_maxYRF = for_each(vLabStr.begin(), vLabStr.end(), 
                      get_max_str<TUPLE_LABEL, IDX_LY>(&labelFont, &gr))._maxRF; 
    m_maxYRF.Width += m_fBorderSpace;
    m_maxYRF.Offset(pntF);

    pntF.X = m_maxYRF.GetRight();
  }
  else
  {
    m_maxNameRF.Offset(pntF.X, 0.0f);
    pntF.X = m_maxNameRF.GetRight();
  }
 
  m_strRF = RectF(0.0f, 0.0f, pntF.X, m_maxNameRF.Height);

// Set dataWnd rectangle
  RectF labRF(0.0f, 0.0f, m_strRF.Width, labStrNmb*m_strRF.Height + m_fBorderSpace); // Client
  CRect wndRect = RectFToCRect(labRF);
  int bX = GetSystemMetrics(SM_CXBORDER);
  int bY = GetSystemMetrics(SM_CYBORDER);
  wndRect.InflateRect(bX, bY);
  wndRect.OffsetRect(bX, bY);

  ReleaseDC(pDC);

  return wndRect;
}

CRect CDataWnd::CalcNameWndRect(CRect labStrRect, CWnd* pParent)
{
  CRect wndRect = labStrRect;
  wndRect.OffsetRect(-2*LABEL_OFFS, LABEL_OFFS);

// Position it in the parent window
  CRect clRect;
  pParent->GetClientRect(&clRect);
  CPoint origPnt = CPoint(clRect.right, clRect.top);
  wndRect.OffsetRect(origPnt.x, 0);
  wndRect.OffsetRect(-wndRect.Width()+ LABEL_OFFS, 0);
  return wndRect;
}

CRect CDataWnd::CalcDataWndRect(CRect& labStrRect, CWnd*pParent, CPoint origPnt)
{
  CRect wndRect = labStrRect;
  int dataRectWidth = wndRect.Width();

  CRect parentRect;
  pParent->GetClientRect(&parentRect);

  CRect intersectRect;
  if (origPnt.x < parentRect.Width()/2)
  {
    wndRect.left = origPnt.x + LABEL_OFFS;
    wndRect.right = wndRect.left + dataRectWidth;
    intersectRect.IntersectRect(&parentRect, &wndRect);
    if (!intersectRect.EqualRect(wndRect))
    {
      wndRect.right = parentRect.right - 1;
      wndRect.left = wndRect.right - dataRectWidth;
    }
  }
  else
  {
    wndRect.right = origPnt.x - LABEL_OFFS;
    wndRect.left = wndRect.right - dataRectWidth;
    intersectRect.IntersectRect(&parentRect, &wndRect);
    if (!intersectRect.EqualRect(&wndRect))
    {
      wndRect.left = parentRect.left + 1;
      wndRect.right = wndRect.left + dataRectWidth;
    }
  }

  int offsY = parentRect.bottom - wndRect.Height() - LABEL_OFFS;

  wndRect.OffsetRect(0, offsY);
  return wndRect;
}

bool CDataWnd::CreateLegend(CWnd* pParent)
{
  BOOL bRes = TRUE;

  if (!IsWindow(m_hWnd))
    bRes = CreateEx(WS_EX_TRANSPARENT|WS_EX_NOACTIVATE,
                  AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS),
                  NULL,         // Wnd Title (Caption string ?
                  WS_CHILD|WS_VISIBLE, //|WS_CLIPSIBLINGS,//|WS_VISIBLE,
                  0, 0, 0, 0,
                  pParent->GetSafeHwnd(),
                  NULL,
                  0);
  return bRes ? true : false;
}

bool CDataWnd::CreateLegend(CWnd* pParent, CPoint origPnt, bool bData)
{
  BOOL bRes = TRUE;

  if (!IsWindow(m_hWnd))
    bRes = CreateEx(WS_EX_TRANSPARENT|WS_EX_NOACTIVATE,
                  AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS),
                  NULL,         // Wnd Title (Caption string ?
                  WS_CHILD|WS_VISIBLE, //|WS_CLIPSIBLINGS,//|WS_VISIBLE,
                  0, 0, 0, 0,
                  pParent->GetSafeHwnd(),
                  NULL,
                  0);

  if (bRes) // Calculate the layout
  {
    m_bData = bData;

// Now calculate the layout
    m_labelStrRect = CalcLabelStrRect();
    if (bData)
      m_labelWndRect = CalcDataWndRect(m_labelStrRect, pParent, origPnt);
    else
      m_labelWndRect = CalcNameWndRect(m_labelStrRect, pParent);
    return true;
  }
  return false;
}

bool CDataWnd::ShowLegendWnd()
{
  if (!IsWindow(m_hWnd)) 
    return false;
  if (!m_mapLabs.empty())
  {
    MoveWindow(&m_labelWndRect);
    Invalidate(FALSE);
    ShowWindow(SW_SHOWNOACTIVATE);
  }
  else
    ShowWindow(SW_HIDE);
  return true;
}

bool CDataWnd::UpdateDataLegend(MAP_LABSTR& mapLabStr, CWnd*pParent, CPoint origPnt)
{
  if (IsWindow(m_hWnd))
  {
    if (mapLabStr.empty())
    {
      ShowWindow(SW_HIDE);
      return false;
    }
    m_mapLabs = mapLabStr;
    m_labelStrRect = CalcLabelStrRect();
    m_labelWndRect = CalcDataWndRect(m_labelStrRect, pParent, origPnt);
    return ShowLegendWnd();
  }
  return false;
}

// Draw one string
float CDataWnd::DrawLabel(const TUPLE_LABEL& tupleLabel, RectF labRect, Gdiplus::Font* pFont, Graphics* grPtr)
{
  bool bName = true;
  string_t nameStr  = get<IDX_LNAME>(tupleLabel);
  string_t nameXStr, valXStr, nameYStr, valYStr;
  if (m_bData)
  {
    bName = false;
    nameStr += string_t(_T(":"));
    nameXStr = get<IDX_LNAMEX>(tupleLabel);
    valXStr  = get<IDX_LX>(tupleLabel);
    nameYStr = get<IDX_LNAMEY>(tupleLabel);
    valYStr  = get<IDX_LY>(tupleLabel);
  }

  Color labCol(get<IDX_LCOLOR>(tupleLabel));
  DashStyle dashStyle = get<IDX_LDASH>(tupleLabel);
  float penWidth = get<IDX_LPEN>(tupleLabel);

  DrawLabelLine(labRect, labCol, dashStyle, penWidth, grPtr);
  SolidBrush lbBr(labCol);

// Get the string orig
  RectF maxNameRF = m_maxNameRF, maxNameXRF = m_maxNameXRF, maxNameYRF = m_maxNameYRF;
  RectF maxValXRF = m_maxXRF, maxValYRF = m_maxYRF;
  maxNameRF.Offset(labRect.X, labRect.Y);
  maxNameXRF.Offset(labRect.X, labRect.Y);
  maxValXRF.Offset(labRect.X, labRect.Y);
  maxNameYRF.Offset(labRect.X, labRect.Y);
  maxValYRF.Offset(labRect.X, labRect.Y);
  PointF strOrigF(maxNameRF.X, labRect.Y);
  grPtr->DrawString(nameStr.c_str(), -1, pFont, strOrigF, &lbBr);

  if (m_bData)
  {
    if (!nameXStr.empty())
    {
      strOrigF.X = maxNameXRF.X;
      grPtr->DrawString(nameXStr.c_str(), -1, pFont, strOrigF, &lbBr);
    }

    strOrigF.X = maxValXRF.X;
    grPtr->DrawString(valXStr.c_str(), -1, pFont, strOrigF, &lbBr);

    if (!nameYStr.empty())
    {
      strOrigF.X = maxNameYRF.X;
      grPtr->DrawString(nameYStr.c_str(), -1, pFont, strOrigF, &lbBr);
    }

    strOrigF.X = maxValYRF.X;
    grPtr->DrawString(valYStr.c_str(), -1, pFont, strOrigF, &lbBr);

  }
  return labRect.Height;
}

// Draw label line
void CDataWnd::DrawLabelLine(RectF labelRectF, Color labCol, 
                      DashStyle dashStyle, float penWidth, Graphics* grPtr)
{
// Get the line start and end
  float stX = labelRectF.X + m_fBorderSpace;
  float stY = labelRectF.Y + labelRectF.Height/2.0f;
  float endX = stX + m_fBulletSize;
  float endY = stY;

  Pen linePen(labCol, dashStyle != DashStyleCustom ? penWidth : 1.0f);
  if (dashStyle != DashStyleCustom)
  {
    Pen linePen(labCol, penWidth);
    linePen.SetDashStyle(dashStyle);
    grPtr->DrawLine(&linePen, stX, stY, endX, endY);
  }
  else
  {
    GraphicsPath grPath;
    PointF pntXF(CHART_DTPNTSZ/2, 0.0f);
    PointF pntYF(0.0f, CHART_DTPNTSZ/2);
    PointF pntF(stX + CHART_DTPNTSZ/2, stY);
    for (int i = 0; i < 3; ++i)
    {
      grPath.AddLine(pntF - pntXF, pntF + pntXF);
      grPath.StartFigure();
      grPath.AddLine(pntF - pntYF, pntF + pntYF);
      grPath.StartFigure();

      pntF.X += CHART_DTPNTSZ + 1.0f;
    }
    grPtr->DrawPath(&linePen, &grPath);  
  }
}

// Draw all strings in the label window
void CDataWnd::DrawLabels(Graphics* grPtr, float offsXF, float offsYF)
{
  if (m_mapLabs.empty())
    return; 
  ENSURE(!m_strRF.IsEmptyArea());

  FontFamily fontFamily(m_strFontFamilyName.c_str());
  Gdiplus::Font labelFont(&fontFamily, static_cast<float>(m_fontSize), FontStyleBold);
  grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

  RectF labRect = m_strRF;
  labRect.Offset(offsXF, offsYF + m_fBorderSpace);
  MAP_LABSTR::iterator it;
  MAP_LABSTR::iterator itE = m_mapLabs.end();
  for (it = m_mapLabs.begin(); it != itE; ++it)
  {
    float offY = DrawLabel(it->second, labRect, &labelFont, grPtr);
    labRect.Offset(0.0f, offY);
  }
}

void CDataWnd::DrawDataWnd(int offsX, int offsY, Graphics* grPtr)
{
  CRect clR;
  GetClientRect(&clR);
  clR.OffsetRect(offsX, offsY);

  CRect strR = m_labelStrRect;
  strR.OffsetRect(offsX, offsY);

  RectF drRF = CRectToGdiRectF(clR);
  RectF strRF = CRectToGdiRectF(strR);

  drRF.Width -= 1.0f;
  drRF.Height -= 1.0f;

  GraphicsPath grPath;
  CreateRoundedRect(grPath, drRF, 2, true);

  SolidBrush fillBr(m_colBkgnd);
  grPtr->FillPath(&fillBr, &grPath);
  Pen pen(m_colBorder, 1);
  grPtr->DrawPath(&pen, &grPath);
  DrawLabels(grPtr, float(offsX), float(offsY));
}

BEGIN_MESSAGE_MAP(CDataWnd, CWnd)
  ON_WM_PAINT()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CDataWnd message handlers

void CDataWnd::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  Graphics gr(dc.m_hDC);                          // Graphics to paint

  Rect rGdi;
  gr.GetVisibleClipBounds(&rGdi);                 // The same as the clip rect

  Bitmap clBmp(rGdi.Width, rGdi.Height);          // Mem bitmap
  Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC
  grPtr->SetSmoothingMode(SmoothingModeAntiAlias);

  RectF drRF = GdiRectToRectF(rGdi);
  drRF.Width -= 1.0f;
  drRF.Height -= 1.0f;

  GraphicsPath grPath;
  CreateRoundedRect(grPath, drRF, 2, true);

  SolidBrush fillBr(m_colBkgnd);
  grPtr->FillPath(&fillBr, &grPath);
  Pen pen(m_colBorder, 1);
  grPtr->DrawPath(&pen, &grPath);
  DrawLabels(grPtr);

  gr.DrawImage(&clBmp, rGdi);
  delete grPtr;
}
