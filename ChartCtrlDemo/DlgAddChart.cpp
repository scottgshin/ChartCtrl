// DlgAddChart.cpp : implementation file
//

#include "stdafx.h"
#include "ChartContainer.h"
#include "ChartCtrlDemo.h"
#include "SliderGDICtrl.h"
#include "DlgAddChart.h"
#include "afxdialogex.h"


// CDlgAddChart dialog

IMPLEMENT_DYNAMIC(CDlgAddChart, CDialogEx)

CDlgAddChart::CDlgAddChart(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAddChart::IDD, pParent),
    m_strName(_T("")), m_strYValName(_T("Y")), m_defaultYName(_T("Y")), m_precisionY(3),
    m_tension(0), m_fPenWidth(2.0f), m_colChart(Color(255, 157, 59, 45)),
    m_strRGB(_T("RGB(157,59,45)")),
    m_showPnts(BST_CHECKED), m_dashStyle(0), m_pointStyle(0), m_func(0), 
    m_multY(1.0), 
    m_fChMinX(-10.0), m_fChMaxX(10.0) , m_nPntsNmb(100),
    m_pContainer(NULL)
{
}

CDlgAddChart::~CDlgAddChart()
{
}

void CDlgAddChart::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_EDLABEL, m_edChartName);
  DDX_Radio(pDX, IDC_RBTNLINE, m_tension);
  DDX_Control(pDX, IDC_BTNPENCOLOR, m_colBtnPenCol);
  DDX_Check(pDX, IDC_CHKSHOWPNTS, m_showPnts);
  DDX_Control(pDX, IDC_SLPENWIDTH, m_slPenWidth);
  DDX_Text(pDX, IDC_STRGB, m_strRGB);
  DDX_Control(pDX, IDC_LBDASH, m_lbDash);
  DDX_LBIndex(pDX, IDC_LBDASH, m_dashStyle);
  DDX_Control(pDX, IDC_POINT_STYLE, m_lbPoint);
  DDX_LBIndex(pDX, IDC_POINT_STYLE, m_pointStyle);
  DDX_Control(pDX, IDC_LBFUNCTION, m_lbFunc);
  DDX_LBIndex(pDX, IDC_LBFUNCTION, m_func);
  DDX_Control(pDX, IDC_SLMULT, m_slMultOrder);
  DDX_Text(pDX, IDC_EDMULT, m_strMultY);
  DDX_Control(pDX, IDC_SLMINX, m_slMinX);
  DDX_Control(pDX, IDC_SLMAXX, m_slMaxX);
  DDX_Control(pDX, IDC_SLPNTSNMB, m_slPntsNmb);
  DDX_Control(pDX, IDC_EYLABELY, m_edValYName);
  DDX_Control(pDX, IDC_SLPRECY, m_slPrecisionY);
}

STRUCT_FUNCDATA CDlgAddChart::GenerateData(int funcID, double fMinX, 
                                        double fMaxX, size_t pntsNmb, V_CHARTDATAD& vData, double multY)
{
  if (fMinX >= fMaxX)
    return STRUCT_FUNCDATA();
  double fDeltaX = (fMaxX - fMinX)/pntsNmb; 
  double alpha = 0.0;
  size_t period = funcID == 3 ? pntsNmb/2 : 0;  // Used to append RectWave only
  

  switch (funcID)
  {
  case 0: alpha = GenerateSinWave(pntsNmb, fMinX, fDeltaX, vData, multY);  break;
  case 1: alpha = GenerateSincWave(pntsNmb, fMinX, fDeltaX, vData, multY); break;
  case 2: alpha = GenerateExp(pntsNmb, fMinX, fDeltaX, vData, multY);      break;
  case 3: alpha = GenerateRectWave(pntsNmb, fMinX, fDeltaX, vData, multY); break;
  case 4: alpha = GenerateRandomNmbs(pntsNmb, fMinX, fDeltaX, vData, multY); break;
  }

  STRUCT_FUNCDATA funcData = {funcID, vData.back().X, fDeltaX, alpha, multY, pntsNmb, period};
  return funcData;
}

