// DlgGenProp.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "ChartContainer.h"
#include "DlgGenProp.h"
#include "afxdialogex.h"


///////////////////////////////////////////////////////////////////////////////
// ValString function

string_t __stdcall GetLabNamedValStr(double val, int precision, bool bAddEqSign)
{
  sstream_t stream_t;
  stream_t << std::setprecision(precision) << val ;
  string_t valStr  = stream_t.str() + string_t(_T(" kM"));
  return bAddEqSign ? string_t(_T("= ")) + valStr : valStr;
}

///////////////////////////////////////////////////////////////////////////////
// CStColors

IMPLEMENT_DYNAMIC(CStColors, CStatic)

CStColors::CStColors()
{
  m_vPntsF.push_back(PointF(0.0f, 113.0f));
  m_vPntsF.push_back(PointF(29.4f, 75.71f));
  m_vPntsF.push_back(PointF(58.8f, 49.72f));
  m_vPntsF.push_back(PointF(88.2f, 34.01f));
  m_vPntsF.push_back(PointF(117.6f, 22.83f));
  m_vPntsF.push_back(PointF(147.0f, 15.25f));

  m_legPntF = m_vPntsF[1];
  m_dataPntF = m_vPntsF[3];

  
}

CStColors::~CStColors()
{
}

