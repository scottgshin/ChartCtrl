
// ChartContainer.cpp : implementation file
//

#include "stdafx.h"
#include <ctime>
#include "ChartDef.h"
#include "Chart.h"
#include "DataLabel.h"
#include "ChartContainer.h"
#include "ChartDataView.h"
#include "ChartsXMLSerializer.h"
#include "Util.h"
#include <Shlwapi.h>
#include <filesystem>
#include <locale>

using namespace std;
using namespace Gdiplus;
namespace fs = std::filesystem;

#ifdef _UNICODE
#else if _MBCS
string_t ToNarrow(const wchar_t* s, char dfault = '?',
    const std::locale& loc = std::locale())
{
    sstream_t stm;

    while (*s != L'\0') {
        stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
    }
    return stm.str();
}
#endif

///////////////////////////////////////////////////////////////////////////////
// CChartContainer

IMPLEMENT_DYNAMIC(CChartContainer, CWnd)

CChartContainer::CChartContainer(string_t name) :
    m_name(name),
    m_pLabValStrFnPtr(&GetLabelValStr)
{
}

CChartContainer::~CChartContainer()
{
    if (m_pChartDataView != nullptr)
    {
        if (IsWindow(m_pChartDataView->m_hWnd))
            m_pChartDataView->DestroyWindow();
        delete m_pChartDataView;
        m_pChartDataView = nullptr; // Really you do not need this line
    }

    DestroyLabWnd(m_pDataWnd);
    DestroyLabWnd(m_pLegWnd);

    if (m_pMatrixD != nullptr)
    {
        delete m_pMatrixD;
        m_pMatrixD = nullptr;
    }

    if (!m_mapCharts.empty())
    {
        MAP_CHARTS::iterator it = m_mapCharts.begin();
        MAP_CHARTS::iterator itE = m_mapCharts.end();
        for (; it != itE; ++it)
        {
            delete it->second;
            it->second = nullptr;
        }
        m_mapCharts.clear();  // Again, just in case
    }
    m_pLabValStrFnPtr = nullptr;
}

CChartContainer& CChartContainer::operator=(const CChartContainer& rhs)
{
    if (this != &rhs)
    {
        m_pDataWnd = DestroyLabWnd(m_pDataWnd); // NULL
        m_pLegWnd = DestroyLabWnd(m_pLegWnd);  //NULL

        m_colBkground = rhs.m_colBkground;
        m_colBorder = rhs.m_colBorder;
        m_colAxis = rhs.m_colAxis;
        m_colZoomBorder = rhs.m_colZoomBorder;
        m_colLegPnts = rhs.m_colLegPnts;
        m_colDataViewPnts = rhs.m_colDataViewPnts;
        m_colLegBkgnd = rhs.m_colLegBkgnd;
        m_colLegBorder = rhs.m_colLegBorder;
        m_colGrid = rhs.m_colGrid;

        m_minExtY = rhs.m_minExtY;
        m_maxExtY = rhs.m_maxExtY;
        m_startX = rhs.m_startX;
        m_endX = rhs.m_endX;

        m_axOffsXF = 0.0f;         // Can't be defined until container wnd is created
        m_axXPos = BETWEEN;
        m_axYPos = MIDDLE;

        m_scX = 1.0;     // Will be calculated in OnPaint()
        m_scY = 1.0;

        m_bTracking = rhs.m_bTracking;    // Call ShowLegend after wnd creation to see
        m_bHasNamesLeg = rhs.m_bHasNamesLeg;
        m_bUserEnabled = rhs.m_bUserEnabled;
        m_dataLegPntD = rhs.m_dataLegPntD;  // data legend window
        m_epsX = rhs.m_epsX;         // Neighbourhood to search for nearest pnts
        m_bAxBoundaries = rhs.m_bAxBoundaries,
            m_chartSelIdx = rhs.m_chartSelIdx;
        m_dataViewChartIdx = -1;  // No Data View for transfer

        if (m_pMatrixD)
            delete m_pMatrixD;
        m_pMatrixD = nullptr; // To calculate in OnPaint()

        m_precision = rhs.m_precision;

        m_name = rhs.m_name;
        m_labelX = rhs.m_labelX.empty() ? string_t(_T("X")) : rhs.m_labelX;
        m_pLabValStrFnPtr = rhs.m_pLabValStrFnPtr;

        // Copy charts
        if (!m_mapCharts.empty())
        {
            MAP_CHARTS::iterator itDest = m_mapCharts.begin();
            MAP_CHARTS::iterator itDestE = m_mapCharts.end();
            for (; itDest != itDestE; ++itDest)
                delete (itDest->second);
            m_mapCharts.clear();
        }

        MAP_CHARTS::const_iterator itCharts = rhs.m_mapCharts.begin();
        MAP_CHARTS::const_iterator itChartsE = rhs.m_mapCharts.end();
        for (; itCharts != itChartsE; ++itCharts)
        {
            CChart* chartPtr = itCharts->second->CloneChart(true);
            m_mapCharts.insert(MAP_CHARTS::value_type(itCharts->first, chartPtr));
        }

        m_vHistX = rhs.m_vHistX;
        m_vHistY = rhs.m_vHistY;
        m_chModeX = rhs.m_chModeX;
        m_chModeY = rhs.m_chModeY;

        m_mapSelPntsD = rhs.m_mapSelPntsD;
        m_mapDataViewPntsD.clear();

        if (IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (rhs.IsLabWndExist(false))
                CopyDataLegendWnd();
            else
                RefreshWnd();

            if (rhs.IsLabWndExist(true))
                ShowNamesLegend();
        }
    }

    m_use_coordinates = rhs.m_use_coordinates;
    m_show_legend = rhs.m_show_legend;
    m_first_time = rhs.m_first_time;
    m_x_axis_is_logarithmic = rhs.m_x_axis_is_logarithmic;
    m_y_axis_is_logarithmic = rhs.m_y_axis_is_logarithmic;
    m_show_y_axis_second_scale = rhs.m_show_y_axis_second_scale;
    m_legendLocation = rhs.m_legendLocation;
    m_x_axis_label = rhs.m_x_axis_label;
    m_y_axis_label = rhs.m_y_axis_label;
    m_showXLabel = rhs.m_showXLabel;
    m_showYLabel = rhs.m_showYLabel;
    m_image_path = rhs.m_image_path;

    return *this;
}

bool CChartContainer::CreateChartCtrlWnd(DWORD dwExStyle, DWORD dwStyle,
    const CRect& wndRect, CWnd* pParent, UINT nID)
{
    BOOL bRes = CWnd::CreateEx(dwExStyle,
        AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS),
        NULL,
        dwStyle,
        wndRect,
        pParent,
        nID
    );

    if (bRes)
    {
        if (m_bTracking && !m_mapSelPntsD.empty() && !m_pDataWnd->GetSafeHwnd())
            CopyDataLegendWnd();
        if (m_bHasNamesLeg && !m_pLegWnd->GetSafeHwnd())
            ShowNamesLegend();

    }

    return bRes == TRUE ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// Helpers and operations

CChartContainer* CChartContainer::CloneChartContainer(string_t name, bool bCloneState)
{
    CChartContainer* pContainer = new CChartContainer;

    if (name.empty())
        pContainer->m_name = m_name;
    else
        pContainer->m_name = name;

    MAP_CHARTS::iterator itCharts = m_mapCharts.begin();
    MAP_CHARTS::iterator itChartsE = m_mapCharts.end();
    for (; itCharts != itChartsE; ++itCharts)
    {
        CChart* cloneChartPtr = itCharts->second->CloneChart(bCloneState);
        pContainer->m_mapCharts.insert(MAP_CHARTS::value_type(cloneChartPtr->GetChartIdx(),
            cloneChartPtr));
    }

    pContainer->m_colBkground = m_colBkground;
    pContainer->m_colBorder = m_colBorder;
    pContainer->m_colAxis = m_colAxis;
    pContainer->m_colZoomBorder = m_colZoomBorder;
    pContainer->m_colLegPnts = m_colLegPnts;
    pContainer->m_colDataViewPnts = m_colDataViewPnts;
    pContainer->m_colLegBkgnd = m_colLegBkgnd;
    pContainer->m_colLegBorder = m_colLegBorder;
    pContainer->m_colGrid = m_colGrid;

    pContainer->m_labelX = m_labelX;
    pContainer->m_pLabValStrFnPtr = m_pLabValStrFnPtr;

    if (bCloneState)
    {
        pContainer->m_bTracking = m_bTracking;
        pContainer->m_bHasNamesLeg = m_bHasNamesLeg;
        pContainer->m_dataLegPntD = m_dataLegPntD;
        pContainer->m_epsX = m_epsX;

        pContainer->m_startX = m_startX;
        pContainer->m_endX = m_endX;
        pContainer->m_minExtY = m_minExtY;
        pContainer->m_maxExtY = m_maxExtY;
        pContainer->m_vHistX = m_vHistX;
        pContainer->m_vHistY = m_vHistY;
        pContainer->m_bUserEnabled = m_bUserEnabled;
        pContainer->m_chartSelIdx = m_chartSelIdx;
        pContainer->m_bAxBoundaries = m_bAxBoundaries;
        pContainer->m_chModeX = m_chModeX;
        pContainer->m_chModeY = m_chModeY;

        if (m_bTracking && IsLabWndExist(false))
            pContainer->m_mapSelPntsD = m_mapSelPntsD;

        pContainer->m_use_coordinates = m_use_coordinates;
        pContainer->m_show_legend = m_show_legend;
        pContainer->m_first_time = m_first_time;
        pContainer->m_x_axis_is_logarithmic = m_x_axis_is_logarithmic;
        pContainer->m_y_axis_is_logarithmic = m_y_axis_is_logarithmic;
        pContainer->m_show_y_axis_second_scale = m_show_y_axis_second_scale;
        pContainer->m_legendLocation = m_legendLocation;
        pContainer->m_x_axis_label = m_x_axis_label;
        pContainer->m_y_axis_label = m_y_axis_label;
        pContainer->m_showXLabel = m_showXLabel;
        pContainer->m_showYLabel = m_showYLabel;
        pContainer->m_image_path = m_image_path;
    }
    else
    {
        pContainer->m_startX = GetInitialStartX();
        pContainer->m_endX = GetInitialEndX();
        pContainer->m_minExtY = GetInitialMinExtY();
        pContainer->m_maxExtY = GetInitialMaxExtY();
        pContainer->m_vHistX.clear(); // Just in case; the constructor sets them empty
        pContainer->m_vHistY.clear();
        pContainer->m_chModeX = MODE_FULLX;
        pContainer->m_chModeY = MODE_FULLY;
    }

    pContainer->m_precision = m_precision;

    return pContainer;
}

CChart* CChartContainer::CloneChart(int chartIdx, bool bCloneState)
{
    MAP_CHARTS::iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.end())
        return it->second->CloneChart(bCloneState);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Setting colors

void CChartContainer::SetBkColor(Color col, bool bRedraw)
{
    m_colBkground = col;
    if (bRedraw)
        RefreshWnd();
}

void CChartContainer::SetBorderColor(Color col, bool bRedraw)
{
    m_colBorder = col;
    if (bRedraw)
        RefreshWnd();
}

void CChartContainer::SetAxesColor(Color col, bool bRedraw)
{
    m_colAxis = col;
    if (bRedraw)
        RefreshWnd();
}

void CChartContainer::SetZoomBorderColor(Color col)
{
    if (m_colZoomBorder.GetValue() != col.GetValue())
    {
        m_colZoomBorder = col;

        if (m_chModeX == MODE_ZOOMINGX)
        {
            PointD tmpPntD(m_tmpZX, 0.0);
            PointF tmpPntF = m_pMatrixD->TransformToPntF(1.0, tmpPntD);
            CPoint tmpPnt = CPointFromPntF(tmpPntF);
            DrawZoomBorderX(tmpPnt.x);
        }
        else if (m_chModeY == MODE_ZOOMINGY)
        {
            PointD tmpPntD(0.0, m_tmpZY);
            PointF tmpPntF = m_pMatrixD->TransformToPntF(1.0, tmpPntD);
            CPoint tmpPnt = CPointFromPntF(tmpPntF);
            DrawZoomBorderY(tmpPnt.y);
        }
    }
}

void CChartContainer::SetLegPntsColor(Color col, bool bRedraw)
{
    m_colLegPnts = col;
    if (bRedraw && IsLabWndVisible(false))
        RefreshWnd();
}

void CChartContainer::SetDataViewPntsColor(Color col, bool bRedraw)
{
    m_colDataViewPnts = col;
    if (bRedraw && (!m_mapDataViewPntsD.empty()))
        RefreshWnd();
}

void CChartContainer::SetLegBkgndColor(Color col, bool bRedraw)
{
    m_colLegBkgnd = col;
    if (m_pLegWnd != nullptr)
    {
        m_pLegWnd->SetBkColor(col);
        if (bRedraw && m_pLegWnd->IsWindowVisible())
            RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }

    if (m_pDataWnd != nullptr)
    {
        m_pDataWnd->SetBkColor(col);
        if (bRedraw && m_pDataWnd->IsWindowVisible())
            RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }
}

void CChartContainer::SetLegBorderColor(Color col, bool bRedraw)
{
    m_colLegBorder = col;
    if (m_pLegWnd != nullptr)
    {
        m_pLegWnd->SetBorderColor(col);
        if (bRedraw && m_pLegWnd->IsWindowVisible())
            RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }

    if (m_pDataWnd != nullptr)
    {
        m_pDataWnd->SetBorderColor(col);
        if (bRedraw && m_pDataWnd->IsWindowVisible())
            RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }
}

void CChartContainer::SetGridColor(Color col, bool bRedraw)
{
    m_colGrid = col;
    if (bRedraw)
        RefreshWnd();
}

double CChartContainer::GetInitialStartX(void) const
{
    return m_vHistX.empty() ? m_startX : m_vHistX.front().first;
}

double CChartContainer::GetInitialEndX(void) const
{
    return m_vHistX.empty() ? m_endX : m_vHistX.front().second;
}

double CChartContainer::GetInitialMinExtY(void) const
{
    return m_vHistY.empty() ? m_minExtY : m_vHistY.front().first;
}

double CChartContainer::GetInitialMaxExtY(void) const
{
    return m_vHistY.empty() ? m_maxExtY : m_vHistY.front().second;
}

void CChartContainer::EnableUser(bool bEnable, bool bClearState)
{
    m_bUserEnabled = bEnable;
    if (!m_bUserEnabled && bClearState && !m_mapCharts.empty())
    {
        m_chartSelIdx = SelectChart(-1);
        m_bTracking = false;
        m_bHasNamesLeg = false;
        m_dataLegPntD = PointD(DBL_MAX, DBL_MAX);
        m_pDataWnd = DestroyLabWnd(m_pDataWnd, false);
        m_pLegWnd = DestroyLabWnd(m_pLegWnd);
        m_pChartDataView = DestroyChartDataView();
        ResetCharts(true);
    }
}

void CChartContainer::UpdateExtX(double minExtX, double maxExtX, bool bRedraw)
{
    if (maxExtX < minExtX)                   // Possible if from app
        return;

    double initStartX = GetInitialStartX();   // Old initial m_startX, m_endX
    double initEndX = GetInitialEndX();

    double startX, endX;

    if (initStartX > initEndX)  // The container is empty
    {
        startX = minExtX;
        endX = maxExtX;
    }
    else
    {
        startX = min(minExtX, initStartX);
        endX = max(maxExtX, initEndX);
    }

    if (startX == endX)
    {
        endX += fabs(startX * 0.01);
    }

    if (m_vHistX.size() > 0)   // Was zoomed or panned
        m_vHistX.front() = make_pair(startX, endX);
    else                      // Has no history
    {
        m_startX = startX;
        m_endX = endX;
    }

    if (bRedraw)
    {
        if (m_bTracking && IsLabWndExist(true))
            UpdateDataLegend(false);
        else
            RefreshWnd();
    }
}

void CChartContainer::UpdateExtY(double minExtY, double maxExtY, bool bRedraw)
{
    if (maxExtY < minExtY)                   // Possible if from app
        return;

    double initMinY = GetInitialMinExtY();   // Old initial m_startX, m_endX
    double initMaxY = GetInitialMaxExtY();

    double startY, endY;

    if (initMinY > initMaxY)  // The container is empty
    {
        startY = minExtY;
        endY = maxExtY;
    }
    else
    {
        startY = min(minExtY, initMinY);
        endY = max(maxExtY, initMaxY);
    }

    if (startY == endY)
    {
        double delta = fabs(startY * 0.01);
        startY -= delta * 4.0;
        endY += delta;
    }

    if (m_vHistY.size() > 0)   // Was zoomed or panned
        m_vHistY.front() = make_pair(startY, endY);
    else                      // Has no history
    {
        m_minExtY = startY;
        m_maxExtY = endY;
    }

    if (bRedraw)
    {
        if (m_bTracking && IsLabWndExist(false))
            UpdateDataLegend(true);
        else
            RefreshWnd();
    }
}

PAIR_DBLS CChartContainer::SetExtX(bool bRedraw)
{
    double startX, endX;
    std::tie(startX, endX) = GetMinMaxExtX();
    if (startX == endX)
        endX += fabs(startX * 0.01);

    if (m_vHistX.size() > 0)   // Was zoomed or panned
        m_vHistX.front() = make_pair(startX, endX);
    else                      // Has no history
    {
        m_startX = startX;
        m_endX = endX;
    }

    if (bRedraw)
    {
        if (m_bTracking && IsLabWndExist(true))
            UpdateDataLegend(false);
        else
            RefreshWnd();
    }

    return make_pair(startX, endX);
}

PAIR_DBLS CChartContainer::SetExtY(bool bRedraw)
{
    double minY, maxY;
    std::tie(minY, maxY) = GetMinMaxExtY();

    if (minY == maxY)
    {
        double delta = fabs(maxY * 0.01);
        minY -= delta * 4.0;
        maxY += delta;
    }

    if (m_vHistY.size() > 0)   // Was zoomed or panned
        m_vHistY.front() = make_pair(minY, maxY);
    else                      // Has no history
    {
        m_minExtY = minY;
        m_maxExtY = maxY;
    }

    if (bRedraw)
    {
        if (m_bTracking && IsLabWndExist(false))
            UpdateDataLegend(true);
        else
            RefreshWnd();
    }

    return make_pair(minY, maxY);
}

void CChartContainer::SetContainerPrecision(int precision, bool bRedraw)
{
    m_precision = precision;
    if (bRedraw)
    {
        if (IsLabWndVisible(false)) // This version of Update redraws dataWnd only
            UpdateDataLegend(false);  //(m_mapSelPntsD, m_pDataWnd->m_mapLabs);
        else if (m_bAxBoundaries)
            RefreshWnd();
        if (m_dataViewChartIdx != -1)
            UpdateDataView(m_dataViewChartIdx, F_NODATACHANGE);
    }
}

size_t CChartContainer::SetDataViewSelPnts(int chartIdx,
    const MAP_SELPNTSD& mapSelPntsD, V_CHARTDATAD* pChartData)
{
    if (chartIdx == -1)
        return 0;
    if (chartIdx != m_dataViewChartIdx)
        return 0;
    if (m_pChartDataView != nullptr)
    {
        m_pChartDataView->m_mapSelCells = mapSelPntsD;
        if (pChartData == nullptr)
        {
            m_pChartDataView->RefreshSelCells();
            if (IsWindow(m_pChartDataView->m_hWnd))
            {
                if (m_pChartDataView->IsIconic())
                    m_pChartDataView->ShowWindow(SW_RESTORE);
                m_pChartDataView->RedrawWindow(NULL, NULL,
                    RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE | RDW_ALLCHILDREN);
            }
        }
        else
            ReplaceChartData(chartIdx, *pChartData, false, false, false, true);
    }
    return mapSelPntsD.size();
}

bool CChartContainer::ClearDataViewChartIdx(void)
{
    if (m_pChartDataView != nullptr)
    {
        if (m_dataViewChartIdx != -1)
        {
            m_dataViewChartIdx = -1;
            UpdateDataViewPnts(-1, 0, PointD(0.0, 0.0), false);
        }

        DestroyChartDataView();
        return true;
    }
    return false;
}

void CChartContainer::SetAxisXName(string_t nameX, bool bRedraw)
{
    m_labelX = NormalizeString(nameX, STR_MAXLEN, STR_NORMSIGN);

    if (bRedraw)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        else if (m_bAxBoundaries)
            RefreshWnd();
        if (m_dataViewChartIdx != -1)
            UpdateDataView(m_dataViewChartIdx, F_NODATACHANGE);
    }
}

void CChartContainer::SetLabXValStrFn(val_label_str_fn pLabValStrFn, bool bRedraw)
{
    m_pLabValStrFnPtr = pLabValStrFn;

    if (bRedraw)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        else if (m_bAxBoundaries)
            RefreshWnd();
        if (m_dataViewChartIdx != -1)
            UpdateDataView(m_dataViewChartIdx, F_NODATACHANGE);
    }
}

bool CChartContainer::HasChartWithData(int chartIdx, bool bVisibleOnly) const
{
    MAP_CHARTS::const_iterator it = chartIdx == -1 ? m_mapCharts.cbegin() :
        m_mapCharts.find(chartIdx);
    bool bHasChart = false;

    for (; it != m_mapCharts.cend(); ++it)
    {
        if (it->second->HasData())
        {
            if (!bVisibleOnly || it->second->IsChartVisible())
                bHasChart = true;
        }
        if (bHasChart || (chartIdx != -1))
            break;
    }

    return bHasChart;
}

int CChartContainer::HasChartDifferentPnts(int chartIdx, bool bVisibleOnly) const
{
    MAP_CHARTS::const_iterator it = chartIdx == -1 ? m_mapCharts.cbegin() :
        m_mapCharts.find(chartIdx);
    int resIdx = -1;

    for (; it != m_mapCharts.cend(); ++it)
    {
        CChart* pChart = it->second;
        if ((pChart->GetMaxValX() > pChart->GetMinValX()) ||
            (pChart->GetMaxValY() > pChart->GetMinValY()))
        {
            if (!bVisibleOnly || pChart->IsChartVisible())
                resIdx = pChart->GetChartIdx();
        }
        if ((resIdx > -1) || (chartIdx != -1))
            break;
    }

    return resIdx;
}

bool CChartContainer::HasVisiblePntsDInYBand(int chartIdx, double bottomY, double topY)
{
    bool bHasPntsD = false;
    CRect clR;
    GetClientRect(&clR);
    CPoint pntTop(clR.left, clR.top);
    CPoint pntBottom(clR.right, clR.bottom);
    PointD pntTopD, pntBottomD;
    MatrixD* pMatrixD = GetTransformMatrixD(m_startX, m_endX, bottomY, topY);
    MousePntToPntD(pntTop, pntTopD, pMatrixD);
    MousePntToPntD(pntBottom, pntBottomD, pMatrixD);
    delete pMatrixD;

    auto chartIt = chartIdx == -1 ? m_mapCharts.cbegin() : m_mapCharts.find(chartIdx);

    for (; chartIt != m_mapCharts.cend(); ++chartIt)
    {
        CChart* chartPtr = chartIt->second;
        if (!chartPtr->IsChartVisible())
            continue;

        double locScY = chartPtr->GetLocScaleY();
        double topY1 = pntTopD.Y;
        double bottomY1 = pntBottomD.Y;
        // Select visible X-part of the chart
        V_CHARTDATAD::iterator itB, itE;
        std::tie(itB, itE) = chartPtr->GetStartEndDataIterators(chartPtr->m_vDataPnts, m_startX, m_endX);
        itB = find_if(itB, itE,
            [topY1, bottomY1, locScY](const PointD& pntD) {return in_range(bottomY1, topY1, locScY * pntD.Y); });
        if (itB != itE)
        {
            bHasPntsD = true;
            break;
        }
        if (chartIdx != -1)
            break;
    }

    return bHasPntsD;
}

MatrixD* CChartContainer::GetTransformMatrixD(double startX, double endX, double minY, double maxY)
{
    CDC* pDC = GetDC();
    Graphics gr(pDC->m_hDC);

    RectF rGdiF;
    gr.GetVisibleClipBounds(&rGdiF);                 // The same as the clip rect

    rGdiF.Width -= 1.0f;
    rGdiF.Height -= 1.0f;
    rGdiF.Inflate(-DR_SPACE, -DR_SPACE);
    // Axis X: Its Y Position and Offset; also Y coord of orig point
    PAIR_XAXPOS axXPosY = GetXAxisPos(rGdiF, minY, maxY);
    // Axis Y: Its X Position and ofset; also X-coord of orig. point
    PAIR_YAXPOS axYPosX = GetYAxisPos(rGdiF, startX, endX);

    std::pair<double, double> pair_scales = UpdateScales(rGdiF,
        startX, endX, minY, maxY);
    double scX = pair_scales.first;
    double scY = pair_scales.second;

    double offsetY(axXPosY.second);

    MatrixD matrixD;
    matrixD.Translate(axYPosX.second, offsetY);
    matrixD.Scale(scX, -scY);

    if ((axYPosX.first != BETWEEN) || (axXPosY.first != MIDDLE))
    {
        double translateX = (axYPosX.first == RIGHT) ? -endX :
            (axYPosX.first == LEFT) ? -startX : 0.0;
        double translateY = (axXPosY.first == BOTTOM) ? -minY :
            (axXPosY.first == TOP) ? -maxY : 0.0;
        matrixD.Translate(float(translateX), float(translateY));
    }

    ReleaseDC(pDC);
    return matrixD.Clone();
}

void CChartContainer::SetTransformMatrixD(MatrixD* pMatrixD)
{
    if (m_pMatrixD != nullptr)
        delete m_pMatrixD;
    m_pMatrixD = pMatrixD->Clone();
}

void CChartContainer::SetTransformMatrixD(void)
{
    if (m_pMatrixD != nullptr)
        delete m_pMatrixD;
    m_pMatrixD =
        GetTransformMatrixD(m_startX, m_endX, m_minExtY, m_maxExtY);
}

PAIR_DBLS CChartContainer::GetMinMaxExtX(void) const
{
    double fMinExtX = DBL_MAX;
    double fMaxExtX = -DBL_MAX;

    double currMinX, currMaxX;
    CChart* chartPtr;

    MAP_CHARTS::const_iterator it = m_mapCharts.cbegin();
    MAP_CHARTS::const_iterator itE = m_mapCharts.cend();
    for (; it != itE; ++it)
    {
        chartPtr = it->second;
        currMinX = chartPtr->GetMinValX();
        currMaxX = chartPtr->GetMaxValX();
        if (currMinX < fMinExtX)
            fMinExtX = currMinX;
        if (currMaxX > fMaxExtX)
            fMaxExtX = currMaxX;
    }

    return make_pair(fMinExtX, fMaxExtX);
}

PAIR_DBLS CChartContainer::GetMinMaxExtY(void) const
{
    double fMinExtY = DBL_MAX;
    double fMaxExtY = -DBL_MAX;

    double currMinY, currMaxY;
    CChart* chartPtr;

    MAP_CHARTS::const_iterator it = m_mapCharts.cbegin();
    MAP_CHARTS::const_iterator itE = m_mapCharts.cend();
    for (; it != itE; ++it)
    {
        chartPtr = it->second;
        currMinY = chartPtr->GetMinValY();
        currMaxY = chartPtr->GetMaxValY();
        if (currMinY < fMinExtY)
            fMinExtY = currMinY;
        if (currMaxY > fMaxExtY)
            fMaxExtY = currMaxY;
    }

    return make_pair(fMinExtY, fMaxExtY);
}

void CChartContainer::UndoHistStepX(bool bRedraw)
{
    if (m_vHistX.empty())
        return;

    PAIR_POS zh = m_vHistX.back();
    m_startX = zh.first;
    m_endX = zh.second;
    m_vHistX.pop_back();

    // Restore state
    if (m_vHistX.empty())
        m_chModeX = MODE_FULLX;
    else
        m_chModeX = MODE_ZOOMEDX;

    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
    {
        if (m_bTracking && (m_pDataWnd != nullptr))
            UpdateDataLegend(true);
        else
            RefreshWnd();
    }
}

