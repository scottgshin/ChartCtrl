
// ChartCtrlDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChartDef.h"
#include "resource.h"
#include "ChartCtrlDemo.h"
#include "DlgGenProp.h"
#include "DlgAddChart.h"
#include "DlgChangeChart.h"
#include "DlgAppendChart.h"
#include "DlgTruncate.h"
#include "DlgRemoveChart.h"
#include "DlgMisc.h"
#include "ChartCtrlDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int GetRangePrecision(double minX, double maxX, int stepsNmb)
{
  double step = fabs((maxX - minX)/stepsNmb);
  if (step == 0)
    return 0;
  int stepOrderOfMagnitude = int(floor(log10(step)));
  return 1 - stepOrderOfMagnitude;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CChartCtrlDemoDlg dialog

CChartCtrlDemoDlg::CChartCtrlDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChartCtrlDemoDlg::IDD, pParent), 
    m_pDlgGenProp(NULL), m_pDlgAddChart(NULL), m_pDlgChange(NULL), m_pDlgAppendChart(NULL),
    m_pDlgTruncate(NULL), m_pDlgRemoveChart(NULL), m_pDlgMisc(NULL)

{
}

CChartCtrlDemoDlg::~CChartCtrlDemoDlg()
{
  if (m_pDlgGenProp != NULL)
  {
    if (IsWindow(m_pDlgGenProp->m_hWnd))
      m_pDlgGenProp->DestroyWindow();
    delete m_pDlgGenProp;
  }

  if (m_pDlgAddChart != NULL)
  {
    if (IsWindow(m_pDlgAddChart->m_hWnd))
      m_pDlgAddChart->DestroyWindow();
    delete m_pDlgAddChart;
  }

  if (m_pDlgChange != NULL)
  {
    if (IsWindow(m_pDlgChange->m_hWnd))
      m_pDlgChange->DestroyWindow();
    delete m_pDlgChange;
  }


  if (m_pDlgAppendChart != NULL)
  {
    if (IsWindow(m_pDlgAppendChart->m_hWnd))
      m_pDlgAppendChart->DestroyWindow();
    delete m_pDlgAppendChart;
  }

  if (m_pDlgTruncate != NULL)
  {
    if (IsWindow(m_pDlgTruncate->m_hWnd))
      m_pDlgTruncate->DestroyWindow();
    delete m_pDlgTruncate;
  }

  if (m_pDlgRemoveChart != NULL)
  {
    if (IsWindow(m_pDlgRemoveChart->m_hWnd))
      m_pDlgRemoveChart->DestroyWindow();
    delete m_pDlgRemoveChart;
  }

  if (m_pDlgMisc != NULL)
  {
    if (IsWindow(m_pDlgMisc->m_hWnd))
      m_pDlgMisc->DestroyWindow();
    delete m_pDlgMisc;
  }
}

void CChartCtrlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STCHARTCONTAINER, m_chartContainer);
  DDX_Control(pDX, IDC_TABDEMO, m_tabLoc);
}

void CChartCtrlDemoDlg::UpdateFuncMap(void)
{
  MAP_FUNCTIONS::iterator itF = m_mapFunctions.begin();
  MAP_FUNCTIONS::iterator itFE = m_mapFunctions.end();

  for (; itF != itFE; ++itF)
  {
    int chartIdx = itF->first;
    std::tuple<double, double, bool> tuple_minmaxVals = // Gets X bounds
                                m_chartContainer.GetChartBoundaries(chartIdx);
    if (get<2>(tuple_minmaxVals) == false)
      continue;
    double fMinX = get<0>(tuple_minmaxVals);
    double fMaxX = get<1>(tuple_minmaxVals);
    itF->second.lastX = fMaxX;
    itF->second.pntsNmb = size_t((fMaxX - fMinX)/itF->second.deltaX + 0.5);
  }
}


BEGIN_MESSAGE_MAP(CChartCtrlDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, OnTabChanged)
  ON_NOTIFY(CODE_VISIBILITY, IDC_STCHARTCONTAINER, OnChartVisibilityChanged)
END_MESSAGE_MAP()


// CChartCtrlDemoDlg message handlers