void CStColors::RefreshCtrl(MAP_COLORS& mapCols, bool bRedraw)
{
  m_mapCols = mapCols;
  if (bRedraw)
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

void CStColors::DrawBkgnd(RectF rClF, Graphics* grPtr)
{
  Color bkColor = m_mapCols[F_COLBKGND];
  SolidBrush br(bkColor);
  grPtr->FillRectangle(&br, rClF);

  Color borderCol = m_mapCols[F_COLBORDER];
  Pen pen(borderCol, 2.0f);
  grPtr->DrawRectangle(&pen, rClF);

  RectF rFocusF = rClF;
  rFocusF.Inflate(-3.0f, -3.0f);
  Pen focusPen(borderCol, 1.0);
  focusPen.SetDashStyle(DashStyleDash);
  grPtr->DrawRectangle(&focusPen, rFocusF);
}

void CStColors::DrawAxes(RectF rClF, Graphics* grPtr)
{
  Color axColor = m_mapCols[F_COLAXES];
  Pen pen(axColor, 1.0f);;
  pen.SetWidth(1.0f);
  grPtr->DrawLine(&pen, rClF.GetLeft(), rClF.GetTop() + rClF.Height/2.0f,
                                            rClF.GetRight(), rClF.GetTop() + rClF.Height/2.0f);
  grPtr->DrawLine(&pen, rClF.GetLeft() + 0.1f*rClF.Width, rClF.GetTop(), 
                                            rClF.GetLeft() + 0.1f*rClF.Width, rClF.GetBottom());
}

void CStColors::DrawZoomBorder(RectF rClF, Graphics* grPtr)
{
  Color zoomCol = m_mapCols[F_COLZOOM];
  Pen pen(zoomCol, 1.0f);
  grPtr->DrawLine(&pen, rClF.GetLeft() + 0.4f*rClF.Width, rClF.GetTop(), 
                                            rClF.GetLeft() + 0.4f*rClF.Width, rClF.GetBottom());
}

void CStColors::DrawLegPnts(RectF rClF, Graphics* grPtr)
{
  V_CHARTDATAF::pointer pV = m_vPntsF.data();

  Color legPntsCol = m_mapCols[F_COLLEGPNTS];
  Pen pen(legPntsCol, 2.0f);
  grPtr->DrawCurve(&pen, pV, m_vPntsF.size());

  pen.SetWidth(1.0f);
  grPtr->DrawLine(&pen, m_legPntF.X, rClF.GetTop(), m_legPntF.X, rClF.GetBottom());

  RectF rEllF(m_legPntF.X - 3.0f, m_legPntF.Y - 3.0f, 6.0f, 6.0f);
  grPtr->DrawEllipse(&pen, rEllF);
}

void CStColors::DrawDataPnt(Graphics* grPtr)
{
  RectF rEllF(m_dataPntF.X - 4.5f, m_dataPntF.Y - 4.5f, 9.0f, 9.0f);

  Color dataPntsCol = m_mapCols[F_COLDATAPNTS];
  Pen pen(dataPntsCol, 1.0f);
  grPtr->DrawEllipse(&pen, rEllF);
}

void CStColors::DrawLegWnd(RectF rClF, Graphics* grPtr)
{
  float legW = 0.3f*rClF.Width;
  float legH = 0.2f*rClF.Height;
  float legRX = rClF.GetRight() - 3.0f;
  float legTY = rClF.GetTop() + 3.0f;
  RectF rLegF(legRX - legW, legTY, legW, legH);
  Color legBkgndCol = m_mapCols[F_COLLEGBKGND];
  SolidBrush br(legBkgndCol);
  grPtr->FillRectangle(&br, rLegF);
  Color legBorderCol = m_mapCols[F_COLLEGBORDER];
  Pen pen(legBorderCol, 1.0f);
  grPtr->DrawRectangle(&pen, rLegF);
}

void CStColors::DrawGrid(RectF rClF, Graphics* grPtr)
{
  float fDeltaX = rClF.Width/10.0f;
  float fDeltaY = rClF.Height/10.0f;

  float limTop    = rClF.GetTop();
  float limBottom = rClF.GetBottom();
  float limLeft   = rClF.GetLeft();
  float limRight  = rClF.GetRight();

  Color colGrid = m_mapCols[F_COLGRID];
  Pen gridPen(colGrid, 1.0f);
  gridPen.SetDashStyle(DashStyleDash);

  float xF, yF;

  yF = rClF.Y + fDeltaY;
  while (yF < limBottom)
  {
    grPtr->DrawLine(&gridPen, limLeft, yF, limRight, yF);
    yF += fDeltaY;
  }

  xF = rClF.X + 0.1f*rClF.Width + fDeltaX;
  while (xF < limRight)
  {
    grPtr->DrawLine(&gridPen, xF, limTop, xF, limBottom);
    xF += fDeltaX;
  }
}


BEGIN_MESSAGE_MAP(CStColors, CStatic)
  ON_WM_PAINT()
END_MESSAGE_MAP()



// CStColors message handlers

void CStColors::OnPaint()
{
  CPaintDC dc(this);                              // Device context for painting
  Graphics gr(dc.m_hDC);                          // Graphics to paint

  Rect rGdi;
  gr.GetVisibleClipBounds(&rGdi);                 // The same as the clip rect
  RectF rGdiF;
  gr.GetVisibleClipBounds(&rGdiF);

  Bitmap clBmp(rGdi.Width, rGdi.Height);          // Mem bitmap
  Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC
  grPtr->SetSmoothingMode(SmoothingModeAntiAlias);

  rGdiF.Width -= 1.0f;
  rGdiF.Height -= 1.0f;

  DrawBkgnd(rGdiF, grPtr);
  DrawGrid(rGdiF, grPtr);
  DrawAxes(rGdiF, grPtr);
  DrawZoomBorder(rGdiF, grPtr);
  DrawLegPnts(rGdiF, grPtr);
  DrawDataPnt(grPtr);
  DrawLegWnd(rGdiF, grPtr);

  gr.DrawImage(&clBmp, rGdi);
  delete grPtr;

}

///////////////////////////////////////////////////////////////////////////////
// CDlgGenProp dialog

IMPLEMENT_DYNAMIC(CDlgGenProp, CDialogEx)

CDlgGenProp::CDlgGenProp(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGenProp::IDD, pParent)
  , m_colorType(0)
{

}

CDlgGenProp::~CDlgGenProp()
{
}

void CDlgGenProp::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_GENPROP_SLPRECISION, m_slPrecision);
  DDX_Control(pDX, IDC_GENPROP_SLEXTLT, m_slExtLT);
  DDX_Control(pDX, IDC_GENPROP_SLEXTRB, m_slExtRB);
  DDX_Radio(pDX, IDC_GENPROP_RBTNBKGND, m_colorType);
  DDX_Control(pDX, IDC_GENPROP_RBTNBKGND, m_rbtnColBkgnd);
  DDX_Control(pDX, IDC_GENPROP_RBTNBORDER, m_rbtnColBorder);
  DDX_Control(pDX, IDC_GENPROP_RBTNAXES, m_rbtnColAxes);
  DDX_Control(pDX, IDC_GENPROP_RBTNZOOM, m_rbtnColZoom);
  DDX_Control(pDX, IDC_GENPROP_RBTNLEGPNTS, m_rbtnColLegPnts);
  DDX_Control(pDX, IDC_GENPROP_RBTNDATAPNTS, m_rbtnColDataPnts);
  DDX_Control(pDX, IDC_GENPROP_RBTNLEGBKGND, m_rbtnColLegBkgnd);
  DDX_Control(pDX, IDC_GENPROP_RBTNLEGBORDER, m_rbtnColLegBorder);
  DDX_Control(pDX, IDC_GENPROP_RBTNGRID, m_rbtnColGrid);
  DDX_Control(pDX, IDC_GENPROP_STCOLORS, m_stColors);
  DDX_Control(pDX, IDC_GENPROP_BTNCOLS, m_btnColors);
  DDX_Control(pDX, IDC_GENPROP_STCOLTXT, m_stColorTxt);
  DDX_Control(pDX, IDC_GENPROP_BTNUNDO, m_btnUndo);
  DDX_Control(pDX, IDC_GENPROP_BTNDEFAULT, m_btnDefault);
  DDX_Control(pDX, IDC_GENPROP_EDNAMEX, m_edNameX);
}