void CChartContainer::UndoHistStepY(bool bRedraw)
{
    if (m_vHistY.empty())
        return;

    PAIR_POS zh = m_vHistY.back();
    m_minExtY = zh.first;
    m_maxExtY = zh.second;

    if (m_vHistY.size() > 1)    // Must check whether it is moves only
    {
        auto itZ = adjacent_find(m_vHistY.rbegin(), m_vHistY.rend(),
            [](const PAIR_POS& lhs, const PAIR_POS& rhs) ->bool
            {return (fabs(1.0 - fabs((rhs.first - lhs.first) / (rhs.second - lhs.second))) > 4.0 * DBL_EPSILON); });
        if (itZ == m_vHistY.rend())
            m_chModeY = MODE_MOVEDY;
        else
            m_chModeY = MODE_ZOOMEDY;
    }
    else
        m_chModeY = MODE_FULLY;

    m_vHistY.pop_back();

    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
    {
        if (m_bTracking && (m_pDataWnd != nullptr))
            UpdateDataLegend(true);
        else
            RefreshWnd();
    }
}

bool CChartContainer::UnwindHistX(double minX, double maxX)
{
    if (m_vHistX.empty())
        return false;

    PAIR_POS pair_step(minX, maxX);
    auto itB = find_if(m_vHistX.begin(), m_vHistX.end(),
        [minX, maxX](const PAIR_POS& pair_pos) {return (pair_pos.first == minX) && (pair_pos.second == maxX); });
    if (itB == m_vHistX.end())
        return false;
    m_vHistX.erase(++itB, m_vHistX.end());
    return true;
}

void CChartContainer::OnChangedSize(int cx, int cy)
{
    HideLabWnds();            // Called from parent's/owner's OnSize handler
    MoveWindow(0, 0, cx, cy);
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    SetTimer(2, 50, NULL);
}

// Added by SGS
void CChartContainer::OnChangedSize(const PointT<int> pt, const int width, const int height)
{
    HideLabWnds();            // Called from parent's/owner's OnSize handler
    MoveWindow(pt.X, pt.Y, width, height);
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    SetTimer(2, 50, NULL);
}

void CChartContainer::RefreshWnd(void)
{
    CRect clR, legR, dataR;
    GetClientRect(&clR);

    CRgn rgn1, rgn2, rgn3;
    rgn1.CreateRectRgn(0, 0, clR.right, clR.bottom);
    if (IsLabWndVisible(true))
    {
        m_pLegWnd->GetWindowRect(&legR);
        ScreenToClient(&legR);
        rgn2.CreateRectRgn(legR.left, legR.top, legR.right, legR.bottom);
        rgn1.CombineRgn(&rgn1, &rgn2, RGN_DIFF);
    }
    if (IsLabWndVisible(false))
    {
        m_pDataWnd->GetWindowRect(&dataR);
        ScreenToClient(&dataR);
        rgn3.CreateRectRgn(dataR.left, dataR.top, dataR.right, dataR.bottom);
        rgn1.CombineRgn(&rgn1, &rgn3, RGN_DIFF);
    }

    RedrawWindow(NULL, &rgn1, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);//|RDW_ALLCHILDREN);
}

Status CChartContainer::SaveContainerImage(string_t pathName)
{
    if (!HasChartWithData(-1, true))   // Repeat to provide for standalone use
        return GenericError;

    Status status = Aborted;
    UINT  num;        // number of image encoders
    UINT  size;       // size, in bytes, of the image encoder array

    // How many encoders are there? How big (in bytes) is the array of all ImageCodecInfo objects?
    GetImageEncodersSize(&num, &size);
    // Create a buffer large enough to hold the array of ImageCodecInfo objects that will be returned by GetImageEncoders.
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    // GetImageEncoders creates an array of ImageCodecInfo objects and copies that array into a previously allocated buffer.
    GetImageEncoders(num, size, pImageCodecInfo);
    // Get filter string
    sstream_t stream_t;
    string_t str_t, tmp_t;
    string_t szFilter;
    CLSID clsID;
    typedef std::map<string_t, CLSID> MAP_CLSID;
    typedef MAP_CLSID::value_type TYPE_VALCLSID;
    typedef MAP_CLSID::iterator IT_CLSID;

    MAP_CLSID mapCLSID;

    for (UINT j = 0; j < num; ++j)
    {
#ifdef _UNICODE
        str_t = pImageCodecInfo[j].MimeType;
#else if _MBCS
        str_t = ToNarrow(pImageCodecInfo[j].MimeType);
#endif
        size_t delPos = str_t.find(TCHAR('/'), 0);
        str_t.erase(0, delPos + 1);
        clsID = pImageCodecInfo[j].Clsid;
        mapCLSID.insert(TYPE_VALCLSID(str_t, clsID));
        tmp_t = str_t;
        std::transform(tmp_t.begin(), tmp_t.end(), tmp_t.begin(), [](const TCHAR& tch) ->TCHAR {return (TCHAR)toupper(tch); });
        szFilter += tmp_t + string_t(_T(" File|*.")) + str_t + string_t(_T("|"));
    }

    szFilter += string_t(_T("|"));
    free(pImageCodecInfo);

    if (pathName.empty())     // Let the user choose
    {
        // Use a common path - SGS
        auto path = fs::current_path();
        if (!m_image_path.empty())
        {
            path = m_image_path;
        }
#ifdef _UNICODE
        string_t dirStr = path.parent_path().wstring();
        string_t fullpath = path.wstring();
#else if _MBCS
        string_t dirStr = path.parent_path().string();
        string_t fullpath = path.string();
#endif
        if (!path.has_filename())
        {
            dirStr = fullpath;
        }

        szFilter += string_t(_T("|"));
        // Default to jpeg file
        CFileDialog fileDlg(FALSE, _T("JPEG File"), PathFindFileName(fullpath.c_str()),
            OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER,
            szFilter.c_str(), this);

        fileDlg.m_ofn.lpstrInitialDir = dirStr.c_str();
        fileDlg.m_ofn.lpstrTitle = _T("Save As Image");
        fileDlg.m_ofn.nFilterIndex = 2;

        string_t strTitle(_T("Save "));

        if (fileDlg.DoModal() == IDOK)
        {
            pathName = string_t(fileDlg.GetPathName());
        }
        else
            return Ok;
    }

    if (pathName.empty())
        return InvalidParameter;

    // Allow a period in the name as long as a period is found before the extension - SGS
    size_t pos = pathName.find_last_of(_T("."));
    if (pos == string_t::npos)
        return GenericError;

    string_t szExt = pathName.substr(pos);
    pos = szFilter.find(szExt);
    if (pos == string_t::npos)
        return UnknownImageFormat;
    szExt.erase(0, 1);

    IT_CLSID it = mapCLSID.find(szExt);
    if (it != mapCLSID.end())
    {
        SendNotification(CODE_SAVEIMAGE);
        CRect clR;
        GetClientRect(&clR);
        Rect rGdi = CRectToGdiRect(clR);
        Bitmap bmp(rGdi.Width, rGdi.Height);
        DrawContainerToBmp(rGdi, bmp);

        clsID = it->second;
#ifdef _UNICODE
        status = bmp.Save(pathName.c_str(), &clsID);
#else if _MBCS
        USES_CONVERSION;
        status = bmp.Save(CA2W(pathName.c_str()), &clsID);
#endif
        SendNotification(CODE_SAVEDIMAGE);
    }
    else status = UnknownImageFormat;
    return status;
}

HRESULT CChartContainer::SaveChartData(string_t pathName, bool bAll)
{
    HRESULT hr = S_FALSE;
    // See what you are going to save
    if (m_mapCharts.empty())
        return hr;
    if (!bAll && (nullptr == GetVisibleChart(m_chartSelIdx)))
        return hr;

    // Select file
    if (pathName.empty())       // The user should enter the path
    {
        TCHAR szFilters[] = _T("XML Files (*.xml)|*.xml||");
        TCHAR szWorkDirPath[255];
        GetModuleFileName(nullptr, szWorkDirPath, 255);
        PathRemoveFileSpec(szWorkDirPath);

        string_t dirStr(szWorkDirPath);
        size_t lastSlash = dirStr.find_last_of(_T("\\")) + 1;
        dirStr.erase(lastSlash, dirStr.size() - lastSlash);
        dirStr += string_t(_T("Charts"));

        CFileDialog fileDlg(FALSE, _T("xml"), _T("*.xml"),
            OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER,
            szFilters, this);
        fileDlg.m_ofn.lpstrInitialDir = dirStr.c_str();
        fileDlg.m_ofn.lpstrTitle = _T("Save Charts");

        string_t strTitle(_T("Save "));

        if (fileDlg.DoModal() == IDOK)
        {
            pathName = string_t(fileDlg.GetPathName());
        }
    }
    if (pathName.empty())
        return S_OK;

    size_t extPos = pathName.rfind(string_t(_T(".xml")));
    if (extPos != string_t::npos)
    {
        SendNotification(CODE_SAVECHARTS);
        CChartContainer* pContainer = CloneChartContainer(string_t(_T("")), true);
        SendNotification(CODE_SAVEDCHARTS);
        string_t chartName;
        if (m_chartSelIdx != -1)
            chartName = pContainer->GetChartName(m_chartSelIdx);
        hr = CChartsXMLSerializer::ChartDataToXML(pathName.c_str(), pContainer, chartName, bAll);
        delete pContainer;
    }
    else
        hr = S_OK;

    return hr;
}

HRESULT CChartContainer::LoadCharts(LPCTSTR fileName, const MAP_CHARTCOLS& mapContent, bool bClearCharts)
{
    HRESULT hr = CChartsXMLSerializer::XMLToCharts(fileName, this, mapContent, bClearCharts);
    if (hr == S_OK)
    {
        if (IsLabWndExist(false))
            PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, false);

        UpdateContainerWnds();
    }
    return hr;
}

HRESULT CChartContainer::ReplaceContainerCharts(LPCTSTR fileName)
{
    CChartContainer tmpContainer;
    HRESULT hr = CChartsXMLSerializer::ReplaceChartsFromXMLFile(fileName, &tmpContainer);
    if (hr == S_OK)
    {
        *this = tmpContainer;
        UpdateContainerWnds();
    }
    return hr;
}

HRESULT CChartContainer::GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_CHARTCOLS& mapContent)
{
    return CChartsXMLSerializer::GetChartNamesFromXMLFile(fileName, mapContent);
}

HRESULT CChartContainer::GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_NAMES& mapNames)
{
    return CChartsXMLSerializer::GetChartNamesFromXMLFile(fileName, mapNames);
}

bool CChartContainer::ExportChartData(string_t chartName, V_CHARTDATAD& vDataPnts) const
{
    CChart* chartPtr = FindChartByName(chartName);
    if (chartPtr != nullptr)
    {
        size_t dataSize = chartPtr->m_vDataPnts.size();
        if (dataSize != 0)
        {
            vDataPnts.resize(dataSize);
            copy(chartPtr->m_vDataPnts.cbegin(), chartPtr->m_vDataPnts.cend(), vDataPnts.begin());
        }
        else
            vDataPnts.clear();
        return true;
    }
    return false;
}

bool CChartContainer::ExportChartData(string_t chartName,
    std::vector<std::pair<double, double> >& vPairs) const
{
    CChart* chartPtr = FindChartByName(chartName);
    if (chartPtr != nullptr)
    {
        size_t dataSize = chartPtr->m_vDataPnts.size();
        if (dataSize != 0)
        {
            vPairs.resize(dataSize);
            for (size_t i = 0; i < dataSize; ++i)
            {
                PointD pntD = chartPtr->m_vDataPnts[i];
                vPairs[i] = make_pair(pntD.X, pntD.Y);
            }
        }
        else
            vPairs.clear();
        return true;
    }
    return false;
}

bool CChartContainer::ExportChartData(string_t chartName,
    std::vector<double>& vX, std::vector<double>& vY) const
{
    CChart* chartPtr = FindChartByName(chartName);
    if (chartPtr != nullptr)
    {
        size_t dataSize = chartPtr->m_vDataPnts.size();
        if (dataSize != 0)
        {
            vX.resize(dataSize);
            vY.resize(dataSize);
            for (size_t i = 0; i < dataSize; ++i)
            {
                PointD pntD = chartPtr->m_vDataPnts[i];
                vX[i] = pntD.X;
                vY[i] = pntD.Y;
            }
        }
        else
        {
            vX.clear();
            vY.clear();
        }
        return true;
    }
    return false;
}

// Chart access and manipulation functions
///
/// Returns false in the third member of the tuple if the chart does not exist
///
std::tuple<double, double, bool> CChartContainer::GetChartBoundaries(int chartIdx, bool bY) const
{
    MAP_CHARTS::const_iterator itCh = m_mapCharts.find(chartIdx);
    if (itCh != m_mapCharts.cend())
        return bY == false ? make_tuple(itCh->second->GetMinValX(), itCh->second->GetMaxValX(), true) :
        make_tuple(itCh->second->GetMinValY(), itCh->second->GetMaxValY(), true);
    return make_tuple(DBL_MAX, -DBL_MAX, false);
}

CChart* CChartContainer::GetVisibleChart(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = (chartIdx) == -1 ? m_mapCharts.cbegin() :
        m_mapCharts.find(chartIdx);
    for (; it != m_mapCharts.cend(); ++it)
    {
        CChart* chartPtr = it->second;
        if (chartPtr->IsChartVisible() && chartPtr->HasData())
            return chartPtr;
        if (chartIdx != -1)
            break;
    }
    return nullptr;
}

bool CChartContainer::IsChartVisible(int chartIdx) const
{
    return GetVisibleChart(chartIdx) == nullptr ? false : true;
}

bool CChartContainer::SetChartVisibility(int chartIdx, bool bVisible, bool bRedraw)
{
    bool bRes = false;
    MAP_CHARTS::iterator it = chartIdx == -1 ? m_mapCharts.begin() : m_mapCharts.find(chartIdx);
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        bRes = true;
        bool bVisState = it->second->IsChartVisible();
        if (bVisState != bVisible)
        {
            it->second->SetChartVisibility(bVisible);
            if (bVisible == false)
            {
                if (it->second->IsChartSelected())
                {
                    m_chartSelIdx = -1;
                    it->second->SelectChart(false);
                }

                if (m_dataViewChartIdx == it->first)
                {
                    if (m_pChartDataView != nullptr)
                    {
                        if (IsWindow(m_pChartDataView->m_hWnd))
                            m_pChartDataView->DestroyWindow();
                        delete m_pChartDataView;
                        m_pChartDataView = nullptr;
                    }
                    UpdateDataViewPnts(-1, 0, PointD(0.0, 0.0), false);
                }
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRedraw && bRes && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(chartIdx))
        UpdateContainerWnds();

    return bRes;
}

int CChartContainer::ToggleChartVisibility(int chartIdx)
{
    MAP_CHARTS::iterator it = m_mapCharts.find(chartIdx);
    if (it == m_mapCharts.end())
        return -1;
    CChart* chartPtr = it->second;

    bool bVisible = chartPtr->ToggleChartVisibility();
    if (!bVisible)
    {
        if (chartPtr->IsChartSelected())
        {
            m_chartSelIdx = -1;
            chartPtr->SelectChart(false);
        }
        if (m_dataViewChartIdx == chartIdx)
        {
            if (m_pChartDataView != nullptr)
            {
                if (IsWindow(m_pChartDataView->m_hWnd))
                    m_pChartDataView->DestroyWindow();
                delete m_pChartDataView;
                m_pChartDataView = nullptr;
            }
            UpdateDataViewPnts(-1, 0, PointD(0.0, 0.0), false);
        }
    }

    if (chartPtr->HasData() && IsWindow(m_hWnd) && IsWindowVisible())
        UpdateContainerWnds();

    return bVisible ? 1 : 0;
}

std::pair<float, bool> CChartContainer::GetChartPenWidth(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return make_pair(it->second->GetPenWidth(), true);
    return make_pair(0.0f, false);
}

bool CChartContainer::UpdateChartPenWidth(int chartIdx, float penWidth, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    MAP_CHARTS::iterator it = (chartIdx == -1) ? m_mapCharts.begin() :
        m_mapCharts.find(chartIdx);
    for (; it != m_mapCharts.end(); ++it)
    {
        CChart* chartPtr = it->second;

        if (chartPtr->GetPenWidth() != penWidth)
        {
            if (!bVisibleOnly || chartPtr->IsChartVisible())
            {
                chartPtr->SetPenWidth(penWidth);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(chartIdx, true))
        UpdateContainerWnds();

    return bRes;
}

std::pair<float, bool> CChartContainer::GetChartTension(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return make_pair(it->second->GetTension(), true);
    return make_pair(FLT_MAX, false);
}

bool CChartContainer::SetChartTension(int chartIdx, float fTension, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    if (in_range(0.0f, 1.0f, fTension))
    {
        MAP_CHARTS::iterator it = (chartIdx != -1) ? m_mapCharts.find(chartIdx) : m_mapCharts.begin();
        MAP_CHARTS::iterator itE = m_mapCharts.end();
        for (; it != itE; ++it)
        {
            if (it->second->GetTension() != fTension)
            {
                if (!bVisibleOnly || it->second->IsChartVisible())
                {
                    it->second->SetTension(fTension);
                    bRes = true;
                }
            }
            if (chartIdx != -1)
                break;
        }

        if (bRes && bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(chartIdx, true))
            RefreshWnd();
    }
    return bRes;
}

std::pair<bool, bool> CChartContainer::AreChartPntsAllowed(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return make_pair(it->second->ArePntsAllowed(), true);
    return make_pair(false, false);
}

bool CChartContainer::AllowChartPnts(int chartIdx, bool bAllowed, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    MAP_CHARTS::iterator it = (chartIdx != -1) ? m_mapCharts.find(chartIdx) :
        m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        CChart* chartPtr = it->second;

        if (chartPtr->ArePntsAllowed() != bAllowed)
        {
            if (!bVisibleOnly || chartPtr->IsChartVisible())
            {
                it->second->AllowPnts(bAllowed);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && (-1 < HasChartDifferentPnts(chartIdx, true)))
        RefreshWnd();

    return bRes;
}

std::pair<Color, bool> CChartContainer::GetChartColor(int chartIdx) const
{
    if (chartIdx != -1)
    {
        MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
        if (it != m_mapCharts.cend())
            return make_pair(it->second->GetChartColor(), true);
    }

    return make_pair(Color::Black, false);
}

bool CChartContainer::SetChartColor(int chartIdx, Color col, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;

    MAP_CHARTS::iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.end())
    {
        CChart* chartPtr = it->second;
        if (chartPtr->GetChartColor().GetValue() != col.GetValue())
        {
            if (!bVisibleOnly || chartPtr->IsChartVisible())
            {
                it->second->SetChartColor(col);
                if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(chartIdx, true))
                    UpdateContainerWnds();
                bRes = true;
            }
        }
    }
    return bRes;
}

std::pair<DashStyle, bool> CChartContainer::GetChartDashStyle(int chartIdx) const
{
    if (chartIdx != -1)
    {
        MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
        if (it != m_mapCharts.cend())
            return make_pair(it->second->GetChartDashStyle(), true);
    }

    return make_pair(DashStyleSolid, false);
}

bool CChartContainer::SetChartDashStyle(int chartIdx, DashStyle dashStyle, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    MAP_CHARTS::iterator it = (chartIdx != -1) ? m_mapCharts.find(chartIdx) : m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();

    for (; it != itE; ++it)
    {
        CChart* chartPtr = it->second;

        if (chartPtr->GetChartDashStyle() != dashStyle)
        {
            if (!bVisibleOnly || chartPtr->IsChartVisible())
            {
                chartPtr->SetDashStyle(dashStyle);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && (-1 < HasChartDifferentPnts(chartIdx, true)))
        UpdateContainerWnds();

    return bRes;
}

int CChartContainer::HasSelectedChart(void) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.begin();
    MAP_CHARTS::const_iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        if (it->second->IsChartSelected())
            return it->first;
    }
    return -1;
}

int CChartContainer::SelectChart(int chartIdx, bool bRedraw)
{
    bool bSel = true;
    CChart* chartPtr = nullptr;
    if (m_chartSelIdx != -1)    // Has chart to deselect
    {
        chartPtr = GetChart(m_chartSelIdx);
        if (chartPtr != nullptr) // Somebody programmatically deleted before this call
            chartPtr->SelectChart(false);
        if ((chartIdx == -1) || (chartIdx == m_chartSelIdx))
        {
            m_chartSelIdx = -1;
            bSel = false;
        }
    }

    if (bSel)      // New selection was requested
    {
        chartPtr = GetVisibleChart(chartIdx);
        if (chartPtr != nullptr)
        {
            chartPtr->SelectChart(true);
            m_chartSelIdx = chartPtr->GetChartIdx();
        }
        else
            m_chartSelIdx = -1;
    }

    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(chartIdx, true))
        RefreshWnd();

    return m_chartSelIdx;

}

int CChartContainer::SelectChartByMouseClick(CPoint pnt, bool bRedraw)
{
    if (!m_mapCharts.empty())
    {
        PointF epsF(CHART_DTPNTSZ, CHART_PNTSTRSH);
        PointF pntF(float(pnt.x), float(pnt.y));

        PointD leftPntD = m_pMatrixD->TransformToPntD(1.0, pntF - epsF);
        PointD rightPntD = m_pMatrixD->TransformToPntD(1.0, pntF + epsF);

        CChart* chartPtr;
        MAP_CHARTS::iterator it = m_mapCharts.begin();
        MAP_CHARTS::iterator itE = m_mapCharts.end();

        V_CHARTDATAF vDataPntsF;

        int chartIdx = -1;

        for (; it != itE; ++it)
        {
            chartPtr = it->second;
            if (!chartPtr->HasData())
                continue;
            if (!chartPtr->IsChartVisible())
                continue;
            // Now look for sel pnts: Convert to the scteen pntsF
            V_CHARTDATAD vDataPntsD = chartPtr->m_vDataPnts;
            chartPtr->ConvertChartData(vDataPntsD, vDataPntsF, m_pMatrixD, leftPntD.X, rightPntD.X);
            if (vDataPntsF.size() > 0)
            {
                V_CHARTDATAF::iterator itData = find_if(vDataPntsF.begin(), vDataPntsF.end(),
                    in_vicinity<double>(pntF, epsF));
                if (itData != vDataPntsF.end())
                {
                    chartIdx = chartPtr->GetChartIdx();
                    break;
                }
            }
        }
        // ChartIdx == -1 if click is not in vicinity of any data point: deselect selected or select forst visible
        SelectChart(chartIdx == -1 ? m_chartSelIdx : chartIdx, bRedraw);
    }

    return m_chartSelIdx;
}

// Chart Y axis precision
int CChartContainer::GetChartPrecisionY(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return it->second->GetPrecisionY();
    return INT_MAX;
}

bool CChartContainer::SetChartPrecisionY(int chartIdx, int precisionY, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    MAP_CHARTS::iterator it = (chartIdx != -1) ? m_mapCharts.find(chartIdx) :
        m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        if (!bVisibleOnly || it->second->IsChartVisible())
        {
            if (it->second->GetPrecisionY() != precisionY)
            {
                it->second->SetPrecisionY(precisionY);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        if (chartIdx == -1)
            chartIdx = m_dataViewChartIdx;
        UpdateDataView(chartIdx, F_NODATACHANGE);
    }
    return bRes;
}

string_t CChartContainer::GetChartName(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return it->second->GetChartName();
    return string_t(_T(""));     // Not a name at all
}

bool CChartContainer::ChangeChartName(int chartIdx, string_t name, bool bRedraw)
{
    MAP_CHARTS::iterator itCh = m_mapCharts.find(chartIdx);
    MAP_CHARTS::iterator it = m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();

    if (itCh != itE)                    // Chart is found
    {
        name = NormalizeString(name, STR_MAXLEN, STR_NORMSIGN);

        for (; it != itE; ++it)           // Check for uniqueness
        {
            if (it->second->GetChartName() == name)
                return false;
        }
        itCh->second->SetChartName(name); // Assign the new nane

        if (bRedraw)
            UpdateContainerWnds(chartIdx);

        return true;
    }
    return false;
}

// Chart Y data name
string_t CChartContainer::GetChartYValName(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return it->second->GetAxisYName();
    return string_t(_T(""));
}

bool CChartContainer::ChangeChartYValName(int chartIdx, string_t nameY, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;

    nameY = NormalizeString(nameY, STR_MAXLEN, STR_NORMSIGN);
    MAP_CHARTS::iterator it = chartIdx != -1 ? m_mapCharts.find(chartIdx) :
        m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        if (!bVisibleOnly || it->second->IsChartVisible())
        {
            string_t oldNameY = it->second->GetAxisYName();
            if (oldNameY != nameY)
            {
                it->second->SetAxisYName(nameY);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        UpdateDataView(chartIdx != -1 ? chartIdx : m_dataViewChartIdx, F_NODATACHANGE);
    }
    return bRes;
}

// Function to get Y val strung function
val_label_str_fn CChartContainer::GetLabYValStrFnPtr(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return it->second->GetLabYValStrFnPtr();
    return nullptr;
}

bool CChartContainer::SetLabYValStrFn(int chartIdx, val_label_str_fn pLabYValStrFn, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;

    MAP_CHARTS::iterator it = chartIdx != -1 ? m_mapCharts.find(chartIdx) : m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();

    for (; it != itE; ++it)
    {
        if (!bVisibleOnly || it->second->IsChartVisible())
        {
            val_label_str_fn oldFnPtr = it->second->GetLabYValStrFnPtr();
            if (oldFnPtr != pLabYValStrFn)
            {
                it->second->SetLabYValStrFn(pLabYValStrFn);
                bRes = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        UpdateDataView(chartIdx != -1 ? chartIdx : m_dataViewChartIdx, F_NODATACHANGE);
    }

    return bRes;
}

CChart* CChartContainer::GetChart(int chartIdx, bool bVisibleOnly) const
{
    CChart* chartPtr = nullptr;
    MAP_CHARTS::const_iterator it = chartIdx == -1 ? m_mapCharts.cbegin() : m_mapCharts.find(chartIdx);
    for (; it != m_mapCharts.end(); ++it)
    {
        chartPtr = it->second;
        if (!bVisibleOnly || chartPtr->IsChartVisible())
        {
            if (chartPtr != nullptr)
                break;
        }
        if (chartIdx != -1)
            break;
    }
    return chartPtr;
}

V_CHARTDATAD* CChartContainer::GetChartData(int chartIdx)
{
    CChart* chartPtr = GetChart(chartIdx);
    if (chartPtr != nullptr)
        return &(chartPtr->m_vDataPnts);
    return nullptr;
}

CChart* CChartContainer::FindChartByName(string_t nameStr) const
{
    CChart* chartPtr = nullptr;
    MAP_CHARTS::const_iterator it = m_mapCharts.cbegin();
    MAP_CHARTS::const_iterator itE = m_mapCharts.cend();
    for (; it != itE; ++it)
    {
        if (it->second->GetChartName() == nameStr)
        {
            chartPtr = it->second;
            break;
        }
    }
    return chartPtr;
}

int CChartContainer::GetChartIdx(string_t chartName) const
{
    CChart* chartPtr = FindChartByName(chartName);
    if (chartPtr != nullptr)
        return chartPtr->GetChartIdx();
    return -1;
}

std::pair<double, bool>CChartContainer::GetChartLocScaleY(int chartIdx) const
{
    MAP_CHARTS::const_iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.cend())
        return make_pair(it->second->GetLocScaleY(), true);
    return make_pair(0.0, false);
}

bool CChartContainer::UpdateChartLocScaleY(int chartIdx, double locScY, bool bVisibleOnly, bool bRedraw)
{
    bool bRes = false;
    bool bChangeView = false;

    MAP_CHARTS::iterator it = (chartIdx != -1) ? m_mapCharts.find(chartIdx) :
        m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        CChart* chartPtr = it->second;
        if (!bVisibleOnly || chartPtr->IsChartVisible())
        {
            double locScaleY = chartPtr->GetLocScaleY();
            if (locScaleY != locScY)
            {
                chartPtr->SetLocScaleY(max(0.1, locScY));
                bRes = true;
                if (!bChangeView && bRedraw && chartPtr->IsChartVisible() && chartPtr->HasData())
                    bChangeView = true;
            }
        }
        if (chartIdx != -1)
            break;
    }

    if (bChangeView)
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(false);
        else
            RefreshWnd();
    }
    return bRes;
}

bool CChartContainer::ZoomMoveContainerX(double startX, double endX, bool bRedraw)
{
    if (startX == endX)
        return false;

    double fStartX = GetInitialStartX();
    double fEndX = GetInitialEndX();

    // Don't zoom/move an empty space: one of the points should be inside
    if (in_range(fStartX, fEndX, startX) || in_range(fStartX, fEndX, endX))
    {
        m_vHistX.push_back(PAIR_POS(m_startX, m_endX));
        m_startX = min(startX, endX);
        m_endX = max(startX, endX);
        m_chModeX = MODE_ZOOMX;   // All prepared but not rendered
        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible() && HasChartWithData(-1, true))
        {
            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();
        }
        return true;
    }
    return false;
}

CH_MODE CChartContainer::ZoomContainerX(double startX, double endX, bool bRedraw)
{
    if (true == ZoomMoveContainerX(startX, endX, false))
    {
        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();
            m_chModeX = MODE_ZOOMEDX; // Old extensionX is in m_dqHist
        }
        else
            m_chModeX = MODE_ZOOMX;   // Wait for redrawing
    }
    else
        m_chModeX = m_vHistX.empty() ? MODE_FULLX : MODE_ZOOMEDX;

    return m_chModeX;
}

