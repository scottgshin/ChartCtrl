// DlgTruncate.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "ChartContainer.h"
#include "DlgTruncate.h"
#include "afxdialogex.h"


// CDlgTruncate dialog

IMPLEMENT_DYNAMIC(CDlgTruncate, CDialogEx)

CDlgTruncate::CDlgTruncate(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTruncate::IDD, pParent), m_currStartX(-DBL_MAX), m_currEndX(DBL_MAX)
{

}

CDlgTruncate::~CDlgTruncate()
{
}

void CDlgTruncate::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TRUNCATE_LBCHARTNAMES, m_lbChartNames);
  DDX_Control(pDX, IDC_TRUNCATE_CHKRECALC, m_chkRecalc);
  DDX_Control(pDX, IDC_TRUNCATE_SLENDX, m_slEndX);
  DDX_Control(pDX, IDC_TRUNCATE_SLSTARTX, m_slStartX);
  DDX_Control(pDX, IDC_TRUNCATE_BTNAPPLY, m_btnApply);
  DDX_Control(pDX, IDC_TRUNCATE_BTNUNDO, m_btnUndo);
  DDX_Control(pDX, IDC_TRUNCATE_CHKRANGE, m_chkRange);
}

void CDlgTruncate::InitControls(bool bEnable)
{
  m_lbChartNames.ResetContent();

  if (bEnable) // Fill list box
  {
    CChartCtrlDemoDlg* pOwner = (CChartCtrlDemoDlg*)GetOwner();
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    MAP_CHARTS::const_iterator it = pContainer->GetFirstChart();
    MAP_CHARTS::const_iterator itE = pContainer->GetChartsEnd();
  
    MAP_STORAGE tmpMapS;
    MAP_STORAGE::iterator itS, itSE = m_mapStorage.end(); 
    int itemID = 0;
    for (; it != itE; ++it)
    { 
      int chartIdx = it->first;
      string_t chartName = pContainer->GetChartName(chartIdx);
      m_lbChartNames.InsertString(itemID, chartName.c_str());
      m_lbChartNames.SetItemData(itemID++, it->first);

      itS = m_mapStorage.find(chartIdx);
      if (itS != itSE)
        tmpMapS.insert(*itS);
    }
    if (!tmpMapS.empty())
    {
      m_mapStorage = tmpMapS;
      tmpMapS.clear();
    }
    else
      m_mapStorage.clear();
  }
  else
    m_lbChartNames.AddString(_T("No charts in the container"));

  m_chkRecalc.EnableWindow(FALSE);
  if ((m_currStartX != -DBL_MAX) && (m_currEndX != DBL_MAX))
    m_chkRange.EnableWindow(TRUE);
  else
  {
    m_chkRange.SetCheck(BST_UNCHECKED);
    m_chkRange.EnableWindow(FALSE);
  }

  m_slEndX.EnableWindow(FALSE);
  m_slStartX.EnableWindow(FALSE);

  m_btnApply.EnableWindow(FALSE);
  if (m_mapStorage.empty())
    m_btnUndo.EnableWindow(FALSE);
  else
    m_btnUndo.EnableWindow(TRUE);
}

BEGIN_MESSAGE_MAP(CDlgTruncate, CDialogEx)
  ON_LBN_SELCHANGE(IDC_TRUNCATE_LBCHARTNAMES, &CDlgTruncate::OnLbnSelchangeTruncateLbchartnames)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_TRUNCATE_SLENDX, OnSlPosChanged)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_TRUNCATE_SLSTARTX, OnSlPosChanged)
  ON_BN_CLICKED(IDC_TRUNCATE_BTNAPPLY, &CDlgTruncate::OnBnClickedTruncatApply)
  ON_BN_CLICKED(IDC_TRUNCATE_CHKRANGE, &CDlgTruncate::OnBnClickedTruncateRange)
  ON_BN_CLICKED(IDC_TRUNCATE_BTNUNDO, &CDlgTruncate::OnBnClickedTruncatUndo)
END_MESSAGE_MAP()


// CDlgTruncate message handlers


void CDlgTruncate::OnLbnSelchangeTruncateLbchartnames()
{
  int selIdx = m_lbChartNames.GetCurSel();
  if (selIdx != LB_ERR)
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &(pOwner->m_chartContainer);
    int chartIdx = m_lbChartNames.GetItemData(selIdx);
    std::tuple<double, double, bool> tuple_extX =
                                   pContainer->GetChartBoundaries(chartIdx); 
    if (get<2>(tuple_extX) == true)
    {
      m_chkRecalc.SetCheck(BST_UNCHECKED);
      m_chkRecalc.EnableWindow(TRUE);

      double minX = get<0>(tuple_extX);
      double maxX = get<1>(tuple_extX);

      UINT chkRangeState = m_chkRange.GetCheck();

      double currMinX = chkRangeState == BST_UNCHECKED ? minX : m_currStartX;
      double currMaxX = chkRangeState == BST_UNCHECKED ? maxX : m_currEndX;
      int slPrecision = GetRangePrecision(currMinX, currMaxX, 10);
      m_slEndX.SetInitVals(minX, maxX, slPrecision, currMaxX, true);
      m_slStartX.SetInitVals(minX, maxX, slPrecision, currMinX, true);
     
      bool bEnable = chkRangeState == BST_UNCHECKED ? true : false;
      m_slEndX.EnableWindow(bEnable ? TRUE : FALSE);
      m_slStartX.EnableWindow(bEnable ? TRUE : FALSE);

      m_btnApply.EnableWindow(TRUE);
    }
  }
  else
  {
    m_chkRecalc.EnableWindow(FALSE);
  
    m_slEndX.EnableWindow(FALSE);
    m_slStartX.EnableWindow(FALSE);

    m_btnApply.EnableWindow(FALSE);
  }
}