void CDlgGenProp::InitControls(bool bEnable)
{
// Enable/disable controls
  m_edNameX.EnableWindow(bEnable ? TRUE : FALSE);
  m_slPrecision.EnableWindow(bEnable ? TRUE : FALSE);
  m_slExtLT.EnableWindow(bEnable ? TRUE : FALSE);
  m_slExtRB.EnableWindow(bEnable ? TRUE : FALSE);

  m_rbtnColBkgnd.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColBorder.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColAxes.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColZoom.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColLegPnts.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColDataPnts.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColLegBkgnd.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColLegBorder.EnableWindow(bEnable ? FALSE : TRUE);
  m_rbtnColGrid.EnableWindow(bEnable ? FALSE : TRUE);

  m_btnColors.EnableWindow(bEnable ? FALSE : TRUE);
  m_stColors.EnableWindow(bEnable ? FALSE : TRUE); 

  m_flags     = 0;
  m_colorType = 0;

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &(pOwner->m_chartContainer);

  if (bEnable)        // Enable extent and precision, disable colors
  {
    pContainer->EnableUser(false, false);

    m_btnUndo.EnableWindow(FALSE);
    m_btnDefault.EnableWindow(FALSE);
    m_vTupleVals.clear();

    string_t nameX = pContainer->GetAxisXName();
    int precision = pContainer->GetContainerPrecisionX();
    double startX = pContainer->GetStartX();
    double endX   = pContainer->GetEndX();

    get<FTU_LABELX>(m_tupleValsOld)  = nameX;
    get<FTU_PREC>(m_tupleValsOld)   = precision;
    get<FTU_STARTX>(m_tupleValsOld) = startX;
    get<FTU_ENDX>(m_tupleValsOld)   = endX;
// Was not able to set defaults on empty container
    get<FTU_STARTX>(m_tupleValsDefault) = pContainer->GetInitialStartX();
    get<FTU_ENDX>(m_tupleValsDefault) = pContainer->GetInitialEndX();

// Set the edit box text
    m_edNameX.SetWindowText(nameX.c_str());

// Init the sliders
    m_slPrecision.SetCurrValue(precision, true);

    int slPrecision = GetRangePrecision(startX, endX, 10);
    double initStartX = pContainer->GetInitialStartX();
    double initEndX   = pContainer->GetInitialEndX();
    m_slExtLT.SetInitVals(initStartX, initEndX, slPrecision, startX, true);
    m_slExtRB.SetInitVals(initStartX, initEndX, slPrecision, endX, true);
 
    m_stColorTxt.SetWindowText(_T("")); 

    m_tupleValsNew = m_tupleValsOld;
  }
  else          // Fill old colors' map
  {
    m_mapColsOld[F_COLBKGND]     = pContainer->GetBkColor();
    m_mapColsOld[F_COLBORDER]    = pContainer->GetBorderColor();
    m_mapColsOld[F_COLAXES]      = pContainer->GetAxesColor();
    m_mapColsOld[F_COLZOOM]      = pContainer->GetZoomBorderColor();
    m_mapColsOld[F_COLLEGPNTS]   = pContainer->GetLegPntsColor();
    m_mapColsOld[F_COLDATAPNTS]  = pContainer->GetDataViewPntsColor();
    m_mapColsOld[F_COLLEGBKGND]  = pContainer->GetLegBkgndColor();
    m_mapColsOld[F_COLLEGBORDER] = pContainer->GetLegBorderColor();
    m_mapColsOld[F_COLGRID]      = pContainer->GetGridColor();

    Color bkCol = m_mapColsOld[F_COLBKGND];
    m_btnColors.SetColor(bkCol.ToCOLORREF());
    CString colStr;
    colStr.Format(_T("RGB(%d,%d,%d)"), bkCol.GetR(), bkCol.GetG(), bkCol.GetB()); 
    m_stColorTxt.SetWindowText(colStr);

    m_mapColsNew = m_mapColsOld;
    m_stColors.RefreshCtrl(m_mapColsNew);
  }

  m_bValsChanged = false;
  m_bColsChanged = false;
}

