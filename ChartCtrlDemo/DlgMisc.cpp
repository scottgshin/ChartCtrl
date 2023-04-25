// DlgMisc.cpp : implementation file
//

#include "stdafx.h"
#include "ChartCtrlDemo.h"
#include "ChartCtrlDemoDlg.h"
#include "DlgMisc.h"
#include "afxdialogex.h"


// CDlgMisc dialog
using namespace std;
using namespace Gdiplus;

IMPLEMENT_DYNAMIC(CDlgMisc, CDialogEx)

CDlgMisc::CDlgMisc(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMisc::IDD, pParent)
{

}

CDlgMisc::~CDlgMisc()
{
}

void CDlgMisc::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_MISC_BTNCLONE, m_btnClone);
  DDX_Control(pDX, IDC_MISC_LBCHARTS, m_lbChartNames);
  DDX_Control(pDX, IDC_MISC_STFILENAME, m_stFileName);
  DDX_Control(pDX, IDC_MISC_LBCOLS, m_lbCols);
  DDX_Control(pDX, IDC_MISC_BTNCOLS, m_btnCols);
  DDX_Control(pDX, IDC_MISC_STCOLS, m_stCols);
}

string_t CDlgMisc::ColorToString(Color color)
{
  sstream_t stream_t;
  stream_t << _T("RGB(") << color.GetRed() << _T(", ") << color.GetGreen() << _T(", ")
           << color.GetBlue() << _T(")");
  return stream_t.str();
}

void CDlgMisc::InitControls(bool bEnable)
{
  m_btnClone.EnableWindow(bEnable ? TRUE : FALSE);

  if (!m_pathName.empty())
  {
    if (!PathFileExists(m_pathName.c_str()))
    {
      m_pathName.clear();
      m_lbChartNames.ResetContent();
      m_stFileName.SetWindowText(_T(""));
    }
    else
    {
      string_t fileName = m_pathName;
      size_t lastSlash = fileName.find_last_of(_T("\\")) + 1;
      fileName.erase(0, lastSlash);
      fileName = string_t(_T("Sel. file: ")) + fileName;
      m_stFileName.SetWindowText(fileName.c_str());
      m_lbChartNames.SetSel(-1, FALSE);
    }
  }
  m_lbCols.ResetContent();
  m_btnCols.SetColor(RGB(255, 255, 255));
  m_stCols.SetWindowText(_T("RGB(255, 255, 255)"));
}

void CDlgMisc::InitChartNamesListBox(const MAP_CHARTCOLS& mapContent)
{
  MAP_CHARTCOLS::const_iterator it = mapContent.cbegin();
  MAP_CHARTCOLS::const_iterator itE = mapContent.cend();
  int i = 0;
  for (; it != itE; ++it)
  {
    m_lbChartNames.InsertString(i, it->first.c_str());
    m_lbChartNames.SetItemData(i++, it->second.GetValue());
  }
}

BEGIN_MESSAGE_MAP(CDlgMisc, CDialogEx)
  ON_BN_CLICKED(IDC_MISC_BTNCLONE, &CDlgMisc::OnBnClickedClone)
  ON_BN_CLICKED(IDC_MISC_BTNLOAD, &CDlgMisc::OnBnClickedLoad)
  ON_BN_CLICKED(IDC_MISC_BTNAPPLY, &CDlgMisc::OnBnClickedApply)
  ON_LBN_SELCHANGE(IDC_MISC_LBCOLS, &CDlgMisc::OnLbColsSelChange)
  ON_BN_CLICKED(IDC_MISC_BTNCOLS, &CDlgMisc::OnBnClickedCols)
  ON_LBN_SELCHANGE(IDC_MISC_LBCHARTS, &CDlgMisc::OnLbSelChangeCharts)
END_MESSAGE_MAP()


// CDlgMisc message handlers


void CDlgMisc::OnBnClickedClone()
{
  CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
  if (!IsWindow(m_dlgCharts.m_hWnd))
    m_dlgCharts.Create(IDD_DLGCHARTS, &(pOwner->m_chartContainer));
  if (!m_dlgCharts.IsWindowVisible())
    m_dlgCharts.ShowWindow(SW_SHOW);
  m_dlgCharts.InitDlgCharts(pOwner->m_chartContainer);
}

