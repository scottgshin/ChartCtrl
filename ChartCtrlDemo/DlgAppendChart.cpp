// DlgAppendChart.cpp : implementation file
//

#include "stdafx.h"
#include "ChartContainer.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "DlgAppendChart.h"
#include "afxdialogex.h"


// CDlgAppendCharts dialog

IMPLEMENT_DYNAMIC(CDlgAppendCharts, CDialogEx)

CDlgAppendCharts::CDlgAppendCharts(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAppendCharts::IDD, pParent), m_bAppend(false)
{

}

CDlgAppendCharts::~CDlgAppendCharts()
{
}

void CDlgAppendCharts::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_APPEND_LBCHARTNAMES, m_lbChartNames);
  DDX_Control(pDX, IDC_APPEND_SLMAXX, m_slAppendX);
  DDX_Control(pDX, IDC_APPEND_CHKANIMATE, m_chkAnimate);
  DDX_Control(pDX, IDC_APPEND_BTNAPPEND, m_btnAppend);
  DDX_Control(pDX, IDC_APPEND_BTNUNDOAPPEND, m_btnUndo);
  DDX_Control(pDX, IDC_APPEND_BTNSAVE, m_btnSave);
}

void CDlgAppendCharts::GenerateAppendData(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  int funcID = funcData.funcID;

  switch (funcID)
  {
  case 0: AppendSinWave(funcData, fMaxX, vData);    break;
  case 1: AppendSincWave(funcData, fMaxX, vData);   break;
  case 2: AppendExp(funcData, fMaxX, vData);        break;
  case 3: AppendRectWave(funcData, fMaxX, vData);  break;
  case 4: AppendRandomNmbs(funcData, fMaxX, vData); break;
  }
}

size_t CDlgAppendCharts::AppendSinWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  vData.clear();

  double fChartLastX = funcData.lastX;
  if (fMaxX <= fChartLastX)   // Already has the data point
    return 0;
 
  double lim = pow(10.0, -numeric_limits<double>::digits10);

  double fRange = fMaxX - fChartLastX;
  double fDeltaX = funcData.deltaX;
  double alpha = funcData.alpha;
  double multY = funcData.multY;

  double intPart;
  modf(fRange/fDeltaX, &intPart);
  size_t pntsNmb = size_t((funcData.lastX + fDeltaX*intPart) >= fMaxX ? intPart : intPart + 1);

// Fill the vector
  vData.resize(pntsNmb, PointD(0.0, 0.0));      // Include the last point
  for (size_t i = 0; i < pntsNmb; ++i)
  {
    double pntX = fChartLastX + (i + 1)*fDeltaX;
    double pntY = sin(alpha*pntX);
    if (fabs(pntY) < lim)
      pntY = 0.0;
    vData[i] = PointD(pntX, multY*pntY);
  }

  funcData.lastX = vData.back().X;
  funcData.pntsNmb += pntsNmb;

  return pntsNmb;
}

size_t CDlgAppendCharts::AppendSincWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  vData.clear();

  double fChartLastX = funcData.lastX;
  if (fMaxX <= fChartLastX)   // Already has the data point
    return 0;

  double lim = pow(10.0, -numeric_limits<double>::digits10);
  double fRange = fMaxX - fChartLastX;
  double fDeltaX = funcData.deltaX;
  double alpha = funcData.alpha;
  double multY = funcData.multY;

  double intPart;
  modf(fRange/fDeltaX, &intPart);
  size_t pntsNmb = size_t((funcData.lastX + fDeltaX*intPart) >= fMaxX ? intPart : intPart + 1);

  vData.resize(pntsNmb, PointD(0.0, 0.0));
  for (size_t i = 0; i < pntsNmb; ++i)
  {
    double pntX = fChartLastX + (i + 1)*fDeltaX;
    double sinArg = alpha *pntX;
    if ((pntX > -0.00001)&&(pntX < 0.00001))
      vData[i] = PointD(pntX, multY);
    else
    {
      double pntY = sin(sinArg)/sinArg;
      if (fabs(pntY) < lim)
        pntY = 0.0;
      vData[i] = PointD(pntX, multY*pntY); 
    }
  }

  funcData.lastX = vData.back().X;
  funcData.pntsNmb += pntsNmb;

  return pntsNmb;
}