void CDlgGenProp::ApplyVals(CChartContainer* pContainer, short flags, TUPLE_VALS& tuple_vals)
{
  if (flags & F_LABELX)
    pContainer->SetAxisXName(get<FTU_LABELX>(tuple_vals));
  if (flags & F_PRECISION)
    pContainer->SetContainerPrecision(get<FTU_PREC>(tuple_vals));
  if (flags & F_EXTX)
  {
    pContainer->ZoomMoveContainerX(get<FTU_STARTX>(tuple_vals), get<FTU_ENDX>(tuple_vals), true);
  }
}

void CDlgGenProp::ApplyVals(CChartContainer* pContainer, short flags, int action)
{
  TUPLE_VALS& tuple_vals = action == FTU_NEW ? m_tupleValsNew : 
                                action == FTU_UNDO ? m_tupleValsOld : m_tupleValsDefault;

  if (flags & F_LABELX)
    pContainer->SetAxisXName(get<FTU_LABELX>(tuple_vals));
  if (flags & F_PRECISION)
    pContainer->SetContainerPrecision(get<FTU_PREC>(tuple_vals));
  if (flags & F_EXTX)
  {
    switch (action)
    {
    case FTU_NEW: 
      pContainer->ZoomMoveContainerX(get<FTU_STARTX>(tuple_vals), get<FTU_ENDX>(tuple_vals), false);
      break;
    case FTU_DEFAULT:
      {
        bool bRes = pContainer->UnwindHistX(get<FTU_STARTX>(tuple_vals), get<FTU_ENDX>(tuple_vals));
        if (!bRes)
          break;
      }
    case FTU_UNDO:
      pContainer->UndoHistStepX();
      break;
    }
  }
}

void CDlgGenProp::ApplyColors(CChartContainer* pContainer, short flags, MAP_COLORS& mapCols)
{
  if (flags & F_COLBKGND)
    pContainer->SetBkColor(mapCols[F_COLBKGND]);
  if (flags & F_COLBORDER)
    pContainer->SetBorderColor(mapCols[F_COLBORDER]);
  if (flags & F_COLAXES)
    pContainer->SetAxesColor(mapCols[F_COLAXES]);
  if (flags & F_COLZOOM)
    pContainer->SetZoomBorderColor(mapCols[F_COLZOOM]);
  if (flags & F_COLLEGPNTS)
    pContainer->SetLegPntsColor(mapCols[F_COLLEGPNTS]);
  if (flags & F_COLDATAPNTS)
    pContainer->SetDataViewPntsColor(mapCols[F_COLDATAPNTS]);
  if (flags & F_COLLEGBKGND)
    pContainer->SetLegBkgndColor(mapCols[F_COLLEGBKGND]);
  if (flags & F_COLLEGBORDER)
    pContainer->SetLegBorderColor(mapCols[F_COLLEGBORDER]);
  if (flags & F_COLGRID)
    pContainer->SetGridColor(mapCols[F_COLGRID]);
}