void CDlgMisc::OnBnClickedLoad()
{
// Select XML file and get chart names to fill list box
  TCHAR szFilters[]= _T("Xml Files (*.xml)|*.xml");
  TCHAR szWorkDirPath[255];
  GetModuleFileName(NULL, szWorkDirPath, 255);
  PathRemoveFileSpec(szWorkDirPath);

  string_t dirStr(szWorkDirPath);
  size_t lastSlash = dirStr.find_last_of(_T("\\")) + 1;
  string_t chartDir(_T("Charts"));
  dirStr.replace(lastSlash, chartDir.length(), chartDir.c_str());

  CFileDialog fileDlg(TRUE, _T("xml"), _T("*.xml"), 
                  OFN_NOCHANGEDIR|OFN_EXPLORER, szFilters, this);
  fileDlg.m_ofn.lpstrInitialDir = dirStr.c_str();
  
  string_t strTitle(_T("Save "));

  m_lbChartNames.ResetContent();
  m_lbCols.ResetContent();
  m_btnCols.SetColor(RGB(255, 255, 255));
  m_stCols.SetWindowText(_T("RGB(255, 255, 255)"));

  if (fileDlg.DoModal() == IDOK)
  {
    CString pathName = fileDlg.GetPathName();
    CString ext = fileDlg.GetFileExt();
    if (ext == CString(_T("xml")))
    {
      m_pathName = string_t(pathName);
      V_CHARTNAMES vNames;
      std::map<string_t, Color> mapCharts;
      MAP_NAMES mapNames;
      CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
      HRESULT hr = pOwner->m_chartContainer.GetChartNamesFromXMLFile(m_pathName.c_str(), mapCharts); 
      if (hr == S_OK)
      {
        InitChartNamesListBox(mapCharts);
        string_t fileName = m_pathName;
        size_t lastSlash = fileName.find_last_of(_T("\\")) + 1;
        fileName.erase(0, lastSlash);
        fileName = string_t(_T("Sel. file: ")) + fileName;

        m_stFileName.SetWindowText(fileName.c_str());
      }
      else
      {
        m_pathName.clear();
        m_stFileName.SetWindowText(_T(""));
      }
    }
  }
}

void CDlgMisc::OnBnClickedApply()
{
  int selCount = m_lbChartNames.GetSelCount();
  if (selCount == 0)
    return;

  std::map<string_t, Color> mapReadCont;

  int itemCount = m_lbChartNames.GetCount();
  for (int i = 0; i < itemCount; ++i)
  {
    if (m_lbChartNames.GetSel(i))
    {
      CString nameS;
      m_lbChartNames.GetText(i, nameS);
      int colIdx = m_lbCols.FindStringExact(0, nameS);
      Color color = m_lbCols.GetItemData(colIdx);
      mapReadCont.insert(std::map<string_t, Color>::value_type(string_t(nameS), color));
    }
  }

  if (itemCount > 0)
  {
    CChartCtrlDemoDlg* pOwner = static_cast<CChartCtrlDemoDlg*>(GetOwner());
    CChartContainer* pContainer = &(pOwner->m_chartContainer);
    HRESULT hr = pContainer->LoadCharts(m_pathName.c_str(), mapReadCont); // Inside WaiFor the single object!
    if (hr == S_OK)
    {
      m_lbCols.ResetContent();
      m_btnCols.SetColor(RGB(255, 255, 255));
      m_stCols.SetWindowText(_T("RGB(255, 255, 255)"));

      std::map<string_t, Color>::iterator it = mapReadCont.begin(), itE = mapReadCont.end();
      for (; it != itE; ++it)
      {
        string_t name_t = it->first;
        int selIdx = m_lbChartNames.FindStringExact(0, name_t.c_str());
        m_lbChartNames.DeleteString(selIdx);
      }
//      m_btnClone.EnableWindow();
    }
  }
}

void CDlgMisc::OnLbColsSelChange()
{
  int selIdx = m_lbCols.GetCurSel();
  Color color(m_lbCols.GetItemData(selIdx));
  m_btnCols.SetColor(color.ToCOLORREF());
  CString colStr;
  colStr.Format(_T("RGB(%d,%d,%d)"), color.GetR(), color.GetG(), color.GetB()); 
  m_stCols.SetWindowText(colStr);
}

void CDlgMisc::OnBnClickedCols()
{
  int selIdx = m_lbCols.GetCurSel();
  if (selIdx != LB_ERR)
  {
    COLORREF colRef = m_btnCols.GetColor();
    Color color = Color( 255, GetRValue(colRef), GetGValue(colRef), GetBValue(colRef));
    m_stCols.SetWindowText(ColorToString(color).c_str());
    m_lbCols.SetItemData(selIdx, color.GetValue());
  }
  else
  {
    m_btnCols.SetColor(RGB(255, 255, 255));
    m_stCols.SetWindowText(_T("RGB(255, 255, 255)"));
  }
}

void CDlgMisc::OnLbSelChangeCharts()
{
  int colIdx;
  int selIdx = m_lbChartNames.GetCurSel();
  CString chartName;
  m_lbChartNames.GetText(selIdx, chartName);
  int isSelected = m_lbChartNames.GetSel(selIdx);
  if (isSelected > 0)
  {
    DWORD_PTR color = m_lbChartNames.GetItemData(selIdx);
    colIdx = m_lbCols.AddString(chartName);
    m_lbCols.SetItemData(colIdx, color);
 //   m_btnCols.EnableWindow();
  }
  else
  {
    colIdx = m_lbCols.FindStringExact(0, chartName);
    m_lbCols.DeleteString(colIdx);
  }

  if (m_lbCols.GetCount() > 0)
  {
    m_lbCols.SetCurSel(0);
    Color color(m_lbCols.GetItemData(0));
    m_btnCols.SetColor(color.ToCOLORREF());
    m_stCols.SetWindowTextW(ColorToString(color).c_str());
  }
  else
  {
    m_btnCols.SetColor(RGB(255, 255, 255));
    m_stCols.SetWindowTextW(_T("RGB(255, 255, 255)"));
  }
}