CH_MODE CChartContainer::MoveContainerChartsX(double deltaX, bool bRedraw)
{
    if (true == ZoomMoveContainerX(m_startX - deltaX, m_endX - deltaX, false))
    {
        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();
            m_chModeX = MODE_ZOOMEDX; // Old extensionX is in m_dqHist
        }
        else
            m_chModeX = MODE_ZOOMX;   // Wait for redrawing
    }
    else
        m_chModeX = m_vHistX.empty() ? MODE_FULLX : MODE_ZOOMEDX;

    return m_chModeX;
}

bool CChartContainer::ZoomMoveContainerY(double startY, double endY, bool bMoving, bool bRedraw)
{
    if (startY == endY)
        return false;

    if (startY > endY)
        swap(startY, endY);

    // Don't zoom/move an empty space: one of the points should be inside the band
    if (HasVisiblePntsDInYBand(-1, startY, endY))
    {
        m_vHistY.push_back(PAIR_POS(m_minExtY, m_maxExtY));
        m_minExtY = startY;
        m_maxExtY = endY;
        if (bMoving && ((m_chModeY == MODE_FULLY) || (m_chModeY == MODE_MOVEDY)))
            m_chModeY = MODE_MOVEY;
        else
            m_chModeY = MODE_ZOOMY;   // All prepared but not rendered

        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (bMoving && (m_chModeY == MODE_FULLY))
                m_chModeY = MODE_MOVEDY;
            else
                m_chModeY = MODE_ZOOMEDY;   // Had zoom Y before

            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();
        }
        return true;
    }
    return false;
}

CH_MODE CChartContainer::ZoomContainerY(double startY, double endY, bool bRedraw)
{
    // Get real minmax ext;
    double rangeY = endY - startY;      // It is called from menu, so startY < endY
    endY = startY + 0.9 * rangeY;      // 0.9 screen height down from the top
    startY += 0.1 * rangeY;               // 0.1 screen height up from the bottom

    if (true == ZoomMoveContainerY(startY, endY, false, false))  // Sets chModeY to MODE_ZOOMY on success
    {
        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();

            m_chModeY = MODE_ZOOMEDY; // Old extensionX is in m_dqHist
        }

    }

    return m_chModeY;
}

CH_MODE CChartContainer::MoveContainerChartsY(double deltaY, bool bRedraw)
{
    if (true == ZoomMoveContainerY(m_minExtY - deltaY, m_maxExtY - deltaY, true, false)) // Sets modeY to MODE_MOVEY
    {                                                                                    // or MODE_ZOOMY (if was zoomed previously
        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        {
            if (m_bTracking && IsLabWndExist(false))
                UpdateDataLegend(true);
            else
                RefreshWnd();

            if (m_chModeY == MODE_MOVEY)
                m_chModeY = MODE_MOVEDY;
            else
                m_chModeY = MODE_ZOOMEDY;
        }
    }

    return m_chModeY;
}


LRESULT CChartContainer::SendNotification(UINT code, int chartIdx)
{
    NMCHART nmchart;
    nmchart.hdr.hwndFrom = m_hWnd;
    nmchart.hdr.idFrom = GetDlgCtrlID();
    nmchart.hdr.code = code;
    nmchart.chartIdx = chartIdx;

    switch (code)
    {
    case CODE_VISIBILITY: 
        nmchart.bState = IsChartVisible(chartIdx);
        break;
    case CODE_SHOWPNTS:   
        nmchart.bState = AreChartPntsAllowed(chartIdx).first; 
        break;
    case CODE_EXTX:
    case CODE_EXTY:
        nmchart.minX = GetStartX();
        nmchart.maxX = GetEndX();
        nmchart.minY = GetMinY();
        nmchart.maxY = GetMaxY();
        break;
    case CODE_REFRESH:    
        nmchart.minX = GetInitialStartX();
        nmchart.maxX = GetInitialEndX();
        nmchart.minY = GetInitialMinExtY();
        nmchart.maxY = GetInitialMaxExtY();
        break;
    case CODE_SAVEIMAGE:
    case CODE_SAVEDIMAGE:
    case CODE_SAVECHARTS:
    case CODE_SAVEDCHARTS:
    case CODE_PRINTING:
    case CODE_PRINTED:
    case CODE_SCY:        
        break;
    case CODE_TRACKING:   
        nmchart.bState = m_bTracking;
        break;
    default:              
        return 0;
    }

    CWnd* parentPtr = (CWnd*)GetParent();
    if (parentPtr != nullptr)
        return parentPtr->SendMessage(WM_NOTIFY, WPARAM(nmchart.hdr.hwndFrom), LPARAM(&nmchart));
    return 0;
}

// Chart interface functions
int CChartContainer::AddChart(bool bVisible, bool bShowPnts, string_t label,
    string_t labelY, int precisionY,
    DashStyle dashStyle, float penWidth, float tension,
    Color colChart, V_CHARTDATAD& vData, PointStyle pntStyle, 
    bool bRedraw)
{
    int chartIdx = GetMaxChartIdx() + 1;
    bool bAddIdx = false;
    if (!label.empty())
    {
        label = NormalizeString(label, STR_MAXLEN, STR_NORMSIGN);
        CChart* twinPtr = FindChartByName(label);
        if (twinPtr != nullptr)
            bAddIdx = true;
    }
    else
    {
        label = string_t(_T("Chart"));
        bAddIdx = true;
    }

    if (bAddIdx)
    {
        _TCHAR buffer_t[64] = {0};
        _itot_s(chartIdx, buffer_t, 10);  // Chart idx to string
        string_t idxStr(buffer_t);
        label += string_t(_T("_")) + string_t(buffer_t);
    }

    CChart* chartPtr = new CChart;

    chartPtr->SetChartAttr(bVisible, bShowPnts, chartIdx, label, labelY,
        precisionY, dashStyle, penWidth, tension, colChart, pntStyle);

    size_t dataSize = vData.size();

    // Now transfer data and set min max values
    if (dataSize > 0)
    {
        chartPtr->m_vDataPnts.assign(vData.begin(), vData.end());
        chartPtr->m_vDataPnts.shrink_to_fit();

        // It is cheaper to sort right away than to look for max/min x and sort later if needed
        if (dataSize > 1)
            std::sort(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), less_pnt<double, false>());

        double minValX = chartPtr->m_vDataPnts.front().X;
        double maxValX = chartPtr->m_vDataPnts.back().X;

        // Find min and max Y; works even for one-point vector
        PAIR_ITS pair_minmaxY =
            minmax_element(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(),
                less_pnt<double, true>());
        double minValY = pair_minmaxY.first->Y;
        double maxValY = pair_minmaxY.second->Y;

        // Save in the CChart
        chartPtr->SetMinValX(minValX);
        chartPtr->SetMaxValX(maxValX);
        chartPtr->SetMinValY(minValY);
        chartPtr->SetMaxValY(maxValY);
    }

    // Just in case: idx is unique for this container
    if (m_mapCharts.insert(MAP_CHARTS::value_type(chartPtr->GetChartIdx(), chartPtr)).second == false)
    {
        delete chartPtr;
        return -1;
    }

    // Now update the container's min maxes, saving the history of X
    if (dataSize > 0)
    {
        // Wil automatically take care of previous one-point charts
        UpdateExtX(chartPtr->GetMinValX(), chartPtr->GetMaxValX());
        UpdateExtY(chartPtr->GetMinValY(), chartPtr->GetMaxValY());

        if (IsWindow(m_hWnd) && m_bTracking && IsLabWndExist(false))
            PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, true);

        if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
            UpdateContainerWnds(-1, true);
    }

    return chartIdx;
}

// Overload for time series
int CChartContainer::AddChart(bool bVisible, bool bShowPnts, string_t label,
    string_t labelY, int precisionY, DashStyle dashStyle,
    float penWidth, float tension,
    Color colChart, std::vector<double>& vTmSeries,
    double startX, double stepX, PointStyle pntStyle, 
    bool bRedraw)
{
    V_CHARTDATAD vData(vTmSeries.size());
    if (vData.size() > 0)
        transform(vTmSeries.begin(), vTmSeries.end(), vData.begin(), time_series_to_pnt<double>(startX, stepX));

    return AddChart(bVisible, bShowPnts, label, labelY, precisionY,
        dashStyle, penWidth, tension, colChart, vData, pntStyle, bRedraw);
}

int CChartContainer::AddChart(bool bVisible, bool bShowPnts, string_t label,
    string_t labelY, int precisionY, DashStyle dashStyle,
    float penWidth, float tension, Color colChart,
    std::vector<std::pair<double, double> >& vXYData, PointStyle pntStyle, 
    bool bRedraw)
{
    V_CHARTDATAD vData(vXYData.size());
    if (vData.size() > 0)
        transform(vXYData.begin(), vXYData.end(), vData.begin(), xy_to_pnt<double>());

    return AddChart(bVisible, bShowPnts, label, labelY, precisionY,
        dashStyle, penWidth, tension, colChart, vData, pntStyle, bRedraw);
}

int CChartContainer::AddChart(bool bVisible, bool bShowPnts, string_t label, string_t labelY,
    int precisionY, DashStyle dashStyle, float penWidth, float tension,
    Color colChart, std::vector<double>& vX, std::vector<double>& vY,
    PointStyle pntStyle, bool bRedraw)
{
    size_t sizeX = vX.size();
    size_t sizeY = vY.size();
    if (sizeX != sizeY)
        return -1;

    V_CHARTDATAD vData(sizeX);
    if (vData.size() > 0)
    {
        for (size_t i = 0; i < sizeX; ++i)
            vData[i] = PointD(vX[i], vY[i]);
    }
    return AddChart(bVisible, bShowPnts, label, labelY, precisionY,
        dashStyle, penWidth, tension, colChart, vData, pntStyle, bRedraw);
}

int CChartContainer::AddChart(bool bVisible, bool bShowPnts, string_t label, string_t labelY,
                              int precisionY, DashStyle dashStyle, float penWidth, float tension,
                              Color colChart, std::vector<time_t>& vX, std::vector<double>& vY,
                              PointStyle pntStyle, bool bRedraw)
{
    size_t sizeX = vX.size();
    size_t sizeY = vY.size();
    if (sizeX != sizeY)
        return -1;

    // Assumes that the data is sorted, oldest first
    auto first_time_stamp(vX[0]);

    V_CHARTDATAD vData(sizeX);
    if (vData.size() > 0)
    {
        // Turn the time stamp data into an X offset
        for (size_t i = 0; i < sizeX; ++i) 
        { 
            vData[i] = PointD(static_cast<double>(vX[i]-first_time_stamp), vY[i]); 
        }
    }
    return AddChart(bVisible, bShowPnts, label, labelY, precisionY, dashStyle, penWidth, tension,
                    colChart, vData, pntStyle, bRedraw);
}


// Appends points to the end of chart data vector. Discards all new points with X < m_endX
bool CChartContainer::AppendChartData(int chartIdx, V_CHARTDATAD& vData, bool bUpdate)
{
    size_t dataSize = vData.size();
    if (dataSize == 0)
        return false;

    MAP_CHARTS::iterator it = m_mapCharts.find(chartIdx);
    if (it != m_mapCharts.end())
    {
        V_CHARTDATAD vLocData(vData);
        // Sort; it is cheaper than search for min/max X and check whether they are at start and end
        if (dataSize > 1)
            sort(vLocData.begin(), vLocData.end(), less_pnt<double, false>());

        CChart* chartPtr = it->second;  // Append data vector
        size_t appended = chartPtr->AppendChartData(vLocData);
        if (appended == 0)
            return false;

        if (IsLabWndExist(false))     // Might update m_pMatrix if m_dataLegPntD is in X range
            PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, true);

        UpdateDataView(chartIdx, F_APPEND);

        if (bUpdate && chartPtr->IsChartVisible())  // Update and display if only one chart has to be processed
        {
            UpdateExtX(chartPtr->GetMinValX(), chartPtr->GetMaxValX()); // New startX/endX are at the front dqHist
            UpdateExtY(chartPtr->GetMinValY(), chartPtr->GetMaxValY()); // New startY/endY are at the front dqHist

            if (IsWindow(m_hWnd) && IsWindowVisible())
                UpdateContainerWnds(chartIdx, true);
        }
        return true;
    }
    return false;
}

// Overload for time series
bool CChartContainer::AppendChartData(int chartIdx, std::vector<double>& vTmSeries, double startX,
    double stepX, bool bUpdate)
{
    if (vTmSeries.size() == 0)
        return false;

    V_CHARTDATAD vData(vTmSeries.size());
    transform(vTmSeries.begin(), vTmSeries.end(), vData.begin(), time_series_to_pnt<double>(startX, stepX));

    return AppendChartData(chartIdx, vData, bUpdate);
}

// Overload for vector of pairs
bool CChartContainer::AppendChartData(int chartIdx,
    std::vector<std::pair<double, double> >& vXYData, bool bUpdate)
{
    if (vXYData.size() == 0)
        return false;

    V_CHARTDATAD vData(vXYData.size());
    transform(vXYData.begin(), vXYData.end(), vData.begin(), xy_to_pnt<double>());

    return AppendChartData(chartIdx, vData, bUpdate);
}

// Overload for vectors of doubles
bool CChartContainer::AppendChartData(int chartIdx,
    std::vector<double>& vX, std::vector<double>& vY, bool bUpdate)
{
    size_t sizeX = vX.size();
    size_t sizeY = vY.size();
    if (sizeX != sizeY)
        return false;
    if (sizeX == 0)
        return false;

    V_CHARTDATAD vData(sizeX);
    for (size_t i = 0; i < sizeX; ++i)
        vData[i] = PointD(vX[i], vY[i]);

    return AppendChartData(chartIdx, vData, bUpdate);
}

bool CChartContainer::ReplaceChartData(int chartIdx, V_CHARTDATAD& vData, bool bClip,
    bool bUpdate, bool bVerbose, bool bRedraw)
{
    MAP_CHARTS::iterator mapIt = m_mapCharts.find(chartIdx);
    if (mapIt == m_mapCharts.end())
        return false;                           // Wrong Chart ID

     // Warning and reset if (bReset)
    if (bVerbose && (AfxMessageBox(CString("You will replace all old chart data. Replace anyway?"),
        MB_YESNO | MB_ICONQUESTION) == IDNO))
        return false;

    // Sort input vector; it is cheaper than check first whether it was sorted
    V_CHARTDATAD vLocData(vData);
    if (vLocData.size() > 1)
        sort(vLocData.begin(), vLocData.end(), less_pnt<double, false>());

    CChart* chartPtr = mapIt->second;

    // Define what take from vData
    V_CHARTDATAD::iterator itStart = vLocData.begin();
    V_CHARTDATAD::iterator itEnd = vLocData.end();

    if (bClip) // Asked to keep the new range inside the old startX, endX
    {
        PAIR_ITS pair_its = find_border_pnts(vLocData.begin(), vLocData.end(),
            not_inside_range<double, false>(GetInitialStartX(), GetInitialEndX()));
        itStart = pair_its.first;
        itEnd = pair_its.second;
        if (itEnd != vLocData.end())
            ++itEnd;
    }

    chartPtr->m_vDataPnts.assign(itStart, itEnd);
    chartPtr->m_vDataPnts.shrink_to_fit();

    size_t dataSize = chartPtr->m_vDataPnts.size();

    // Set new min/max
    double startX = dataSize == 0 ? DBL_MAX : chartPtr->m_vDataPnts.front().X;
    double endX = dataSize == 0 ? -DBL_MAX : chartPtr->m_vDataPnts.back().X;
    chartPtr->SetMinValX(startX);
    chartPtr->SetMaxValX(endX);

    // Update scales - indirect
    double minY = DBL_MAX, maxY = -DBL_MAX;
    if (dataSize > 0)
    {
        std::pair< V_CHARTDATAD::iterator, V_CHARTDATAD::iterator> pair_minmaxY =
            minmax_element(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), less_pnt<double, true>());
        minY = pair_minmaxY.first->Y;
        maxY = pair_minmaxY.second->Y;
    }

    chartPtr->SetMinValY(minY);
    chartPtr->SetMaxValY(maxY);
    chartPtr->SetLocScaleY(1.0);

    // Update m_startX,m_endX, min and max Y
    if (!HasChartWithData(-1))
    {
        ResetChartContainer(true);
    }
    else
    {
        if (IsLabWndExist(false))     // Might update m_pMatrix if m_dataLegPntD is in X range
            PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, true);

        UpdateDataView(chartIdx, F_REPLACE);

        if (bUpdate)   // Update and display
        {
            SetExtX();
            SetExtY();

            if (!HasVisiblePntsDInYBand(-1, m_minExtY, m_maxExtY))
                ResetCharts();
            else
            {
                if (!m_vHistX.empty())
                    m_vHistX.erase(++m_vHistX.begin(), m_vHistX.end());
                if (!m_vHistY.empty())
                    m_vHistY.erase(++m_vHistY.begin(), m_vHistY.end());
            }
        }
    }

    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        UpdateContainerWnds(chartIdx, true);

    return true;
}

// Overload for time series
bool CChartContainer::ReplaceChartData(int chartIdx, std::vector<double>& vTmSeries,
    double startX, double stepX, bool bClip, bool bUpdate, bool bVerbose, bool bRedraw)
{
    V_CHARTDATAD vData;
    if (!vTmSeries.empty())
        vData.resize(vTmSeries.size());
    transform(vTmSeries.begin(), vTmSeries.end(), vData.begin(), time_series_to_pnt<double>(startX, stepX));

    return ReplaceChartData(chartIdx, vData, bClip, bUpdate, bVerbose, bRedraw);
}

// Overload for vector of pairs
bool CChartContainer::ReplaceChartData(int chartIdx,
    std::vector<std::pair<double, double> >& vXYData, bool bClip, bool bUpdate, bool bVerbose, bool bRedraw)
{
    V_CHARTDATAD vData;
    if (!vXYData.empty())
        vData.resize(vXYData.size());
    transform(vXYData.begin(), vXYData.end(), vData.begin(), xy_to_pnt<double>());

    return ReplaceChartData(chartIdx, vData, bUpdate, bClip, bVerbose, bRedraw);
}
// Overload for vectors of doubles
bool CChartContainer::ReplaceChartData(int chartIdx,
    std::vector<double>& vX, std::vector<double>& vY, bool bClip, bool bUpdate, bool bVerbose, bool bRedraw)
{

    size_t sizeX = vX.size();
    size_t sizeY = vY.size();
    if (sizeX != sizeY)
        return false;

    V_CHARTDATAD vData;
    if (sizeX > 0)
        vData.resize(sizeX);
    for (size_t i = 0; i < sizeX; ++i)
        vData[i] = PointD(vX[i], vY[i]);

    return ReplaceChartData(chartIdx, vData, bClip, bUpdate, bVerbose, bRedraw);
}

void CChartContainer::ResetCharts(bool bResetLocY)
{
    // Restore startX, endX
    m_startX = GetInitialStartX();
    m_endX = GetInitialEndX();
    m_vHistX.clear();

    m_minExtY = GetInitialMinExtY();
    m_maxExtY = GetInitialMaxExtY();
    m_vHistY.clear();

    // Restore visibility
    MAP_CHARTS::iterator it = m_mapCharts.begin(), itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        CChart* chartPtr = it->second;
        if (!chartPtr->IsChartVisible())
            chartPtr->ToggleChartVisibility();
        if (bResetLocY)
            chartPtr->SetLocScaleY(1.0);
    }

    m_chModeX = MODE_FULLX;
    m_chModeY = MODE_FULLY;

    UpdateContainerWnds(-1, true);
}

void CChartContainer::ResetChartContainer(bool bKeepCharts, bool bResetCols)
{
    CChartContainer* pContainer = new CChartContainer;

    // Set defaula colors
    if (bResetCols)
    {
        m_colBkground = pContainer->GetBkColor();
        m_colBorder = pContainer->GetBorderColor();
        m_colAxis = pContainer->GetAxesColor();
        m_colZoomBorder = pContainer->GetZoomBorderColor();
        m_colLegPnts = pContainer->GetLegPntsColor();
        m_colDataViewPnts = pContainer->GetDataViewPntsColor();
        m_colLegBorder = pContainer->GetLegBkgndColor();
        m_colLegBorder = pContainer->GetLegBorderColor();
        m_colGrid = pContainer->GetGridColor();
    }

    //  Set default values
    m_minExtY = pContainer->GetMinY();
    m_maxExtY = pContainer->GetMaxY();
    m_startX = pContainer->GetStartX();
    m_endX = pContainer->GetEndX();
    m_axOffsXF = pContainer->GetAxisOffsetX();
    m_axXPos = pContainer->GetXAxisPos();
    m_axYPos = pContainer->GetYAxisPos();
    m_scX = pContainer->GetScX();
    m_scY = pContainer->GetScY();
    m_bTracking = false;
    m_dataLegPntD = pContainer->GetDataLegPntD();
    m_bAxBoundaries = false;
    m_chartSelIdx = -1;
    m_dataViewChartIdx = -1;   // Ti be sure, must be -1 if there is no data view
    m_tmpZX = 0.0;
    m_tmpZY = 0.0;
    m_chModeX = pContainer->GetChModeX();
    m_chModeY = pContainer->GetChModeY();

    delete pContainer;

    // Clear maps and vectors
    if (!bKeepCharts && !m_mapCharts.empty())
    {
        MAP_CHARTS::iterator it = m_mapCharts.begin();
        MAP_CHARTS::iterator itE = m_mapCharts.end();
        for (; it != itE; ++it)
            delete it->second;
        m_mapCharts.clear();
    }
    m_mapDataViewPntsD.clear();
    m_vHistX.clear();
    m_vHistY.clear();
    // Reset the matrix
    delete m_pMatrixD;
    m_pMatrixD = nullptr;
    // Get rid of the legend wnds
    m_pDataWnd = DestroyLabWnd(m_pDataWnd);  // Deletes m_mapSelPntsD, reset m_dataLegPntD
    m_pLegWnd = DestroyLabWnd(m_pLegWnd);
    // Clear the data view
    DestroyChartDataView();
}

bool CChartContainer::TruncateChart(int chartIdx, double startX, double endX,
    bool bUpdateExt, bool bRedraw)
{
    if (!HasChartWithData(chartIdx))                                 // No chart with data
        return false;

    if (startX >= endX)                                              // Invalid parameters
        return false;

    if ((GetInitialStartX() >= startX) && (GetInitialEndX() <= endX))  // Nothing to truncate
        return false;

    startX = max(startX, GetInitialStartX());
    endX = min(endX, GetInitialEndX());

    bool bCorrectDataView = false;

    size_t chartCnt = 0;
    MAP_CHARTS::iterator it = (chartIdx == -1) ? m_mapCharts.begin() :
        m_mapCharts.find(chartIdx);
    for (; it != m_mapCharts.end(); ++it)
    {
        CChart* chartPtr = it->second;  // Append data vector
        if (!chartPtr->HasData())
            continue;
        bool bRes = chartPtr->TruncateChartData(startX, endX);
        if (bRes)
        {
            ++chartCnt;
            if (m_dataViewChartIdx == chartPtr->GetChartIdx())
                bCorrectDataView = true;
        }

        if (chartIdx > -1)
            break;
    }

    if (chartCnt == 0)
        return false;

    if (bUpdateExt) // Update extensions X and Y
    {
        SetExtX();
        SetExtY();

        if (!HasVisiblePntsDInYBand(-1, m_minExtY, m_maxExtY))
            ResetCharts();
        else
        {
            if (!m_vHistX.empty())
                m_vHistX.erase(++m_vHistX.begin(), m_vHistX.end());
            if (!m_vHistY.empty())
                m_vHistY.erase(++m_vHistY.begin(), m_vHistY.end());
        }
    }

    if (bCorrectDataView)
        UpdateDataView(m_dataViewChartIdx, F_TRUNCATE);

    if (m_bTracking && IsLabWndExist(false))    // Will calculate and set the new translation matrix
        PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, true);

    // Now redraw
    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
    {
        if (IsLabWndVisible(false))
            UpdateDataLegend(true);    // The matrix was not changed yet
        else
            RefreshWnd();               // The matrix will changed in OnPaint()
    }
    return true;
}