size_t CDlgAppendCharts::AppendExp(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  vData.clear();

  double fChartLastX = funcData.lastX;
  if (fMaxX <= fChartLastX)   // Already has the data point
    return 0;

  double lim = pow(10.0, -numeric_limits<double>::digits10);
  double fRange = fMaxX - fChartLastX;
  double fDeltaX = funcData.deltaX;
  double alpha = funcData.alpha;
  double multY = funcData.multY;

  double intPart;
  modf(fRange/fDeltaX, &intPart);
  size_t pntsNmb = size_t((funcData.lastX + fDeltaX*intPart) >= fMaxX ? intPart : intPart + 1);

  vData.resize(pntsNmb, PointD(0.0, 0.0));
  for (size_t i = 0; i < pntsNmb; ++i)
  {
    double pntX = fChartLastX + (i + 1)*fDeltaX;
    double expX = fabs(alpha*pntX);
    double pntY = exp(-expX);
    if (pntY < lim)
      pntY = 0.0;
    vData[i] = PointD(pntX, multY*pntY);
  }

  funcData.lastX = vData.back().X;
  funcData.pntsNmb += pntsNmb;

  return pntsNmb;
}

size_t CDlgAppendCharts::AppendRectWave(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  vData.clear();

  double fChartLastX = funcData.lastX;
  if (fMaxX <= fChartLastX)   // Already has the data point
    return 0;

  double fRange = fMaxX - fChartLastX;
  double fDeltaX = funcData.deltaX;
  double alpha = funcData.alpha*funcData.multY;

  double intPart;
  modf(fRange/fDeltaX, &intPart);
  size_t pntsNmb = size_t((funcData.lastX + fDeltaX*intPart) >= fMaxX ? intPart : intPart + 1);

// Where we are:
  size_t oldRange = funcData.pntsNmb;
  size_t period   = funcData.period;

  size_t residuePnts = oldRange % period;
  size_t nPositive = period/2;
  size_t nNegative = period - nPositive;
  size_t nLim ;
  if (residuePnts >= nPositive)
  {
    alpha = -alpha;
    residuePnts = residuePnts - nPositive;
    nLim = nNegative - residuePnts;
  }
  else
    nLim = nPositive - residuePnts;

// Now calculate data to append
  if (residuePnts == 0)
    vData.push_back(PointD(fChartLastX, alpha));

  double pntX0 = fChartLastX;
  double pntX = pntX0;// = fChartLastX;
  
  for (size_t cnt = 0; cnt < pntsNmb; )
  {
    size_t tmpLim = min(nLim, pntsNmb - cnt);
    for (size_t j = 0; j < tmpLim; ++j)
    {
 //     pntX += fDeltaX;
      pntX = pntX0 + (j + 1)*fDeltaX;
      vData.push_back(PointD(pntX, alpha));
    }
    cnt += tmpLim;
    pntX0 = pntX;
    if (cnt < pntsNmb)
    {
      alpha = -alpha;
      vData.push_back(PointD(pntX0, alpha));
      nLim = alpha < 0 ? nNegative : nPositive;
    }
  }
 
  vData.shrink_to_fit();

  funcData.lastX = vData.back().X;
  funcData.pntsNmb += pntsNmb;

  return pntsNmb;
}

size_t CDlgAppendCharts::AppendRandomNmbs(STRUCT_FUNCDATA& funcData, double fMaxX, V_CHARTDATAD& vData)
{
  vData.clear();

  double fChartLastX = funcData.lastX;
  if (fMaxX <= fChartLastX)   // Already has the data point
    return 0;

  double fRange = fMaxX - fChartLastX;
  double fDeltaX = funcData.deltaX;
  double alpha = funcData.alpha*funcData.multY;

  double intPart;
  modf(fRange/fDeltaX, &intPart);
  size_t pntsNmb = size_t((funcData.lastX + fDeltaX*intPart) >= fMaxX ? intPart : intPart + 1);

  uniform_real<double> uniDistrib(-alpha, alpha);

 // Fill the vector
  vData.resize(pntsNmb, PointD(0.0, 0.0));
  for (size_t i = 0; i < pntsNmb; ++i)
    vData[i] = PointD(fChartLastX + (i + 1)*fDeltaX, uniDistrib(m_rndGenNmbs));

  funcData.lastX = vData.back().X;
  funcData.pntsNmb += pntsNmb;

  return pntsNmb;
}

