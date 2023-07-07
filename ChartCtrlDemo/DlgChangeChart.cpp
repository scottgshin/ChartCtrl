// DlgChangeChart.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "ChartContainer.h"
#include "DlgChangeChart.h"
#include "afxdialogex.h"

using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// CDlgChange dialog

IMPLEMENT_DYNAMIC(CDlgChange, CDialogEx)

CDlgChange::CDlgChange(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgChange::IDD, pParent), m_chartIdx(-1), m_chartCol((ARGB)Color::White), 
    m_dashStyle(0), m_penWidth(0), m_tension(0)

{
}

CDlgChange::~CDlgChange()
{
}

void CDlgChange::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CHANGE_EDNAME, m_edName);
  DDX_Control(pDX, IDC_CHANGE_COLOR, m_btnColor);
  DDX_Control(pDX, IDC_CHANGE_EDNAMEY, m_edNameY);
  DDX_Control(pDX, IDC_SLPRECY, m_slPrecY);
  DDX_Control(pDX, IDC_CHANGE_LBDASH, m_lbDash);
  DDX_LBIndex(pDX, IDC_CHANGE_LBDASH, m_dashStyle);
  DDX_Control(pDX, IDC_CHANGE_SLPENW, m_slPenWidth);
  DDX_Control(pDX, IDC_CHANGE_SLTENSION, m_slTension);
  DDX_Control(pDX, IDC_CHANGE_EDCOLOR, m_edCols);
  DDX_Control(pDX, IDC_CHANGE_LBNAMES, m_lbNames);
}


BEGIN_MESSAGE_MAP(CDlgChange, CDialogEx)
  ON_BN_CLICKED(IDC_CHANGE_BTNAPPLY, &CDlgChange::OnBnClickedChangeBtnapply)
  ON_BN_CLICKED(IDC_CHANGE_BTNUNDO, &CDlgChange::OnBnClickedChangeBtnundo)
  ON_LBN_SELCHANGE(IDC_CHANGE_LBNAMES, &CDlgChange::OnLbnSelchangeChangeLbnames)
END_MESSAGE_MAP()


// CDlgChange message handlers


void CDlgChange::OnBnClickedChangeBtnapply()
{ 
// Save data 
  if (m_mapChange.find(m_chartIdx) == m_mapChange.end())
  {
    TUPLE_CHANGE tuple_change;;
    get<CH_CHARTNAME>(tuple_change) = m_chartName;
    get<CH_NAMEY>(tuple_change)     = m_nameY;
    get<CH_PRECY>(tuple_change)     = m_precisionY;
    get<CH_CHARTCOL>(tuple_change)  = m_chartCol;
    get<CH_DASH>(tuple_change)      = m_dashStyle;
    get<CH_PENW>(tuple_change)      = m_penWidth;
    get<CH_TENSION>(tuple_change)   = m_tension;
    m_mapChange.insert(MAP_CHANGE::value_type(m_chartIdx, tuple_change));
    CWnd* btnUndo = GetDlgItem(IDC_CHANGE_BTNUNDO);
    btnUndo->EnableWindow();
  }

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &pOwner->m_chartContainer;
  bool bDataWnd = pContainer->IsLabWndVisible(false);
  bool bNamesWnd = pContainer->IsLabWndVisible(true);
  bool bDataView = pContainer->GetDataViewChartIdx() == m_chartIdx;

  bool bRedraw = false;
  
 
  CString tmpStr;
  m_edName.GetWindowText(tmpStr);
  string_t name(tmpStr);
  if (name != m_chartName)
  {
    pContainer->ChangeChartName(m_chartIdx, name);
    int nameID = m_lbNames.FindString(0, m_chartName.c_str());
    m_lbNames.DeleteString(nameID);
    nameID = m_lbNames.InsertString(nameID, name.c_str());
    m_lbNames.SetItemData(nameID, m_chartIdx);
    m_lbNames.SetCurSel(nameID);
    m_chartName = name;

    if (bDataWnd||bNamesWnd||bDataView)
      bRedraw = true;
  }

  m_edNameY.GetWindowText(tmpStr);
  name = string_t(tmpStr);
  if (name != m_nameY)
  {
    pContainer->ChangeChartYValName(m_chartIdx, name, true);
    m_nameY = name;
    if (bDataWnd||bDataView)
      bRedraw = true;
  }

  int precisionY = m_slPrecY.GetCurrValue();
  if (precisionY != m_precisionY)
  {
    pContainer->SetChartPrecisionY(m_chartIdx, precisionY, true);
    m_precisionY = precisionY;
    if (bDataWnd||bDataView)
      bRedraw = true;
  }

  COLORREF colRef = m_btnColor.GetColor();
  if (colRef != m_chartCol.ToCOLORREF())
  {
    Color col(GetRValue(colRef), GetGValue(colRef), GetBValue(colRef));
    pContainer->SetChartColor(m_chartIdx, col, true);
    m_chartCol = col;
    bRedraw = true;
  }

  int dashStyle = m_lbDash.GetCurSel();
  if (dashStyle != m_dashStyle)
  {
    pContainer->SetChartDashStyle(m_chartIdx, DashStyle(dashStyle), true);
    m_dashStyle = dashStyle;
    bRedraw = true;
  }

  float penW = m_slPenWidth.GetCurrValue();
  if (penW != m_penWidth)
  {
    pContainer->UpdateChartPenWidth(m_chartIdx, penW, true);
    m_penWidth = penW;
    bRedraw = true;
  }

  float tension = m_slTension.GetCurrValue();
  if (tension != m_tension)
  {
    pContainer->SetChartTension(m_chartIdx, tension, true);
    m_tension = tension;
    bRedraw = true;
  }

  if (bRedraw)
    pContainer->UpdateContainerWnds(m_chartIdx);
}