bool CChartContainer::RemoveChart(int chartIdx, bool bCorrectMinMax, bool bRedraw)
{
    MAP_CHARTS::iterator mapIt = m_mapCharts.find(chartIdx);
    if (mapIt == m_mapCharts.end())
        return false;

    CChart* chartPtr = mapIt->second;

    delete chartPtr;
    m_mapCharts.erase(chartIdx);

    if (!HasChartWithData(-1))
        ResetChartContainer(true);
    else
    {
        if (m_chartSelIdx == chartIdx)
        {
            m_chartSelIdx = -1;
        }

        if (m_dataViewChartIdx == chartIdx)
        {
            if (m_pChartDataView != nullptr)
                DestroyChartDataView();
        }

        if (bCorrectMinMax)
        {
            SetExtX();
            SetExtY();

            if (!HasVisiblePntsDInYBand(-1, m_minExtY, m_maxExtY))
                ResetCharts();
            else
            {
                if (!m_vHistX.empty())
                    m_vHistX.erase(++m_vHistX.begin(), m_vHistX.end());
                if (!m_vHistY.empty())
                    m_vHistY.erase(++m_vHistY.begin(), m_vHistY.end());
            }

            if (IsLabWndExist(false))
                PrepareDataLegend(m_dataLegPntD, m_epsX, m_pDataWnd->m_mapLabs, m_mapSelPntsD, bCorrectMinMax);
        }
    }

    if (bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        UpdateContainerWnds();

    return true;
}

bool CChartContainer::ShowAxisXBoundaries(bool bSet, bool bRedraw)
{
    m_bAxBoundaries = HasChartWithData(-1) ? bSet : false;
    if (bRedraw)
        RefreshWnd();
    return m_bAxBoundaries;
}

// The function is called from OnLButtonDown; the container is visible
CDataWnd* CChartContainer::ShowDataLegend(CPoint pnt)
{
    if (m_mapCharts.empty())            // No charts, no legends
        return m_pDataWnd;

    if (m_pDataWnd == nullptr) // Need to allocate memory
    {
        CDataWnd* pDataWnd = new CDataWnd(m_colLegBkgnd, m_colLegBorder);
        if (pDataWnd == nullptr)
            return nullptr;
        m_pDataWnd = pDataWnd;
    }

    // After PrepareDataLegend mapSelData might have some points out the range Y, but mapLabs might be empty
    if (PrepareDataLegend(pnt, m_pDataWnd->m_mapLabs, m_mapSelPntsD) > 0)
    {                                   // Create dataWnd if needed, and calculate dataWnd rects
        m_pDataWnd->CreateLegend(this, pnt, true);
        RefreshWnd();
        m_pDataWnd->ShowLegendWnd();      // Garanteed m_pDataWnd exists
    }
    else
        m_pDataWnd = DestroyLabWnd(m_pDataWnd);

    return m_pDataWnd;
}

CDataWnd* CChartContainer::ShowDataLegend(double dataLegX)
{
    if (!IsWindow(m_hWnd) || !IsWindowVisible())  // Means container wnd was created and is visible
        return m_pDataWnd;
    if (m_mapCharts.empty())                    // No charts, no legends
        return m_pDataWnd;

    MAP_LABSTR tmpMapStr;
    m_dataLegPntD = PointD(dataLegX, m_minExtY);
    // After this point mapSelData might have some points out the range Y, but mapLabs might be empty
    
    // SGS
    auto tolerance = CHART_DTPNTSZ / m_scX;
    if (m_x_axis_is_logarithmic)
    {
        tolerance = CHART_DTPNTSZ / log10(m_scX);
    }

    size_t selPnts = PrepareDataLegend(m_dataLegPntD, tolerance, tmpMapStr, m_mapSelPntsD, false);
    if (selPnts > 0)
    {
        // Set m_pDataLeg
        CDataWnd* pDataWnd = nullptr;
        if (m_pDataWnd == nullptr)
        {
            pDataWnd = new CDataWnd(m_colLegBkgnd, m_colLegBorder);
            if (pDataWnd == nullptr)
                return nullptr;
            m_pDataWnd = pDataWnd;
        }
        if (m_pDataWnd)
        {
            m_pDataWnd->m_mapLabs.swap(tmpMapStr);
            PointF dataLegPntF = m_pMatrixD->TransformToPntF(1.0, m_dataLegPntD);
            CPoint pnt = CPointFromPntF(dataLegPntF);
            m_pDataWnd->CreateLegend(this, pnt, true);
            m_bTracking = true;

            RefreshWnd();
            m_pDataWnd->ShowLegendWnd();  // Garanteed m_pDataWnd exists
            return m_pDataWnd;
        }
    }

    m_bTracking = false;
    m_pDataWnd = DestroyLabWnd(m_pDataWnd);
    return m_pDataWnd;
}

CDataWnd* CChartContainer::ShowNamesLegend(void)
{
    if (IsWindow(m_hWnd) && IsWindowVisible() && !m_mapCharts.empty() && HasChartWithData(-1))
    {
        MAP_LABSTR tmpMapStr;
        PrepareNamesLegend(tmpMapStr);

        // No matter what, allocate memory because there are charts, may be hidden
        CDataWnd* pLegWnd = nullptr;
        if (m_pLegWnd == nullptr)
        {
            pLegWnd = new CDataWnd(m_colLegBkgnd, m_colLegBorder);
            if (pLegWnd == nullptr)
                return nullptr;
            m_pLegWnd = pLegWnd;
        }
        if (m_pLegWnd)
        {
            m_pLegWnd->m_mapLabs.swap(tmpMapStr);
            // Create dataWnd if needed, and calculate dataWnd rects
            if (m_pLegWnd->CreateLegend(this, CPoint(0, 0), false, m_legendLocation) == true)
            {
                m_bHasNamesLeg = true;
                RefreshWnd();
                m_pLegWnd->ShowLegendWnd();  // Garanteed m_pLegWnd exists
                m_show_legend = true;
            }
        }
    }
    else
    {
        // It makes no sense to keep memory if there is no container wnd, or charts in container, or chartst with data
        m_pLegWnd = DestroyLabWnd(m_pLegWnd);
    }

    return m_pLegWnd;
}

bool CChartContainer::ShowChartPoints(int chartIdx, bool bShow, bool bRedraw)
{
    bool bRes = false;

    MAP_CHARTS::iterator it = chartIdx == -1 ? m_mapCharts.begin() :
        m_mapCharts.find(chartIdx);
    for (; it != m_mapCharts.end(); ++it)
    {
        CChart* chartPtr = it->second;
        if ((chartPtr->GetMaxValX() > chartPtr->GetMinValX()) || (chartPtr->GetMaxValY() > chartPtr->GetMinValY()))
        {
            chartPtr->AllowPnts(bShow);
            if (chartPtr->IsChartVisible())
                bRes = true;
        }

        if (chartIdx != -1)
            break;
    }

    if (bRes && bRedraw && IsWindow(m_hWnd) && IsWindowVisible())
        UpdateContainerWnds();
    return bRes;
}


void CChartContainer::UpdateDataLegend(bool bChangeMatrix)
{
    if (m_bTracking && IsLabWndExist(false))
    {
        if (bChangeMatrix)
            SetTransformMatrixD();
        UpdateDataLegend(m_mapSelPntsD, m_pDataWnd->m_mapLabs);

        RefreshWnd(); // UpdateDataLegend draws dataWnd; RefreshWnd redraws the region excluding dataWnd
    }
}

void CChartContainer::CopyDataLegendWnd(void)
{
    if (m_pMatrixD == nullptr)
        SetTransformMatrixD();

    if (m_pDataWnd == nullptr)
        m_pDataWnd = new CDataWnd(m_colLegBkgnd, m_colLegBorder);
    bool bRes = false;
    if ((m_pDataWnd != nullptr) && !IsWindow(m_pDataWnd->m_hWnd))
        bRes = m_pDataWnd->CreateLegend(this);
    if (bRes)
        UpdateDataLegend(false);
}

// True for info on m_pLegWnd, false on m_pDataWnd
bool CChartContainer::IsLabWndVisible(bool bLeg) const
{
    CDataWnd* pDataWnd = bLeg ? m_pLegWnd : m_pDataWnd;
    if (pDataWnd != nullptr)
        return (IsWindow(pDataWnd->m_hWnd) && pDataWnd->IsWindowVisible()) ? true : false;

    return false;
}

bool CChartContainer::IsLabWndExist(bool bLeg) const
{
    CDataWnd* pDataWnd = bLeg ? m_pLegWnd : m_pDataWnd;
    if (pDataWnd != nullptr)
        return IsWindow(pDataWnd->m_hWnd) ? true : false;

    return false;
}

CDataWnd* CChartContainer::DestroyLabWnd(CDataWnd* pWnd, bool bRedraw)
{
    if (pWnd != nullptr)
    {
        if (IsWindow(pWnd->m_hWnd))
        {
            pWnd->DestroyWindow();
            if (pWnd->IsDataLeg())
            {
                m_mapSelPntsD.clear();
                m_dataLegPntD = PointD(DBL_MAX, DBL_MAX);
                // To remove selected data line
                if (bRedraw)
                    RefreshWnd();
            }
            else
                m_bHasNamesLeg = false;
        } // IsWindow
        delete pWnd;      // Here mapLegs goes out
        pWnd = nullptr;
    }
    return pWnd;
}

void CChartContainer::UpdateContainerWnds(int chartIdx, bool bMatrix, DATAVIEW_FLAGS dataChange)
{
    if (m_bTracking && IsLabWndExist(false))
    {
        if (HasChartWithData(-1))
            UpdateDataLegend(bMatrix);
        else
        {
            m_bTracking = false;
            m_pDataWnd = DestroyLabWnd(m_pDataWnd);
        }
    }
    else
        RefreshWnd();

    if (IsLabWndExist(true))
    {
        if (HasChartWithData(-1))
            ShowNamesLegend();
        else
            m_pLegWnd = DestroyLabWnd(m_pLegWnd);
    }

    UpdateDataView(chartIdx, dataChange);
    if (!HasChartWithData(-1))
    {
        delete m_pMatrixD;
        m_pMatrixD = nullptr;
    }
}

bool CChartContainer::HideLabWnds(bool bRedraw)
{
    bool bRes = false;
    if (IsLabWndVisible(false))
    {
        bRes = true;
        m_pDataWnd->ShowWindow(SW_HIDE);
        if (bRedraw)    // To remove sel line
            RefreshWnd();
    }
    if (IsLabWndVisible(true))
    {
        bRes = true;
        m_pLegWnd->ShowWindow(SW_HIDE);
    }
    return bRes;
}

CChartDataView* CChartContainer::DestroyChartDataView(void)
{
    if (m_pChartDataView != nullptr)
    {
        if (IsWindow(m_pChartDataView->m_hWnd))
            m_pChartDataView->DestroyWindow();
        m_dataViewChartIdx = -1;
        if (!m_mapDataViewPntsD.empty())
            m_mapDataViewPntsD.clear();
        delete m_pChartDataView;
        m_pChartDataView = nullptr;
    }
    return m_pChartDataView;
}

// Protected helpers

// To get max chart Idx in the mapCharts
int CChartContainer::GetMaxChartIdx(void) const
{
    int chartIdx = -1;
    if (!m_mapCharts.empty())
    {
        MAP_CHARTS::const_reverse_iterator rIt = m_mapCharts.rbegin();
        chartIdx = rIt->first;
    }

    return chartIdx;
}

void CChartContainer::MousePntToPntD(CPoint pnt, PointD& dataPntD, MatrixD* pMatrixD, double locScY)
{
    ENSURE(pMatrixD != nullptr);

    PointF pntF(static_cast<float>(pnt.x), static_cast<float>(pnt.y));
    // Return to a native coordinates; in most uses pntD.Y is not important, hence 1.0
    dataPntD = pMatrixD->TransformToPntD(locScY, pntF);

    // Test for NAN
    if (dataPntD.X != dataPntD.X)
        dataPntD.X = DBL_MAX;
    if (dataPntD.Y != dataPntD.Y)
        dataPntD.Y = DBL_MAX;
}

// Calculate scaleX, scaleY
double CChartContainer::UpdateScaleX(const float drawRectWidth, double startX, double endX)
{
    double scX = 1.0;
    if (!m_mapCharts.empty())
    {
        double rangeX = endX - startX;
        scX = drawRectWidth / rangeX;
    }
    return scX;
}

double CChartContainer::UpdateScaleY(const float drawRectHeight, double minY, double maxY)
{
    double scY = 1.0;
    if (!m_mapCharts.empty())
    {
        double rangeY = maxY - minY;
        scY = drawRectHeight / rangeY;
    }
    return scY;
}

// Calculates scX, scY but does not set m_scX, m_scY, leaves it to the user
PAIR_DBLS CChartContainer::UpdateScales(RectF& drawRectF,
    double startX, double endX, double minY, double maxY)
{
    if (m_mapCharts.empty())
        return make_pair(1.0, 1.0);

    // SGS - Legacy - Only centers the graph if the graph is centered around y = 0
    //RectF dRF = drawRectF;
    //if ((m_chModeY == MODE_FULLY)||(m_chModeY == MODE_MOVEDY)||(m_chModeY == MODE_MOVEY))
    //{
    //    // Legacy - Only centers the graph if the graph is centered around y = 0
    //    drawRectF.Inflate(0.0f, -0.1f*drawRectF.Height); // Reserve 20% to beautify full picture
    //}
    double scX = UpdateScaleX(drawRectF.Width, startX, endX);
    double scY = UpdateScaleY(drawRectF.Height, minY, maxY);
    return make_pair(scX, scY);
}

// Zooming X on mouse left click + MK_SHIFT: first click
double CChartContainer::BeginZoomingX(CPoint pnt, bool bDraw)
{
    m_chModeX = MODE_ZOOMINGX;

    PointD mousePntD;
    MousePntToPntD(pnt, mousePntD, m_pMatrixD);
    double startX = mousePntD.X;
    if (bDraw)
        DrawZoomBorderX(pnt);

    return startX;
}

// Zooming on mouse left click + MK_SHIFT: the second click
double CChartContainer::PrepareZoomX(CPoint pnt, double startX, bool bDraw)
{
    PointD mousePntD;
    MousePntToPntD(pnt, mousePntD, m_pMatrixD);
    double endX = mousePntD.X;

    PointF startPntF = m_pMatrixD->TransformToPntF(1.0, PointD(m_tmpZX, 0.0));

    float zoomW = fabs(pnt.x - startPntF.X);
    if (zoomW > DR_SPACE)
    {
        if (true == ZoomMoveContainerX(startX, endX, false)) // old m_endX, m_startX in m_vHistX
        {
            if (bDraw)
                DrawZoomBorderX(pnt);
        }
    }

    return endX;
}

// Restores the mode to ZOOM or FULLX after interruption between the first and second zoom clicks
CH_MODE CChartContainer::RestoreModeX(void)
{
    if (m_chModeX == MODE_ZOOMINGX)
    {
        if (m_vHistX.empty())
            m_chModeX = MODE_FULLX;
        else
            m_chModeX = MODE_ZOOMEDX;
    }
    return m_chModeX;
}

// Zooming Y on mouse double left click + MK_SHIFT: first click
double CChartContainer::BeginZoomingY(CPoint pnt, bool bDraw)
{
    PointD mousePntD;
    MousePntToPntD(pnt, mousePntD, m_pMatrixD);
    double startY = mousePntD.Y;
    if (bDraw)
    {
        RefreshWnd();         // Because first click of a double draws zoom X boundary
        DrawZoomBorderY(pnt);
    }

    m_chModeY = MODE_ZOOMINGY;
    return startY;
}

double CChartContainer::PrepareZoomY(CPoint pnt, double startY, bool bDraw)  // Second click
{
    PointD mousePntD;
    MousePntToPntD(pnt, mousePntD, m_pMatrixD);
    double endY = mousePntD.Y;

    PointF startPntF = m_pMatrixD->TransformToPntF(1.0, PointD(0.0, startY));
    float zoomW = fabs(pnt.y - startPntF.Y);

    if (zoomW > DR_SPACE)
    {
        if (true == ZoomMoveContainerY(startY, endY, false, false)) // old m_endX, m_startX in m_vHistY
        {
            if (bDraw)
                DrawZoomBorderY(pnt);
        }
    }

    return endY;
}

CH_MODE CChartContainer::RestoreModeY(void)
{
    if (m_chModeY == MODE_ZOOMINGY)
    {
        if (m_vHistY.empty())
            m_chModeY = MODE_FULLY;
        else
            m_chModeY = MODE_ZOOMEDY;
    }
    return m_chModeY;
}

// Protected data and names legend functions
size_t CChartContainer::PrepareDataLegend(CPoint pnt, MAP_LABSTR& mapLabels,
    MAP_SELPNTSD& mapSelPntsD)
{
    PointD dataLegPntD;
    MousePntToPntD(pnt, dataLegPntD, m_pMatrixD);

    double epsX = CHART_DTPNTSZ / m_pMatrixD->m_scX; // m11 is the scX

    // SGS
    if (m_x_axis_is_logarithmic)
    {
        epsX = CHART_DTPNTSZ / log10(m_pMatrixD->m_scX);
    }

    size_t mapLabelsSize = PrepareDataLegend(dataLegPntD, epsX, mapLabels, mapSelPntsD, false);
    if (mapSelPntsD.size() > 0)
    {
        m_dataLegPntD = dataLegPntD;
        m_epsX = epsX;
    }
    else
        m_dataLegPntD = PointD(DBL_MAX, DBL_MAX);

    return mapLabelsSize;
}

size_t CChartContainer::PrepareDataLegend(PointD origPntD, double epsX, MAP_LABSTR& mapLabels,
    MAP_SELPNTSD& mapSelPntsD, bool bChangeMatrix)
{
    if (bChangeMatrix)
        SetTransformMatrixD();

    mapSelPntsD.clear();
    mapLabels.clear();

    auto [startX, endX, startY, endY] = GetMinMaxes();

    if (in_range(startX, endX, origPntD.X)) // Make sense if pnt was sent by a programmer
    {
        CRect clientR;              // Get Y-boundaries in the data space
        GetClientRect(&clientR);
        CPoint topPnt(0, clientR.top);
        CPoint bottomPnt(0, clientR.bottom);
        PointD topPntD;
        PointD bottomPntD;
        MousePntToPntD(topPnt, topPntD, m_pMatrixD);
        MousePntToPntD(bottomPnt, bottomPntD, m_pMatrixD);

        //Search the chart map
        PointD selPntD;

        MAP_CHARTS::iterator it = m_mapCharts.begin();
        MAP_CHARTS::iterator itE = m_mapCharts.end();

        for (; it != itE; ++it)
        {
            CChart* chartPtr = it->second;
            bool bVisible = chartPtr->IsChartVisible();
            
            if (m_x_axis_is_logarithmic)
            {
                origPntD.X = pow(10.0f, origPntD.X);
            }
            if (m_y_axis_is_logarithmic)
            {
                origPntD.Y = pow(10.0f, origPntD.Y);
            }

            PAIR_ITNEAREST pair_res =
                chartPtr->GetNearestPointD(origPntD, epsX, selPntD);
            int nSel = pair_res.second;
            if (nSel > 0)    // Might be out of the client rect
            {
                V_CHARTDATAD::iterator itSel = pair_res.first;
                for (int i = nSel; i > 0; --i)
                {
                    selPntD = *itSel;
                    double selY = selPntD.Y * chartPtr->GetLocScaleY();
                    int chartIdx = chartPtr->GetChartIdx();
                    mapSelPntsD.insert(MAP_SELPNTSD::value_type(chartIdx, selPntD));

                    // SGS
                    if (m_y_axis_is_logarithmic)
                    {
                        selY = log10(selY);
                    }

                    if (bVisible && in_range(bottomPntD.Y, topPntD.Y, selY))    // Remember, Y was reversed
                    {
                        TUPLE_LABEL tuple_res = chartPtr->GetSelValString(selPntD, m_labelX, m_precision, m_pLabValStrFnPtr);
                        mapLabels.insert(MAP_LABSTR::value_type(chartIdx, tuple_res));
                    }
                    if (i > 1)
                        --itSel;
                }
            }
        }

        return mapLabels.size();
    }
    return 0;
}

size_t CChartContainer::PrepareNamesLegend(MAP_LABSTR& mapLabels)
{
    if (m_mapCharts.empty())
        return 0;

    if (!mapLabels.empty())
        mapLabels.clear();

    TUPLE_LABEL tuple_res;
    MAP_CHARTS::iterator it = m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        bool bSelected = it->second->GetVisibleChartNameAndVisuals(tuple_res);
        if (bSelected)
            mapLabels.insert(MAP_LABSTR::value_type(it->first, tuple_res));
    }
    return mapLabels.size();
}

// Refreshes label string map according to charts' visibility and X range
size_t CChartContainer::UpdateDataLegend(MAP_SELPNTSD& mapSelPntsD, MAP_LABSTR& mapLabStr)
{
    mapLabStr.clear();
    if (!mapSelPntsD.empty() && in_range(m_startX, m_endX, m_dataLegPntD.X))
    {
        CRect clRect;
        GetClientRect(&clRect);
        CPoint pntLimYL(0, clRect.bottom);
        CPoint pntLimYR(0, clRect.top);
        PointD pntLimYLD, pntLimYRD;
        MousePntToPntD(pntLimYL, pntLimYLD, m_pMatrixD);
        MousePntToPntD(pntLimYR, pntLimYRD, m_pMatrixD);

        MAP_SELPNTSD::iterator itSel = mapSelPntsD.begin();
        MAP_SELPNTSD::iterator itSelE = mapSelPntsD.end();
        while (itSel != itSelE)
        {
            int chartIdx = itSel->first;
            CChart* chartPtr = GetChart(chartIdx);
            if (chartPtr != nullptr)
            {
                if (chartPtr->IsChartVisible())
                {
                    PointD selPntD = itSel->second;
                    if (in_range(m_startX, m_endX, selPntD.X) &&
                        in_range(pntLimYLD.Y, pntLimYRD.Y, selPntD.Y * chartPtr->GetLocScaleY()))
                    {
                        TUPLE_LABEL tuple_res = chartPtr->GetSelValString(selPntD, m_labelX, m_precision, m_pLabValStrFnPtr);
                        mapLabStr.insert(MAP_LABSTR::value_type(chartIdx, tuple_res));
                    }
                }
                ++itSel;
            }
            else
                itSel = mapSelPntsD.erase(itSel);
        }
    }

    CPoint origPnt(-1, -1); // Not used on empty mapLabStr
    if (!mapLabStr.empty())
    {
        PointF origPntF = m_pMatrixD->TransformToPntF(1.0, m_dataLegPntD);
        origPnt = CPointFromPntF(origPntF);
    }
    // Recalc dataWnd window rects and show data wnd or will hide it
    m_pDataWnd->UpdateDataLegend(mapLabStr, this, origPnt);
    return mapLabStr.size();
}

// Gets axis X pos AX_YPOS (first) and axis X float offsY(second)
PAIR_XAXPOS CChartContainer::GetXAxisPos(RectF rChartF, double minY, double maxY)
{
    AX_YPOS axYPos;
    float vertOffs;

    // Let's begin with X axis
    if ((minY < 0) && (maxY > 0))
    {
        double offsXY = rChartF.Height * fabs(maxY) / (maxY - minY);
        vertOffs = rChartF.GetTop() + float(offsXY);
        axYPos = MIDDLE;        // Y pos of Axis X, somewhere inside the y range
    }
    else if (minY >= 0)
    {
        vertOffs = rChartF.GetBottom();
        axYPos = BOTTOM;        // Y pos of Axis X at bottom of the draw rect
    }
    else if (maxY <= 0)
    {
        vertOffs = rChartF.GetTop();
        axYPos = TOP;           // At the top of the draw rect
    }

    return make_pair(axYPos, vertOffs);
}

// Gets axis Y pos AX_XPOS (first) and axis Y float offsX(second)
PAIR_YAXPOS CChartContainer::GetYAxisPos(RectF rChartF, double startX, double endX)
{
    // Y axis
    AX_XPOS axXPos;
    float horzOffs;

    if ((startX < 0) && (endX > 0))
    {
        double offsYX = rChartF.Width * fabs(startX) / (endX - startX);
        horzOffs = rChartF.GetLeft() + float(offsYX);
        axXPos = BETWEEN;      // Somewhere between minX, maxX
    }
    else if (startX >= 0)
    {
        horzOffs = rChartF.GetLeft();
        axXPos = LEFT;         // At the left of the draw rect
    }
    else if (endX <= 0)
    {
        horzOffs = rChartF.GetRight();
        axXPos = RIGHT;       // At the right side of the draw rect
    }

    return make_pair(axXPos, horzOffs);
}

bool CChartContainer::ShowDataView(CChart* chartPtr, bool bClearMap, bool bRefresh)
{
    if (m_pChartDataView == nullptr)
        m_pChartDataView = new CChartDataView;

    if (m_pChartDataView != nullptr)
    {
        if (!IsWindow(m_pChartDataView->m_hWnd))
        {
            CRect parentWndRect;
            GetParent()->GetWindowRect(&parentWndRect); // App main dlg window

            CRect workRect;
            SystemParametersInfo(SPI_GETWORKAREA, NULL, &workRect, 0);

            int leftX = parentWndRect.right + DV_SPACE;
            int rightX = leftX + DV_RECTW;
            int topY = parentWndRect.top - DV_SPACE;
            int bottomY = topY + DV_RECTH;

            CRect dataViewRect(leftX, topY, rightX, bottomY);
            CRect interRect;
            interRect.IntersectRect(&dataViewRect, workRect);
            if (interRect != dataViewRect)
            {
                dataViewRect.right = workRect.right - DV_SPACE;
                dataViewRect.left = max(dataViewRect.right - DV_RECTW, workRect.left + DV_SPACE);
                dataViewRect.top = workRect.top + DV_SPACE;
                dataViewRect.bottom = min(dataViewRect.top + DV_RECTH, workRect.bottom - DV_SPACE);
            }

            BOOL bRes = m_pChartDataView->CreateEx(
                0, AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS),
                _T("Chart Data View"), WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
                dataViewRect.left, dataViewRect.top, dataViewRect.Width(), dataViewRect.Height(),
                nullptr, nullptr, nullptr);
            if (!bRes)
            {
                delete m_pChartDataView;
                m_pChartDataView = nullptr;
                return false;
            }
        }
        else if (m_pChartDataView->IsIconic())
            m_pChartDataView->ShowWindow(SW_RESTORE);

        m_pChartDataView->SetOwner(this);
        int chartIdx = chartPtr->GetChartIdx();
        m_pChartDataView->ShowWaitMessage(chartIdx, chartPtr->m_vDataPnts.size());
        m_pChartDataView->InitParams(chartPtr, bClearMap, this);

        if (m_dataViewChartIdx != chartIdx)
        {
            m_dataViewChartIdx = chartIdx;
            bClearMap = true;
        }

        if (bClearMap)
        {
            m_mapDataViewPntsD.clear();
            if (bRefresh)
                RefreshWnd();
        }
    }
    return true;
}

bool CChartContainer::ShowDataView(const string_t& szChartName, bool bRefresh)
{
    if (m_mapCharts.empty() && !HasChartWithData(-1, true))
        return false;

    int chartIdx = GetChartIdx(szChartName);
    if (chartIdx == -1)
        return false;

    int selChartIdx = SelectChart(chartIdx, true);      // Return -1 if the chart is not visible or without data
    if (selChartIdx == -1)
        return false;

    CChart* chartPtr = GetChart(chartIdx);
    return ShowDataView(chartPtr, bRefresh);
}

bool CChartContainer::UpdateDataView(int chartIdx, DATAVIEW_FLAGS flagsData)
{
    if ((m_pChartDataView != nullptr) && (IsWindow(m_pChartDataView->m_hWnd)))
    {
        if ((chartIdx != -1) && (chartIdx == m_dataViewChartIdx))
        {
            CChart* chartPtr = GetChart(chartIdx);
            if ((chartPtr != nullptr) && chartPtr->IsChartVisible())
                return m_pChartDataView->UpdateParams(chartPtr, flagsData);
        }
    }
    return false;
}

size_t CChartContainer::UpdateDataViewPntsDMap(void)
{
    if (m_pChartDataView != nullptr)
        m_mapDataViewPntsD = m_pChartDataView->m_mapSelCells;
    else
        m_mapDataViewPntsD.clear();

    return m_mapDataViewPntsD.size();
}

// This returns chartPtr->m_loScY as a first, and deltaLocScY as a second; delta = 1% of Y range
PAIR_DBLS CChartContainer::GetLocScYDelta(CChart* chartPtr)
{
    if (IsWindow(m_hWnd))
    {
        CRect clR;
        GetClientRect(&clR);
        double deltaY = 0.01 * clR.Height() / m_scY;
        double locScY = chartPtr->GetLocScaleY();
        double maxY = max(fabs(chartPtr->GetMaxValY()), fabs(chartPtr->GetMinValY()));
        double deltaK = deltaY / maxY;
        return make_pair(locScY, deltaK);
    }
    return make_pair(1.0, 0.0);
}

void CChartContainer::MoveChartsByArrowKeysX(UINT nChar)
{
    m_vHistX.push_back(make_pair(m_startX, m_endX));
    double deltaX = 0.0;
    if (nChar == VK_RIGHT)
        deltaX = MOVE_DELTAX * (m_endX - m_startX);
    else if (nChar == VK_LEFT)
        deltaX = -MOVE_DELTAX * (m_endX - m_startX);
    else
        return;

    MoveContainerChartsX(deltaX, true);
}

void CChartContainer::ChangeLocScaleYByArrowKeys(UINT nChar)
{
    if (m_chartSelIdx > -1)
    {
        MAP_CHARTS::iterator it = m_mapCharts.find(m_chartSelIdx);
        if (it != m_mapCharts.end())
        {
            CChart* chartPtr = it->second;
            if (chartPtr->IsChartSelected() && chartPtr->IsChartVisible())
            {
                std::pair<double, double> pair_scY = GetLocScYDelta(chartPtr);
                double locScY = pair_scY.first;
                double deltaK = pair_scY.second;

                switch (nChar)
                {
                case VK_DOWN: locScY = max(locScY - deltaK, DELTA_LOCY);      break;
                case VK_UP:   locScY += deltaK;                               break;
                case VK_PRIOR: locScY += 10.0 * deltaK;                         break;
                case VK_NEXT:locScY = max(locScY - 10.0 * deltaK, DELTA_LOCY);  break;
                }

                chartPtr->SetLocScaleY(locScY);

                if (IsLabWndExist(false))
                {
                    if (m_pDataWnd->IsWindowVisible())
                        m_pDataWnd->ShowWindow(SW_HIDE);
                    SetTimer(1, 500, nullptr); // To fight flicker
                }
                RefreshWnd();
                SendNotification(CODE_SCY, chartPtr->GetChartIdx());
            }
        }
    }
}

void CChartContainer::MoveChartsByArrowKeysY(UINT nChar)
{
    // Calculate shift
    double shiftValY = 0.0;
    double deltaY = MOVE_DELTAY * (m_maxExtY - m_minExtY);
    switch (nChar)
    {
    case VK_DOWN:  shiftValY = -deltaY;      break;
    case VK_UP:    shiftValY = deltaY;      break;
    case VK_PRIOR: shiftValY = 10.0 * deltaY;  break;
    case VK_NEXT:  shiftValY = -10.0 * deltaY; break;
    }

    MoveContainerChartsY(shiftValY, true);
    SendNotification(CODE_EXTY);
}