double CDlgAddChart::GenerateSinWave(size_t nPntNmb, double fMinX, double fDeltaX, 
                                                                  V_CHARTDATAD& vData, double multY)
{
// Calculate period: one for range
  double alpha = 2*M_PI/(fDeltaX * nPntNmb);
  double lim = pow(10.0, -numeric_limits<double>::digits10);

// Fill the vector
  vData.resize(nPntNmb+1, PointD(0.0, 0.0));
  for (size_t i = 0; i <= nPntNmb; ++i)
  {
    double pntX = fMinX + i*fDeltaX;
    double pntY = sin(alpha*pntX);
    if (fabs(pntY) < lim)
      pntY = 0.0;
    vData[i] = PointD(pntX, multY*pntY);
  }
  return alpha;
}

double CDlgAddChart::GenerateSincWave(size_t nPntNmb, double fMinX, double fDeltaX, 
                                                                  V_CHARTDATAD& vData, double multY)
{
// Calculate period: four for nPntNmb
  double alpha = 4*M_PI/(fDeltaX * nPntNmb);;
  double lim = pow(10.0, -numeric_limits<double>::digits10);
 // Fill the vector
  vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
  for (size_t i = 0; i <= nPntNmb; ++i)
  {
    double pntX = fMinX + i*fDeltaX;
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
  return alpha;
}

double CDlgAddChart::GenerateExp(size_t nPntNmb, double fMinX, double fDeltaX, 
                                                                  V_CHARTDATAD& vData, double multY)
{
// Exp argument is between 0 and 4 for range [0, fMax]
  double alpha = 4.0/(fDeltaX * nPntNmb);
  double lim = pow(10.0, -numeric_limits<double>::digits10);
 // Fill the vector
  vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
  for (size_t i = 0; i <= nPntNmb; ++i)
  {
    double pntX = fMinX + i*fDeltaX;
    double expX = fabs(alpha*pntX);
    double pntY = exp(-expX);
    if (pntY < lim)
      pntY = 0.0;
    vData[i] = PointD(pntX, multY*pntY);
  }
  return alpha;
}

double CDlgAddChart::GenerateRectWave(size_t nPntNmb, double fMinX, double fDeltaX, 
                                                                  V_CHARTDATAD& vData, double multY)
{
// Two periods
  size_t nTau = nPntNmb/2; // Two periods
  size_t nPositive = nTau/2;
  size_t nNegative = nTau - nPositive;
  double alpha = 0.5*multY;

  vData.clear();
  vData.push_back(PointD(fMinX, -alpha));
  double pntX = fMinX;

  for (size_t nPeriod = 0; nPeriod < nPntNmb;)
  {
    for (size_t j = 0; j <= nPositive; ++j)
    {
      vData.push_back(PointD(pntX, alpha));
      pntX += fDeltaX;
    }
    pntX -= fDeltaX;
    for (size_t k = 0; k <= nNegative; ++k)
    {
      vData.push_back(PointD(pntX, -alpha));
      pntX += fDeltaX;
    }
    pntX -= fDeltaX;
    nPeriod += nTau;
  }

  vData.shrink_to_fit();

  return 0.5;
}

double CDlgAddChart::GenerateRandomNmbs(size_t nPntNmb, double fMinX, double fDeltaX, 
                                                                    V_CHARTDATAD& vData, double multY)
{
  double alpha = multY;
  uniform_real<double> uniDistrib(-alpha, alpha);

 // Fill the vector
  vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
  for (size_t i = 0; i <= nPntNmb; ++i)
    vData[i] = PointD(fMinX + i*fDeltaX, uniDistrib(m_rndGenNmbs));
  return 1.0;
}

BEGIN_MESSAGE_MAP(CDlgAddChart, CDialogEx)
  ON_BN_CLICKED(IDC_BTNPENCOLOR, &CDlgAddChart::OnBnClickedBtnpencolor)
  ON_BN_CLICKED(IDC_BTNADD, &CDlgAddChart::OnBnClickedBtnadd)
  ON_NOTIFY(TB_THUMBPOSITION, IDC_SLMULT, OnMultYNotification)
END_MESSAGE_MAP()


// CDlgAddChart message handlers


BOOL CDlgAddChart::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_strName.clear();

  m_tension = 0;

  m_colBtnPenCol.EnableOtherButton (_T("Other..."));
   m_colBtnPenCol.SetColor (m_colChart.ToCOLORREF());
  m_colBtnPenCol.SetColumnsNumber(10);
	m_strRGB.Format(_T("RGB(%d,%d,%d)"), m_colChart.GetR(), m_colChart.GetG(), 
                                                              m_colChart.GetB());
  m_showPnts = BST_CHECKED;

  m_slPenWidth.SetInitVals(1.0f, 6.0f, 0, m_fPenWidth);
  m_slPenWidth.SetCaption(L"PenWidth");

  m_lbDash.InsertString(0, _T("Solid"));
  m_lbDash.InsertString(1, _T("Dash"));
  m_lbDash.InsertString(2, _T("Dot"));
  m_lbDash.InsertString(3, _T("DashDot"));
  m_lbDash.InsertString(4, _T("DashDotDot"));
  m_lbDash.InsertString(5, _T("Points only"));
  m_lbDash.SetCurSel(m_dashStyle);

  /*
  enum class PointStyle
  {
      Ellipse,
      Triangle_Up,
      Square,
      Diamond,
      Triangle_Down
  };
  */

  m_lbPoint.InsertString(0, _T("Ellipse"));
  m_lbPoint.InsertString(1, _T("Triangle_Up"));
  m_lbPoint.InsertString(2, _T("Square"));
  m_lbPoint.InsertString(3, _T("Diamond"));
  m_lbPoint.InsertString(4, _T("Triangle_Down"));
  m_lbPoint.SetCurSel(m_pointStyle);

  m_lbFunc.InsertString(0, _T("Sine Wave"));
  m_lbFunc.InsertString(1, _T("sin(x)/x Function"));
  m_lbFunc.InsertString(2, _T("Exponent"));
  m_lbFunc.InsertString(3, _T("Rect wave"));
  m_lbFunc.InsertString(4, _T("Random"));
  m_lbFunc.SetCurSel(m_func);

  m_slMultOrder.SetInitVals(-10, 10, 0, int(log10(m_multY)));
  m_slMultOrder.SetCaption(L"Y-Mult");;
  m_strMultY.Format(_T("%g"), m_multY);

  m_slPrecisionY.SetInitVals(1, 10, 0, 3);
  m_slPrecisionY.SetCaption(L"PrecY");

  m_slMinX.SetInitVals(-10.0, 0.0, 1, m_fChMinX);
  m_slMinX.SetCaption(L"Chart MIN X");

  m_slMaxX.SetInitVals(0.0, 10.0, 1, m_fChMaxX);
  m_slMaxX.SetCaption(L"Chart MAX X");

  m_slPntsNmb.SetInitVals(10UL, 1000UL, -1, m_nPntsNmb);
  m_slPntsNmb.SetCaption(L"PNTS NMB");

  m_edChartName.SetWindowTextW(m_strName.c_str());
  m_edValYName.SetWindowTextW(m_defaultYName.c_str());

  UpdateData(FALSE);
  // TODO:  Add extra initialization here

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgAddChart::OnBnClickedBtnpencolor()
{
  COLORREF color = m_colBtnPenCol.GetColor();
  m_colChart = Color( 255, GetRValue(color), GetGValue(color), GetBValue(color));
	m_strRGB.Format(_T("RGB(%d,%d,%d)"), GetRValue(color), GetGValue(color), GetBValue(color));
  CWnd* pWnd = GetDlgItem(IDC_STRGB);
  pWnd->SetWindowText(m_strRGB);
}

void CDlgAddChart::OnBnClickedBtnadd()
{
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());

  if (m_pContainer == NULL)
  {
    m_pContainer = &(pOwner->m_chartContainer);
    ENSURE(IsWindow(m_pContainer->m_hWnd));
  }

  UpdateData();

  m_precisionY = m_slPrecisionY.GetCurrValue();
  m_fChMinX   = m_slMinX.GetCurrValue();
  m_fChMaxX   = m_slMaxX.GetCurrValue();
  m_nPntsNmb  = m_slPntsNmb.GetCurrValue();
  m_fPenWidth = m_slPenWidth.GetCurrValue();
  m_multY     = pow(10.0, m_slMultOrder.GetCurrValue());
  CString tmpStr;
  m_edChartName.GetWindowText(tmpStr);
  m_strName = string_t(tmpStr);
  m_edValYName.GetWindowText(tmpStr);
  m_strYValName = string_t(tmpStr);

  V_CHARTDATAD vData; 
  STRUCT_FUNCDATA funcData = 
    GenerateData(m_func, m_fChMinX, m_fChMaxX, m_nPntsNmb, vData, m_multY);
// After that vData.size() != m_pntNmb (end point and included)

// Generate chart name
  if (m_strName.empty())
  {
    switch (m_func)     // Generate a default name
    {
    case 0: m_strName = string_t(_T("SineWave_"));  
//    vData.erase(vData.begin(), vData.begin() + 75); //TODO: Do not forget to remove these two lines
//    vData.erase(vData.begin() + 1, vData.end());
///      vData.begin()->Y = -0.5;
//      vData.begin()->X = -2.0;
//      vData.shrink_to_fit();
//      funcData.lastX = 2.0;
//      vData.clear();
//      m_pContainer->UpdateExtX(-10, 10.0);
//      m_pContainer->UpdateExtY(-1.0, 1.0);
      break;
    case 1: m_strName = string_t(_T("SincWave_"));  break;
    case 2: m_strName = string_t(_T("Exp_"));       break;
    case 3: m_strName = string_t(_T("RectWave_"));  break;
    case 4: m_strName = string_t(_T("RandomNmb_")); 
  //          vData.clear(); 
//            vData.shrink_to_fit();
//            funcData.lastX = 0.0;
            break;
    }

    _TCHAR buffer_t[64];
    _itot_s(m_pContainer->GetMaxChartIdx() + 1, buffer_t, 10);  // Chart idx to string
    string_t idxStr(buffer_t);
    m_strName += string_t(buffer_t);

    m_edChartName.SetWindowTextW(m_strName.c_str());
  }

  int chartIdx = m_pContainer->AddChart(true, 
                                        m_showPnts == BST_CHECKED ? true : false, 
                                        m_strName, m_strYValName, m_precisionY, 
                                        DashStyle(m_dashStyle), 
                                        m_fPenWidth, float(m_tension), 
                                        m_colChart, 
                                        vData, static_cast<PointStyle>(m_pointStyle), true);
                                        
  if (chartIdx == -1)
  {
    CString msgStr;
    msgStr.Format(_T("Can't add CChart ID=%d, Name=%s\n") 
                  _T("Check the data vector and/or\nchart name(should be unique)"), 
                                                               m_chartIdx, m_strName.c_str());
    AfxMessageBox(msgStr, MB_OK|MB_ICONERROR);
  }
  else
  {
    pOwner->m_mapFunctions.insert(MAP_FUNCTIONS::value_type(chartIdx, funcData));

    m_strName.clear();
    m_edChartName.SetWindowText(m_strName.c_str());
    m_strYValName = m_defaultYName;
    m_edValYName.SetWindowText(m_strYValName.c_str());

    UpdateData(FALSE);
    m_pContainer->SetFocus();
  }
}

void CDlgAddChart::OnMultYNotification(NMHDR* , LRESULT*)
{
  UpdateData();
  int multOrder = m_slMultOrder.GetCurrValue();
  m_multY = pow(10.0, multOrder);
  m_strMultY.Format(_T("%g"), m_multY);
  UpdateData(FALSE);
}