void CDlgChange::OnBnClickedChangeBtnundo()
{
  MAP_CHANGE::iterator it = m_mapChange.find(m_chartIdx);
  if (it != m_mapChange.end())
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    bool bDataWnd = pContainer->IsLabWndVisible(false);
    bool bNamesWnd = pContainer->IsLabWndVisible(true);
    bool bDataView = pContainer->GetDataViewChartIdx() == m_chartIdx;

    bool bRedraw = false;

    TUPLE_CHANGE tuple_change = it->second;
    string_t name = get<CH_CHARTNAME>(tuple_change);
    if (name != m_chartName)
    {
      pContainer->ChangeChartName(m_chartIdx, name);
      m_edName.SetWindowTextW(name.c_str());
      int nameID = m_lbNames.FindString(0, m_chartName.c_str());
      m_lbNames.DeleteString(nameID);
      nameID = m_lbNames.InsertString(nameID, name.c_str());
      m_lbNames.SetItemData(nameID, m_chartIdx);
      m_lbNames.SetCurSel(nameID);
      m_chartName = name;
      if (bDataWnd||bNamesWnd||bDataView)
        bRedraw = true;
    }

    name = get<CH_NAMEY>(tuple_change);
    if (name != m_nameY)
    {
      pContainer->ChangeChartYValName(m_chartIdx, name, true);
      m_nameY = name;
      m_edNameY.SetWindowText(m_nameY.c_str());
      if (bDataWnd||bDataView)
        bRedraw = true;
    }

    int precisionY = get<CH_PRECY>(tuple_change);
    if (precisionY != m_precisionY)
    {
      pContainer->SetChartPrecisionY(m_chartIdx, precisionY, true);   
      m_precisionY = precisionY;
      m_slPrecY.SetCurrValue(m_precisionY, true);
      if (bDataWnd||bDataView)
        bRedraw = true;
    }

    Color chartCol = get<CH_CHARTCOL>(tuple_change);
    ARGB colARGB = chartCol.GetValue();
    if (colARGB != m_chartCol.GetValue())
    {
      pContainer->SetChartColor(m_chartIdx, chartCol, true);
      m_chartCol.SetValue(colARGB);
      COLORREF colRef = m_chartCol.ToCOLORREF();
      m_btnColor.SetColor(colRef);
      sstream_t stream_t;
      stream_t << _T("RGB(") << GetRValue(colRef) << _T(", ") << GetGValue(colRef)
               << _T(", ") << GetBValue(colRef) << _T(")");
      m_edCols.SetWindowText(stream_t.str().c_str());
      bRedraw = true;
    }

    int dashStyle = get<CH_DASH>(tuple_change);
    if (dashStyle != m_dashStyle)
    {
      pContainer->SetChartDashStyle(m_chartIdx, DashStyle(dashStyle), true);
      m_dashStyle = dashStyle;
      m_lbDash.SetCurSel(m_dashStyle);
     bRedraw = true;
    }

    float penW = get<CH_PENW>(tuple_change);
    if (penW != m_penWidth)
    {
      pContainer->UpdateChartPenWidth(m_chartIdx, penW, true);
      m_penWidth = penW;
      m_slPenWidth.SetCurrValue(m_penWidth);
      bRedraw = true;
    }

    float tension = get<CH_TENSION>(tuple_change);
    if (tension != m_tension)
    {
      pContainer->SetChartTension(m_chartIdx, tension, true);
      m_tension = tension;
      m_slTension.SetCurrValue(m_tension);
      bRedraw = true;
    }

    if (bRedraw)
      pContainer->UpdateContainerWnds(m_chartIdx);
    m_mapChange.erase(m_chartIdx);
    if (m_mapChange.empty())
    {
      CWnd* btnUndo = GetDlgItem(IDC_CHANGE_BTNUNDO);
      btnUndo->EnableWindow(FALSE);
    }

  }
}