// Draw functions
PointF CChartContainer::DrawAxes(RectF rClientF, double startX, double endX, double minY,
    double maxY, Color colAxis, Graphics* grPtr, float dpiRatio)
{
    Pen axisPen(colAxis, AXIS_PENWIDTH * dpiRatio);

    PointF pntStartF, pntEndF, pntCoordZeroF;
    // Let's begin with X axis

    PAIR_XAXPOS pair_axXYPos = GetXAxisPos(rClientF, minY, maxY);
    m_axYPos = pair_axXYPos.first;
    pntStartF.Y = pntEndF.Y = pair_axXYPos.second;
    pntStartF.X = rClientF.GetLeft() - DR_SPACE * dpiRatio;   // Don't worry: draw area is clipped
    pntEndF.X = rClientF.GetRight() + DR_SPACE * dpiRatio;
    grPtr->DrawLine(&axisPen, pntStartF, pntEndF);

    pntCoordZeroF.Y = pntStartF.Y;

    // Y axis
    PAIR_YAXPOS pair_axYXPos = GetYAxisPos(rClientF, startX, endX);
    m_axXPos = pair_axYXPos.first;
    pntStartF.X = pntEndF.X = pair_axYXPos.second;
    pntStartF.Y = rClientF.GetBottom() + DR_SPACE * dpiRatio;
    pntEndF.Y = rClientF.GetTop() - DR_SPACE * dpiRatio;
    grPtr->DrawLine(&axisPen, pntStartF, pntEndF);

    pntCoordZeroF.X = pntStartF.X;

    m_axOffsXF = pntCoordZeroF.Y;

    return pntCoordZeroF;
}

void CChartContainer::DrawBkground(RectF rClientF, Color colBkground, Color colBorder,
    Graphics* grPtr)
{
    SolidBrush bkBrush(colBkground);
    grPtr->FillRectangle(&bkBrush, rClientF);

    Pen borderPen(colBorder);
    grPtr->DrawRectangle(&borderPen, rClientF);

    if (this == GetFocus())
    {
        rClientF.Inflate(-3.0f, -3.0f);
        borderPen.SetDashStyle(DashStyleDash);
        grPtr->DrawRectangle(&borderPen, rClientF);
    }
}

bool CChartContainer::DrawSelPntsD(CChart* chartPtr, const MAP_SELPNTSD& mapSelPntsD,
    GraphicsPath* grPathPtr, Graphics* grPtr, RectF drawRF, float dpiRatio)
{
    if (!in_range(m_startX, m_endX, m_dataLegPntD.X))
        return false;
    if (mapSelPntsD.empty() || (chartPtr == nullptr))
        return false;

    drawRF.Inflate(DR_SPACE * dpiRatio, DR_SPACE * dpiRatio);

    // Get screen pntsF for the given chart
    V_CHARTDATAD vDataPntsD(mapSelPntsD.size());
    V_CHARTDATAF vDataPntsF;

    int chartIdx = chartPtr->GetChartIdx();
    // Find first entry
    MAP_SELPNTSD::const_iterator itSel = mapSelPntsD.find(chartIdx);
    if (itSel != m_mapSelPntsD.cend())                              // The chart has selected points
    {
        size_t count = m_mapSelPntsD.count(chartIdx);
        for (size_t i = 0; i < count; ++i)
        {
            PointD selPntD = itSel->second;
            if (in_range(m_startX, m_endX, selPntD.X))             // Sometimes it will draw out of clip area
            {
                PointF selPntF = m_pMatrixD->TransformToPntF(chartPtr->GetLocScaleY(), selPntD);
                if (drawRF.Contains(selPntF))
                    vDataPntsF.push_back(selPntF);
            }
            ++itSel;
        }
        // Draw
        if (!vDataPntsF.empty())
        {
            V_CHARTDATAF::iterator it = vDataPntsF.begin(), itE = vDataPntsF.end();

            for (; it != itE; ++it)
            {
                PointF selPntF = *it;
                RectF rPntF = RectFFromCenterF(selPntF, dpiRatio * CHART_DTPNTSZ, dpiRatio * CHART_DTPNTSZ);
                rPntF.Inflate(dpiRatio * 1.5f, dpiRatio * 1.5f);
                grPathPtr->AddEllipse(rPntF);
            }

            Pen selPen(m_colLegPnts, dpiRatio * 1.0f);
            grPtr->DrawPath(&selPen, grPathPtr);

            grPathPtr->Reset();
            return true;
        }   // For vDataPntsF
    }
    return false;
}

bool CChartContainer::DrawDataViewPntsD(const MAP_SELPNTSD& mapDataViewPntsD, double startX,
    double endX, GraphicsPath* grPathPtr, Graphics* grPtr, float dpiRatio)
{
    if (mapDataViewPntsD.empty())
        return false;
    if (nullptr == GetVisibleChart(m_dataViewChartIdx))
        return false;

    CChart* chartPtr = GetChart(m_dataViewChartIdx);
    if ((chartPtr == nullptr) || !chartPtr->IsChartVisible())
        return false;

    // Extract dataPoints from the map into a vector
    V_CHARTDATAD vDataPntsD(mapDataViewPntsD.size());
    transform(mapDataViewPntsD.begin(), mapDataViewPntsD.end(), vDataPntsD.begin(),
        get_map_value<int, PointD>());
    // Exclude elements out of the range and convert to screen pntsF
    V_CHARTDATAF vDataPntsF;
    if (!chartPtr->ConvertChartData(vDataPntsD, vDataPntsF, m_pMatrixD, startX, endX))
        return false;

    vDataPntsF.shrink_to_fit();

    // Finally, draw
    V_CHARTDATAF::iterator it = vDataPntsF.begin();
    V_CHARTDATAF::iterator itE = vDataPntsF.end();
    for (; it != itE; ++it)
    {
        RectF rPntF = RectFFromCenterF(*it, dpiRatio * (CHART_DTPNTSZ + 2.0f),
            dpiRatio * (CHART_DTPNTSZ + 2.0f));
        grPathPtr->AddEllipse(rPntF);
    }

    Pen selPen(m_colDataViewPnts, dpiRatio * 2.0f);
    grPtr->DrawPath(&selPen, grPathPtr);
    grPathPtr->Reset();

    return true;
}

void CChartContainer::DrawSelLine(RectF clRF, Graphics* grPtr, int selChartIdx, float dpiRatio)
{
    if (m_mapSelPntsD.empty())
        return;
    if (nullptr == GetVisibleChart(-1))
        return;

    bool bShowLine = false;
    if (IsLabWndVisible(false))
        bShowLine = true;
    else if (dpiRatio != 1.0f)   // The second check: on a printout there is no dataWnd
    {
        clRF.Inflate(DR_SPACE * dpiRatio, DR_SPACE * dpiRatio);
        PointF upperLeftF(clRF.X, clRF.Y);
        PointF bottomRightF(clRF.GetRight(), clRF.GetBottom());
        PointD upperLeftD = m_pMatrixD->TransformToPntD(1.0, upperLeftF);
        PointD bottomRightD = m_pMatrixD->TransformToPntD(1.0, bottomRightF);

        auto itSel = selChartIdx > -1 ? m_mapSelPntsD.find(selChartIdx) : m_mapSelPntsD.cbegin();
        auto itSelE = m_mapSelPntsD.cend();
        for (; itSel != itSelE; ++itSel)
        {
            if ((selChartIdx > -1) && (itSel->first != selChartIdx))
                break;
            PointD pntD = itSel->second;
            CChart* chartPtr = GetChart(itSel->first);
            double locScY = chartPtr->GetLocScaleY();
            PointF pntF = m_pMatrixD->TransformToPntF(locScY, pntD);
            if (clRF.Contains(pntF))
            {
                bShowLine = true;
                break;
            }
        }
    }

    if (bShowLine)
    {
        // Get client coord of the data line; scY does notmatter
        PointF pntF = m_pMatrixD->TransformToPntF(1.0, m_dataLegPntD);
        // Again, no problem with printing out of a clipping rectangle
        Pen selPen(m_colLegPnts, dpiRatio * 1.0f);
        grPtr->DrawLine(&selPen, pntF.X, clRF.Y, pntF.X, clRF.GetBottom());
    }
}

void CChartContainer::DrawAxXBoundaryStr(RectF boundRF, Graphics* grPtr, float dpiRatio)
{
    string_t strStartX, strEndX;

    bool bAddEqSign = m_labelX.empty() ? false : true;

    string_t strSpace(_T(" "));
    strStartX = m_labelX + strSpace + m_pLabValStrFnPtr(m_startX, m_precision, bAddEqSign);
    strEndX = m_labelX + strSpace + m_pLabValStrFnPtr(m_endX, m_precision, bAddEqSign);

#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font axFont(&fontFamily, 7.0f, FontStyleBold);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    // Measure strings
    boundRF.Inflate(-dpiRatio * 2.0f, -dpiRatio * 2.0f);
    RectF startRF, endRF;
    PointF startPntF, endPntF;

    MeasureString(grPtr, strStartX, -1, &axFont, boundRF, &startRF);
    MeasureString(grPtr, strEndX, -1, &axFont, boundRF, &endRF);

    if ((m_axYPos == MIDDLE) || (m_axYPos == TOP))
    {
        startPntF.Y = m_axOffsXF + dpiRatio * DR_SPACE;
        endPntF.Y = m_axOffsXF + dpiRatio * DR_SPACE;
        if ((m_axYPos == MIDDLE) && ((endPntF.Y + startRF.Width) > boundRF.GetBottom()))
        {
            startPntF.Y = m_axOffsXF - startRF.Height - dpiRatio * (3.0f + DR_SPACE);
            endPntF.Y = m_axOffsXF - endRF.Height - dpiRatio * (3.0f + DR_SPACE);
        }
    }
    else if (m_axYPos == BOTTOM)
    {
        startPntF.Y = m_axOffsXF - startRF.Height - dpiRatio * (3.0f + DR_SPACE);
        endPntF.Y = m_axOffsXF - endRF.Height - dpiRatio * (3.0f + DR_SPACE);
    }

    if (m_axXPos == BETWEEN)
    {
        startPntF.X = startRF.X + dpiRatio * DR_SPACE;
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * DR_SPACE;
    }
    else if (m_axXPos == LEFT)
    {
        startPntF.X = startRF.X + dpiRatio * (3.0f + DR_SPACE);
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * DR_SPACE;
    }
    else if (m_axXPos == RIGHT)
    {
        startPntF.X = startRF.X + dpiRatio * DR_SPACE;
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * (3.0f + DR_SPACE);
    }
    else
        return;

    startRF.X = startPntF.X;
    startRF.Y = startPntF.Y;
    endRF.X = endPntF.X;
    endRF.Y = endPntF.Y;

    SolidBrush axBkBrush(m_colBkground);
    SolidBrush axLabBr(m_colAxis);

    grPtr->FillRectangle(&axBkBrush, startRF);
    DrawString(grPtr, strStartX, -1, &axFont, startPntF, &axLabBr);

    grPtr->FillRectangle(&axBkBrush, endRF);
    DrawString(grPtr, strEndX, -1, &axFont, endPntF, &axLabBr);
}

// Used to draw Zoom Border out of OnPaint function
void CChartContainer::DrawZoomBorderX(CPoint pnt)
{
    CDC* pDC = GetDC();
    GraphicsPath grPath;

    CRect clR;
    GetClientRect(&clR);

    CPoint tmpPnt;
    CRect nameR;
    if (IsLabWndVisible(true))
    {
        m_pLegWnd->GetWindowRect(&nameR);
        ScreenToClient(&nameR);
        tmpPnt = CPoint(pnt.x, nameR.CenterPoint().y);
        if (!nameR.PtInRect(tmpPnt))
            nameR.SetRectEmpty();
    }

    CRect dataR;
    if (IsLabWndVisible(false))
    {
        m_pDataWnd->GetWindowRect(&dataR);
        ScreenToClient(&dataR);
        tmpPnt = CPoint(pnt.x, dataR.CenterPoint().y);
        if (!dataR.PtInRect(tmpPnt))
            dataR.SetRectEmpty();
    }

    Point startPnt(pnt.x, clR.top);
    Point endX;
    if (!nameR.IsRectEmpty())
    {
        grPath.AddLine(startPnt, Point(pnt.x, nameR.top));
        grPath.StartFigure();
        startPnt.Y = nameR.bottom;
        if (!dataR.IsRectEmpty())
        {
            grPath.AddLine(startPnt, Point(pnt.x, dataR.top));
            grPath.StartFigure();
            startPnt.Y = dataR.bottom;
        }
        grPath.AddLine(startPnt, Point(pnt.x, clR.bottom));
    }
    else if (!dataR.IsRectEmpty())
    {
        grPath.AddLine(startPnt, Point(pnt.x, dataR.top));
        grPath.StartFigure();
        startPnt.Y = dataR.bottom;
    }
    grPath.AddLine(startPnt, Point(pnt.x, clR.bottom));

    Graphics gr(pDC->m_hDC);
    Pen brdPen(m_colZoomBorder);
    gr.DrawPath(&brdPen, &grPath);
    //  gr.DrawLine(&brdPen, pnt.x, clR.top, pnt.x, dataR.top);//clR.bottom);
    ReleaseDC(pDC);
}

void CChartContainer::DrawZoomBorderY(CPoint pnt)
{
    CDC* pDC = GetDC();
    GraphicsPath grPath;

    CRect clR;
    GetClientRect(&clR);

    CPoint tmpPnt;

    CRect dataR;
    if (IsLabWndVisible(false))
    {
        m_pDataWnd->GetWindowRect(&dataR);
        ScreenToClient(&dataR);
        tmpPnt = CPoint(dataR.CenterPoint().x, pnt.y);
        if (!dataR.PtInRect(tmpPnt))
            dataR.SetRectEmpty();
    }

    CRect nameR;
    if (IsLabWndVisible(true))
    {
        m_pLegWnd->GetWindowRect(&nameR);
        ScreenToClient(&nameR);
        tmpPnt = CPoint(nameR.CenterPoint().x, pnt.y);
        if (!nameR.PtInRect(tmpPnt))
            nameR.SetRectEmpty();
    }

    Point startPnt(clR.left, pnt.y);
    Point endY;

    if (!dataR.IsRectEmpty())
    {
        grPath.AddLine(startPnt, Point(dataR.left, pnt.y));
        grPath.StartFigure();
        startPnt.X = dataR.right;
    }

    if (!nameR.IsRectEmpty())
    {
        grPath.AddLine(startPnt, Point(nameR.left, pnt.y));
        grPath.StartFigure();
        startPnt.X = nameR.right;
        if (!dataR.IsRectEmpty())
        {
            grPath.AddLine(startPnt, Point(dataR.right, pnt.y));
            grPath.StartFigure();
            startPnt.X = dataR.right;
        }
    }

    grPath.AddLine(startPnt, Point(clR.right, pnt.y));

    Graphics gr(pDC->m_hDC);
    Pen brdPen(m_colZoomBorder);
    gr.DrawPath(&brdPen, &grPath);
    ReleaseDC(pDC);
}

void  CChartContainer::DrawGrid(RectF boundRF, PointF coordOrigF, Graphics* grPtr, float dpiRatio)
{
    float fDeltaX = boundRF.Width / 10.0f;
    float fDeltaY = boundRF.Height / 10.0f;

    float limTop = boundRF.GetTop();
    float limBottom = boundRF.GetBottom();
    float limLeft = boundRF.GetLeft();
    float limRight = boundRF.GetRight();

    Pen gridPen(m_colGrid, 1.0f * dpiRatio);
    gridPen.SetDashStyle(DashStyleDash);
    gridPen.SetDashCap(DashCapRound);

    float xF, yF;

    yF = coordOrigF.Y + fDeltaY;
    while (yF < limBottom)
    {
        grPtr->DrawLine(&gridPen, limLeft, yF, limRight, yF);
        yF += fDeltaY;
    }

    yF = coordOrigF.Y - fDeltaY;
    while (yF > limTop)
    {
        grPtr->DrawLine(&gridPen, limLeft, yF, limRight, yF);
        yF -= fDeltaY;
    }

    xF = coordOrigF.X + fDeltaX;
    while (xF < limRight)
    {
        grPtr->DrawLine(&gridPen, xF, limTop, xF, limBottom);
        xF += fDeltaX;
    }

    xF = coordOrigF.X - fDeltaX;
    while (xF > limLeft)
    {
        grPtr->DrawLine(&gridPen, xF, limTop, xF, limBottom);
        xF -= fDeltaX;
    }
}


// SGS
void  CChartContainer::DrawGridWithCoordinates(RectF boundRF, PointF coordOrigF, Graphics* grPtr, float dpiRatio)
{
    // If empty return
    if (m_mapCharts.empty())
    {
        return;
    }

    auto [startX, endX, startY, endY] = GetMinMaxes();
    auto spreadX = endX - startX;
    auto spreadY = endY - startY;

    string_t strStartX, strEndX;
    string_t strSpace(_T(" "));
    strStartX = m_pLabValStrFnPtr(startX, m_precision, false);
    strEndX = m_pLabValStrFnPtr(endX, m_precision, false);

#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font axFont(&fontFamily, 7.0f, FontStyleBold);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    // Measure strings
    RectF startRF, endRF;
    PointF startPntF, endPntF;

    MeasureString(grPtr, strStartX, -1, &axFont, boundRF, &startRF);
    MeasureString(grPtr, strEndX, -1, &axFont, boundRF, &endRF);

    if ((m_axYPos == MIDDLE) || (m_axYPos == TOP))
    {
        startPntF.Y = m_axOffsXF + dpiRatio * DR_SPACE;
        endPntF.Y = m_axOffsXF + dpiRatio * DR_SPACE;
        if ((m_axYPos == MIDDLE) && ((endPntF.Y + startRF.Width) > boundRF.GetBottom()))
        {
            startPntF.Y = m_axOffsXF - startRF.Height - dpiRatio * (3.0f + DR_SPACE);
            endPntF.Y = m_axOffsXF - endRF.Height - dpiRatio * (3.0f + DR_SPACE);
        }
    }
    else if (m_axYPos == BOTTOM)
    {
        startPntF.Y = m_axOffsXF - startRF.Height - dpiRatio * (3.0f + DR_SPACE);
        endPntF.Y = m_axOffsXF - endRF.Height - dpiRatio * (3.0f + DR_SPACE);
    }

    if (m_axXPos == BETWEEN)
    {
        startPntF.X = startRF.X + dpiRatio * DR_SPACE;
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * DR_SPACE;
    }
    else if (m_axXPos == LEFT)
    {
        startPntF.X = startRF.X + dpiRatio * (3.0f + DR_SPACE);
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * DR_SPACE;
    }
    else if (m_axXPos == RIGHT)
    {
        startPntF.X = startRF.X + dpiRatio * DR_SPACE;
        endPntF.X = boundRF.GetRight() - endRF.Width - dpiRatio * (3.0f + DR_SPACE);
    }
    else
        return;

    startRF.X = startPntF.X;
    startRF.Y = startPntF.Y;
    endRF.X = endPntF.X;
    endRF.Y = endPntF.Y;

    SolidBrush axBkBrush(m_colBkground);
    SolidBrush axLabBr(m_colAxis);

    // DrawGrid
    float fDeltaX = boundRF.Width / 10.0f;  // Assume Linear spacing unless otherwise changed.
    float fDeltaY = boundRF.Height / 10.0f;

    if (m_x_axis_is_logarithmic)
    {
        // Logarithmic base 10 spacing
        fDeltaX = static_cast<float>(boundRF.Width / spreadX);
    }

    if (m_y_axis_is_logarithmic)
    {
        // Logarithmic base 10 spacing
        fDeltaY = static_cast<float>(boundRF.Height / spreadY);
    }

    float limTop = boundRF.GetTop();
    float limBottom = boundRF.GetBottom();
    float limLeft = boundRF.GetLeft();
    float limRight = boundRF.GetRight();

    Pen gridPen(m_colGrid, 1.0f * dpiRatio);
    gridPen.SetDashStyle(DashStyleDash);
    gridPen.SetDashCap(DashCapRound);

    // Get the color of the labels for each Y axis
    auto iter = m_mapCharts.begin();
    auto second_chart = (*iter).second;
    SolidBrush axY1LabBr((*iter).second->GetChartColor());
    Color temp((*m_mapCharts.begin()).second->GetChartColor());
    if (m_mapCharts.size() == 2)
    {
        ++iter;
        temp = (*iter).second->GetChartColor();
        second_chart = (*iter).second;
    }

    SolidBrush axY2LabBr(temp);

    /////////////////////// Y-AXIS //////////////////////////
    //
    // Y-Axis Labels and verticle grid lines
    // Negative Side of X-Axis
    // Work from Top (Least Negitive) to Bottom (Most Negative)
    //

    // Linear Spacing
    auto delta_coord = spreadY / 10.0f;
    if (m_y_axis_is_logarithmic)
    {
        // Logarithmic spacing is powers of 10 so the delta coordinate is 1.
        delta_coord = 1;
    }
    if (0.0f > delta_coord)
    {
        delta_coord *= -1.0f;
    }

    float xF, yF;
    auto curr_coord(endY - delta_coord);
    auto almost_zero(abs(delta_coord / pow(10.0f, m_precision)));
    RectF measure_width_height;
    RectF reuse_X_value = startRF;
    auto chart_1_fcn_ptr((*m_mapCharts.begin()).second->GetLabYValStrFnPtr());

    yF = coordOrigF.Y + fDeltaY;
    while (yF < limBottom)
    {
        grPtr->DrawLine(&gridPen, limLeft, yF, limRight, yF);
        auto label_coord = curr_coord;
        if (m_y_axis_is_logarithmic)
        {
            label_coord = pow(10.0f, curr_coord);
        }
        string_t coord = chart_1_fcn_ptr(label_coord, m_precision, false);
        if (yF < (limBottom - fDeltaX / 4))
        {
            // Center the label on the grid line.
            reuse_X_value.Y = static_cast<Gdiplus::REAL>(yF - startRF.Height / 2.0f);
        }
        else
        {
            // Do not center the label on the line next to the edge.
            // Right Justify
            reuse_X_value.Y = static_cast<Gdiplus::REAL>(yF - startRF.Height);
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_X_value.Width = measure_width_height.Width;
        reuse_X_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_X_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_X_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(limLeft + 5, reuse_X_value.Y), &axY1LabBr);
        if (m_show_y_axis_second_scale)
        {
            auto chart2_fcn_ptr(second_chart->GetLabYValStrFnPtr());
            coord = chart2_fcn_ptr(curr_coord, m_precision, false);
            MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
            reuse_X_value.Width = measure_width_height.Width;
            reuse_X_value.Height = measure_width_height.Height;
            RectF copy = reuse_X_value;
            copy.X = limRight - reuse_X_value.Width - 8;
            BoundsCheckRectangle(copy, startX, endX, startY, endY);
            grPtr->FillRectangle(&axBkBrush, copy);
            DrawString(grPtr, coord, -1, &axFont, PointF(copy.X, copy.Y), &axY2LabBr);
        }
        curr_coord -= delta_coord;
        yF += fDeltaY;
    }

    //
    // Y-Axis Labels and verticle grid lines
    // Positive Side of X-Axis
    // Work from Bottom (Least Positive) to Top (Most Positive)
    //

    curr_coord = startY + delta_coord;

    yF = coordOrigF.Y - fDeltaY;
    while (yF > limTop)
    {
        grPtr->DrawLine(&gridPen, limLeft, yF, limRight, yF);
        auto label_coord = curr_coord;
        if (m_y_axis_is_logarithmic)
        {
            label_coord = pow(10.0f, curr_coord);
        }
        string_t coord = chart_1_fcn_ptr(label_coord, m_precision, false);
        if (yF < (limTop + fDeltaX / 4))
        {
            // Center the label on the grid line.
            reuse_X_value.Y = yF;
        }
        else
        {
            // Do not center the label on the line next to the edge.
            // Bottom Justify
            reuse_X_value.Y = static_cast<Gdiplus::REAL>(yF - startRF.Height / 2.0);
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_X_value.Width = measure_width_height.Width;
        reuse_X_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_X_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_X_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(limLeft + 5, reuse_X_value.Y), &axY1LabBr);
        if (m_show_y_axis_second_scale)
        {
            auto chart2_fcn_ptr(second_chart->GetLabYValStrFnPtr());
            coord = chart2_fcn_ptr(curr_coord, m_precision, false);
            MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
            reuse_X_value.Width = measure_width_height.Width;
            reuse_X_value.Height = measure_width_height.Height;
            RectF copy = reuse_X_value;
            copy.X = limRight - reuse_X_value.Width - 8;
            BoundsCheckRectangle(copy, startX, endX, startY, endY);
            grPtr->FillRectangle(&axBkBrush, copy);
            DrawString(grPtr, coord, -1, &axFont, PointF(copy.X, copy.Y), &axY2LabBr);
        }
        curr_coord += delta_coord;
        yF -= fDeltaY;
    }


    /////////////////////// X-AXIS //////////////////////////
    //
    //
    // Print the origin label for X
    //

    // Linear Spacing
    // Print the 0 on the axis
    string_t coord = m_pLabValStrFnPtr(0.0f, m_precision, false);

    delta_coord = (spreadX) / 10.0f;
    if (m_x_axis_is_logarithmic)
    {
        // Logarithmic spacing is powers of 10 so the delta coordinate is 1.
        delta_coord = 1;
        coord = m_pLabValStrFnPtr(1.0f, m_precision, false);
    }

    if (0.0f > delta_coord)
    {
        delta_coord *= -1.0f;
    }
    almost_zero = abs(delta_coord / pow(10.0f, m_precision));
    RectF reuse_Y_value = startRF;

    if (endX >= 0.0f && startX <= 0.0f)
    {
        if ((coordOrigF.X < (limRight + limLeft)) && (coordOrigF.X > limRight))
        {
            // Do not center the label on the line next to the edge.
            // Right Justify
            if (almost_zero > endX && -1.0 * almost_zero < endX)
            {
            }
            else
            {
                coord = m_pLabValStrFnPtr(endX, m_precision, false);
            }
            reuse_Y_value.X = limRight - StringLength(coord) * 6;
        }
        else if (coordOrigF.X > limLeft)
        {
            // Center the label on the axis line.
            reuse_Y_value.X = coordOrigF.X - StringLength(coord) * 3;
        }
        else if ((coordOrigF.X > 0) && (coordOrigF.X <= limLeft))
        {
            // Do not center the label on the line next to the edge.
            // Left Justify
            if (almost_zero > startX && -1.0 * almost_zero < startX)
            {
            }
            else
            {
                coord = m_pLabValStrFnPtr(startX, m_precision, false);
            }
            reuse_Y_value.X = limLeft;
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_Y_value.Width = measure_width_height.Width;
        reuse_Y_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_Y_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_Y_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(reuse_Y_value.X, startPntF.Y), &axLabBr);
    }
    else if (RIGHT == m_axXPos || LEFT == m_axXPos)
    {
        // Need to account for the right most point when the axis is on the right and the left most point when the axis is on the left.
        if (RIGHT == m_axXPos)
        {
            // Account for right most point because the axis is on the right.
            auto value = endX;
            if (m_x_axis_is_logarithmic)
            {
                value = pow(10.0f, value);
            }
            coord = m_pLabValStrFnPtr(value, m_precision, false);
            reuse_Y_value.X = limRight - StringLength(coord) * 6;
        }
        else if (LEFT == m_axXPos)
        {
            // Account for left most point because the axis is on the left.
            auto value = startX;
            if (m_x_axis_is_logarithmic)
            {
                value = pow(10.0f, value);
            }
            coord = m_pLabValStrFnPtr(value, m_precision, false);
            reuse_Y_value.X = limLeft + StringLength(coord) * 3;
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_Y_value.Width = measure_width_height.Width;
        reuse_Y_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_Y_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_Y_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(reuse_Y_value.X, startPntF.Y), &axLabBr);
    }

    //
    // X-Axis Labels and verticle grid lines
    // Positive Side of Y-Axis
    // Work from Left to Right
    //

    // Linear Spacing
    delta_coord = (spreadX) / 10.0;

    if (m_x_axis_is_logarithmic)
    {
        // Logarithmic spacing is powers of 10 so the delta coordinate is 1.
        delta_coord = 1;
    }

    if (0.0f > delta_coord)
    {
        delta_coord *= -1.0f;
    }
    curr_coord = (0.0f > startX) ? delta_coord : startX + delta_coord;
    xF = coordOrigF.X + fDeltaX;
    while (xF < limRight)
    {
        grPtr->DrawLine(&gridPen, xF, limTop, xF, limBottom);
        auto label_coord = curr_coord;
        if (m_x_axis_is_logarithmic)
        {
            label_coord = pow(10.0f, curr_coord);
        }
        coord = m_pLabValStrFnPtr(label_coord, m_precision, false);
        if (xF < (limRight - fDeltaX / 4))
        {
            // Center the label on the grid line.
            reuse_Y_value.X = xF - StringLength(coord) * 3;
        }
        else
        {
            // Do not center the label on the line next to the edge.
            // Right Justify
            reuse_Y_value.X = xF - StringLength(coord) * 6;
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_Y_value.Width = measure_width_height.Width;
        reuse_Y_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_Y_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_Y_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(reuse_Y_value.X, startPntF.Y), &axLabBr);
        xF += fDeltaX;
        curr_coord += delta_coord;
    }

    //
    // X-Axis Labels and verticle grid lines
    // Negative Side of Y-Axis
    // Work from Right to Left
    //

    curr_coord = endX - delta_coord;

    if (BETWEEN == m_axXPos)
    {
        // The distance between the points is correct but now we need to deterXmine the space between the edge and the rightmost line
        // to offset the coordinates.
        curr_coord = -1.0f * delta_coord;
        if (0.0f < curr_coord)
        {
            curr_coord *= -1.0f;
        }
    }

    xF = coordOrigF.X - fDeltaX;
    while (xF > limLeft)
    {
        grPtr->DrawLine(&gridPen, xF, limTop, xF, limBottom);
        auto label_coord = curr_coord;
        if (m_x_axis_is_logarithmic)
        {
            label_coord = pow(10.0f, curr_coord);
        }
        coord = m_pLabValStrFnPtr(label_coord, m_precision, false);
        if (xF > (limLeft + fDeltaX / 4))
        {
            // Center the label on the grid line.
            reuse_Y_value.X = xF - StringLength(coord) * 3;
        }
        else
        {
            // Do not center the label on the line next to the edge.
            reuse_Y_value.X = xF;
        }

        MeasureString(grPtr, coord, -1, &axFont, boundRF, &measure_width_height);
        reuse_Y_value.Width = measure_width_height.Width;
        reuse_Y_value.Height = measure_width_height.Height;
        BoundsCheckRectangle(reuse_Y_value, startX, endX, startY, endY);
        grPtr->FillRectangle(&axBkBrush, reuse_Y_value);
        DrawString(grPtr, coord, -1, &axFont, PointF(reuse_Y_value.X, startPntF.Y), &axLabBr);
        xF -= fDeltaX;
        curr_coord -= delta_coord;
    }
}

