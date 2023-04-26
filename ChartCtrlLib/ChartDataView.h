#include "ChartDef.h"

#pragma once

using namespace Gdiplus;

class CChart;

///////////////////////////////////////////////////////////////////////////////
// CPageCtrl - child windows to control pages in CCharDataView

class CPageCtrl : public CButton
{
    DECLARE_DYNAMIC(CPageCtrl)

public:
    CPageCtrl(bool bEnd = false, bool bRotate = false, UINT idx = 0);
    virtual ~CPageCtrl();

public:
  void InitParams(bool bEnd, bool bRotate, UINT idx)
  {m_bEnd = bEnd; m_bRotate = bRotate; m_idx = idx;}


protected:
  void DrawArrow(const RectF boundRF, Graphics* grPtr);


protected:
    DECLARE_MESSAGE_MAP()

    afx_msg void OnMouseMove(UINT, CPoint);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnPaint();

public:

  bool m_bEnd;
  bool m_bRotate;
  bool m_bOnButton;
  long m_idx;
};


///////////////////////////////////////////////////////////////////////////////
// CChartDataView

class CChartDataView : public CWnd
{
    DECLARE_DYNAMIC(CChartDataView)

public:
    CChartDataView();
    virtual ~CChartDataView();

protected:
    DECLARE_MESSAGE_MAP()
public:
  size_t GetPagesNmb(void) const { return m_nPages;}
  size_t GetCurrPageNmb(void) const { return m_currPageID + 1;}

  void InitParams(const CChart* chartPtr, bool bClearMap, const CChartContainer* pHost);
  bool UpdateParams(const CChart* chartPtr, int flagsData);
  size_t ChangeDataPage(int pageNmb);
  void ShowWaitMessage(int chartIdx, size_t newSize);
  size_t UpdateDataIdx(void);
  size_t RefreshSelCells(void);
protected:
  size_t OnChartAppended(const V_CHARTDATAD& vChartData);
  size_t OnChartTruncated(const V_CHARTDATAD& vChartData);
  size_t OnChartDataReplaced(const V_CHARTDATAD& vChartData, bool bHasCellsMap = false);
  void CalcLayout(void);
  void CalcLayout(int flags, size_t dataItOffs = 0);
  void CreateChildren(void);
  string_t GetDataIDStr(size_t dataID);
  string_t GetDataValStr(double val, int precision);
  string_t GetTableHeader(void);
  size_t DataItemFromMousePnt(CPoint pnt, PointD& dataPntD, RectF& itemRF);
  bool RowRectFromDataID(size_t dataID, RectF& rowRF);
  void DrawPageHeader(float offsetX, Graphics* grPtr, size_t pageNmb, float dpiRatio);
  float DrawTableHeader(float offsetX, float offsetY, Graphics* grPtr, float dpiRatio);
  size_t DrawDataPage(size_t dataStartID, float tableOffsX, float tableOffsY, Graphics* grPtr, float dpiRatio);
  void DrawSelCells(Graphics*grPtr);

public:
  size_t PrintData(size_t nPage, Graphics* grPtr);

private:
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

  afx_msg void OnBnClickedFirstPage(void);
  afx_msg void OnBnClickedPrevPage(void);
  afx_msg void OnBnClickedNextPage(void);
  afx_msg void OnBnClickedLastPage(void);
  afx_msg void OnBnClickedClose(void);
  afx_msg void OnBnClickedPrint(void);

  afx_msg void OnClose();


protected:
  int m_chartIdx;
  int m_precision;
  int m_precisionY;
  string_t m_label;                 // Chart Name
  string_t m_header;                // Page Header String
  string_t m_labelX;
  string_t m_labelY;
  val_label_str_fn m_pXLabelStrFn;
  val_label_str_fn m_pYLabelStrFn;

public:
  V_CHARTDATAD m_vDataPnts;
  V_VALSTRINGS m_vStrX;
  V_VALSTRINGS m_vStrY;
  V_ROWS m_vRows;
  MAP_SELPNTSD m_mapSelCells;
protected:
  Color m_bkColor;
  Color m_selColor;
  Color m_textColor;
public:
  string_t m_fontName;
  float m_fontHeight;
  float m_headerFontHeight;

  RectF m_clipRF;
protected:
  RectF m_headerRF;
  RectF m_rowRF;
  RectF m_nmbRF;
  RectF m_dataYRF;
  RectF m_dataXRF;

  float m_offsX;
  float m_offsY;
  float m_deltaX;
  float m_deltaY;
  size_t m_page;
  size_t m_cols;
  size_t m_rows;
  size_t m_currPageID;
public:
  size_t m_nPages;
protected:
  float m_dpiRatio;

  CPageCtrl m_leftEnd;
  CPageCtrl m_leftArr;
  CPageCtrl m_rightArr;
  CPageCtrl m_rightEnd;
  CButton   m_btnPrint;
  CButton   m_btnClose;
};