BOOL CDlgChange::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_btnColor.SetColor(RGB(255, 255, 255));

  m_slPrecY.SetInitVals(1, 10, 0, 3);
  m_slPrecY.SetCaption(L"Change Precision Y");

  m_slPenWidth.SetInitVals(1.0f, 6.0f, 0, 2.0f);
  m_slPenWidth.SetCaption(L"Change Pen Width");

  m_slTension.SetInitVals(0.0f, 1.0f, 1, 0.0f);
  m_slTension.SetCaption(L"Change Curve Tension");

  m_lbDash.InsertString(0, _T("Solid"));
  m_lbDash.InsertString(1, _T("Dash"));
  m_lbDash.InsertString(2, _T("Dot"));
  m_lbDash.InsertString(3, _T("DashDot"));
  m_lbDash.InsertString(4, _T("DashDotDot"));
  m_lbDash.InsertString(5, _T("Points only"));

  m_edCols.SetWindowText(_T("RGB(255, 255, 255)"));

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgChange::SetControls(int selIdx, CChartContainer* pContainer)
{
  m_chartIdx = selIdx;
  m_chartName = pContainer->GetChartName(selIdx); 
  m_edName.SetWindowText(m_chartName.c_str());
  m_nameY = pContainer->GetChartYValName(selIdx);
  m_edNameY.SetWindowText(m_nameY.c_str());

  m_chartCol = pContainer->GetChartColor(selIdx).first;
  COLORREF colRef = m_chartCol.ToCOLORREF();
  m_btnColor.SetColor(colRef);
  sstream_t stream_t;
  stream_t << _T("RGB(") << GetRValue(colRef) << _T(", ") << GetGValue(colRef)
           << _T(", ") << GetBValue(colRef) << _T(")");
  m_edCols.SetWindowText(stream_t.str().c_str());

  m_dashStyle = pContainer->GetChartDashStyle(selIdx).first;
  m_lbDash.SetCurSel(m_dashStyle);

  m_precisionY = pContainer->GetChartPrecisionY(selIdx);
  m_slPrecY.SetCurrValue(m_precisionY, true);

  m_penWidth = pContainer->GetChartPenWidth(selIdx).first;
  m_slPenWidth.SetCurrValue(m_penWidth, true);

  m_tension = pContainer->GetChartTension(selIdx).first;
  m_slTension.SetCurrValue(m_tension, true);
}