BEGIN_MESSAGE_MAP(CDlgGenProp, CDialogEx)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_GENPROP_SLPRECISION, OnSlPosChanged)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_GENPROP_SLEXTLT,  OnSlPosChanged)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_GENPROP_SLEXTRB, OnSlPosChanged)
  ON_BN_CLICKED(IDC_GENPROP_BTNAPPLY, &CDlgGenProp::OnBnClickedGenpropBtnapply)
  ON_BN_CLICKED(IDC_GENPROP_RBTNBKGND, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNBORDER, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNAXES, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNZOOM, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNLEGPNTS, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNDATAPNTS, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNLEGBKGND, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNLEGBORDER, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_RBTNGRID, &CDlgGenProp::OnColRbtnClicked)
  ON_BN_CLICKED(IDC_GENPROP_BTNCOLS, &CDlgGenProp::OnBnClickedGenpropCols)
  ON_BN_CLICKED(IDC_GENPROP_BTNUNDO, &CDlgGenProp::OnBnClickedGenpropUndo)
  ON_BN_CLICKED(IDC_GENPROP_BTNDEFAULT, &CDlgGenProp::OnBnClickedGenpropDefault)
  ON_EN_CHANGE(IDC_GENPROP_EDNAMEX, &CDlgGenProp::OnEnChangeGenpropEdnamex)
  ON_BN_CLICKED(IDC_GENPROP_BTNDONE, &CDlgGenProp::OnBnClickedGenpropBtndone)
END_MESSAGE_MAP()


// CDlgGenProp message handlers


BOOL CDlgGenProp::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_btnColors.EnableOtherButton (_T("Other..."));
  m_btnColors.SetColumnsNumber(8);

  m_slPrecision.SetInitVals(1, 10, 0, 1);
  m_slPrecision.SetCaption(L"Precision", true);

  m_slExtLT.SetCaption(L"Left X", true);
  m_slExtRB.SetCaption(L"Right X", true);

// Fill default color map
  CChartContainer* pContainer = new CChartContainer;
  get<FTU_LABELX>(m_tupleValsDefault) = pContainer->GetAxisXName();
  get<FTU_PREC>(m_tupleValsDefault)   = pContainer->GetContainerPrecisionX();
  get<FTU_STARTX>(m_tupleValsDefault) = pContainer->GetInitialStartX(); // DBL_MAX
  get<FTU_ENDX>(m_tupleValsDefault)   = pContainer->GetInitialEndX();   // -DBL_MAX 

  m_edNameX.SetWindowTextW(pContainer->GetAxisXName().c_str());

  m_mapColsDefault.clear();
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLBKGND, pContainer->GetBkColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLBORDER, pContainer->GetBorderColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLAXES, pContainer->GetAxesColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLZOOM, pContainer->GetZoomBorderColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLLEGPNTS, pContainer->GetLegPntsColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLDATAPNTS, pContainer->GetDataViewPntsColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLLEGBKGND, pContainer->GetLegBkgndColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLLEGBORDER, pContainer->GetLegBorderColor()));
  m_mapColsDefault.insert(MAP_COLORS::value_type(F_COLGRID, pContainer->GetGridColor()));
  delete pContainer;
  
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgGenProp::OnSlPosChanged(NMHDR* nmhdr, LRESULT* lResult)
{
  double startVal = m_slExtLT.GetCurrValue();
  double endVal = m_slExtRB.GetCurrValue();
  double range = 0, step = 0;

  switch (nmhdr->idFrom)
  {
  case IDC_GENPROP_SLPRECISION:
    if (m_bValsChanged)
    {
      get<FTU_PREC>(m_tupleValsOld) = get<FTU_PREC>(m_tupleValsNew);
      m_bValsChanged = false;
    }
    get<FTU_PREC>(m_tupleValsNew) = m_slPrecision.GetCurrValue();
    m_flags |= F_PRECISION;
    break;
  case IDC_GENPROP_SLEXTLT:
    range = m_slExtLT.GetMaxVal() - m_slExtLT.GetMinVal();
    step = range/10.0;
    if (startVal > (endVal - step))
        m_slExtLT.SetCurrValue(endVal - step, true); 
    startVal = m_slExtLT.GetCurrValue();
    get<FTU_STARTX>(m_tupleValsNew) = startVal;
    m_flags |= F_EXTX;
  case IDC_GENPROP_SLEXTRB:
    range = m_slExtRB.GetMaxVal() - m_slExtRB.GetMinVal();
    step = range/10.0;
    if (endVal < (startVal + step))
        m_slExtRB.SetCurrValue(startVal + step, true); 
    endVal = m_slExtRB.GetCurrValue();
    get<FTU_ENDX>(m_tupleValsNew) = endVal;
    m_flags |= F_EXTX;
    break;
  }
  *lResult = 1L;
}