void CDlgAppendCharts::InitControls(bool bEnable)
{
  m_lbChartNames.ResetContent();

  if (bEnable) // Fill list box
  {
    CChartCtrlDemoDlg* pOwner = (CChartCtrlDemoDlg*)GetOwner();
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    MAP_CHARTS::const_iterator it = pContainer->GetFirstChart();
    MAP_CHARTS::const_iterator itE = pContainer->GetChartsEnd();
    int itemID = 0;
    int lbSize = 0;
    for (; it != itE; ++it)
    { 
      int chartIdx = it->first;
      MAP_FUNCTIONS::iterator itF = pOwner->m_mapFunctions.find(chartIdx);
      if (itF != pOwner->m_mapFunctions.end())
      {
        string_t chartName = pContainer->GetChartName(chartIdx);
        m_lbChartNames.InsertString(itemID, chartName.c_str());
        m_lbChartNames.SetItemData(itemID++, it->first);
      }
    }
    lbSize = m_lbChartNames.GetCount();
    if (lbSize == 0)
    {
      m_lbChartNames.InsertString(0, _T("No charts in container or"));
      m_lbChartNames.InsertString(1, _T("can't generate data to append charts"));
      bEnable = false;
    }
    else if (pContainer->GetContainerSize() > size_t(lbSize))
    {
      m_lbChartNames.InsertString(lbSize++, _T(""));
      m_lbChartNames.InsertString(lbSize++, _T("Can append only charts"));
      m_lbChartNames.InsertString(lbSize,_T("shown in the list box"));
    }
  }
  else
    m_lbChartNames.AddString(_T("No charts in the container"));

//  UINT swCommand = bEnable ? SW_SHOW : SW_HIDE;

  m_slAppendX.SetCurrValue(2.0, true);
  m_slAppendX.EnableWindow(bEnable ? TRUE : FALSE);
//  m_slAppendX.ShowWindow(swCommand);

  m_chkAnimate.SetCheck(BST_UNCHECKED);
  m_chkAnimate.EnableWindow(bEnable ? TRUE : FALSE);
//  m_chkAnimate.ShowWindow(swCommand);

  m_btnAppend.EnableWindow(bEnable ? TRUE : FALSE);
//  m_btnAppend.ShowWindow(swCommand);
  m_btnUndo.EnableWindow(bEnable ? TRUE : FALSE);
//  m_btnUndo.ShowWindow(swCommand);

  if (!bEnable)
  {
    m_vSavedHist.clear();
    m_mapPosX.clear();
  }

  if (m_vSavedHist.empty())
    m_btnSave.EnableWindow(FALSE);
  else
    m_btnSave.EnableWindow(TRUE);

  m_appendDelta = 0.0;
  UpdateData(FALSE);
}

PAIR_DBLS CDlgAppendCharts::PrepareToAppendCharts(CChartContainer* pContainer, bool bAnimate)
{
  MAP_CHARTS::iterator it = pContainer->GetFirstChart();
  MAP_CHARTS::iterator itE = pContainer->GetChartsEnd();
  if (m_mapPosX.empty())  // Get chart boundaries for undo
  {
    for (; it != itE; ++it)
    {
      std::tuple<double, double, bool> tuple_posX = 
                pContainer->GetChartBoundaries(it->first, false);
      PAIR_DBLS pair_posX = make_pair(get<0>(tuple_posX), get<1>(tuple_posX));
      m_mapPosX.insert(MAP_POS::value_type(it->first, pair_posX));
    }
  }
  else  // Add new charts; don't correct for deleted charts, will do on Undo
  {
    for (; it != itE; ++it)
    {
      MAP_POS::iterator itPos = m_mapPosX.find(it->first);
      if (itPos == m_mapPosX.end())
      {
        std::tuple<double, double, bool> tuple_posX = 
                pContainer->GetChartBoundaries(it->first, false);
        PAIR_DBLS pair_posX = make_pair(get<0>(tuple_posX), get<1>(tuple_posX));
        m_mapPosX.insert(MAP_POS::value_type(it->first, pair_posX));
      }
    }
  }

  double startX = pContainer->GetInitialStartX();
  double endX   = pContainer->GetInitialEndX();

  if (bAnimate)
  {
    double initRange = endX - startX;
    m_appendDelta = initRange/10.0;
    endX += 10.0*initRange;
  }
  else
  {
    m_appendDelta = m_slAppendX.GetCurrValue();
    endX += m_appendDelta;
  }

  if (m_vSavedHist.empty())                  // Save only state before appending
  {
    pContainer->CopyHistory(m_vSavedHist, pContainer->GetContainerHistory());     // To keep the history
    m_vSavedHist.push_back(make_pair(pContainer->GetStartX(), pContainer->GetEndX()));
  }
  
  return make_pair(startX, endX);
}