void CDlgChange::OnChartVisibilityChanged(int chartIdx, bool bVisible, CChartContainer* pContainer)
{
  if ((m_lbNames.GetCount() == 0) && bVisible)
    InitControls(bVisible, false);
  else
  {
    int selItemID = m_lbNames.GetCurSel();  // Always has selected item
    int selChartIdx = m_lbNames.GetItemData(selItemID);
    string_t changingChartName = pContainer->GetChartName(chartIdx);
    int changingChartID = m_lbNames.FindString(0, changingChartName.c_str());
    if (changingChartID == LB_ERR)          // Not in list; became visible right now
    {
      if (bVisible)
      {
        int newItemID = m_lbNames.InsertString(-1, changingChartName.c_str());
        m_lbNames.SetItemData(newItemID, chartIdx);
        if (m_mapChange.find(chartIdx) != m_mapChange.end())
        {
          CWnd* btnUndo = GetDlgItem(IDC_CHANGE_BTNUNDO);
          btnUndo->EnableWindow();
        }
      }
    }
    else if (!bVisible)                     // Is in list, removing
    {
      m_lbNames.DeleteString(changingChartID);
      if (m_lbNames.GetCount() == 0)
        InitControls(false, false);
      else if (selChartIdx == chartIdx)
      {
        m_lbNames.SetCurSel(0);
        SetControls(0, pContainer);
        if ((m_mapChange.size() == 1)&&(m_mapChange.find(chartIdx) != m_mapChange.end()))
        {
          CWnd* btnUndo = GetDlgItem(IDC_CHANGE_BTNUNDO);
          btnUndo->EnableWindow(FALSE);
        }
      }
    }
  }
}

void CDlgChange::InitControls(bool bEnable, bool bClear)
{
  m_lbNames.ResetContent();
  if (bClear)
    m_mapChange.clear();      // Itkeeps memory while the tab i active

  if (bEnable)
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    bool bHasVisibleCharts = pContainer->IsChartVisible(-1);
    if (bHasVisibleCharts)
    {
      MAP_CHARTS::const_iterator it = pContainer->GetFirstChart();
      MAP_CHARTS::const_iterator itE = pContainer->GetChartsEnd();
      int itemID = 0;
      int selIdx  = 0;
      for (; it != itE; ++it)
      { 
        int chartIdx = it->first;
        if (pContainer->IsChartVisible(chartIdx))
        {
          string_t chartName = pContainer->GetChartName(chartIdx);
          m_lbNames.InsertString(itemID, chartName.c_str());
          m_lbNames.SetItemData(itemID++, chartIdx);
        }
      }
      m_lbNames.SetCurSel(selIdx);
      SetControls(selIdx, pContainer);
    }
    else
      bEnable = false;
  }
  
  if (!bEnable)
  {
    m_edName.Clear();
    m_btnColor.SetColor(RGB(255, 255, 255));
    m_edCols.Clear();
    m_edNameY.Clear();
    m_slPrecY.SetCurrValue(3);
    m_slPenWidth.SetCurrValue(3.0f);
    m_slTension.SetCurrValue(0.0f);
    m_lbDash.SetCurSel(-1);
  }

  m_lbNames.EnableWindow(bEnable);
  m_edName.EnableWindow(bEnable);
  m_edNameY.EnableWindow(bEnable);
  m_btnColor.EnableWindow(bEnable);
  m_edCols.EnableWindow(bEnable);
  m_slPrecY.EnableWindow(bEnable);
  m_lbDash.EnableWindow(bEnable);
  m_slPenWidth.EnableWindow(bEnable);
  m_slTension.EnableWindow(bEnable);

  CWnd* btnApply = GetDlgItem(IDC_CHANGE_BTNAPPLY);
  btnApply->EnableWindow(bEnable); 
  if (m_mapChange.empty()||(m_lbNames.GetCount() == 0))
    bEnable = FALSE;
  else 
    bEnable = TRUE;
  CWnd* btnUndo = GetDlgItem(IDC_CHANGE_BTNUNDO);
  btnUndo->EnableWindow(bEnable);
 }

void CDlgChange::OnLbnSelchangeChangeLbnames()
{
  int curSel = m_lbNames.GetCurSel();
  int selIdx = m_lbNames.GetItemData(curSel);

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &pOwner->m_chartContainer;
  MAP_CHARTS::const_iterator it = pContainer->FindChart(selIdx);
  if (it != pContainer->GetChartsEnd())
    SetControls(it->first, pContainer);
}
