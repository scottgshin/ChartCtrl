// DlgCharts.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartContainer.h"
#include "DlgCharts.h"
#include "afxdialogex.h"


// CDlgCharts dialog

IMPLEMENT_DYNAMIC(CDlgCharts, CDialogEx)

CDlgCharts::CDlgCharts(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCharts::IDD, pParent)
{

}

CDlgCharts::~CDlgCharts()
{
}

void CDlgCharts::InitDlgCharts(const CChartContainer& chartContainer)
{
  m_chartContainer = chartContainer;
  UpdateData(FALSE);
}

void CDlgCharts::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CHARTS, m_chartContainer);
}


BEGIN_MESSAGE_MAP(CDlgCharts, CDialogEx)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CDlgCharts message handlers
void CDlgCharts::OnSize(UINT nType, int cx, int cy)
{
  CDialogEx::OnSize(nType, cx, cy);
  if (IsWindow(m_chartContainer.m_hWnd) && (nType == SIZE_RESTORED))
  {
     m_chartContainer.OnChangedSize(cx, cy);
  }
}

void CDlgCharts::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  lpMMI->ptMinTrackSize = CPoint(CHART_MINX, CHART_MINY);
  lpMMI->ptMaxTrackSize = CPoint(GetSystemMetrics(SM_CXFULLSCREEN) - CHART_SPACE,
                                   GetSystemMetrics(SM_CYFULLSCREEN) - CHART_SPACE);
  CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CDlgCharts::OnCancel()
{
  if(m_chartContainer.GetDataViewChartIdx() != -1)
    m_chartContainer.DestroyChartDataView();
  CDialogEx::OnCancel();
}


BOOL CDlgCharts::OnInitDialog()
{
  CDialogEx::OnInitDialog();

 	m_hIcon = AfxGetApp()->LoadIcon(IDI_CLONE);
 	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
 // TODO:  Add extra initialization here

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}