BOOL CChartCtrlDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect rectTab;
 	m_tabLoc.GetWindowRect (&rectTab);
	ScreenToClient (&rectTab);

  m_tabCtrl.Create (CMFCTabCtrl::STYLE_3D_ROUNDED, rectTab, this, IDC_TABDEMO, CMFCTabCtrl::LOCATION_BOTTOM);
  m_tabCtrl.SetActiveTabBoldFont();

  m_pDlgGenProp = new CDlgGenProp;
  m_pDlgAddChart = new CDlgAddChart;
  m_pDlgChange = new CDlgChange;
  m_pDlgAppendChart = new CDlgAppendCharts;
  m_pDlgTruncate = new CDlgTruncate;
  m_pDlgRemoveChart = new CDlgRemoveChart;
  m_pDlgMisc = new CDlgMisc;

  m_pDlgGenProp->Create(IDD_DLGGENPROP, &m_tabCtrl);
  m_pDlgAddChart->Create(IDD_DLGADDCHART, &m_tabCtrl);
  m_pDlgChange->Create(IDD_DLGCHANGE, &m_tabCtrl);
  m_pDlgAppendChart->Create(IDD_DLGAPPEND, &m_tabCtrl);
  m_pDlgTruncate->Create(IDD_DLGTRUNCATE, &m_tabCtrl);
  m_pDlgRemoveChart->Create(IDD_DLGREMOVE, &m_tabCtrl);
  m_pDlgMisc->Create(IDD_DLGMISC, &m_tabCtrl);

  m_pDlgGenProp->SetOwner(this);
  m_pDlgAddChart->SetOwner(this);
  m_pDlgChange->SetOwner(this);
  m_pDlgAppendChart->SetOwner(this);
  m_pDlgTruncate->SetOwner(this);
  m_pDlgRemoveChart->SetOwner(this);
  m_pDlgMisc->SetOwner(this);

  m_tabCtrl.AddTab(m_pDlgGenProp, _T("Container Properties"));
  m_tabCtrl.AddTab(m_pDlgAddChart, _T("Add Chart"));
  m_tabCtrl.AddTab(m_pDlgChange, _T("Change Chart Attr"));
  m_tabCtrl.AddTab(m_pDlgAppendChart, _T("Append Charts"));
  m_tabCtrl.AddTab(m_pDlgTruncate, _T("Truncate Charts"));
  m_tabCtrl.AddTab(m_pDlgRemoveChart, _T("Remove Chart"));
  m_tabCtrl.AddTab(m_pDlgMisc, _T("Clone/Load"));
  m_tabCtrl.EnableAutoColor();
  m_tabCtrl.SetActiveTab(1);

// Init mersers in m_pDlgAddChart and m_pDlgAppendCharts
  mt19937 seedGen;
  seedGen.seed(_time32(NULL));

  m_pDlgAddChart->m_rndGenNmbs.seed(seedGen());
  m_pDlgAppendChart->m_rndGenNmbs.seed(seedGen());

  m_chartContainer.SetContainerName(string_t(_T("Demo chart container")));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChartCtrlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CChartCtrlDemoDlg::OnCancel()
{
// Because the chart data view is owned by the container
  m_chartContainer.ClearDataViewChartIdx();

  CDialogEx::OnCancel();
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChartCtrlDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChartCtrlDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CChartCtrlDemoDlg::OnTabChanged(WPARAM tabID, LPARAM)
{
  bool bEnable = !(m_chartContainer.IsContainerEmpty());
  if (tabID != 0)
    m_chartContainer.EnableUser(true, false);

  switch (tabID)
  {
  case 0: m_pDlgGenProp->InitControls(bEnable);      break;
  case 2: m_pDlgChange->InitControls(bEnable, true); break;
  case 3: m_pDlgAppendChart->InitControls(bEnable);  break;
  case 4: m_pDlgTruncate->InitControls(bEnable);     break;
  case 5: m_pDlgRemoveChart->InitControls(bEnable);  break;
  case 6: m_pDlgMisc->InitControls(bEnable);         break;
  default: return 0L;
  }
  return 1L;
}

void CChartCtrlDemoDlg::OnChartVisibilityChanged(NMHDR* nmhdr, LRESULT* pRes)
{
  NMCHART* pNmChart = PNMCHART(nmhdr);
  int tabID = m_tabCtrl.GetActiveTab();
  if (tabID == 2)
  {
    m_pDlgChange->OnChartVisibilityChanged(pNmChart->chartIdx, pNmChart->bState, &m_chartContainer);
    *pRes = 1L;
  }
  else
    *pRes = 0L;
}