void CChartContainer::DrawContainerToBmp(Rect rGdi, Bitmap& clBmp)
{
    Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC
    grPtr->SetSmoothingMode(SmoothingModeAntiAlias);

    RectF rGdiF = GdiRectToRectF(rGdi);
    rGdiF.Width -= 1.0f;
    rGdiF.Height -= 1.0f;
    DrawBkground(rGdiF, m_colBkground, m_colBorder, grPtr);

    if (m_mapCharts.size() > 0)
    {
        RestoreModeX();  // Might be in the middle of settung zoon borders; annuls the mode
        RectF drawRF = rGdiF;
        drawRF.Inflate(-DR_SPACE, -DR_SPACE);

        if (!HasChartWithData(-1))
            DrawGrid(drawRF, PointF(0.0f, 0.0f), grPtr);
        else
        {
            // Draw axes
            PointF pntOrigF = DrawAxes(drawRF, m_startX, m_endX, m_minExtY, m_maxExtY, m_colAxis, grPtr);
            // Draw a grid
            // SGS
            if (m_use_coordinates)
            {
                DrawGridWithCoordinates(drawRF, pntOrigF, grPtr);
            }
            else
            {
                DrawGrid(drawRF, pntOrigF, grPtr);
            }

            // Update scales
            std::pair<double, double> pair_scales =
                UpdateScales(drawRF, m_startX, m_endX, m_minExtY, m_maxExtY);
            m_scX = pair_scales.first;
            m_scY = pair_scales.second;

            double offsetY(pntOrigF.Y);

            // Set transform matrix
            MatrixD matrixD;
            matrixD.Translate(pntOrigF.X, offsetY);
            matrixD.Scale(m_scX, -m_scY);

            // Account for not even axes placement
            if ((m_axXPos != BETWEEN) || (m_axYPos != MIDDLE))
            {
                double translateX = (m_axXPos == RIGHT) ? -m_endX : (m_axXPos == LEFT) ? -m_startX : 0.0;
                double translateY = (m_axYPos == BOTTOM) ? -m_minExtY : (m_axYPos == TOP) ? -m_maxExtY : 0.0;
                matrixD.Translate(translateX, translateY);
            }
            if (m_pMatrixD != nullptr)
                delete m_pMatrixD;
            m_pMatrixD = matrixD.Clone();     // Copy constructor is private ??

      		// Begin draw curves
            GraphicsPath grPath;            // Will be used actoss the drawing routines
            MAP_CHARTS::iterator it = m_mapCharts.begin();
            MAP_CHARTS::iterator itE = m_mapCharts.end();
            for (; it != itE; ++it)
            {
                CChart* chartPtr = it->second;
                if (chartPtr->IsChartVisible() == false)
                    continue;
                if (chartPtr->m_vDataPnts.size() == 0)
                    continue;
                chartPtr->DrawChartCurve(chartPtr->m_vDataPnts, m_startX, m_endX,
                    m_pMatrixD, &grPath, grPtr);
                if (m_bTracking)              // Draw selection points if any
                    DrawSelPntsD(chartPtr, m_mapSelPntsD, &grPath, grPtr, drawRF);
            }

            if (m_bTracking)                // And the sel line too
                DrawSelLine(rGdiF, grPtr);

            if (m_bAxBoundaries)            // Left and right visible X-coordinates
                DrawAxXBoundaryStr(rGdiF, grPtr);

            // Now draw children
            if (m_pLegWnd && IsWindow(m_pLegWnd->m_hWnd) && m_pLegWnd->IsWindowVisible())
            {
                CRect legWndRect = m_pLegWnd->GetDataWndRect();
                m_pLegWnd->DrawDataWnd(legWndRect.left, legWndRect.top, grPtr);
            }

            if (m_pDataWnd && IsWindow(m_pDataWnd->m_hWnd) && m_pDataWnd->IsWindowVisible())
            {
                CRect dataWndRect = m_pDataWnd->GetDataWndRect();
                m_pDataWnd->DrawDataWnd(dataWndRect.left, dataWndRect.top, grPtr);
            }
        }
    }
    delete grPtr;
}

// SGS
void  CChartContainer::DrawAxisLabels(Rect boundXlabel, Rect boundYlabel, HDC hdc)
{
    // 
    // The GDI+ method does not rotate the text even thought the font escapment in set.
    // Must resort to another method.
    //

    //
    // This approach is taken from
    // https://docs.microsoft.com/en-us/windows/win32/gdi/rotating-lines-of-text
    //

    // string_t rotatedText = _T("Long String To Help Test String Centering");

    // Retrieve the client-rectangle dimensions.
    RECT rc = {}; 
    LOGFONT lf = {};
    _tcscpy_s(lf.lfFaceName, 8, TEXT("Verdana"));
    lf.lfWeight = FW_NORMAL;
    lf.lfHeight = 13;

    if (m_showYLabel)
    {
        rc.left = boundXlabel.X;
        rc.top = boundXlabel.Y;
        rc.right = boundXlabel.X + boundXlabel.Width;
        rc.bottom = boundXlabel.Y + boundXlabel.Height;

        lf.lfEscapement = 900;

        auto hfnt = CreateFontIndirect(&lf);

        // Set the background mode to transparent for the
        SetBkMode(hdc, TRANSPARENT);

        auto hfntPrev = SelectObject(hdc, hfnt);

        //if (0 == m_rotation)
        //{
        //    TextOut(hdc, static_cast<int>(rc.right / 2 - m_text.size() * 2.75), rc.bottom / 3,
        //        m_text.c_str(), m_text.size());
        //}
        //else if (900 == m_rotation)
        //{
        TextOut(hdc, rc.left, static_cast<int>(rc.bottom / 2 + m_y_axis_label.size() * 2.75),
            m_y_axis_label.c_str(), m_y_axis_label.size());
        //}
        //else if (1800 == m_rotation)
        //{
        //    TextOut(hdc, static_cast<int>(rc.right / 2 + m_text.size() * 2.75), rc.bottom,
        //        m_text.c_str(), m_text.size());
        //}
        //else if (2700 == m_rotation)
        //{
        //    TextOut(hdc, rc.right / 2, rc.bottom / 2, m_text.c_str(), m_text.size());
        //}
        //else
        //{
        //    // Untested for how it will display visually
        //    TextOut(hdc, rc.right / 2, rc.bottom / 2, m_text.c_str(), m_text.size());
        //}

        SelectObject(hdc, hfntPrev);
        DeleteObject(hfnt);
    }

    if (m_showXLabel)
    {
        lf.lfEscapement = 0;
        auto hfnt = CreateFontIndirect(&lf);
        auto hfntPrev = SelectObject(hdc, hfnt);

        rc.left = boundYlabel.X;
        rc.top = boundYlabel.Y;
        rc.right = boundYlabel.X + boundYlabel.Width;
        rc.bottom = boundYlabel.Y + boundYlabel.Height;

        //if (0 == m_rotation)
        //{
        TextOut(hdc, static_cast<int>(rc.right / 2 - m_x_axis_label.size() * 2.75), rc.top,
            m_x_axis_label.c_str(), m_x_axis_label.size());
        //}
        //else if (900 == m_rotation)
        //{
        //    TextOut(hdc, rc.left, static_cast<int>(rc.bottom / 2 + m_text.size() * 2.75),
        //        m_text.c_str(), m_text.size());
        //}
        //else if (1800 == m_rotation)
        //{
        //    TextOut(hdc, static_cast<int>(rc.right / 2 + m_text.size() * 2.75), rc.bottom,
        //        m_text.c_str(), m_text.size());
        //}
        //else if (2700 == m_rotation)
        //{
        //    TextOut(hdc, rc.right / 2, rc.bottom / 2, m_text.c_str(), m_text.size());
        //}
        //else
        //{
        //    // Untested for how it will display visually
        //    TextOut(hdc, rc.right / 2, rc.bottom / 2, m_text.c_str(), m_text.size());
        //}

        SelectObject(hdc, hfntPrev);
        DeleteObject(hfnt);
    }
    // Reset the background mode to its default.
    SetBkMode(hdc, OPAQUE);
}


// Printing
int CChartContainer::GetScreenDpi(void)
{
    CPaintDC containerDC(this);
    int scrDpiX = containerDC.GetDeviceCaps(LOGPIXELSX);
    int scrDpiY = containerDC.GetDeviceCaps(LOGPIXELSY);
    ENSURE(scrDpiX == scrDpiY);
    return scrDpiX;
}

TUPLE_PRNLEGLAYOUT CChartContainer::CalcNamesPntLayout(RectF namesRF,
    const MAP_PRNDATA& mapPrnData, Graphics* grPtr, float dpiRatio)
{
    // Measure strings
    float maxNamesLength = 0, maxScYLength = 0, maxValStrLength = 0;
    float maxNameXStrLength = 0, maxStrXLength = 0;
    float maxNameYStrLength = 0, maxStrYLength = 0;

    RectF tmpRF;

    // Get max size of the printing cells
#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font labelFont(&fontFamily, 8.0f, FontStyleBold);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    // Measure strings
    MAP_PRNDATA::const_iterator it = mapPrnData.begin();
    MAP_PRNDATA::const_iterator itE = mapPrnData.end();

    // Get max strings and str lengths
    while (it != itE)
    {
        TUPLE_PRINT tuple_chart = it->second;
        // Names
        string_t tmpStr = get<IDX_NAME>(tuple_chart);

        if (!tmpStr.empty())
        {
           MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
           if (tmpRF.Width > maxNamesLength)
                maxNamesLength = tmpRF.Width;
        }
        // Scales
        tmpStr = get<IDX_SCY>(tuple_chart);
        if (!tmpStr.empty())
        {
            MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
            if (tmpRF.Width > maxScYLength)
                maxScYLength = tmpRF.Width;
        }
        // Sel pnts if any
        tmpStr = get<IDX_VAL>(tuple_chart);
        if (!tmpStr.empty())  // If empty, no selected points
        {
            MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
            if (tmpRF.Width > maxValStrLength)
                maxValStrLength = tmpRF.Width;
        }

        // X name
        tmpStr = get<IDX_NAMEX>(tuple_chart);
        if (!tmpStr.empty())
        {
            MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
            if (tmpRF.Width > maxNameXStrLength)
                maxNameXStrLength = tmpRF.Width;
        }

        // X values
        tmpStr = get<IDX_X>(tuple_chart);
        MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
        if (tmpRF.Width > maxStrXLength)
            maxStrXLength = tmpRF.Width;

        // Y name str
        tmpStr = get<IDX_NAMEY>(tuple_chart);
        if (!tmpStr.empty())
        {
            MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
            if (tmpRF.Width > maxNameYStrLength)
                maxNameYStrLength = tmpRF.Width;
        }

        // Y values
        tmpStr = get<IDX_Y>(tuple_chart);
        MeasureString(grPtr, tmpStr, -1, &labelFont, namesRF, &tmpRF);
        if (tmpRF.Width > maxStrYLength)
            maxStrYLength = tmpRF.Width;

        ++it;
    }

    // Column offsets, begin from the end
    float endX = namesRF.GetRight();
    float startX = namesRF.GetLeft();

    float valYOffsX = endX - maxStrYLength;
    float nameYStrOffsX = maxNameYStrLength == 0 ? valYOffsX :
        valYOffsX - maxNameYStrLength - dpiRatio * LB_BORDERSPACE;
    float valXOffsX = nameYStrOffsX - maxStrXLength - dpiRatio * LB_BORDERSPACE;
    float nameXStrOffsX = maxNameXStrLength == 0 ? valXOffsX :
        valXOffsX - maxNameXStrLength - -dpiRatio * LB_BORDERSPACE;
    float valStrOffsX = maxValStrLength == 0 ? nameXStrOffsX :
        nameXStrOffsX - maxValStrLength - -dpiRatio * LB_BORDERSPACE;
    float scYOffsX = maxScYLength == 0 ? valStrOffsX :
        valStrOffsX - maxScYLength - dpiRatio * LB_BORDERSPACE;
    float nameStrOffsX = maxNamesLength == 0 ? scYOffsX :
        scYOffsX - maxNamesLength - dpiRatio * LB_BORDERSPACE;
    float bulletOffsX = maxNamesLength == 0 ? nameStrOffsX :
        nameStrOffsX - dpiRatio * LB_BULLETSZ - dpiRatio * LB_BORDERSPACE;
    // Adjust offsets:
    int stepsY = 0;
    if (bulletOffsX >= startX)
    {
        float offsShiftX = (bulletOffsX - startX) / 2.0f;      // Actually startX + (bulletOffsX - startX)/2
        bulletOffsX -= offsShiftX;
        nameStrOffsX -= offsShiftX;
        scYOffsX -= offsShiftX;
        valStrOffsX -= offsShiftX;
        nameXStrOffsX -= offsShiftX;
        valXOffsX -= offsShiftX;
        nameYStrOffsX -= offsShiftX;
        valYOffsX -= offsShiftX;
    }
    else
    {
        float offsShiftX = startX - bulletOffsX;
        bulletOffsX += offsShiftX;
        nameStrOffsX += offsShiftX;
        scYOffsX += offsShiftX;

        stepsY = STEPY_VALSTR;

        if (valStrOffsX < startX)                             // Three lines
        {
            float valWidth = max(maxStrXLength, maxStrYLength);  // Beautification
            float nmWidth = max(maxNameXStrLength, maxNameYStrLength);
            valYOffsX = endX - valWidth;
            nameYStrOffsX = maxNameYStrLength == 0 ? valYOffsX :
                valYOffsX - nmWidth - dpiRatio * LB_BORDERSPACE;
            valXOffsX = valYOffsX;
            nameXStrOffsX = maxNameXStrLength == 0 ? valXOffsX :
                valXOffsX - nmWidth - dpiRatio * LB_BORDERSPACE;
            valStrOffsX = maxValStrLength == 0 ? nameXStrOffsX :
                min(nameXStrOffsX, nameYStrOffsX) - maxValStrLength - dpiRatio * LB_BORDERSPACE;
            stepsY |= STEPY_VALY;
        }
    }

    TUPLE_PRNLEGLAYOUT tuple_res;

    get<IDX_BULLETSTART>(tuple_res) = bulletOffsX,
        get<IDX_NAME>(tuple_res) = nameStrOffsX;
    get<IDX_SCY>(tuple_res) = scYOffsX;
    get<IDX_VAL>(tuple_res) = valStrOffsX;
    get<IDX_NAMEX>(tuple_res) = nameXStrOffsX;
    get<IDX_X>(tuple_res) = valXOffsX;
    get<IDX_NAMEY>(tuple_res) = nameYStrOffsX;
    get<IDX_Y>(tuple_res) = valYOffsX;

    get<IDX_STEPSY>(tuple_res) = stepsY;

    return tuple_res;
}

void CChartContainer::PrintCrossLine(PointF pntStartF, PointF pntEndF, SizeF sizeCrossF,
    Pen* penPtr, Graphics* grPtr, float dpiRatio)
{
    PointF pntXF(dpiRatio * sizeCrossF.Width / 2, 0.0f);
    PointF pntYF(0.0f, dpiRatio * sizeCrossF.Height / 2);
    float space = dpiRatio * (sizeCrossF.Width + 1.0f);
    // Lets calculate number of Crosses
    int CrossNmb = static_cast<int>(floor((pntEndF.X - pntStartF.X + dpiRatio * 1.0f) / space));

    GraphicsPath grPath;
    pntStartF.X += pntXF.X;   // Move to the center of the first fugure

    for (int i = 0; i < CrossNmb; ++i)
    {
        grPath.AddLine(pntStartF - pntXF, pntStartF + pntXF);
        grPath.StartFigure();
        grPath.AddLine(pntStartF - pntYF, pntStartF + pntYF);
        grPath.StartFigure();

        pntStartF.X += space;
    }
    grPtr->DrawPath(penPtr, &grPath);
}

void CChartContainer::GetSelValString(RectF drawRF, CChart* chartPtr, double pageScY,
    MAP_PRNDATA& mapPrnData, float dpiRatio)
{
    string_t emptyStr;
    sstream_t stream_t;
    TUPLE_PRINT tuple_chart;

    // Insert pen and brush attributes
    get<IDX_COL>(tuple_chart) = SetAlpha(chartPtr->GetChartColor(), 0xFF);
    get<IDX_DSTYLE>(tuple_chart) = chartPtr->GetChartDashStyle();
    get<IDX_PWIDTH>(tuple_chart) = chartPtr->GetPenWidth();

    // Insert the chart name
    get<IDX_NAME>(tuple_chart) = chartPtr->GetChartName();

    // Insert the line and point styles
    get<IDX_DSTYLE>(tuple_chart) = chartPtr->GetChartDashStyle();
    get<IDX_PSTYLE>(tuple_chart) = chartPtr->GetChartPointStyle();

    // Insert the Y-scale in unit/inch
    double scaleY = pageScY / chartPtr->GetLocScaleY();        //Dots per unit
    int chartIdx = chartPtr->GetChartIdx();
    int precisionY = chartPtr->GetPrecisionY();
    double minY = chartPtr->GetMinValY();
    double maxY = chartPtr->GetMaxValY();
    val_label_str_fn valStrFnPtr = chartPtr->GetLabYValStrFnPtr();
    drawRF.Inflate(DR_SPACE * dpiRatio, DR_SPACE * dpiRatio);

    stream_t << _T("ScaleY = ") << setprecision(precisionY + 1) << scaleY
        << _T(" units/inch") << _T("\n");
    get<IDX_SCY>(tuple_chart) = stream_t.str();

    // Insert selected points values
    MAP_SELPNTSD::const_iterator it = m_mapSelPntsD.find(chartIdx);
    if (it == m_mapSelPntsD.cend()) // It is names only
    {
        get<IDX_VAL>(tuple_chart) = emptyStr;
        get<IDX_NAMEX>(tuple_chart) = string_t(_T("MinY"));
        get<IDX_X>(tuple_chart) = valStrFnPtr(minY, precisionY, true);
        get<IDX_NAMEY>(tuple_chart) = string_t(_T("MaxY"));
        get<IDX_Y>(tuple_chart) = valStrFnPtr(maxY, precisionY, true);
        mapPrnData.insert(MAP_PRNDATA::value_type(chartIdx, tuple_chart));
    }
    else
    {
        size_t count = m_mapSelPntsD.count(chartIdx);
        for (size_t cnt = 0; cnt < count; ++cnt)
        {
            if (cnt != 0)
            {
                get<IDX_NAME>(tuple_chart) = emptyStr;
                get<IDX_SCY>(tuple_chart) = emptyStr;
            }

            get<IDX_VAL>(tuple_chart) = string_t(_T("Sel. Point: "));

            PointD selPntD = it->second;        // Value X, Y
            PointF selPntF = m_pMatrixD->TransformToPntF(chartPtr->GetLocScaleY(), selPntD);
            if (drawRF.Contains(selPntF))
            {
                get<IDX_NAMEX>(tuple_chart) = m_labelX;
                bool bEqSign = m_labelX.empty() ? false : true;
                get<IDX_X>(tuple_chart) = valStrFnPtr(selPntD.X, m_precision, bEqSign);
                string_t labelY = chartPtr->GetAxisYName();
                get<IDX_NAMEY>(tuple_chart) = labelY;
                bEqSign = labelY.empty() ? false : true;
                get<IDX_Y>(tuple_chart) = valStrFnPtr(selPntD.Y, precisionY, bEqSign);
            }
            else
            {
                get<IDX_VAL>(tuple_chart) = string_t(_T("Sel. N/A, "));
                get<IDX_NAMEX>(tuple_chart) = string_t(_T("MinY"));
                get<IDX_X>(tuple_chart) = valStrFnPtr(minY, precisionY, true);
                get<IDX_NAMEY>(tuple_chart) = string_t(_T("MaxY"));
                get<IDX_Y>(tuple_chart) = valStrFnPtr(maxY, precisionY, true);
            }

            mapPrnData.insert(MAP_PRNDATA::value_type(chartIdx, tuple_chart));
            ++it;
        }
    }
}

size_t CChartContainer::PrintChartNames(RectF namesRF, const MAP_PRNDATA& mapPrnData,
    const TUPLE_PRNLEGLAYOUT& tuple_prnLayout, Graphics* grPtr, float dpiRatio, size_t mapOffs)
{
    // Set the font
#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font labelFont(&fontFamily, 8.0f, FontStyleBold);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    float fontHeight = labelFont.GetHeight(grPtr);
    float lineHeight = 1.5f * fontHeight; // In doc. units, 1/300 of inch

    // Get offsets
    float lineY = namesRF.Y;

    // Get string layout; do not care whether there are val, nMEx, and nameY  strings
    float bulletStartOffsX = get<IDX_BULLETSTART>(tuple_prnLayout);
    float bulletEndX = bulletStartOffsX + dpiRatio * LB_BULLETSZ;
    float nmStrOffsX = get<IDX_NAME>(tuple_prnLayout);
    float scYStrOffsX = get<IDX_SCY>(tuple_prnLayout);
    float valStrOffsX = get<IDX_VAL>(tuple_prnLayout);
    float nmXOffsX = get<IDX_NAMEX>(tuple_prnLayout);
    float valXOffsX = get<IDX_X>(tuple_prnLayout);
    float nmYOffsX = get<IDX_NAMEY>(tuple_prnLayout);
    float valYOffsX = get<IDX_Y>(tuple_prnLayout);

    int stepsY = get<IDX_STEPSY>(tuple_prnLayout);

    Pen bulletPen((ARGB)Color::Black);      // Pen Width is to set later
    SolidBrush txtBr((ARGB)Color::Black);

    MAP_PRNDATA::const_iterator it = mapPrnData.begin();
    MAP_PRNDATA::const_iterator itE = mapPrnData.end();

    advance(it, mapOffs);
    bool bName = mapOffs == 0 ? false : true;   // To begin the new page with full line
    for (; it != itE; ++it)
    {
        TUPLE_PRINT tuple_chart = it->second;     // Get strings prepared
        // Set pen and brush attributes
        Color strColor = get<IDX_COL>(tuple_chart);
        bulletPen.SetColor(strColor);
        DashStyle dashStyle = get<IDX_DSTYLE>(tuple_chart);
        bulletPen.SetDashStyle(dashStyle != DashStyleCustom ? dashStyle : DashStyleSolid);
        bulletPen.SetWidth(dashStyle != DashStyleCustom ?
            get<IDX_PWIDTH>(tuple_chart) * dpiRatio : dpiRatio);
        txtBr.SetColor(strColor);
        PointStyle pntStyle = get<IDX_PSTYLE>(tuple_chart);

        PointF pntStartF, pntEndF;

        string_t tmpStr = get<IDX_NAME>(tuple_chart);
        if (!tmpStr.empty())
        {
            bName = false;
            pntStartF = PointF(bulletStartOffsX, lineY + fontHeight / 2.0f);
            pntEndF = PointF(bulletEndX, lineY + fontHeight / 2.0f);

            if (dashStyle != DashStyleCustom)
            {
                grPtr->DrawLine(&bulletPen, pntStartF, pntEndF);

                // Change the pen to solid line for the point style
                bulletPen.SetDashStyle(DashStyle::DashStyleSolid);

                // Set the center point for the point designator
                PointF pnt((pntEndF.X - pntStartF.X) / 2.0f + pntStartF.X, pntEndF.Y);
                GraphicsPath grPath;
                std::vector<Gdiplus::PointF> vec;
                RectF rPntF = RectFFromCenterF(pnt, dpiRatio * CHART_DTPNTSZ, dpiRatio * CHART_DTPNTSZ);

                switch (pntStyle)
                {
                case PointStyle::Triangle_Up:
                    TriangleUpFFromCenterF(pnt, dpiRatio * CHART_DTPNTSZ, dpiRatio * CHART_DTPNTSZ, vec);
                    if (3 == vec.size())
                    {
                        auto ptr = vec.data();
                        grPath.AddPolygon(ptr, vec.size());
                    }
                    break;
                case PointStyle::Square:
                    grPath.AddRectangle(rPntF);
                    break;
                case PointStyle::Diamond:
                    DiamondFFromCenterF(pnt, dpiRatio * CHART_DTPNTSZ, dpiRatio * CHART_DTPNTSZ, vec);
                    if (4 == vec.size())
                    {
                        auto ptr = vec.data();
                        grPath.AddPolygon(ptr, vec.size());
                    }
                    break;
                case PointStyle::Triangle_Down:
                    TriangleDownFFromCenterF(pnt, dpiRatio * CHART_DTPNTSZ, dpiRatio * CHART_DTPNTSZ, vec);
                    if (3 == vec.size())
                    {
                        auto ptr = vec.data();
                        grPath.AddPolygon(ptr, vec.size());
                    }
                    break;
                case PointStyle::Ellipse:
                default:
                    grPath.AddEllipse(rPntF);
                    break;
                }
                grPtr->DrawPath(&bulletPen, &grPath);

            }
            else
            {
                PrintCrossLine(pntStartF, pntEndF, SizeF(CHART_DTPNTSZ, CHART_DTPNTSZ),
                    &bulletPen, grPtr, dpiRatio);
            }
            // Draw name string
            DrawString(grPtr, tmpStr, -1, &labelFont, PointF(nmStrOffsX, lineY), &txtBr);

            // Draw scY string
            tmpStr = get<IDX_SCY>(tuple_chart);
            DrawString(grPtr, tmpStr, -1, &labelFont, PointF(scYStrOffsX, lineY), &txtBr);
        }
        else if (bName)
        {
            MAP_PRNDATA::const_reverse_iterator rIt = MAP_PRNDATA::const_reverse_iterator(it);
            for (; rIt != mapPrnData.crend(); ++rIt)
            {
                TUPLE_PRINT tuple_nm = rIt->second;
                tmpStr = get<IDX_NAME>(tuple_nm);
                if (tmpStr.empty())
                    continue;
                if (dashStyle != DashStyleCustom)
                {
                    grPtr->DrawLine(&bulletPen, pntStartF, pntEndF);
                }
                else
                {
                    PrintCrossLine(pntStartF, pntEndF, SizeF(CHART_DTPNTSZ, CHART_DTPNTSZ),
                        &bulletPen, grPtr, dpiRatio);
                }
                DrawString(grPtr, tmpStr, -1, &labelFont, PointF(nmStrOffsX, lineY), &txtBr);
                tmpStr = get<IDX_SCY>(tuple_nm);
                DrawString(grPtr, tmpStr, -1, &labelFont, PointF(scYStrOffsX, lineY), &txtBr);
                bName = false;
                break;
            }
        }

        if (stepsY & STEPY_VALSTR)
            lineY += lineHeight;

        tmpStr = get<IDX_VAL>(tuple_chart);
        if (!tmpStr.empty())
        {
            DrawString(grPtr, tmpStr, -1, &labelFont, PointF(valStrOffsX, lineY), &txtBr);
        }
        tmpStr = get<IDX_NAMEX>(tuple_chart);
        if (!tmpStr.empty())
        {
            DrawString(grPtr, tmpStr, -1, &labelFont, PointF(nmXOffsX, lineY), &txtBr);
        }
        DrawString(grPtr, get<IDX_X>(tuple_chart), -1, &labelFont, PointF(valXOffsX, lineY),
                   &txtBr);
        if (stepsY & STEPY_VALY)
            lineY += lineHeight;

        tmpStr = get<IDX_NAMEY>(tuple_chart);
        if (!tmpStr.empty())
        {
            DrawString(grPtr, tmpStr, -1, &labelFont, PointF(nmYOffsX, lineY), &txtBr);
        }
        DrawString(grPtr, get<IDX_Y>(tuple_chart), -1, &labelFont, PointF(valYOffsX, lineY),
                   &txtBr);

        ++mapOffs;
        lineY += lineHeight;

        if ((lineY + 3 * lineHeight) > namesRF.GetBottom())
            return mapOffs;

    }
    return mapOffs;
}