void CDlgTruncate::OnSlPosChanged(NMHDR* nmhdr, LRESULT* lResult)
{
  double endX = m_slEndX.GetCurrValue();
  double startX = m_slStartX.GetCurrValue();
  double range = 0, step = 0;

  switch (nmhdr->idFrom)
  {
  case IDC_TRUNCATE_SLENDX:
    if (m_chkRange.GetCheck() == BST_UNCHECKED)
    {
      range = m_slEndX.GetMaxVal() - m_slEndX.GetMinVal();
      step = range/10.0;
      if (endX < (startX + step))
        m_slEndX.SetCurrValue(startX + step, true); 
      m_currEndX = m_slEndX.GetCurrValue();
    }
    break;
  case IDC_TRUNCATE_SLSTARTX:
    if (m_chkRange.GetCheck() == BST_UNCHECKED)
    {
      range = m_slStartX.GetMaxVal() - m_slStartX.GetMinVal();
      step = range/10.0;
      if (startX > (endX - step))
        m_slStartX.SetCurrValue(endX - step, true); 
      m_currStartX = m_slStartX.GetCurrValue();
    }
    break;
  }
  *lResult = 1L;
}

BOOL CDlgTruncate::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_slEndX.SetCaption(L"EndX");
  m_slStartX.SetCaption(L"StartX");

  return TRUE;  // return TRUE unless you set the focus to a control
}


void CDlgTruncate::OnBnClickedTruncatApply()
{
  int selIdx = m_lbChartNames.GetCurSel();
  if (selIdx != LB_ERR)
  {
    bool bUpdateExt = m_chkRecalc.GetCheck() == BST_UNCHECKED ? false : true;
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &(pOwner->m_chartContainer);
    int chartIdx = m_lbChartNames.GetItemData(selIdx);
    STRUCT_STORAGE structStorage;
    structStorage.vChartDataD = *(pContainer->GetChartData(chartIdx));
    if (chartIdx == pContainer->GetDataViewChartIdx())
      structStorage.mapSelPntsD = *(pContainer->GetDataViewSelPntsD());
    m_mapStorage.insert(MAP_STORAGE::value_type(chartIdx, structStorage));
    pContainer->TruncateChart(chartIdx, m_slStartX.GetCurrValue(), m_slEndX.GetCurrValue(),
                                                                         bUpdateExt, true);
    MAP_FUNCTIONS::iterator itF = pOwner->m_mapFunctions.find(chartIdx);
    if (itF != pOwner->m_mapFunctions.end())
    {
      std::tuple<double, double, bool>  tuple_extX =
          pContainer->GetChartBoundaries(chartIdx);
      itF->second.lastX = get<1>(tuple_extX);
      m_btnUndo.EnableWindow(TRUE);
    }
    if (!m_chkRange.IsWindowEnabled())
      m_chkRange.EnableWindow(TRUE);

    m_lbChartNames.SetCurSel(-1);
  }
}

void CDlgTruncate::OnBnClickedTruncateRange()
{
  UINT chkState = m_chkRange.GetCheck();
  bool bEnable = chkState == BST_UNCHECKED ? true : false;

  if (chkState == BST_CHECKED)
  {
    if ((m_currStartX == -DBL_MAX)||(m_currEndX == DBL_MAX))
    {
      m_currEndX = m_slEndX.GetCurrValue();
      m_currStartX = m_slStartX.GetCurrValue();
    }
    else
    {
      m_slEndX.SetCurrValue(m_currEndX);
      m_slStartX.SetCurrValue(m_currStartX); 
    }
  }

  m_slEndX.EnableWindow(bEnable == true ? TRUE : FALSE);
  m_slStartX.EnableWindow(bEnable == true ? TRUE : FALSE);

}

void CDlgTruncate::OnBnClickedTruncatUndo()
{
  bool bSuccess = true;
  int selIdx = m_lbChartNames.GetCurSel();
  if (selIdx != LB_ERR)
  {
    int chartIdx = m_lbChartNames.GetItemData(selIdx);
    MAP_STORAGE::iterator itStorage = m_mapStorage.find(chartIdx);
    if (itStorage != m_mapStorage.end())
    {
      CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
      CChartContainer* pContainer = &(pOwner->m_chartContainer);
      if (chartIdx == pContainer->GetDataViewChartIdx())
      {
        pContainer->SetDataViewSelPnts(chartIdx, itStorage->second.mapSelPntsD,
                                                  &(itStorage->second.vChartDataD));
      }
      else
      {
        pContainer->ReplaceChartData(chartIdx, itStorage->second.vChartDataD, false, true, false, true);
      }
      m_mapStorage.erase(chartIdx);
    }
    else 
      bSuccess = false;
  }
  else
    bSuccess = false;

  if (!bSuccess)
  {
    string_t msgStr(_T("Undo failed: "));
    if (selIdx == LB_ERR)
      msgStr += string_t(_T("Select chart to undo first"));
    else
      msgStr += string_t(_T("The chart is not truncated\nor undo failed"));
    AfxMessageBox(msgStr.c_str());
  }

  m_lbChartNames.SetCurSel(-1);
}
