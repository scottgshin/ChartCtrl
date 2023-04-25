// DlgRemoveChart.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "DlgRemoveChart.h"
#include "afxdialogex.h"


// CDlgRemoveChart dialog

IMPLEMENT_DYNAMIC(CDlgRemoveChart, CDialogEx)

CDlgRemoveChart::CDlgRemoveChart(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRemoveChart::IDD, pParent)
{

}

CDlgRemoveChart::~CDlgRemoveChart()
{
}

void CDlgRemoveChart::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_REMOVE_LBCHARTS, m_lbChartNames);
  DDX_Control(pDX, IDC_REMOVE_CHKRECALC, m_chkRecalc);
  DDX_Control(pDX, IDC_REMOVE_BTNAPPLY, m_btnApply);
}

void CDlgRemoveChart::InitControls(bool bEnable)
{
  m_lbChartNames.ResetContent();

  if (bEnable) // Fill list box
  {
    CChartCtrlDemoDlg* pOwner = (CChartCtrlDemoDlg*)GetOwner();
    CChartContainer* pContainer = &pOwner->m_chartContainer;
    MAP_CHARTS::const_iterator it = pContainer->GetFirstChart();
    MAP_CHARTS::const_iterator itE = pContainer->GetChartsEnd();

    int itemID = 0;
    for (; it != itE; ++it)
    { 
      int chartIdx = it->first;
      string_t chartName = pContainer->GetChartName(chartIdx);
      m_lbChartNames.InsertString(itemID, chartName.c_str());
      m_lbChartNames.SetItemData(itemID++, it->first);
   }
  }
  else
    m_lbChartNames.AddString(_T("No charts in the container"));

  m_chkRecalc.SetCheck(BST_UNCHECKED);
  m_chkRecalc.EnableWindow(bEnable ? TRUE : FALSE);
  m_btnApply.EnableWindow(bEnable ? TRUE : FALSE);

  UpdateData(FALSE);
}

void CDlgRemoveChart::ApplyRemove(void)
{
  int selCount = m_lbChartNames.GetSelCount();
  if (selCount == 0)
    return;

  bool bRecalc = m_chkRecalc.GetCheck() == BST_UNCHECKED ? false : true;
  std::vector<int> vSelItems(selCount);
  m_lbChartNames.GetSelItems(selCount, vSelItems.data());

  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  CChartContainer* pContainer = &(pOwner->m_chartContainer);

  std::vector<int>::reverse_iterator it = vSelItems.rbegin(), itE = vSelItems.rend();
  for (; it != itE; ++it)
  {
    int listItemID = *it;
    int mapKey = m_lbChartNames.GetItemData(listItemID);
    MAP_CHARTS::iterator mapIt = pContainer->FindChart(mapKey) ;
    if (mapIt == pContainer->GetChartsEnd())
      continue;
    pContainer->RemoveChart(mapKey, bRecalc, false);
    pOwner->m_mapFunctions.erase(mapKey);
  }
 
  pContainer->UpdateContainerWnds();

  if (pContainer->IsContainerEmpty())
  {
    pOwner->m_tabCtrl.SetActiveTab(1);
    return;
  }

// Update list box
  it = vSelItems.rbegin();
  for (; it != itE; ++it)
    m_lbChartNames.DeleteString(*it);
// Reset check buttons
  m_chkRecalc.SetCheck(BST_UNCHECKED);
} 

BEGIN_MESSAGE_MAP(CDlgRemoveChart, CDialogEx)
  ON_BN_CLICKED(IDC_REMOVE_BTNAPPLY, &CDlgRemoveChart::OnBnClickedRemove)
END_MESSAGE_MAP()


// CDlgRemoveChart message handlers


BOOL CDlgRemoveChart::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  return TRUE;  // return TRUE unless you set the focus to a control
}


void CDlgRemoveChart::OnBnClickedRemove()
{
  ApplyRemove();
}