bool CDlgAppendCharts::ApplyAppend(bool bUpdate)
{
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  if (pOwner->m_mapFunctions.empty())       
    return false;

  CChartContainer* pContainer = &pOwner->m_chartContainer;
  if (pContainer->IsContainerEmpty()) // Actually if mapFunctions is not empty, mapCharts is not empty
    return false;            // in this demo

  V_CHARTDATAD vData;

  MAP_CHARTS::iterator itMapCharts;
  MAP_FUNCTIONS::iterator itF = pOwner->m_mapFunctions.begin();
  MAP_FUNCTIONS::iterator itFE = pOwner->m_mapFunctions.end();

  double endX = m_extX.second;

  for (; itF != itFE; ++itF)
  {
    int mapKey = itF->first;
    GenerateAppendData(itF->second, endX, vData);
    pContainer->AppendChartData(mapKey, vData, false);
  }

  if (bUpdate)
  {
    double minValX, maxValX, minValY, maxValY;
    std::tie(minValX, maxValX) = pContainer->GetMinMaxExtX();
    pContainer->UpdateExtX(minValX, maxValX);
    std::tie(minValY, maxValY) = pContainer->GetMinMaxExtY();
    pContainer->UpdateExtY(minValY, maxValY);
  }

  return true;
}

void CDlgAppendCharts::UndoAppend(void)
{
  if (m_bAppend)
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    MAP_CHARTS::iterator itMap, itMapE = pContainer->GetChartsEnd(); 
    MAP_POS::iterator itPos = m_mapPosX.begin();
    MAP_POS::iterator itPosE = m_mapPosX.end();

    double initEndX = m_vSavedHist.front().second;
    
    for (; itPos != itPosE; ++itPos)
    {
      int chartIdx = itPos->first;
      itMap = pContainer->FindChart(chartIdx);
      if (itMap != itMapE)
      {
        PAIR_DBLS pair_extX = itPos->second;
        pContainer->TruncateChart(chartIdx, pair_extX.first, min(pair_extX.second, initEndX));
      }
      else
        pContainer->TruncateChart(chartIdx, m_vSavedHist.front().first, m_vSavedHist.front().second);
    }

    pOwner->UpdateFuncMap();
// Set minmaxX
    pContainer->CopyHistory(pContainer->GetContainerHistory(), m_vSavedHist);
    pContainer->UndoHistStepX(true);
    m_vSavedHist.clear();
    m_mapPosX.clear();

    m_btnSave.EnableWindow(FALSE);

    m_bAppend = false;
  }
}

BEGIN_MESSAGE_MAP(CDlgAppendCharts, CDialogEx)
  ON_BN_CLICKED(IDC_APPEND_BTNAPPEND, &CDlgAppendCharts::OnBnClickedAppend)
  ON_BN_CLICKED(IDC_APPEND_BTNUNDOAPPEND, &CDlgAppendCharts::OnBnClickedUndoAppend)
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_APPEND_BTNSAVE, &CDlgAppendCharts::OnBnClickedSave)
END_MESSAGE_MAP()


// CDlgAppendCharts message handlers


BOOL CDlgAppendCharts::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_slAppendX.SetInitVals(0.0, 10.0, 1, 2.0);
  m_slAppendX.SetCaption(L"Append X");

  return TRUE;  
}

void CDlgAppendCharts::OnBnClickedAppend()
{
  UpdateData();

  UINT nAnimate = m_chkAnimate.GetCheck();
  
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &pOwner->m_chartContainer;
//  pContainer->SetFocus();

  m_extX = PrepareToAppendCharts(pContainer, nAnimate == BST_CHECKED ? true : false);

  if (nAnimate == BST_CHECKED)
  {
    m_animEndX = m_extX.second;
    double nextEndX = pContainer->GetEndX();
    pContainer->UpdateExtX(m_extX.first, m_extX.second);
    pContainer->ResetCharts();
    m_extX.second = nextEndX + m_appendDelta;
    SetTimer(1, 100, NULL);
  }
  else
  {
    ApplyAppend();
    pContainer->ResetCharts();
    pContainer->SetFocus();
  }

  m_bAppend = true;
  if (!m_btnSave.IsWindowEnabled())
    m_btnSave.EnableWindow(TRUE);
}

void CDlgAppendCharts::OnBnClickedUndoAppend()
{
  UndoAppend();
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  pOwner->m_chartContainer.SetFocus();
}

void CDlgAppendCharts::OnBnClickedSave()
{
  m_bAppend = false;
  m_vSavedHist.clear();
  m_mapPosX.clear();

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  pOwner->m_tabCtrl.SetActiveTab(1);
  pOwner->m_chartContainer.SetFocus();
}

void CDlgAppendCharts::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == 1)
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &(pOwner->m_chartContainer);
   if ( m_extX.second < (m_animEndX + m_appendDelta))
    {
      ApplyAppend();
      pContainer->RefreshWnd();//m_extX.first, m_extX.second);
      m_extX.second += m_appendDelta;
      CDialogEx::OnTimer(nIDEvent);
    }
    else
    {
      KillTimer(1);
      CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
      pOwner->m_chartContainer.SetFocus();
    }
  }
}