void CDlgGenProp::OnColRbtnClicked()
{
  UpdateData();

  short flag = 100;

  switch (m_colorType)
  {
  case 0: flag = F_COLBKGND;     break;
  case 1: flag = F_COLBORDER;    break;
  case 2: flag = F_COLAXES;      break;
  case 3: flag = F_COLZOOM;      break;
  case 4: flag = F_COLLEGPNTS;   break;
  case 5: flag = F_COLDATAPNTS;  break;
  case 6: flag = F_COLLEGBKGND;  break;
  case 7: flag = F_COLLEGBORDER; break;
  case 8: flag = F_COLGRID;      break;
  }


  MAP_COLORS::iterator it, itE = m_mapColsNew.end();
  it = m_mapColsNew.find(flag);  // Find color entry in new colors
  if (it == itE)
    it = m_mapColsOld.find(flag);
  Color color = it->second;
  m_btnColors.SetColor(color.ToCOLORREF());
  CString colStr;
  colStr.Format(_T("RGB(%d,%d,%d)"), color.GetR(), color.GetG(), color.GetB()); 
  m_stColorTxt.SetWindowText(colStr);

}

void CDlgGenProp::OnBnClickedGenpropCols()
{
  UpdateData();
  COLORREF colRef = m_btnColors.GetColor();
  Color color = Color( 255, GetRValue(colRef), GetGValue(colRef), GetBValue(colRef));
  CString colStr;
  colStr.Format(_T("RGB(%d,%d,%d)"), color.GetR(), color.GetG(), color.GetB()); 
  m_stColorTxt.SetWindowText(colStr);

  short flag = 100;
  switch (m_colorType)
  {
  case 0: flag = F_COLBKGND;     break;
  case 1: flag = F_COLBORDER;    break;
  case 2: flag = F_COLAXES;      break;
  case 3: flag = F_COLZOOM;      break;
  case 4: flag = F_COLLEGPNTS;   break;
  case 5: flag = F_COLDATAPNTS;  break;
  case 6: flag = F_COLLEGBKGND;  break;
  case 7: flag = F_COLLEGBORDER; break;
  case 8: flag = F_COLGRID;      break;
  }
// Is the  color new ?
  MAP_COLORS::iterator it = m_mapColsNew.find(flag);
  if ((it == m_mapColsNew.end()) || (it->second.ToCOLORREF() != color.ToCOLORREF()))
  {
    if (m_bColsChanged)
    {
      m_mapColsOld[flag] = m_mapColsNew[flag];
      m_bColsChanged = false;
    }
    m_mapColsNew[flag] = color; 
    m_flags |= flag;
    m_stColors.RefreshCtrl(m_mapColsNew);
  }
}

void CDlgGenProp::OnBnClickedGenpropBtnapply()
{
  m_bValsChanged = false;
  m_bColsChanged = false;

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &(pOwner->m_chartContainer);

  if ((m_flags & F_ALLVALS) != 0)
  {
    if (m_flags & F_LABELX)
    {
      CString szNameX;
      m_edNameX.GetWindowText(szNameX);
      get<FTU_LABELX>(m_tupleValsNew) = string_t(szNameX);
    }
    ApplyVals(pContainer, m_flags, FTU_NEW);
    m_vTupleVals.push_back(m_tupleValsOld);
    m_tupleValsOld = m_tupleValsNew;
    m_btnUndo.EnableWindow();
    m_btnDefault.EnableWindow();
    m_bValsChanged = true;
  }
  if ((m_flags & F_ALLCOLS) != 0)
  {
    ApplyColors(pContainer, m_flags, m_mapColsNew);
    m_vMapColors.push_back(m_mapColsOld);
    m_mapColsOld = m_mapColsNew;
    m_bColsChanged = true;
  }

  if (m_flags != 0)
  {
    pContainer->UpdateContainerWnds(pContainer->GetDataViewChartIdx(), true);
    m_btnUndo.EnableWindow();
    m_btnDefault.EnableWindow();
  }
}