// Added by SGS
void CChartContainer::PrintXAxisName(RectF namesRF, const CChartContainer* pContainer, Graphics* grPtr)
{
    // Set the font
#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font labelFont(&fontFamily, 8.0f, FontStyleBold);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    SolidBrush brHeader((ARGB)Color::Black);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentNear);
    strFormat.SetLineAlignment(StringAlignmentCenter);

    DrawString(grPtr, pContainer->m_x_axis_label, -1, &labelFont, PointF(namesRF.X, namesRF.Y),
               &brHeader);
}

// Added by SGS
std::tuple <double, double, double, double> CChartContainer::GetMinMaxes()
{
    auto startX = m_startX;
    auto endX = m_endX;
    auto startY = m_minExtY;
    auto endY = m_maxExtY;

    // Allow Logarithmic base 10
    if (m_x_axis_is_logarithmic && m_y_axis_is_logarithmic)
    {
        // Need to find the minimum positive number.
        auto positive_minimum_x = DBL_MAX;
        auto positive_maximum_x = 0.0000001;
        auto positive_minimum_y = DBL_MAX;
        auto positive_maximum_y = 0.0000001;
        for (const auto& chart : m_mapCharts)
        {
            for (const auto& point : chart.second->m_vDataPnts)
            {
                if ((point.X > 0) && (point.Y > 0))
                {
                    if (point.X < positive_minimum_x)
                    {
                        positive_minimum_x = point.X;
                    }

                    if (point.X > positive_maximum_x)
                    {
                        positive_maximum_x = point.X;
                    }

                    if (point.Y < positive_minimum_y)
                    {
                        positive_minimum_y = point.Y;
                    }

                    if (point.Y > positive_maximum_y)
                    {
                        positive_maximum_y = point.Y;
                    }
                }
            }
        }

        // Cannot handle negative numbers nor 0.
        startX = log10(positive_minimum_x);
        endX = log10(positive_maximum_x);
        startY = log10(positive_minimum_y);
        endY = log10(positive_maximum_y);
    }
    else if (m_x_axis_is_logarithmic)
    {
        // Need to find the minimum positive number.
        auto positive_minimum = 1.79769e+308;
        auto positive_maximum = 0.0000001;
        for (const auto& chart : m_mapCharts)
        {
            for (const auto& point : chart.second->m_vDataPnts)
            {
                if ((point.X > 0) && (point.X < positive_minimum))
                {
                    positive_minimum = point.X;
                }

                if ((point.X > 0) && (point.X > positive_maximum))
                {
                    positive_maximum = point.X;
                }
            }
        }

        // Cannot handle negative numbers nor 0.
        startX = log10(positive_minimum);
        endX = log10(positive_maximum);
    }
    else if (m_y_axis_is_logarithmic)
    {
        // Need to find the minimum positive number.
        auto positive_minimum = 1.79769e+308;
        auto positive_maximum = 0.0000001;
        for (const auto& chart : m_mapCharts)
        {
            for (const auto& point : chart.second->m_vDataPnts)
            {
                if ((point.Y > 0) && (point.Y < positive_minimum))
                {
                    positive_minimum = point.Y;
                }

                if ((point.Y > 0) && (point.Y > positive_maximum))
                {
                    positive_maximum = point.Y;
                }
            }
        }

        // Cannot handle negative numbers nor 0.
        startY = log10(positive_minimum);
        endY = log10(positive_maximum);
    }
    return std::make_tuple(startX, endX, startY, endY);
}


string_t CChartContainer::PrintPageHeader(RectF rGdiF, const CChartContainer* pContainer,
    string_t tmStr, Graphics* grPtr)
{
    // Form bounding rectangles
    RectF headerRF = rGdiF;
    headerRF.Height = 80.0f; // Approx. 7 mm
    headerRF.Inflate(-200.0f, 0.0f);
    headerRF.Offset(0.0f, 250.0f);

    // Draw header
#ifdef _UNICODE
    FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W("Verdana"));
#endif
    Gdiplus::Font headerFont(&fontFamily, 16.0f, FontStyleRegular, UnitPoint);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    SolidBrush brHeader((ARGB)Color::Black);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentNear);
    strFormat.SetLineAlignment(StringAlignmentCenter);

    DrawString(grPtr, pContainer->m_name, -1, &headerFont, headerRF, &strFormat, &brHeader);

    // 20150303 SGS Remove the Date stamp.
    //  if (tmStr.empty())
    //  {
    //    // Print current local time
    //    CTime tmD = CTime::GetCurrentTime();
    //    CString strTmD = tmD.Format(_T("%a, %b %d, %Y, %H:%M"));
    //    tmStr = string_t(strTmD);
    //  }
    //
    //// Draw time string
    //  strFormat.SetAlignment(StringAlignmentFar);
    //  DrawString(grPtr, tmStr, -1, &headerFont,  headerRF, &strFormat, &brHeader);

    return tmStr;
}

void CChartContainer::PrintCharts(CChartContainer* pContainer, int scrDpiX, HDC printDC)
{
    if (printDC != nullptr)
    {
        pContainer->m_bAxBoundaries = true;

        CDC* pDC = new CDC;
        pDC->Attach(printDC);
        Graphics* grPtr = new Graphics(printDC);

        grPtr->SetPageUnit(UnitDocument);
        grPtr->SetSmoothingMode(SmoothingModeAntiAlias);
        // Get dpiRatio
        float dpiRatioX = 300.0f / scrDpiX;

        RectF rGdiF;
        grPtr->GetVisibleClipBounds(&rGdiF);                 // The same as the clip rect

        // Begin Printing
        pDC->StartDoc(pContainer->m_name.c_str());          // MFC functions
        pDC->StartPage();

        // Draw header
        string_t tmStr;
        tmStr = pContainer->PrintPageHeader(rGdiF, pContainer, tmStr, grPtr);

        // Format drawing area and draw a border
        RectF drawRF = rGdiF;
        drawRF.Inflate(-200.0f, -200.0f);
        drawRF.Height = rGdiF.Width * 0.75f;
        drawRF.Offset(0.0f, 200.0f);

        SolidBrush bkBr(pContainer->m_colBkground);
        grPtr->FillRectangle(&bkBr, drawRF);

        // 20150303 SGS - Reduce the width of the box around the graph.
        //Pen pen((ARGB)Color::Black, 3*dpiRatioX);
        Pen pen((ARGB)Color::Black, 2 * dpiRatioX);
        pen.SetAlignment(PenAlignmentInset);
        grPtr->DrawRectangle(&pen, drawRF);
        // Set clipping region
        grPtr->SetClip(drawRF);

        // Draw charts
        if (pContainer->m_mapCharts.size() > 0)
        {
            // Get the proper start end values in case of logarithmic scale.
            auto [startX, endX, startY, endY] = pContainer->GetMinMaxes();

            // Draw axes
            PointF pntOrigF = pContainer->DrawAxes(drawRF, startX, endX, startY, endY, pContainer->m_colAxis, grPtr);
            // Draw Grid
            // SGS
            if (pContainer->m_use_coordinates)
            {
                pContainer->DrawGridWithCoordinates(drawRF, pntOrigF, grPtr, dpiRatioX);
            }
            else
            {
                pContainer->DrawGrid(drawRF, pntOrigF, grPtr, dpiRatioX);
            }

            // Set margins for drawing
            drawRF.Inflate(-DR_SPACE*dpiRatioX, -DR_SPACE*dpiRatioX);
            // Update scales
            std::pair <double, double> pair_scales = pContainer->UpdateScales(drawRF, startX, endX, startY, endY);
            pContainer->m_scX = pair_scales.first;
            pContainer->m_scY = pair_scales.second;

            double offsetY(pntOrigF.Y);

            // Set transform matrix
            MatrixD matrixD;
            matrixD.Translate(pntOrigF.X, offsetY); // Add drawRF
            matrixD.Scale(float(pContainer->m_scX), float(-pContainer->m_scY));

            if ((pContainer->m_axXPos != BETWEEN) || (pContainer->m_axYPos != MIDDLE))
            {
                double translateX = (pContainer->m_axXPos == RIGHT) ? -endX : (pContainer->m_axXPos == LEFT) ? -startX : 0.0;
                double translateY = (pContainer->m_axYPos == BOTTOM) ? -startY : (pContainer->m_axYPos == TOP) ? -endY : 0.0;
                matrixD.Translate(float(translateX), float(translateY));
            }
            if (pContainer->m_pMatrixD != nullptr)
                delete pContainer->m_pMatrixD;
            pContainer->m_pMatrixD = matrixD.Clone();     // Can't copy: copy constructor is private

            // Calculate page scale Y to display on the legend
            ENSURE(grPtr->GetPageUnit() == UnitDocument); // Sure it is: set it at the start of printing
            PointD pntStartD, pntEndD;
            CPoint pntStart(int(drawRF.X), int(drawRF.Y)), pntEnd(int(drawRF.GetRight()), int(drawRF.GetBottom()));
            pContainer->MousePntToPntD(pntStart, pntStartD, pContainer->m_pMatrixD);
            pContainer->MousePntToPntD(pntEnd, pntEndD, pContainer->m_pMatrixD);
            double pageScY = 300.0 * (pntStartD.Y - pntEndD.Y) / drawRF.Height;  // 300 dpi/inch for the Doc. Mode

            // Begin draw all visible curves or selected only
            int chartSelIdx = pContainer->HasSelectedChart();
            MAP_LABSTR mapLabStr;
            size_t mapOffs = 0;

            GraphicsPath grPath;            // Will be used by the drawing routines
            MAP_CHARTS::iterator it;
            if (chartSelIdx == -1)
                it = pContainer->m_mapCharts.begin();
            else
                it = pContainer->m_mapCharts.find(chartSelIdx);
            MAP_CHARTS::iterator itE = pContainer->m_mapCharts.end();

            MAP_PRNDATA mapPrnData;

            for (; it != itE; ++it)
            {
                CChart* chartPtr = it->second;
                if (chartPtr->IsChartVisible() == false)
                    continue;
                if (chartPtr->m_vDataPnts.size() == 0)
                    continue;
                if (pContainer->XAxisIsLogarithmic() || pContainer->YAxisIsLogarithmic())
                {
                    // Have to handle all 3 cases depending on which axi are logarithmic
                    if (pContainer->XAxisIsLogarithmic() && pContainer->YAxisIsLogarithmic())
                    {
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return ((pt.X > 0) && (pt.Y > 0)); });
                        for (auto& pt : DataPnts)
                        {
                            pt.X = log10(pt.X);
                            pt.Y = log10(pt.Y);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, pContainer->m_pMatrixD, &grPath, grPtr, dpiRatioX);
                    }
                    else if (pContainer->XAxisIsLogarithmic())
                    {
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return pt.X > 0; });
                        for (auto& pt : DataPnts)
                        {
                            pt.X = log10(pt.X);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, pContainer->m_pMatrixD, &grPath, grPtr, dpiRatioX);
                    }
                    else
                    {
                        // Y is Logarithmic
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return pt.Y > 0; });
                        for (auto& pt : DataPnts)
                        {
                            pt.Y = log10(pt.Y);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, pContainer->m_pMatrixD, &grPath, grPtr, dpiRatioX);
                    }
                }
                else
                {
                    chartPtr->DrawChartCurve(chartPtr->m_vDataPnts, startX, endX, pContainer->m_pMatrixD, &grPath, grPtr, dpiRatioX);
                }

                if (pContainer->m_bTracking)
                    pContainer->DrawSelPntsD(chartPtr, pContainer->m_mapSelPntsD, &grPath, grPtr, drawRF, dpiRatioX);

                pContainer->GetSelValString(drawRF, chartPtr, pageScY, mapPrnData, dpiRatioX);

                if (chartSelIdx > -1)
                    break;
            }

            if (pContainer->m_bTracking)
                pContainer->DrawSelLine(drawRF, grPtr, chartSelIdx, dpiRatioX);

            // 20150303 SGS - Do NOT print the boundary values on the X-axis
            //if (pContainer->m_bAxBoundaries)
            if (!pContainer->m_use_coordinates && pContainer->m_bAxBoundaries)
                pContainer->DrawAxXBoundaryStr(drawRF, grPtr);
            // Now print the legend: chart names, etc.
            grPtr->ResetClip();
            // 20150303 SGS - Added the printing of the X-Axis title.
            RectF axisRF(static_cast<Gdiplus::REAL>(drawRF.X + drawRF.Width / 2.0 - 400), drawRF.GetBottom() + 200.0f, drawRF.Width, 0.0f);
            pContainer->PrintXAxisName(axisRF, pContainer, grPtr);
            // 20150303 SGS - Set the rectangle lower to miss the bottom of the graph.
            //RectF namesRF(drawRF.X, drawRF.GetBottom() + 100.0f, drawRF.Width, 0.0f);
            RectF namesRF(drawRF.X, drawRF.GetBottom() + 300.0f, drawRF.Width, 0.0f);
            namesRF.Height = rGdiF.GetBottom() - namesRF.Y; // temporary; will be set in PrintChartNames
            TUPLE_PRNLEGLAYOUT tuple_prnLayout =
                pContainer->CalcNamesPntLayout(namesRF, mapPrnData, grPtr, dpiRatioX);
            mapOffs = pContainer->PrintChartNames(namesRF, mapPrnData, tuple_prnLayout, grPtr, dpiRatioX);
            if (mapOffs < mapPrnData.size())        // Print the rest of the legend on next pages
            {
                // Get new namesRF
                namesRF = rGdiF;
                namesRF.Inflate(-200.0f, -200.0f);
                namesRF.Height = rGdiF.Height - 300.0f;
                namesRF.Offset(0.0f, 300.0f);
                // Print extra pages
                while (mapOffs < mapPrnData.size())
                {
                    pDC->EndPage();
                    pDC->StartPage();
                    pContainer->PrintPageHeader(rGdiF, pContainer, tmStr, grPtr);
                    mapOffs = pContainer->PrintChartNames(namesRF, mapPrnData, tuple_prnLayout, grPtr,
                        dpiRatioX, mapOffs);
                }
            }
        }
        // End printing
        pDC->EndPage();
        pDC->EndDoc();

        delete grPtr;
        grPtr = nullptr;
        pDC->Detach();
        pDC->RestoreDC(-1);
        pDC->DeleteDC();
        delete pDC;
        pDC = nullptr;
    }
}


BEGIN_MESSAGE_MAP(CChartContainer, CWnd)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_GETDLGCODE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_SETCURSOR()
    ON_WM_MBUTTONDOWN()
    ON_WM_MOUSEWHEEL()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(IDM_CHART_LEGEND, OnMnShowLegend)
    ON_COMMAND(IDM_CHART_LEGEND_MOVE, OnMnMoveLegend)
    ON_COMMAND(IDM_CHART_BOUNDARIES, OnMnShowAxBoundaries)
    ON_COMMAND(IDM_CHART_X_AXIS, OnMnXAxisIsLogarithmic)
    ON_COMMAND(IDM_CHART_Y_AXIS, OnMnYAxisIsLogarithmic)
    ON_COMMAND(IDM_CHART_REFRESH, OnMnRefresh)
    ON_COMMAND(IDM_CHART_VIEWDATA, OnMnViewData)
    ON_COMMAND(IDM_CHART_SAVE, OnMnSaveCharts)
    ON_COMMAND(IDM_CHART_SAVEIMAGE, OnMnSaveImage)
    ON_COMMAND(IDM_CHART_PRINT, OnMnPrintCharts)

    ON_COMMAND(IDM_CHART_ZOOMINX, OnMnZoomInX)
    ON_COMMAND(IDM_CHART_MOVERIGHT, OnMnMoveRight)
    ON_COMMAND(IDM_CHART_MOVELEFT, OnMnMoveLeft)
    ON_COMMAND(IDM_CHART_ZOOMINY, OnMnZoomInY)
    ON_COMMAND(IDM_CHART_MOVEUP, OnMnMoveUp)
    ON_COMMAND(IDM_CHART_MOVEDOWN, OnMnMoveDown)
    ON_COMMAND(IDM_CHART_TRACK, OnMnEnableTracking)
    ON_COMMAND(IDM_CHART_SHOWPNTS, OnMnShowPnts)
    ON_COMMAND(IDM_CHART_UNDOZOOMMOVEX, OnMnUndoHistStepX)
    ON_COMMAND(IDM_CHART_UNDOZOOMMOVEY, OnMnUndoHistStepY)
    ON_COMMAND_RANGE(IDM_CHART_FIRSTCHART, IDM_CHART_LASTCHART, OnMnShowChart)
#ifdef _TEST_
    ON_COMMAND(IDM_CHART_TEST, OnMnTest)
#endif
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_TIMER()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CChartContainer message handlers
void CChartContainer::OnPaint()
{
    CPaintDC dc(this);                              // Device context for painting
    Graphics gr(dc.m_hDC);                          // Graphics to paint

    Rect rGdi;
    gr.GetVisibleClipBounds(&rGdi);                 // The same as the clip rect

    Bitmap clBmp(rGdi.Width, rGdi.Height);          // Mem bitmap
    Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC
    grPtr->SetSmoothingMode(SmoothingModeAntiAlias);

    auto yAxisLabelRect = rGdi;
    auto xAxisLabelRect = rGdi;

    if (m_showXLabel)
    {
        // 10 is an arbitrary number but works with 13 pt fonts.
        yAxisLabelRect.Width = 10;
        rGdi.X += 10;
        rGdi.Width -= 10;
    }

    if (m_showYLabel)
    {
        // Adjust the chart rectangle to give space for the X axis label
        xAxisLabelRect.Y = xAxisLabelRect.Height - 12;
        xAxisLabelRect.Height = 12;
        // Choose a smaller number because the inflate method makes the space between the axis label and the 
        // edge of the chart is too much.
        rGdi.Height -= 10;
    }

    RectF rGdiF = GdiRectToRectF(rGdi);
    rGdiF.Width -= 1.0f;
    rGdiF.Height -= 1.0f;
    DrawBkground(rGdiF, m_colBkground, m_colBorder, grPtr);

    if (m_mapCharts.size() > 0)
    {
        // Get maxes based on logarithmic or linear scale.
        auto [startX, endX, startY, endY] = GetMinMaxes();

        RestoreModeX();  // Might be in the middle of setting zoom borders; annuls the mode
        RestoreModeY();
        RectF drawRF = rGdiF;
        drawRF.Inflate(-DR_SPACE, -DR_SPACE);
        
        if (m_showXLabel || m_showYLabel)
        {
            // Doesn't work - shrinks the graph a little to make space for axis labels and makes the points
            // and text fuzzy. Use with setXAxisLabel, setYAxisLabel, ShowXAxisLabel, ShowYAxisLabel
            // GetClientRectBase, GetClientRectAccountingForLabels, and DrawAxisLabels.
            DrawAxisLabels(yAxisLabelRect, xAxisLabelRect, dc.m_hDC);
        }
        
        // Draw axes
        if (!HasChartWithData(-1))
            DrawGrid(drawRF, PointF(0.0f, 0.0f), grPtr);
        else  // Proceed normally even with one point data
        {
            PointF pntOrigF = DrawAxes(drawRF, startX, endX, startY, endY, m_colAxis, grPtr);
            // Draw a grid
            // SGS
            if (m_use_coordinates)
            {
                DrawGridWithCoordinates(drawRF, pntOrigF, grPtr);
            }
            else
            {
                DrawGrid(drawRF, pntOrigF, grPtr);
            }

            // Update scales
            std::pair<double, double> pair_scales =
                UpdateScales(drawRF, startX, endX, startY, endY);
            m_scX = pair_scales.first;
            m_scY = pair_scales.second;

            double offsetY(pntOrigF.Y);

            // Set transform matrix
            MatrixD matrixD;
            matrixD.Translate(pntOrigF.X, offsetY);
            matrixD.Scale(m_scX, -m_scY);

            // Account for not even axes placement
            if ((m_axXPos != BETWEEN) || (m_axYPos != MIDDLE))
            {
                double translateX = (m_axXPos == RIGHT) ? -endX : (m_axXPos == LEFT) ? -startX : 0.0;
                double translateY = (m_axYPos == BOTTOM) ? -startY : (m_axYPos == TOP) ? -endY : 0.0;
                matrixD.Translate(translateX, translateY);
            }
            if (m_pMatrixD != nullptr)
                delete m_pMatrixD;
            m_pMatrixD = matrixD.Clone();     // Copy constructor is private ??

            // Begin draw curves
            GraphicsPath grPath;            // Will be used actoss the drawing routines
            MAP_CHARTS::iterator it = m_mapCharts.begin();
            MAP_CHARTS::iterator itE = m_mapCharts.end();
            for (; it != itE; ++it)
            {
                CChart* chartPtr = it->second;
                if (chartPtr->IsChartVisible() == false)
                    continue;
                if (chartPtr->m_vDataPnts.size() == 0)
                    continue;
                if (m_x_axis_is_logarithmic || m_y_axis_is_logarithmic)
                {
                	// Have to handle all 3 cases depending on which axi are logarithmic
                    if (m_x_axis_is_logarithmic && m_y_axis_is_logarithmic)
                    {
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return ((pt.X > 0) && (pt.Y > 0)); });
                        for (auto& pt : DataPnts)
                        {
                            pt.X = log10(pt.X);
                            pt.Y = log10(pt.Y);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, m_pMatrixD, &grPath, grPtr);
                    }
                    else if (m_x_axis_is_logarithmic)
                    {
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return pt.X > 0; });
                        for (auto& pt : DataPnts)
                        {
                            pt.X = log10(pt.X);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, m_pMatrixD, &grPath, grPtr);
                    }
                    else
                    {
                        // Y is Logarithmic
                        V_CHARTDATAD DataPnts;
                        std::copy_if(chartPtr->m_vDataPnts.begin(), chartPtr->m_vDataPnts.end(), std::back_inserter(DataPnts), [](auto pt) {return pt.Y > 0; });
                        for (auto& pt : DataPnts)
                        {
                            pt.Y = log10(pt.Y);
                        }
                        chartPtr->DrawChartCurve(DataPnts, startX, endX, m_pMatrixD, &grPath, grPtr);
                    }
                }
                else
                {
                    chartPtr->DrawChartCurve(chartPtr->m_vDataPnts, startX, endX, m_pMatrixD, &grPath, grPtr);
                }
                if (m_bTracking && IsLabWndExist(false))              // Draw selection points if any
                    DrawSelPntsD(chartPtr, m_mapSelPntsD, &grPath, grPtr, drawRF);
            }

            if (m_bTracking)                // And the sel line too
                DrawSelLine(rGdiF, grPtr);
            if (!m_mapDataViewPntsD.empty())// Were selected in the data view
                DrawDataViewPntsD(m_mapDataViewPntsD, startX, endX, &grPath, grPtr);

            if (m_bAxBoundaries)            // Left and right visible X-coordinates
                DrawAxXBoundaryStr(rGdiF, grPtr);
        }   // Normal painring
    }
    // Transfer to the screen
    delete grPtr;
    gr.DrawImage(&clBmp, rGdi);
    if (m_first_time)
    {
        // If the user has set the m_show_legend flag then do show after the first drawing of the window.
        SetTimer(3, 50, nullptr);
        m_first_time = false;
    }
}

void CChartContainer::OnSize(UINT nType, int cx, int cy)
{
    UNREFERENCED_PARAMETER(cx);
    UNREFERENCED_PARAMETER(cy);

    CWnd* pParent = GetParent();
    if ((pParent == nullptr) && ((nType == SIZE_RESTORED) || (nType == SIZE_MAXIMIZED)))
    {
        bool bRes = HideLabWnds();
        if (bRes)
            SetTimer(2, 50, nullptr);
    }
}

UINT CChartContainer::OnGetDlgCode()
{
    UINT nCode = CWnd::OnGetDlgCode();
    nCode |= DLGC_WANTARROWS;
    return nCode;
}

void CChartContainer::OnLButtonDown(UINT nFlags, CPoint point)
{
    if ((m_bUserEnabled) && HasChartWithData(-1, true))
    {
        SetFocus();
        if (nFlags & MK_SHIFT)
        {
            if (m_chModeY == MODE_ZOOMINGY)
                PrepareZoomY(point, m_tmpZY, true);     // Zoom Y
            else if ((m_chModeX != MODE_ZOOMINGX) && (m_chModeX != MODE_ZOOMX))
                m_tmpZX = BeginZoomingX(point, true);    // Sets m_startX, m_chMode = MODE_ZOOMING
            else if (m_chModeX == MODE_ZOOMINGX)
                PrepareZoomX(point, m_tmpZX, true);     // Sets m_startX, m_endX, m_chMode = MODE_ZOOM
        }
        else if (nFlags & MK_CONTROL)
        {
            m_chartSelIdx = SelectChartByMouseClick(point);
            UpdateContainerWnds();
        }
        else if (m_bTracking)
            ShowDataLegend(point);
    }

    CWnd::OnLButtonDown(nFlags, point);
}

void CChartContainer::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if ((m_bUserEnabled) && HasChartWithData(-1, true))
    {
        if ((nFlags & MK_SHIFT) && (m_chModeY != MODE_ZOOMINGY) && (m_chModeY != MODE_ZOOMY))
        {
            m_tmpZY = BeginZoomingY(point, true);    // Sets m_startX, m_chMode = MODE_ZOOMING
        }
    }

    CWnd::OnLButtonDblClk(nFlags, point);
}

void CChartContainer::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bUserEnabled && HasChartWithData(-1, true))
    {
        if ((m_chModeX == MODE_ZOOMX) || (m_chModeY == MODE_ZOOMY))
        {
            UpdateContainerWnds(-1, true);

            m_chModeX = m_vHistX.empty() ? MODE_FULLX : MODE_ZOOMEDX;
            m_chModeY = m_vHistY.empty() ? MODE_FULLY : MODE_ZOOMEDY;
        }
    }

    CWnd::OnLButtonUp(nFlags, point);
}

void CChartContainer::OnMButtonDown(UINT nFlags, CPoint point)
{
    if (m_bUserEnabled)   // Responsibility of the programmer to set visiuals and cursor on m_bUserEnabled
    {
        SetFocus();
        bool bHasVisibleChart = HasChartWithData(-1, true);
        bool bOldTrack = m_bTracking;
        m_bTracking = bHasVisibleChart ? !m_bTracking : false;
        if ((bOldTrack != m_bTracking) && !m_bTracking)
        {
            m_pDataWnd = DestroyLabWnd(m_pDataWnd);
            m_dataLegPntD = PointD(DBL_MAX, DBL_MAX);
        }
    }

    CWnd::OnMButtonDown(nFlags, point);
}