void CDlgGenProp::OnBnClickedGenpropUndo()
{
  bool bSaveValsFlags = false;
  bool bSaveColsFlags = false;
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &(pOwner->m_chartContainer);
  if ((m_flags & F_ALLVALS) != 0)
  {
    m_tupleValsOld = m_vTupleVals.back();
    m_vTupleVals.pop_back();
    ApplyVals(pContainer, F_ALLVALS, FTU_UNDO);
    m_tupleValsNew = m_tupleValsOld;
    if (m_flags & F_LABELX)
      m_edNameX.SetWindowText(get<FTU_LABELX>(m_tupleValsOld).c_str());
    if (m_flags & F_PRECISION)
      m_slPrecision.SetCurrValue(get<FTU_PREC>(m_tupleValsOld), true);
    if (m_flags & F_EXTX)
    {
      double startX = get<FTU_STARTX>(m_tupleValsOld);
      double endX   = get<FTU_ENDX>(m_tupleValsOld);
      m_slExtLT.SetCurrValue(startX, true);
      m_slExtRB.SetCurrValue(endX, true);
      if (m_vTupleVals.empty())
      {
        m_btnUndo.EnableWindow(FALSE);
        m_btnDefault.EnableWindow(FALSE);
      }
      else
        bSaveValsFlags = true;
    }    
  }
  
  if ((m_flags & F_ALLCOLS) != 0)
  {
    m_mapColsNew = m_mapColsOld;
    m_mapColsOld = m_vMapColors.back();
    m_vMapColors.pop_back();
    ApplyColors(pContainer, F_ALLCOLS, m_mapColsOld);
    OnColRbtnClicked();
    m_stColors.RefreshCtrl(m_mapColsNew);
    if (m_vMapColors.empty())
    {
      m_btnUndo.EnableWindow(FALSE);
      m_btnDefault.EnableWindow(FALSE);
    }
    else
      bSaveColsFlags = true;
  }

  if (m_flags != 0)
  {
    pContainer->UpdateContainerWnds(-1, true);
    if (!bSaveValsFlags)
      m_bValsChanged = false;
    if (!bSaveColsFlags)
      m_bColsChanged = false;
    if (!bSaveValsFlags && !bSaveColsFlags)
      m_flags = 0;
  }
}

void CDlgGenProp::OnBnClickedGenpropDefault()
{
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &(pOwner->m_chartContainer);
  if (m_slPrecision.IsWindowEnabled())
  {
    m_flags = F_ALLVALS;
    ApplyVals(pContainer, F_ALLVALS, FTU_DEFAULT);
    m_tupleValsNew = m_tupleValsDefault;
    m_tupleValsOld = m_tupleValsDefault;
    m_vTupleVals.clear();
    m_edNameX.SetWindowText(get<FTU_LABELX>(m_tupleValsDefault).c_str());
    m_slPrecision.SetCurrValue(get<FTU_PREC>(m_tupleValsDefault), true);
    m_slExtLT.SetCurrValue(get<FTU_STARTX>(m_tupleValsDefault), true);
    m_slExtRB.SetCurrValue(get<FTU_ENDX>(m_tupleValsDefault), true);
  }
  else
  {
    m_flags = F_ALLCOLS;
    m_vMapColors.clear();
    ApplyColors(pContainer, F_ALLCOLS, m_mapColsDefault);
    m_mapColsNew = m_mapColsDefault;
    m_mapColsOld = m_mapColsDefault;
    m_stColors.RefreshCtrl(m_mapColsNew);
  }

  if (m_flags != 0)
  {
    pContainer->UpdateContainerWnds(-1, true);
    OnColRbtnClicked();    
    m_flags = 0;
    m_bValsChanged = false;
    m_bColsChanged = false;
    m_btnUndo.EnableWindow(FALSE);
    m_btnDefault.EnableWindow(FALSE);
  }
}

void CDlgGenProp::OnEnChangeGenpropEdnamex()
{
  m_flags |= F_LABELX;
}


void CDlgGenProp::OnBnClickedGenpropBtndone()
{
  if (m_flags != 0)
  {

    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &(pOwner->m_chartContainer);
    if ((m_flags & F_ALLVALS) != 0)
    {
      if (m_flags & F_LABELX)
      {
        CString szNameX;
        m_edNameX.GetWindowText(szNameX);
        get<FTU_LABELX>(m_tupleValsNew) = string_t(szNameX);
      }
      ApplyVals(pContainer, m_flags, m_tupleValsNew);
      m_bValsChanged = true;
    }

    if ((m_flags & F_ALLCOLS) != 0)
    {
      ApplyColors(pContainer, m_flags, m_mapColsNew);
      m_bColsChanged = true;
    }

    pContainer->UpdateContainerWnds();
  }

  CMFCTabCtrl* pTabCtrl = (CMFCTabCtrl*)GetParent();
  pTabCtrl->SetActiveTab(1);
}