BOOL CChartContainer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (m_bUserEnabled && HasChartWithData(-1, true))
    {
        if (nFlags & MK_SHIFT)            // Move
        {
            double deltaX = 0.0;
            if (zDelta < 0)
                deltaX = MOVE_DELTAX * (m_endX - m_startX);
            else if (zDelta > 0)
                deltaX = -MOVE_DELTAX * (m_endX - m_startX);
            else
                return CWnd::OnMouseWheel(nFlags, zDelta, pt);

            MoveContainerChartsX(deltaX, true);
        }
        else if ((nFlags & MK_CONTROL) && (m_chartSelIdx != -1))
        {
            MAP_CHARTS::iterator it = m_mapCharts.find(m_chartSelIdx);
            if (it != m_mapCharts.end())
            {
                CChart* chartPtr = it->second;
                if (chartPtr->IsChartVisible() && chartPtr->IsChartSelected())
                {
                    std::pair<double, double> pair_scY = GetLocScYDelta(chartPtr);
                    double locScY = pair_scY.first;
                    double deltaK = pair_scY.second;

                    if (zDelta < 0)
                        locScY = locScY = max(locScY - deltaK, DELTA_LOCY);
                    else
                        locScY += deltaK;

                    if (IsLabWndExist(false))
                    {
                        if (m_pDataWnd->IsWindowVisible())
                            m_pDataWnd->ShowWindow(SW_HIDE);
                        SetTimer(1, 500, nullptr);
                    }

                    chartPtr->SetLocScaleY(locScY);
                    RefreshWnd();
                }
            }
        }
    }
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChartContainer::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_bUserEnabled && HasChartWithData(-1, true))
    {
        if (nChar == VK_DELETE)
        {
            if (IsWindow(m_hWnd) && IsWindowVisible() && ((m_chModeX == MODE_ZOOMINGX) || (m_chModeY == MODE_ZOOMINGY)))
                RefreshWnd();
        }
        else if ((nChar == VK_LEFT) || (nChar == VK_RIGHT))    // Move
            MoveChartsByArrowKeysX(nChar);
        else if ((nChar == VK_DOWN) || (nChar == VK_UP) || (nChar == VK_PRIOR) || (nChar == VK_NEXT))
        {
            if (m_chartSelIdx != -1)
                ChangeLocScaleYByArrowKeys(nChar);
            else
                MoveChartsByArrowKeysY(nChar);
        }
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CChartContainer::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (nHitTest == HTCLIENT)
    {
        HCURSOR hCursor = nullptr;
        if (m_bTracking)
            hCursor = LoadCursor(nullptr, IDC_CROSS);
        else
            hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (hCursor != nullptr)
        {
            SetCursor(hCursor);
        }
        return TRUE;

    }
    return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CChartContainer::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1)        // Append
    {
        KillTimer(nIDEvent);

        if (m_bTracking && IsLabWndExist(false))
            UpdateDataLegend(true);
    }
    else if (nIDEvent == 2)   // LocScY or window size was changed or size
    {
        KillTimer(nIDEvent);
        UpdateContainerWnds();
    }
    else if (nIDEvent == 3)
    {
        KillTimer(nIDEvent);
        if (m_show_legend)
        {
            ShowNamesLegend();
        }
    }
}

void CChartContainer::OnKillFocus(CWnd* pNewWnd)
{
    CWnd::OnKillFocus(pNewWnd);
    RefreshWnd();
}

void CChartContainer::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);
    RefreshWnd();
}

// Context menu functions
void CChartContainer::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    if (!m_bUserEnabled)
        return;
    if (m_mapCharts.empty())
        return;

    bool bHasChartWithData = HasChartWithData(-1, true);

    CMenu popMenu;
    popMenu.CreatePopupMenu();
    if (bHasChartWithData)
    {
        CMenu popSubMenu1;
        popSubMenu1.CreatePopupMenu();

        UINT chEnabledLeg = IsLabWndVisible(true) ? MF_CHECKED : MF_UNCHECKED;
        popSubMenu1.AppendMenu(MF_ENABLED | chEnabledLeg, IDM_CHART_LEGEND, _T("Show Legend"));
        auto enabled = MF_ENABLED;
        if (!m_show_legend)
        {
            enabled = MF_DISABLED;
        }
        popSubMenu1.AppendMenu(enabled, IDM_CHART_LEGEND_MOVE, _T("Move Legend"));

        popMenu.AppendMenu(MF_STRING | MF_POPUP, UINT(popSubMenu1.m_hMenu), _T("Legend"));
        popMenu.AppendMenu(MF_SEPARATOR);

        UINT chShowBounds = m_bAxBoundaries ? MF_CHECKED : MF_UNCHECKED;
        popMenu.AppendMenu(MF_ENABLED | chShowBounds, IDM_CHART_BOUNDARIES, _T("Show Axis X boundaries"));
        popMenu.AppendMenu(MF_SEPARATOR);

        UINT chXIsLog = m_x_axis_is_logarithmic ? MF_CHECKED : MF_UNCHECKED;
        popMenu.AppendMenu(MF_ENABLED | chXIsLog, IDM_CHART_X_AXIS, _T("Logarithmic Scale for X-Axis"));
        UINT chYIsLog = m_y_axis_is_logarithmic ? MF_CHECKED : MF_UNCHECKED;
        popMenu.AppendMenu(MF_ENABLED | chYIsLog, IDM_CHART_Y_AXIS, _T("Logarithmic Scale for Y-Axis"));
        popMenu.AppendMenu(MF_SEPARATOR);

        popMenu.AppendMenu(MF_ENABLED, IDM_CHART_VIEWDATA, _T("View Chart Data"));
    }

    CMenu popSubMenu1;
    popSubMenu1.CreatePopupMenu();

    popSubMenu1.AppendMenu(MF_ENABLED, IDM_CHART_SAVE, _T("Save chart(s)"));
    if (bHasChartWithData)
    {
        popSubMenu1.AppendMenu(MF_ENABLED, IDM_CHART_SAVEIMAGE, _T("Save Charts As Image"));
        popSubMenu1.AppendMenu(MF_SEPARATOR);
        popSubMenu1.AppendMenu(MF_ENABLED, IDM_CHART_PRINT, _T("Print charts"));
    }

    string_t subMnStr(_T("Save"));
    if (bHasChartWithData)
        subMnStr += string_t(_T("/Print"));
    subMnStr += string_t(_T(" Charts"));

    popMenu.AppendMenu(MF_STRING | MF_POPUP, UINT(popSubMenu1.m_hMenu), subMnStr.c_str());
    popMenu.AppendMenu(MF_SEPARATOR);

    if (bHasChartWithData)
    {
        CMenu popSubMenu2;
        popSubMenu2.CreatePopupMenu();

        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_ZOOMINX, _T("Zoom In X "));
        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_MOVERIGHT, _T("Move Right"));
        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_MOVELEFT, _T("Move Left"));
        popSubMenu2.AppendMenu(MF_SEPARATOR);
        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_ZOOMINY, _T("Zoom In Y"));
        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_MOVEUP, _T("Move Up"));
        popSubMenu2.AppendMenu(MF_ENABLED, IDM_CHART_MOVEDOWN, _T("Move Down"));

        popMenu.AppendMenu(MF_STRING | MF_POPUP, UINT(popSubMenu2.m_hMenu), _T("Zoom/Move"));
        popMenu.AppendMenu(MF_SEPARATOR);

        if (!m_vHistX.empty())
        {
            popMenu.AppendMenu(MF_ENABLED, IDM_CHART_UNDOZOOMMOVEX, _T("Undo Last Zoom/Move X"));
            popMenu.AppendMenu(MF_SEPARATOR);
        }

        if (!m_vHistY.empty())
        {
            popMenu.AppendMenu(MF_ENABLED, IDM_CHART_UNDOZOOMMOVEY, _T("Undo Last Zoom/Move Y"));
            popMenu.AppendMenu(MF_SEPARATOR);
        }

        if (!m_vHistX.empty() || !m_vHistY.empty())
        {
            popMenu.AppendMenu(MF_ENABLED, IDM_CHART_REFRESH, _T("Refresh Charts"));
            popMenu.AppendMenu(MF_SEPARATOR);
        }

        // If the mouse has no middle button replace middle button clicks
        if (GetSystemMetrics(SM_CMOUSEBUTTONS) < 3)
        {
            UINT chTracking = m_bTracking ? MF_CHECKED : MF_UNCHECKED;
            popMenu.AppendMenu(MF_ENABLED | chTracking, IDM_CHART_TRACK, _T("Enable Tracking"));

        }

        UINT chShowCharts = MF_CHECKED;         // By default points are shown
        if (m_chartSelIdx != -1)
        {
            CChart* selChartPtr = GetChart(m_chartSelIdx);
            if (!selChartPtr->ArePntsAllowed())
                chShowCharts = MF_UNCHECKED;
        }

        if (HasChartDifferentPnts(-1) > -1)
        {
            popMenu.AppendMenu(MF_ENABLED | chShowCharts, IDM_CHART_SHOWPNTS, _T("Show/Hide Chart Pnts"));
            popMenu.AppendMenu(MF_SEPARATOR);
        }
    }

    // Add CChart names (m_mapCharts is not empty)
    string_t mnPrefix1(_T("Show "));
    string_t mnSuffix2(_T(" has no data"));
    string_t mnItemName;
    UINT chkFlag = MF_CHECKED;
    UINT cmdID = IDM_CHART_FIRSTCHART;
    MAP_CHARTS::iterator it = m_mapCharts.begin();
    MAP_CHARTS::iterator itE = m_mapCharts.end();
    for (; it != itE; ++it)
    {
        bool bAllowedToBeDrawn = it->second->HasData();
        UINT enableFlag = bAllowedToBeDrawn ? MF_ENABLED : MF_DISABLED;
        chkFlag = (it->second->IsChartVisible()) ? MF_CHECKED : MF_UNCHECKED;
        if (bAllowedToBeDrawn)
            mnItemName = mnPrefix1 + it->second->GetChartName();
        else
            mnItemName = it->second->GetChartName() + mnSuffix2;
        popMenu.AppendMenu(enableFlag | chkFlag, cmdID + it->first, mnItemName.c_str());
    }
#ifdef _TEST_
    popMenu.AppendMenu(MF_SEPARATOR);
    popMenu.AppendMenu(MF_ENABLED, IDM_CHART_TEST, _T("Test"));
#endif
    popMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
    popMenu.Detach();
}

void CChartContainer::OnMnShowLegend(void)
{
    if (IsLabWndVisible(true))
    {
        m_pLegWnd = DestroyLabWnd(m_pLegWnd);
        m_show_legend = false;
    }
    else
    {
        ShowNamesLegend();
    }
}

void CChartContainer::OnMnMoveLegend(void)
{
    // Unfortunately, scoped enums are not guarenteed to be continuous so no ++ operator.
    switch (m_legendLocation)
    {
    case LegendLocation::Upper_Right:
        m_legendLocation = LegendLocation::Upper_Left;
        break;
    case LegendLocation::Upper_Left:
        m_legendLocation = LegendLocation::Lower_Left;
        break;
    case LegendLocation::Lower_Left:
        m_legendLocation = LegendLocation::Lower_Right;
        break;
    case LegendLocation::Lower_Right:
    default:
        m_legendLocation = LegendLocation::Upper_Right;
        break;
    }

    OnMnShowLegend();   // Hide the legend.
    OnMnShowLegend();   // Show it in its new location.
}

void CChartContainer::OnMnShowAxBoundaries(void)
{
    m_bAxBoundaries = !m_bAxBoundaries;
    RefreshWnd();
}

// SGS
void CChartContainer::OnMnXAxisIsLogarithmic(void)
{
    m_x_axis_is_logarithmic = !m_x_axis_is_logarithmic;
    RefreshWnd();
}

void CChartContainer::OnMnYAxisIsLogarithmic(void)
{
    m_y_axis_is_logarithmic = !m_y_axis_is_logarithmic;
    RefreshWnd();
}

void CChartContainer::OnMnRefresh(void)
{
    ResetCharts();
    SendNotification(CODE_REFRESH);
}

void CChartContainer::OnMnViewData(void)
{
    if (m_mapCharts.empty())
    {
        AfxMessageBox(_T("Chart Container Is Empty"));
        return;
    }

    // Send chart to the data view dlg
    int chartSelIdx = HasSelectedChart();

    if (chartSelIdx == -1)
        chartSelIdx = SelectChart(-1);

    if (chartSelIdx == -1)
    {
        AfxMessageBox(_T("View Chart Data: Has no visible or selected chart"));
        return;
    }

    MAP_CHARTS::iterator it = m_mapCharts.find(chartSelIdx);

    if (it == m_mapCharts.end())
        AfxMessageBox(_T("View Chart Data: Can't find selected chart"));

    ShowDataView(it->second, true);
}

void CChartContainer::OnMnSaveCharts(void)
{
    if (S_FALSE == SaveChartData(string_t(_T(""))))
    {
        AfxMessageBox(_T("Save Failed\n Select the visible chart to save it\n")
            _T("or remove the selection to save all visible charts\n")
            _T("and try again"), MB_OK | MB_ICONERROR);
    }
}

void CChartContainer::OnMnSaveImage(void)
{
    Status stat = SaveContainerImage();

    if (stat != Ok)
    {
        string_t errStr(_T("Save Image Failed:\n"));
        switch (stat)
        {
        case InvalidParameter:   errStr += string_t(_T("Cannot get file path name")); break;
        case UnknownImageFormat: errStr += string_t(_T("Invalid Image Format"));      break;
        default:                 errStr += string_t(_T("Unknown Error"));             break;
        }
        AfxMessageBox(errStr.c_str(), MB_OK | MB_ICONERROR);
    }
}

void CChartContainer::OnMnPrintCharts(void)
{
    if (!HasChartWithData(-1, true))
    {
        AfxMessageBox(_T("Has no visible charts to print"), MB_OK | MB_ICONERROR);
        return;
    }

    CPrintDialog printDlg(FALSE, PD_USEDEVMODECOPIES | PD_HIDEPRINTTOFILE | PD_NOSELECTION | PD_RETURNDC);
    printDlg.m_pd.Flags &= ~PD_SHOWHELP;
    printDlg.m_pd.nMinPage = printDlg.m_pd.nMaxPage = 1;
    printDlg.m_pd.nFromPage = printDlg.m_pd.nToPage = 1;

    if (printDlg.DoModal() == IDOK)
    {
        int scrDpiX = GetScreenDpi();
        SendNotification(CODE_PRINTING);
        CChartContainer* pContainer = CloneChartContainer(string_t(_T("")), true);
        SendNotification(CODE_PRINTED);
        PrintCharts(pContainer, scrDpiX, printDlg.GetPrinterDC());
        delete pContainer;
    }
    GlobalFree(printDlg.m_pd.hDevMode);
    GlobalFree(printDlg.m_pd.hDevNames);
}

void CChartContainer::OnMnZoomInX(void)  // ZoomIn from the context menu
{
    if (m_startX >= m_endX)
        return;

    double fStartZX, fEndZX;
    double extX = m_endX - m_startX;

    if (m_chModeX == MODE_ZOOMINGX)         // Left zoom border was set
    {
        double distStartX = m_tmpZX - m_startX;
        double distEndX = m_endX - m_tmpZX;
        if (distStartX < distEndX)
        {
            fStartZX = m_tmpZX;
            fEndZX = m_endX - distStartX;
        }
        else if (distStartX > distEndX)
        {
            fStartZX = m_startX + distEndX;
            fEndZX = m_tmpZX;
        }
        else
        {
            fStartZX = m_tmpZX + 0.1 * extX;
            fEndZX = m_tmpZX - 0.1 * extX;
        }
    }
    else
    {
        fStartZX = m_startX + 0.1 * extX;
        fEndZX = m_endX - 0.1 * extX;
    }

    ZoomContainerX(fStartZX, fEndZX, true);
    SendNotification(CODE_EXTX);
}

void CChartContainer::OnMnMoveRight(void)
{
    // Move right
    double deltaX = 10.0 * MOVE_DELTAX * (m_endX - m_startX);
    MoveContainerChartsX(deltaX, true);
    SendNotification(CODE_EXTX);
}

void CChartContainer::OnMnMoveLeft(void)
{
    // Move left
    double deltaX = -10.0 * MOVE_DELTAX * (m_endX - m_startX);
    MoveContainerChartsX(deltaX, true);
    SendNotification(CODE_EXTX);
}

void CChartContainer::OnMnZoomInY(void)
{
    if (m_minExtY >= m_maxExtY)
        return;

    double fStartZY, fEndZY;
    double extY = m_maxExtY - m_minExtY;

    if (m_chModeY == MODE_ZOOMINGY)         // Left zoom border was set
    {
        double distStartY = m_tmpZY - m_minExtY;
        double distEndY = m_maxExtY - m_tmpZY;
        if (distStartY < distEndY)
        {
            fStartZY = m_tmpZY;
            fEndZY = m_maxExtY - distStartY;
        }
        else if (distStartY > distEndY)
        {
            fStartZY = m_minExtY + distEndY;
            fEndZY = m_tmpZY;
        }
        else
        {
            fStartZY = m_minExtY;
            fEndZY = m_maxExtY;
            if (!m_vHistY.empty())
            {
                fStartZY += 0.1 * extY;
                fEndZY -= 0.1 * extY;
            }
        }
    }
    else
    {
        fStartZY = m_minExtY;
        fEndZY = m_maxExtY;
        if (fStartZY != fEndZY)
        {
            if (m_chModeY != MODE_FULLY)
            {
                fStartZY += 0.1 * extY;
                fEndZY -= 0.1 * extY;
            }
        }
    }

    ZoomContainerY(fStartZY, fEndZY, true);
    SendNotification(CODE_EXTY);
}

void CChartContainer::OnMnMoveUp(void)
{
    // Move Up
    double deltaY = 10.0 * MOVE_DELTAY * (m_maxExtY - m_minExtY);
    MoveContainerChartsY(deltaY, true);
    SendNotification(CODE_EXTY);
}

void CChartContainer::OnMnMoveDown(void)
{
    double deltaY = -10.0 * MOVE_DELTAY * (m_maxExtY - m_minExtY);
    MoveContainerChartsY(deltaY, true);
    SendNotification(CODE_EXTY);
}

void CChartContainer::OnMnUndoHistStepX(void)
{
    UndoHistStepX(true);
    SendNotification(CODE_EXTX);
}

void CChartContainer::OnMnUndoHistStepY(void)
{
    UndoHistStepY(true);
    SendNotification(CODE_EXTY);
}

void CChartContainer::OnMnEnableTracking(void)
{
    m_bTracking = !m_bTracking;
    if (!m_bTracking)
    {
        m_pDataWnd = DestroyLabWnd(m_pDataWnd);
        m_dataLegPntD = PointD(DBL_MAX, DBL_MAX);
    }
    SendNotification(CODE_TRACKING);
}

void CChartContainer::OnMnShowPnts(void)
{
    int chartIdx = -1;
    CChart* chartPtr = nullptr;
    if (m_chartSelIdx == -1)
    {
        chartIdx = HasChartDifferentPnts(-1, true);
        if (chartIdx != -1)
        {
            m_chartSelIdx = SelectChart(chartIdx);
        }
    }
    else
        chartIdx = HasChartDifferentPnts(m_chartSelIdx, true);

    if (chartIdx == -1)
    {
        AfxMessageBox(_T("Select a chart with enough data points first"), MB_ICONERROR | MB_OK);
        return;
    }

    chartPtr = GetChart(m_chartSelIdx);
    ENSURE(chartPtr != nullptr);
    bool bShow = chartPtr->ArePntsAllowed();
    chartPtr->AllowPnts(!bShow);
    //chartPtr->SelectChart(false);
    UpdateContainerWnds();
    SendNotification(CODE_SHOWPNTS, m_chartSelIdx);
}

void CChartContainer::OnMnShowChart(UINT nCmdID)
{
    int chartIdx = nCmdID - IDM_CHART_FIRSTCHART;
    if (ToggleChartVisibility(chartIdx) > -1)
        SendNotification(CODE_VISIBILITY, chartIdx);
}

#ifdef _TEST_

void CChartContainer::OnMnTest(void)
{
    //  PAIR_DBLS pd = GetMinMaxExtX();
    //  SetContainerPrecision(8, true);
    //  CChartContainer* chartContainer = /*CloneChartContainer(string_t(_T("")), true); //*/new CChartContainer;
    //  *chartContainer = *this;
    //  chartContainer->CreateChartCtrlWnd(0, WS_POPUP|WS_OVERLAPPEDWINDOW|WS_VISIBLE, CRect(100, 100, 500, 400),NULL, NULL);
    //  chartContainer.ShowWindow(SW_SHOW);
    //    V_CHARTDATAD vReplaceD;

    //    V_CHARTDATAD* vChartDataD = GetChartData(1);
    //    vReplaceD.assign(vChartDataD->begin(), vChartDataD->end());
    //    ReplaceChartData(0, vReplaceD, false, false, false, true);
    //    RemoveChart(1, true, true);
    //  vReplaceD.push_back(PointD(5.0, 1.0));
    //  V_CHARTDATAD vAppendD;
    //  vAppendD.assign(vChartDataD->begin(), vChartDataD->end());
    //  AppendChartData(1, vAppendD, true);
    //  TruncateChart(0, vChartDataD->begin()->X, (vChartDataD->begin() + 1)->X, false, true);
    //  TruncateChart(1, 4.999, 8.0, true, true);
    //  RemoveChart(1, true, true);
    //  EqualizeVertRanges(0.95, true);
    //  SaveContainerImage();
    //  SaveChartData(string_t(_T("")), true);
    //  MAP_NAMES mapNames;
    //  GetChartNamesFromXMLFile(_T("C:\\VS2012\\Projects\\ChartCtrlLib\\Charts\\Test.xml"),  mapNames);
    //  auto it = mapNames.begin();
    //  ++it;
    //  TUPLE_NAMES tN = it->second;
    //  string_t name1;
    //  string_t name2;
     // string_t name3;
     // string_t name4;
     // std::tie(name1, name2, name3, name4) = tN;
     // ReplaceContainerCharts(_T("C:\\VS2012\\Projects\\ChartCtrlLib\\Charts\\Test.xml"));
    //  SetChartVisibility(2, false, true);
    //  ToggleChartVisibility(2);
    //  UpdateChartPenWidth(-1, 4.0f, true);
    //  SetChartTension(-1, 1.0f, true);
    //  std::vector<double> vDt;
    //  vDt.push_back(0.465);
    //  vDt.push_back(0.865);
    //  vDt.push_back(0.665);

    //  AddChart(true, true, string_t(_T("")), string_t(_T("Zone")), 5, DashStyleSolid, 2.0f, 0.4f,
    //    Color(ARGB(Color::Green)), vDt, 5.0, 1.0, true);
    //  ResetCharts();

    EqualizeVertRanges(0.9, true);
    //  AppendChartData(1, vDt, 0.0, 6.0);//, 0, 1, true);
    //  m_dataLegPntD = PointD(5.0, 0.0);
    //  ShowDataLegend(5.0);
    //  ShowNamesLegend();
    //  ShowDataView(GetChartName(2), true);

    //  CPrintDialog prnDlg(FALSE);
    //  BOOL bRes = prnDlg.GetDefaults();
    //  PrintCharts(this, 300, prnDlg.m_pd.hDC);

    //    ShowAxisXBoundaries(true, true);

    //  ResetCharts();
    //  ZoomMoveContainer(-5.56, 4.389, true);
    //  ShowChartPoints(1, false, true);
    //  ToggleChartVisibility(1);
    //  bool bEnable = !m_bUserEnabled;
    //  EnableUser(bEnable);
    // UpdateChartLocScaleY(-1, 1.5, false, true);
    //  string_t name = GetChartName(1);
    //  ShowDataView(name, true);
    //  SetLegBkgndColor(Color(32, 230, 0, 0), true);
    Color color;
}

#endif

void CChartContainer::UpdateDataViewPnts(int chartIdx, size_t dataID, PointD dataPntD, bool bAdd)
{
    bool bRedraw = true;

    if (chartIdx == -1)
    {
        m_mapDataViewPntsD.clear();
    }
    else if (m_dataViewChartIdx == chartIdx)
    {
        CChart* chartPtr = GetChart(chartIdx);
        if (chartPtr != nullptr) // Chart is not deleted
        {
            if (bAdd)
                m_mapDataViewPntsD.insert(MAP_SELPNTSD::value_type(dataID, dataPntD));
            else
            {
                MAP_SELPNTSD::iterator it = m_mapDataViewPntsD.find(dataID);
                if (it != m_mapDataViewPntsD.end())
                    m_mapDataViewPntsD.erase(dataID);
                else
                    bRedraw = false;  // Not found the data point
            }
        }
        else                    // Chart was removed, cleanup did not finished
        {
            m_mapDataViewPntsD.clear();
            m_dataViewChartIdx = -1;
            bRedraw = false;      // Chart was redrawn earlier
        }
    }   // Received right chartIdx
    else
        bRedraw = false;

    if (bRedraw && (nullptr != GetVisibleChart(m_dataViewChartIdx)))
        RefreshWnd();
}

void CChartContainer::UpdateDataViewPnts(int chartIdx, const MAP_SELPNTSD& mapSelPntsD)
{

    if (chartIdx == m_dataViewChartIdx)
    {
        if (mapSelPntsD.size() > 0)
            m_mapDataViewPntsD = mapSelPntsD;
        else
            m_mapDataViewPntsD.clear();

        if (nullptr != GetVisibleChart(chartIdx))
            RefreshWnd();
    }
}

void CChartContainer::EqualizeVertRanges(double spaceMult, bool bRedraw)
{
    MAP_CHARTS mapCharts;
    MAP_CHARTS::const_iterator mapIt = m_mapCharts.cbegin();
    MAP_CHARTS::const_iterator mapItE = m_mapCharts.cend();

    for (; mapIt != mapItE; ++mapIt)
    {
        if (mapIt->second->HasData())
            mapCharts.insert(*mapIt);
    }

    // Get Range
    if (mapCharts.size() > 1)
    {
        typedef std::pair<double, CChart*> PAIR_CHR;
        typedef std::vector<PAIR_CHR> V_CHR;

        V_CHR vChr(mapCharts.size());
        transform(mapCharts.cbegin(), mapCharts.cend(), vChr.begin(),
            [](const std::pair<int, CChart*>& pair_ch) ->PAIR_CHR
            {CChart* pChart = pair_ch.second; double val = max(fabs(pChart->GetMaxValY()), fabs(pChart->GetMinValY()));
        return make_pair(val, pChart); });

        sort(vChr.begin(), vChr.end(), [](const PAIR_CHR& lhs, const PAIR_CHR& rhs) ->bool {return lhs.first > rhs.first; });

        double chMaxY = vChr[0].first;
        for_each(++vChr.begin(), vChr.end(),
            [&chMaxY, spaceMult](const PAIR_CHR& pair_chr) ->void {chMaxY = spaceMult * chMaxY; double locScY = chMaxY / pair_chr.first;
        pair_chr.second->SetLocScaleY(locScY); });

        double minY = GetMinY();
        double maxY = GetMaxY();
        switch (m_axYPos)
        {
        case TOP: maxY = 0.0; break;
        case MIDDLE: maxY = max(fabs(minY), maxY); minY = -maxY; break;
        case BOTTOM: minY = 0.0; break;
        }
        UpdateExtY(minY, maxY, bRedraw);
    }
}

void CChartContainer::SetImagePath(std::filesystem::path path)
{
    m_image_path = path;
}

void CChartContainer::setShowYAxisSecondScale(bool show_second_scale)
{
    m_show_y_axis_second_scale = show_second_scale;
}

void CChartContainer::GetClientRectBase(LPRECT lpRect) const
{
    CWnd::GetClientRect(lpRect);
}

void CChartContainer::GetClientRectAccountingForLabels(LPRECT lpRect)
{
    GetClientRectBase(lpRect);

    if (m_showXLabel)
    {
        // 10 is an arbitrary number but works with 13 pt fonts.
        lpRect->left += 10;
    }

    if (m_showYLabel)
    {
        // Adjust the chart rectangle to give space for the X axis label
        lpRect->bottom -= 10;
    }
}

void CChartContainer::setYAxisLabel(string_t label /*= string_t(_T(""))*/)
{
    m_y_axis_label = label;
    m_showYLabel = true;
}
