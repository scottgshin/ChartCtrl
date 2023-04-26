// ChartDataView.cpp : implementation file
//

#include "stdafx.h"
#include "Chart.h"
#include "ChartContainer.h"
#include "ChartDataView.h"
#include "Util.h"

using namespace std;
using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// CPageCtrl implementation


IMPLEMENT_DYNAMIC(CPageCtrl, CButton)

CPageCtrl::CPageCtrl(bool bEnd, bool bRotate, UINT idx) : m_bEnd(bEnd),
m_bRotate(bRotate),
m_bOnButton(false),
m_idx(idx)
{

}

CPageCtrl::~CPageCtrl()
{
}

void CPageCtrl::DrawArrow(RectF arrRF, Graphics* grPtr)
{
    GraphicsPath grPath;
    CreateRoundedRect(grPath, arrRF, 2.0f, false);

    SolidBrush br((ARGB)Color::Black);
    grPtr->FillPath(&br, &grPath);
    grPath.Reset();

    arrRF.Inflate(-4.0f, -4.0f);
    CreateRoundedRect(grPath, arrRF, 2.0f, false);
    if (IsWindowEnabled())
    {
        if (m_bOnButton)
            br.SetColor(Color(0xFF, 0, 172, 0));
        else
            br.SetColor(Color(0xFF, 0, 128, 0));
    }
    else
        br.SetColor((ARGB)Color::LightGray);

    grPtr->FillPath(&br, &grPath);
    Pen pen((ARGB)Color::White, 2.0f);
    pen.SetAlignment(PenAlignmentInset);
    grPtr->DrawPath(&pen, &grPath);

    Matrix matrix;
    if (m_bRotate)
    {
        PointF pntCenterF = GetRectFCenterF(arrRF);
        matrix.RotateAt(180.0f, pntCenterF);
        grPtr->SetTransform(&matrix);
    }

    pen.SetWidth(3.0f);
    PointF pntStartF(arrRF.X + 4.0f, arrRF.Y + arrRF.Height / 2.0f);
    PointF pntEndTopF(pntStartF.X + 0.4f * arrRF.Height, arrRF.Y + 3.0f);
    PointF pntEndMiddleF(arrRF.GetRight() - 4.0f, pntStartF.Y);
    PointF pntEndBottomF(pntStartF.X + 0.4f * arrRF.Height, arrRF.GetRight() - 4.0f);
    grPtr->DrawLine(&pen, pntStartF, pntEndMiddleF);
    grPtr->DrawLine(&pen, pntStartF, pntEndTopF);
    grPtr->DrawLine(&pen, pntStartF, pntEndBottomF);
    if (m_bEnd)
        grPtr->DrawLine(&pen, pntStartF.X, pntEndTopF.Y - 1.0f, pntStartF.X,
            pntEndBottomF.Y + 2.0f);
    if (!matrix.IsIdentity())
        grPtr->ResetTransform();
}

BEGIN_MESSAGE_MAP(CPageCtrl, CButton)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_PAINT()
END_MESSAGE_MAP()

// CPageCtrl message handlers

void CPageCtrl::OnPaint()
{
    CPaintDC dc(this);
    Graphics gr(dc);

    RectF rGdiF;
    gr.GetVisibleClipBounds(&rGdiF);

    DrawArrow(rGdiF, &gr);

}

void CPageCtrl::OnMouseMove(UINT, CPoint)
{
    TRACKMOUSEEVENT     csTME;
    if (!m_bOnButton)
    {
        m_bOnButton = true;

        csTME.cbSize = sizeof(csTME);
        csTME.dwFlags = TME_LEAVE;
        csTME.hwndTrack = m_hWnd;
        TrackMouseEvent(&csTME);

        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }
} // End of OnMouseMove

LRESULT CPageCtrl::OnMouseLeave(WPARAM, LPARAM)
{
    if (m_bOnButton)
    {
        m_bOnButton = false;
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }
    return 0L;
}

///////////////////////////////////////////////////////////////////////////////
// CChartDataView

IMPLEMENT_DYNAMIC(CChartDataView, CWnd)

CChartDataView::CChartDataView()
{
}

CChartDataView::~CChartDataView()
{
}

void CChartDataView::InitParams(const CChart* chartPtr, bool bClearMap, const CChartContainer* pHost)
{
    m_chartIdx = chartPtr->GetChartIdx();
    m_precision = pHost->GetContainerPrecisionX();
    m_precisionY = chartPtr->GetPrecisionY();
    m_label = chartPtr->GetChartName();
    string_t tmpStr = pHost->GetAxisXName();
    m_labelX = tmpStr.empty() ? string_t(_T("X")) : tmpStr;
    tmpStr = chartPtr->GetAxisYName();
    m_labelY = tmpStr.empty() ? string_t(_T("Y")) : tmpStr;
    m_pXLabelStrFn = pHost->GetLabXValStrFnPtr();
    m_pYLabelStrFn = chartPtr->GetLabYValStrFnPtr();
    m_vDataPnts = chartPtr->m_vDataPnts;

    m_vStrX.resize(m_vDataPnts.size());
    transform(m_vDataPnts.begin(), m_vDataPnts.end(), m_vStrX.begin(),
        nmb_to_string<double, false>(m_precision, m_pXLabelStrFn));
    m_vStrY.resize(m_vDataPnts.size());
    transform(m_vDataPnts.begin(), m_vDataPnts.end(), m_vStrY.begin(),
        nmb_to_string<double, true>(m_precisionY, m_pYLabelStrFn));
    m_currPageID = 0;

    SetOwner((CWnd*)pHost);
    m_vRows.clear();
    if (bClearMap)    // TODO: remove bClear
        m_mapSelCells.clear();
    else
        UpdateDataIdx();

    CalcLayout();
    m_header = GetTableHeader();   // Set the header string

    CreateChildren();

    bool bEnableLeft = m_currPageID == 0 ? false : true;
    bool bEnableRight = m_nPages == 1 ? false : true;

    m_leftEnd.EnableWindow(bEnableLeft ? TRUE : FALSE);
    m_leftArr.EnableWindow(bEnableLeft ? TRUE : FALSE);

    m_rightArr.EnableWindow(bEnableRight ? TRUE : FALSE);
    m_rightEnd.EnableWindow(bEnableRight ? TRUE : FALSE);

    if (IsWindow(m_hWnd) && IsWindowVisible())
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE | RDW_ALLCHILDREN);
}

bool CChartDataView::UpdateParams(const CChart* chartPtr, int flagsData) // TODO: Param bClearMap
{
    bool bRes = false;
    int flags = 0;
    size_t dataOffset = 0;

    int chartIdx = chartPtr->GetChartIdx();
    if (chartIdx == m_chartIdx)
    {
        CChartContainer* pHost = dynamic_cast<CChartContainer*>(GetOwner());
        ENSURE(pHost != NULL);
        if (!chartPtr->HasData())
        {
            pHost->DestroyChartDataView();
            return true;
        }

        m_label = chartPtr->GetChartName();    // Page header must be changed with new date stamp

        int precisionX = pHost->GetContainerPrecisionX();
        if (m_precision != precisionX)        // PrecisionX: entire X column is changing
        {
            m_precision = precisionX;
            flags |= F_VALX;
        }

        int precisionY = chartPtr->GetPrecisionY();
        if (m_precisionY != precisionY)       // PrecisionY: entire X column is changing
        {
            m_precisionY = precisionY;
            flags |= F_VALY;
        }

        string_t tmpStr = pHost->GetAxisXName();
        string_t labelX = tmpStr.empty() ? string_t(_T("X")) : tmpStr;
        if (m_labelX != labelX)               // X-axis name: column header and width might change
        {
            m_labelX = labelX;
            flags |= F_NAMEX;
        }

        tmpStr = chartPtr->GetAxisYName();
        string_t labelY = tmpStr.empty() ? string_t(_T("Y")) : tmpStr;
        if (m_labelY != labelY)               // Y-axis name: column header and width might change
        {
            m_labelY = labelY;
            flags |= F_NAMEY;
        }

        val_label_str_fn pXLabelStrFn = pHost->GetLabXValStrFnPtr();
        if (m_pXLabelStrFn != pXLabelStrFn)   // Entire X-column should be changed
        {
            m_pXLabelStrFn = pXLabelStrFn;
            flags |= F_VALX;
        }

        val_label_str_fn pYLabelStrFn = chartPtr->GetLabYValStrFnPtr();
        if (m_pYLabelStrFn != pYLabelStrFn)   // Entire Y-column should be changed
        {
            m_pYLabelStrFn = pYLabelStrFn;
            flags |= F_VALY;
        }

        if (flagsData != F_NODATACHANGE)
        {
            size_t endOffs = 0;
            switch (flagsData)
            {
            case F_APPEND:
                endOffs = OnChartAppended(chartPtr->m_vDataPnts);
                if (!(flags & (F_VALX | F_VALY | F_DSIZE)))
                {
                    dataOffset = endOffs;
                }
                flags |= (F_VALX | F_VALY | F_DSIZE);
                break;
            case F_TRUNCATE:
                endOffs = OnChartTruncated(chartPtr->m_vDataPnts);
                if (!(flags & (F_VALX | F_VALY | F_DSIZE)))
                {
                    dataOffset = 0;
                }
                flags |= (F_VALX | F_VALY | F_DSIZE);
                break;
            case F_REPLACE:
            case F_REPLACE | F_HASCELLSMAP:
                dataOffset = OnChartDataReplaced(chartPtr->m_vDataPnts, flags & F_HASCELLSMAP ? true : false);
                flags |= (F_VALX | F_VALY | F_DSIZE);
                break;
            }
        }
        else
        {
            if (flags & F_VALX)
            {
                transform(m_vDataPnts.begin() + dataOffset, m_vDataPnts.end(),
                    m_vStrX.begin() + dataOffset, nmb_to_string<double, false>(m_precision, m_pXLabelStrFn));
            }

            if (flags & F_VALY)
            {
                transform(m_vDataPnts.begin() + dataOffset, m_vDataPnts.end(),
                    m_vStrY.begin() + dataOffset, nmb_to_string<double, true>(m_precisionY, m_pYLabelStrFn));
            }
        }

        if (IsIconic())
            ShowWindow(SW_RESTORE);

        // Set the header string
        m_header = GetTableHeader();

        m_vRows.clear();
        if ((flags != 0) && (dataOffset != m_vDataPnts.size()))
            CalcLayout(flags, dataOffset);

        if (flagsData != F_NODATACHANGE)
        {
            if (flagsData & F_TRUNCATE)
            {
                if (m_nPages <= m_currPageID)
                    m_currPageID = 0;
            }
            else if ((flagsData & F_APPEND) == 0)
                m_currPageID = 0;
        }
        else
            m_currPageID = 0;


        bool bEnableLeft = m_currPageID == 0 ? false : true;
        bool bEnableRight = (m_currPageID == (m_nPages - 1)) ? false : true;

        m_leftEnd.EnableWindow(bEnableLeft ? TRUE : FALSE);
        m_leftArr.EnableWindow(bEnableLeft ? TRUE : FALSE);

        m_rightArr.EnableWindow(bEnableRight ? TRUE : FALSE);
        m_rightEnd.EnableWindow(bEnableRight ? TRUE : FALSE);

        if (IsWindow(m_hWnd) && IsWindowVisible())
            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE | RDW_ALLCHILDREN);
        bRes = true;
    }
    return bRes;
}

size_t CChartDataView::OnChartAppended(const V_CHARTDATAD& vChartData)
{
    size_t oldDataSize = m_vDataPnts.size();     // Change in chart's data vector
    size_t newDataSize = vChartData.size();
    size_t dataOffset = newDataSize;
    if (newDataSize > oldDataSize)
    {
        m_vDataPnts.resize(newDataSize);
        m_vStrX.resize(m_vDataPnts.size());
        m_vStrY.resize(m_vDataPnts.size());

        V_CHARTDATAD::const_iterator itStart = vChartData.cbegin();
        V_CHARTDATAD::const_iterator itEnd = vChartData.cend();
        V_CHARTDATAD::iterator itSource = m_vDataPnts.begin();

        dataOffset = oldDataSize;
        itStart += dataOffset;
        itSource += dataOffset;

        copy(itStart, itEnd, itSource);
        transform(m_vDataPnts.begin() + dataOffset, m_vDataPnts.end(),
            m_vStrX.begin() + dataOffset, nmb_to_string<double, false>(m_precision, m_pXLabelStrFn));
        transform(m_vDataPnts.begin() + dataOffset, m_vDataPnts.end(),
            m_vStrY.begin() + dataOffset, nmb_to_string<double, true>(m_precisionY, m_pYLabelStrFn));
    }
    return dataOffset;
}

size_t CChartDataView::OnChartTruncated(const V_CHARTDATAD& vChartData)
{
    V_CHARTDATAD::iterator itStart, itEnd, itB = m_vDataPnts.begin(), itE = m_vDataPnts.end();
    PointD startPntD = vChartData.front();
    PointD endPntD = vChartData.back();

    itStart = find(itB, itE, startPntD);
    ENSURE(itStart != itE);
    itEnd = find(itB, itE, endPntD);

    size_t diffStart = distance(itB, itStart);
    size_t diffEnd = distance(itB, itEnd);

    m_vDataPnts.erase(itEnd + 1, itE);
    m_vDataPnts.erase(itB, itStart);
    m_vDataPnts.shrink_to_fit();

    m_vStrX.erase(m_vStrX.begin() + diffEnd + 1, m_vStrX.end());
    m_vStrX.erase(m_vStrX.begin(), m_vStrX.begin() + diffStart);
    m_vStrX.shrink_to_fit();

    m_vStrY.erase(m_vStrY.begin() + diffEnd + 1, m_vStrY.end());
    m_vStrY.erase(m_vStrY.begin(), m_vStrY.begin() + diffStart);
    m_vStrY.shrink_to_fit();

    UpdateDataIdx();
    return m_vDataPnts.size();
}

size_t CChartDataView::OnChartDataReplaced(const V_CHARTDATAD& vChartData, bool bHasCellsMap)
{
    size_t dataSize = vChartData.size();
    m_vDataPnts.assign(vChartData.cbegin(), vChartData.cend());
    m_vDataPnts.shrink_to_fit();
    m_vStrX.resize(dataSize);
    transform(m_vDataPnts.begin(), m_vDataPnts.end(),
        m_vStrX.begin(), nmb_to_string<double, false>(m_precision, m_pXLabelStrFn));
    m_vStrY.resize(dataSize);
    transform(m_vDataPnts.begin(), m_vDataPnts.end(),
        m_vStrY.begin(), nmb_to_string<double, true>(m_precisionY, m_pYLabelStrFn));
    if (!bHasCellsMap)
        RefreshSelCells();
    return 0;
}

// Call it if the displayed chart was changed
size_t CChartDataView::UpdateDataIdx(void)
{
    double startX = m_vDataPnts.front().X;
    double endX = m_vDataPnts.back().X;

    MAP_SELPNTSD mapSelPntsD;
    MAP_SELPNTSD::iterator itMap = m_mapSelCells.begin();
    MAP_SELPNTSD::iterator itMapE = m_mapSelCells.end();

    V_CHARTDATAD::iterator itB = m_vDataPnts.begin();
    V_CHARTDATAD::iterator itE = m_vDataPnts.end();
    V_CHARTDATAD::iterator it = itB;

    for (; itMap != itMapE; ++itMap)
    {
        PointD pntD = itMap->second;
        if (in_range(startX, endX, pntD.X))
        {
            int diff = 0;
            it = find(it, itE, pntD);
            if (it != itE)
            {
                diff = distance(itB, it);
                mapSelPntsD.insert(MAP_SELPNTSD::value_type(int(diff), pntD));
                ++it;
                if (it == itE)
                    break;
            }
            else
                it = itB + diff;
        }
    }
    bool bUpdateContainer = mapSelPntsD.size() < m_mapSelCells.size() ? true : false;

    m_mapSelCells = mapSelPntsD;
    if (bUpdateContainer)
    {
        CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
        if (pContainer)
            pContainer->UpdateDataViewPnts(m_chartIdx, m_mapSelCells);
    }
    return m_mapSelCells.size();
}

size_t CChartDataView::RefreshSelCells(void)
{
    double startX = m_vDataPnts.front().X;
    double endX = m_vDataPnts.back().X;

    MAP_SELPNTSD mapSelPntsD;
    MAP_SELPNTSD::iterator itMap = m_mapSelCells.begin();
    MAP_SELPNTSD::iterator itMapE = m_mapSelCells.end();

    V_CHARTDATAD::iterator itB = m_vDataPnts.begin();
    V_CHARTDATAD::iterator itE = m_vDataPnts.end();
    V_CHARTDATAD::iterator itStart = itB;
    V_CHARTDATAD::iterator it = itE;

    for (; itMap != itMapE; ++itMap)
    {
        PointD oldPntD = itMap->second;
        if (in_range(startX, endX, oldPntD.X))
        {
            it = find_if(itStart, itE, equal_coord<double, false>(oldPntD.X));
            if (it != itE)
            {
                mapSelPntsD.insert(MAP_SELPNTSD::value_type(distance(itB, it), *it));
                itStart = it + 1;
                if (itStart == itE)
                    break;
            }
        }
    }

    m_mapSelCells = mapSelPntsD;
    CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
    if (pContainer)
        pContainer->UpdateDataViewPnts(m_chartIdx, m_mapSelCells);
    return m_mapSelCells.size();
}

void CChartDataView::CalcLayout(void)
{
    CDC* pDC = GetDC();
    Graphics gr(pDC->m_hDC);

    gr.SetPageUnit(UnitDocument);  // 300 dpi/inch
    gr.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    RectF rGdiF;
    gr.GetVisibleClipBounds(&rGdiF);

    // Get the dpiRatio
    int scrDpiX = pDC->GetDeviceCaps(LOGPIXELSX);
    int scrDpiY = pDC->GetDeviceCaps(LOGPIXELSY);
    ENSURE(scrDpiX == scrDpiY);
    m_dpiRatio = 300.0f / scrDpiX;

    rGdiF.Width -= m_dpiRatio * 4.0f;
    rGdiF.Height -= m_dpiRatio;

#ifdef _UNICODE
    FontFamily fontFamily(m_fontName.c_str());
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W(m_fontName.c_str()));
#endif
    Gdiplus::Font headerFont(&fontFamily, m_headerFontHeight, FontStyleBold, UnitPoint);
    float headerHeight = 1.5f * headerFont.GetHeight(&gr);
    Gdiplus::Font textFont(&fontFamily, m_fontHeight, FontStyleBold, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);
    strFormat.SetLineAlignment(StringAlignmentCenter);
    float textStep = 1.5f * textFont.GetHeight(&gr);

    rGdiF.Inflate(-10.0f, -10.0f);
    m_headerRF = RectF(10.0, rGdiF.Y, rGdiF.Width, headerHeight);
    m_headerRF.Inflate(-50.0f, 0.0f);
    m_headerRF.Offset(0.0f, 0.75f * headerHeight);

    rGdiF.Inflate(0.0f, -headerHeight);
    rGdiF.Offset(0.0f, headerHeight);
    rGdiF.Height -= 120.0f;

    RectF nmbRF, valXRF, valYRF, labXRF, labYRF;

    // Number rect
    sstream_t stream;
    stream << m_vDataPnts.size() << _T("99\n");
    string_t strNmb;
    getline(stream, strNmb);
#ifdef _UNICODE
    gr.MeasureString(strNmb.c_str(), -1, &textFont, rGdiF, &strFormat, &nmbRF);
#else if _MBCS
    gr.MeasureString(CA2W(strNmb.c_str()), -1, &textFont, rGdiF, &strFormat, &nmbRF);
#endif


    m_vStrX.push_back(m_labelX);                // Temporary
    RectF maxXRF =
        for_each(m_vStrX.begin(), m_vStrX.end(), get_max_str<string_t, -1>(&textFont, &gr))._maxRF;
    m_vStrX.pop_back();                         // Have it removed

    m_vStrY.push_back(m_labelY);                // Temporary
    RectF maxYRF =
        for_each(m_vStrY.begin(), m_vStrY.end(), get_max_str<string_t, -1>(&textFont, &gr))._maxRF;
    m_vStrY.pop_back();                         // Have it removed

    float widthColX = maxXRF.Width + m_dpiRatio * DATA_ENDOFFS;
    float widthColY = maxYRF.Width + m_dpiRatio * DATA_ENDOFFS;

    float rowWidth = nmbRF.Width + widthColX + widthColY;

    float colNmb;
    float colSpaceX = rowWidth * modf(rGdiF.Width / (rowWidth + DR_SPACE), &colNmb) / (colNmb + 1);
    if ((colSpaceX < m_dpiRatio * DATA_ENDOFFS) && (colNmb > 1))
    {
        --colNmb;
        colSpaceX = (rGdiF.Width - rowWidth * colNmb) / (colNmb + 1);
    }

    float nmbRows = floor(rGdiF.Height / textStep) - 2;   // Account for the table header and reserve space

    m_rowRF = RectF(0.0f, 0.0f, rowWidth, textStep);
    m_nmbRF = RectF(0.0, 0.0, nmbRF.Width, textStep);
    m_dataXRF = RectF(m_nmbRF.GetRight(), 0.0, widthColX, textStep);
    m_dataYRF = RectF(m_dataXRF.GetRight(), 0.0, widthColY, textStep);
    m_offsX = rGdiF.X + colSpaceX;
    m_offsY = rGdiF.Y;
    m_deltaX = m_rowRF.Width + colSpaceX;
    m_deltaY = textStep;
    m_rows = size_t(nmbRows);
    m_cols = size_t(colNmb);
    m_page = m_rows * m_cols;
    m_nPages = m_vDataPnts.size() / m_page;
    if (m_vDataPnts.size() % m_page != 0)
        ++m_nPages;

    ReleaseDC(pDC);
}

void CChartDataView::CalcLayout(int flags, size_t dataItOffs)
{
    CDC* pDC = GetDC();
    Graphics gr(pDC->m_hDC);

    gr.SetPageUnit(UnitDocument);  // 300 dpi/inch
    gr.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    RectF rGdiF;
    gr.GetVisibleClipBounds(&rGdiF);

    // Get the dpiRatio
    int scrDpiX = pDC->GetDeviceCaps(LOGPIXELSX);
    int scrDpiY = pDC->GetDeviceCaps(LOGPIXELSY);
    ENSURE(scrDpiX == scrDpiY);
    m_dpiRatio = 300.0f / scrDpiX;

    rGdiF.Width -= m_dpiRatio * 4.0f;
    rGdiF.Height -= m_dpiRatio;

#ifdef _UNICODE
    FontFamily fontFamily(m_fontName.c_str());
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W(m_fontName.c_str()));
#endif
    Gdiplus::Font headerFont(&fontFamily, m_headerFontHeight, FontStyleBold, UnitPoint);
    float headerHeight = 1.5f * headerFont.GetHeight(&gr);
    Gdiplus::Font textFont(&fontFamily, m_fontHeight, FontStyleBold, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);
    strFormat.SetLineAlignment(StringAlignmentCenter);
    float textStep = 1.5f * textFont.GetHeight(&gr);

    rGdiF.Inflate(-10.0f, -10.0f);
    m_headerRF = RectF(10.0, rGdiF.Y, rGdiF.Width, headerHeight);
    m_headerRF.Inflate(-50.0f, 0.0f);
    m_headerRF.Offset(0.0f, 0.75f * headerHeight);

    rGdiF.Inflate(0.0f, -headerHeight);
    rGdiF.Offset(0.0f, headerHeight);
    rGdiF.Height -= 120.0f;

    RectF nmbRF, labXRF, labYRF;
    RectF maxXRF;// = valXRF;
    RectF maxYRF;// = valYRF;

  // Number rect
    if (flags & F_DSIZE)                        // Size of data vector was increased
    {
        sstream_t stream;
        stream << m_vDataPnts.size() << _T("99\n");
        string_t strNmb;
        getline(stream, strNmb);
#ifdef _UNICODE
        gr.MeasureString(strNmb.c_str(), -1, &textFont, rGdiF, &strFormat, &nmbRF);
#else if _MBCS
        gr.MeasureString(CA2W(strNmb.c_str()), -1, &textFont, rGdiF, &strFormat, &nmbRF);
#endif
    }

    if ((flags & F_NAMEX) && !(flags & F_VALX))
    {
#ifdef _UNICODE
        gr.MeasureString(m_labelX.c_str(), -1, &textFont, rGdiF, &strFormat, &maxXRF);
#else if _MBCS
        gr.MeasureString(CA2W(m_labelX.c_str()), -1, &textFont, rGdiF, &strFormat, &maxXRF);
#endif
    }
    if ((flags & F_NAMEY) && !(flags & F_VALY))
    {
#ifdef _UNICODE
        gr.MeasureString(m_labelY.c_str(), -1, &textFont, rGdiF, &strFormat, &maxYRF);
#else if _MBCS
        gr.MeasureString(CA2W(m_labelY.c_str()), -1, &textFont, rGdiF, &strFormat, &maxYRF);
#endif
    }

    if (flags & F_VALX)
    {
        m_vStrX.push_back(m_labelX);                // Temporary
        maxXRF =
            for_each(m_vStrX.begin() + dataItOffs, m_vStrX.end(), get_max_str<string_t, -1>(&textFont, &gr))._maxRF;
        m_vStrX.pop_back();  // Have it removed
    }

    if (flags & F_VALY)
    {
        m_vStrY.push_back(m_labelY);                // Temporary
        maxYRF =
            for_each(m_vStrY.begin() + dataItOffs, m_vStrY.end(), get_max_str<string_t, -1>(&textFont, &gr))._maxRF;
        m_vStrY.pop_back();                         // Have it removed
    }

    float widthNmb = max(m_nmbRF.Width, nmbRF.Width);
    float widthColX = max(m_dataXRF.Width, (maxXRF.Width + m_dpiRatio * DATA_ENDOFFS));
    float widthColY = max(m_dataYRF.Width, (maxYRF.Width + m_dpiRatio * DATA_ENDOFFS));

    float rowWidth = max(m_rowRF.Width, widthNmb + widthColX + widthColY);

    if (rowWidth > m_rowRF.Width)
    {
        float colNmb = 0.0f, colSpaceX = 0.0f;

        colSpaceX = rowWidth * modf(rGdiF.Width / (rowWidth + DR_SPACE), &colNmb) / (colNmb + 1);
        if ((colSpaceX < m_dpiRatio * DATA_ENDOFFS) && (colNmb > 1))
        {
            --colNmb;
            colSpaceX = (rGdiF.Width - rowWidth * colNmb) / (colNmb + 1);
        }

        float nmbRows = floor(rGdiF.Height / textStep) - 2;   // Account for the table header

        m_rowRF = RectF(0.0f, 0.0f, rowWidth, textStep);
        m_nmbRF = RectF(0.0, 0.0, widthNmb, textStep);
        m_dataXRF = RectF(m_nmbRF.GetRight(), 0.0, widthColX, textStep);
        m_dataYRF = RectF(m_dataXRF.GetRight(), 0.0, widthColY, textStep);
        m_offsX = rGdiF.X + colSpaceX;
        m_offsY = rGdiF.Y;
        m_deltaX = m_rowRF.Width + colSpaceX;
        m_deltaY = textStep;
        m_rows = size_t(nmbRows);
        m_cols = size_t(colNmb);
        m_page = m_rows * m_cols;
    }

    m_nPages = m_vDataPnts.size() / m_page;
    if (m_vDataPnts.size() % m_page != 0)
        ++m_nPages;

    ReleaseDC(pDC);
}

void CChartDataView::CreateChildren(void)
{
    CRect clRect;
    GetClientRect(&clRect);

    int ctrlStartX = clRect.left + clRect.Width() / 2 - 4 * 32 - 160;
    int ctrlStartY = clRect.bottom - 52;
    CRect ctrlRect(ctrlStartX, ctrlStartY, ctrlStartX + 96, ctrlStartY + 32);

    if (!IsWindow(m_leftEnd.m_hWnd)) // All were deleted
    {
        m_btnPrint.Create(_T("Print"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | BS_FLAT,
            ctrlRect,
            this,
            IDC_DV_BTNPRINT);

        ctrlRect.OffsetRect(160, 0);
        ctrlRect.right = ctrlRect.left + 32;
        m_leftEnd.InitParams(true, false, IDC_DV_LEFTENDARR);
        m_leftEnd.Create(NULL,
            WS_CHILD | WS_VISIBLE,
            ctrlRect,
            this,
            IDC_DV_LEFTENDARR);

        ctrlRect.OffsetRect(64, 0);
        m_leftArr.InitParams(false, false, IDC_DV_LEFTARR);
        m_leftArr.Create(NULL,
            WS_CHILD | WS_VISIBLE,
            ctrlRect,
            this,
            IDC_DV_LEFTARR);

        ctrlRect.OffsetRect(96, 0);
        m_rightArr.InitParams(false, true, IDC_DV_RIGHTARR);
        m_rightArr.Create(NULL,
            WS_CHILD | WS_VISIBLE,
            ctrlRect,
            this,
            IDC_DV_RIGHTARR);

        ctrlRect.OffsetRect(64, 0);
        m_rightEnd.InitParams(true, true, IDC_DV_RIGHTENDARR);
        m_rightEnd.Create(NULL,
            WS_CHILD | WS_VISIBLE,
            ctrlRect,
            this,
            IDC_DV_RIGHTENDARR);

        ctrlRect.OffsetRect(96, 0);
        ctrlRect.right = ctrlRect.left + 96;
        m_btnClose.Create(_T("Close"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_TEXT | BS_FLAT,
            ctrlRect,
            this,
            IDC_DV_BTNCLOSE);
    }
}

string_t CChartDataView::GetDataIDStr(size_t dataID)
{
    string_t strRes;
    sstream_t stream;

    stream << _T(" ") << dataID << _T(" \n");
    getline(stream, strRes);
    return strRes;
}

string_t CChartDataView::GetDataValStr(double val, int precision)
{
    string_t strRes;
    sstream_t stream;

    stream << _T(" ") << setprecision(precision) << val << _T("\n");

    getline(stream, strRes);
    return strRes;
}

size_t CChartDataView::DataItemFromMousePnt(CPoint pnt, PointD& dataPntD, RectF& itemRF)
{
    PointF mousePntF(m_dpiRatio * pnt.x, m_dpiRatio * pnt.y);

    size_t dataID = m_page * m_currPageID;

    bool bColFound = false;
    size_t colID = 0;
    float rowStartXF = m_rowRF.X + m_offsX;
    float rowEndXF = rowStartXF + m_rowRF.Width;
    for (; colID < m_cols; ++colID)
    {
        if (in_range(rowStartXF, rowEndXF, mousePntF.X))
        {
            bColFound = true;
            break;
        }
        rowStartXF += m_deltaX;
        rowEndXF += m_deltaX;
    }

    if (bColFound)
    {
        float rowStartYF = m_rowRF.Y + m_offsY + m_deltaY;
        if (in_range(rowStartYF, rowStartYF + m_deltaY * (m_rows + 1), mousePntF.Y))
        {
            size_t rowID = size_t(floor((mousePntF.Y - rowStartYF) / m_deltaY));
            if (rowID < m_vRows[colID])
            {
                itemRF = m_rowRF;
                itemRF.X = rowStartXF;
                itemRF.Y = rowStartYF + rowID * m_deltaY;

                for (size_t colCnt = 0; colCnt < colID; ++colCnt)
                    dataID += m_vRows[colCnt];
                dataID += rowID;

                if (dataID < m_vDataPnts.size())
                {
                    dataPntD = m_vDataPnts[dataID];
                    return dataID;
                }
            }
        }
    }
    return INT_MAX;
}

bool CChartDataView::RowRectFromDataID(size_t dataID, RectF& rowRF)
{
    size_t pageID = dataID / m_page;
    if (pageID != m_currPageID)
        return false;

    size_t dataPos = dataID - m_page * pageID;
    size_t colID = 0;
    for (; colID < m_cols; ++colID)
    {
        if (dataPos < m_vRows[colID])
            break;
        dataPos -= m_vRows[colID];
    }

    if (!in_range(size_t(0), m_vRows[colID], dataPos))
        return false;
    float offsX = m_offsX + colID * m_deltaX;
    float offsY = m_offsY + (dataPos + 1) * m_deltaY;
    rowRF = m_rowRF;
    rowRF.Offset(offsX, offsY);
    return true;
}

string_t CChartDataView::GetTableHeader(void)
{
    sstream_t stream;
    stream << m_label << _T(" (") << m_vDataPnts.size() << _T(" Points")
        << _T(")      \n");
    string_t headerStr;
    getline(stream, headerStr);

    // Show current local time
    CTime tmD = CTime::GetCurrentTime();
    CString strTmD = tmD.Format(_T("%a, %b %d, %Y, %H:%M"));
    string_t tmStr(strTmD);
    headerStr += tmStr;

    return headerStr;
}

size_t CChartDataView::ChangeDataPage(int delta)
{
    if ((m_currPageID == 0) && (delta < 0))
        return m_currPageID + 1;
    if ((m_currPageID == (m_nPages - 1)) && (delta > 0))
        return m_nPages;

    if (delta == INT_MAX)
        m_currPageID = m_nPages - 1;
    else if (delta == INT_MIN)
        m_currPageID = 0;
    else
        m_currPageID += delta;

    bool bEnableLeft = m_currPageID == 0 ? false : true;
    bool bEnableRight = m_currPageID == m_nPages - 1 ? false : true;

    m_leftEnd.EnableWindow(bEnableLeft ? TRUE : FALSE);
    m_leftArr.EnableWindow(bEnableLeft ? TRUE : FALSE);

    m_rightArr.EnableWindow(bEnableRight ? TRUE : FALSE);
    m_rightEnd.EnableWindow(bEnableRight ? TRUE : FALSE);

    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE | RDW_ALLCHILDREN);
    return m_currPageID + 1;
}

void CChartDataView::ShowWaitMessage(int chartIdx, size_t newSize)
{
    if ((chartIdx != m_chartIdx) && (newSize > 10000))
    {
        CDC* pDC = GetDC();
        Graphics gr(pDC->m_hDC);

#ifdef _UNICODE
        FontFamily fontFamily(_T("Verdana"));
#else if _MBCS
        USES_CONVERSION;
        FontFamily fontFamily(CA2W("Verdana"));
#endif
        Gdiplus::Font headerFont(&fontFamily, 16, FontStyleBold, UnitPoint);
        StringFormat strFormat;
        strFormat.SetAlignment(StringAlignmentCenter);
        strFormat.SetLineAlignment(StringAlignmentCenter);
        RectF boundingRect;
        gr.GetVisibleClipBounds(&boundingRect);
        SolidBrush bkBr((ARGB)Color::White);
        gr.FillRectangle(&bkBr, boundingRect);
        SolidBrush br((ARGB)Color::Red);
#ifdef _UNICODE
        gr.DrawString(_T("Calculating Layout"), -1, &headerFont, boundingRect, &strFormat, &br);
#else if _MBCS
        gr.DrawString(CA2W("Calculating Layout"), -1, &headerFont, boundingRect, &strFormat, &br);
#endif
    }
}

void CChartDataView::DrawPageHeader(float offsetX, Graphics* grPtr, size_t pageNmb, float dpiRatio)
{
#ifdef _UNICODE
    FontFamily fontFamily(m_fontName.c_str());
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W(m_fontName.c_str()));
#endif
    Gdiplus::Font headerFont(&fontFamily, m_headerFontHeight * dpiRatio, FontStyleBold, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);
    strFormat.SetLineAlignment(StringAlignmentCenter);
    SolidBrush txtBr(m_textColor);
    RectF headerRF = m_headerRF;
    headerRF.Offset(offsetX, 0.0f);
#ifdef _UNICODE
    grPtr->DrawString(m_header.c_str(), -1, &headerFont, headerRF, &strFormat, &txtBr);
#else if _MBCS
    grPtr->DrawString(CA2W(m_header.c_str()), -1, &headerFont, headerRF, &strFormat, &txtBr);
#endif
    sstream_t stream;
    stream << _T("Page ") << pageNmb << _T(" of ") << m_nPages;
    strFormat.SetAlignment(StringAlignmentNear);
#ifdef _UNICODE
    grPtr->DrawString(stream.str().c_str(), -1, &headerFont, headerRF, &strFormat, &txtBr);
#else if _MBCS
    grPtr->DrawString(CA2W(stream.str().c_str()), -1, &headerFont, headerRF, &strFormat, &txtBr);
#endif
}

float CChartDataView::DrawTableHeader(float offsetX, float offsetY, Graphics* grPtr, float dpiRatio)
{
#ifdef _UNICODE
    FontFamily fontFamily(m_fontName.c_str());
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W(m_fontName.c_str()));
#endif
    Gdiplus::Font txtFont(&fontFamily, m_fontHeight * dpiRatio, FontStyleBold, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);
    strFormat.SetLineAlignment(StringAlignmentCenter);

    SolidBrush cellBr(m_textColor);
    Pen cellPen(m_textColor, 1.0f * m_dpiRatio);
    RectF nmbRF = m_nmbRF;
    RectF dataXRF = m_dataXRF;
    RectF dataYRF = m_dataYRF;

    nmbRF.Offset(offsetX, offsetY);
    dataXRF.Offset(offsetX, offsetY);
    dataYRF.Offset(offsetX, offsetY);

    float deltaX = m_deltaX;

    for (size_t colCnt = 0; colCnt < m_cols; ++colCnt)
    {
        grPtr->DrawRectangle(&cellPen, nmbRF);
        nmbRF.Offset(deltaX, 0.0f);
        grPtr->DrawRectangle(&cellPen, dataXRF);
        RectF textRF(dataXRF);
        textRF.Width -= m_dpiRatio * DATA_ENDOFFS;
#ifdef _UNICODE
        grPtr->DrawString(m_labelX.c_str(), -1, &txtFont, textRF, &strFormat, &cellBr);
#else if _MBCS
        grPtr->DrawString(CA2W(m_labelX.c_str()), -1, &txtFont, textRF, &strFormat, &cellBr);
#endif
        dataXRF.Offset(deltaX, 0.0f);

        grPtr->DrawRectangle(&cellPen, dataYRF);
        textRF = dataYRF;
        textRF.Width -= m_dpiRatio * DATA_ENDOFFS;
#ifdef _UNICODE
        grPtr->DrawString(m_labelY.c_str(), -1, &txtFont, textRF, &strFormat, &cellBr);
#else if _MBCS
        grPtr->DrawString(CA2W(m_labelY.c_str()), -1, &txtFont, textRF, &strFormat, &cellBr);
#endif
        dataYRF.Offset(deltaX, 0.0f);
    }

    return nmbRF.Height;
}

size_t CChartDataView::DrawDataPage(size_t dataStartID, float tableOffsX, float tableOffsY,
    Graphics* grPtr, float dpiRatio)
{
    RectF nmbRF = m_nmbRF;
    RectF dataYRF = m_dataYRF;
    RectF dataXRF = m_dataXRF;

    nmbRF.Offset(tableOffsX, tableOffsY + m_deltaY);
    dataYRF.Offset(tableOffsX, tableOffsY + m_deltaY);
    dataXRF.Offset(tableOffsX, tableOffsY + m_deltaY);

    RectF topNmbRF = nmbRF;
    RectF topDataYRF = dataYRF;
    RectF topDataXRF = dataXRF;

#ifdef _UNICODE
    FontFamily fontFamily(m_fontName.c_str());
#else if _MBCS
    USES_CONVERSION;
    FontFamily fontFamily(CA2W(m_fontName.c_str()));
#endif
    Gdiplus::Font txtFont(&fontFamily, dpiRatio * m_fontHeight, FontStyleBold, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);
    strFormat.SetLineAlignment(StringAlignmentCenter);

    SolidBrush cellBr(m_textColor);
    Pen cellPen(m_textColor, 1.0f * m_dpiRatio);

    size_t rowsToDraw = m_vDataPnts.size() - dataStartID;
    if (rowsToDraw > m_page)
        m_vRows.assign(m_cols, m_rows);
    else
    {
        size_t rowCnt = rowsToDraw / m_cols;
        size_t remnant = rowsToDraw % m_cols;
        m_vRows.resize(m_cols);
        for (size_t colCnt = 0; colCnt < m_cols; ++colCnt)
        {
            if (remnant > 0)
            {
                m_vRows[colCnt] = rowCnt + 1;
                --remnant;
            }
            else
                m_vRows[colCnt] = rowCnt;
        }
    }

    // Draw rows
    size_t rowID = dataStartID;

    float txtEndOffsX = m_dpiRatio * DATA_ENDOFFS;

    for (size_t colCnt = 0; colCnt < m_cols; ++colCnt)
    {
        size_t rowNmb = m_vRows[colCnt];
        for (size_t rowCnt = 0; rowCnt < rowNmb; ++rowCnt)
        {
            string_t nmbStr = GetDataIDStr(rowID);
            grPtr->DrawRectangle(&cellPen, nmbRF);
            RectF txtRF = nmbRF;
            txtRF.Width -= txtEndOffsX;
#ifdef _UNICODE
            grPtr->DrawString(nmbStr.c_str(), -1, &txtFont, txtRF, &strFormat, &cellBr);
#else if _MBCS
            grPtr->DrawString(CA2W(nmbStr.c_str()), -1, &txtFont, txtRF, &strFormat, &cellBr);
#endif
            nmbRF.Offset(0.0, m_deltaY);

            string_t valXStr = m_vStrX[rowID];
            grPtr->DrawRectangle(&cellPen, dataXRF);
            txtRF = dataXRF;
            txtRF.Width -= txtEndOffsX;
#ifdef _UNICODE
            grPtr->DrawString(valXStr.c_str(), -1, &txtFont, txtRF, &strFormat, &cellBr);
#else if _MBCS
            grPtr->DrawString(CA2W(valXStr.c_str()), -1, &txtFont, txtRF, &strFormat, &cellBr);
#endif
            dataXRF.Offset(0.0f, m_deltaY);

            string_t valYStr = m_vStrY[rowID];
            grPtr->DrawRectangle(&cellPen, dataYRF);
            txtRF = dataYRF;
            txtRF.Width -= txtEndOffsX;
#ifdef _UNICODE
            grPtr->DrawString(valYStr.c_str(), -1, &txtFont, txtRF, &strFormat, &cellBr);
#else if _MBCS
            grPtr->DrawString(CA2W(valYStr.c_str()), -1, &txtFont, txtRF, &strFormat, &cellBr);
#endif
            dataYRF.Offset(0.0f, m_deltaY);
            ++rowID;
        }
        nmbRF.Y = topNmbRF.Y;
        dataXRF.Y = topDataXRF.Y;
        dataYRF.Y = topDataYRF.Y;

        nmbRF.Offset(m_deltaX, 0.0f);
        dataXRF.Offset(m_deltaX, 0.0f);
        dataYRF.Offset(m_deltaX, 0.0f);

    }
    return rowID;
}

void CChartDataView::DrawSelCells(Graphics* grPtr)
{
    if (!m_mapSelCells.empty())
    {
        SolidBrush bkBr(m_selColor);

        RectF dataCellRF;
        MAP_SELPNTSD::iterator it = m_mapSelCells.begin();
        MAP_SELPNTSD::iterator itE = m_mapSelCells.end();
        for (; it != itE; ++it)
        {
            size_t dataID = it->first;
            PointD dataPntD = it->second;
            if (RowRectFromDataID(dataID, dataCellRF))
                grPtr->FillRectangle(&bkBr, dataCellRF);
        }

    }
}

size_t CChartDataView::PrintData(size_t nPage, Gdiplus::Graphics* grPtr)
{
    RectF rGdiF;
    grPtr->GetVisibleClipBounds(&rGdiF);

    rGdiF.Width -= m_dpiRatio * 2.0f;
    rGdiF.Height -= m_dpiRatio * 2.0f;
    rGdiF.Inflate(-75.0f, 0.0f);
    rGdiF.Offset(75.0f, 0.0f);

    float prnWidth = (m_cols - 1) * m_deltaX + m_rowRF.Width;
    float prnHeight = (m_rows + 1) * m_deltaY;//+ m_offsY; //(m_rowRF.Y - m_headerRF.Y);

    float offsX = rGdiF.X + (rGdiF.Width - prnWidth) / 2.0f;
    float offsY = rGdiF.Y + (rGdiF.Height - prnHeight) / 2.0f;

    Pen borderPen((ARGB)Color::Black, m_dpiRatio * 1.0f);
    borderPen.SetAlignment(PenAlignmentInset);
    grPtr->DrawRectangle(&borderPen, rGdiF);

    DrawPageHeader(rGdiF.X, grPtr, nPage, 1.0f);
    DrawTableHeader(offsX, offsY, grPtr, 1.0f);
    size_t startDataID = m_page * (nPage - 1);
    DrawDataPage(startDataID, offsX, offsY, grPtr, 1.0f);
    return 0;

}

BEGIN_MESSAGE_MAP(CChartDataView, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_BN_CLICKED(IDC_DV_LEFTENDARR, OnBnClickedFirstPage)
    ON_BN_CLICKED(IDC_DV_LEFTARR, OnBnClickedPrevPage)
    ON_BN_CLICKED(IDC_DV_RIGHTARR, OnBnClickedNextPage)
    ON_BN_CLICKED(IDC_DV_RIGHTENDARR, OnBnClickedLastPage)
    ON_BN_CLICKED(IDC_DV_BTNCLOSE, OnBnClickedClose)
    ON_BN_CLICKED(IDC_DV_BTNPRINT, OnBnClickedPrint)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////////
// CChartDataView message handlers

void CChartDataView::OnPaint()
{
    CPaintDC dc(this);
    Graphics gr(dc);

    gr.SetPageUnit(UnitDocument);  // 300 dpi/inch

    Rect rGdi;
    gr.GetVisibleClipBounds(&rGdi);
    Bitmap bmp(rGdi.Width, rGdi.Height);
    Graphics* grPtr = Graphics::FromImage(&bmp);
    grPtr->SetSmoothingMode(SmoothingModeAntiAlias);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    RectF rGdiF;
    grPtr->GetVisibleClipBounds(&rGdiF);

    rGdiF.Width -= m_dpiRatio * 2.0f;
    rGdiF.Height -= m_dpiRatio * 2.0f;

    SolidBrush bkBrush(m_bkColor);
    grPtr->FillRectangle(&bkBrush, rGdiF);
    Pen borderPen((ARGB)Color::Black);
    borderPen.SetAlignment(PenAlignmentInset);
    grPtr->DrawRectangle(&borderPen, rGdiF);

    DrawPageHeader(rGdiF.X, grPtr, m_currPageID + 1, m_dpiRatio);
    DrawTableHeader(m_offsX, m_offsY, grPtr, m_dpiRatio);
    size_t startDataID = m_page * m_currPageID;
    DrawDataPage(startDataID, m_offsX, m_offsY, grPtr, m_dpiRatio);

    DrawSelCells(grPtr);

    gr.DrawImage(&bmp, rGdi);
    delete grPtr;
}

void CChartDataView::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bAdd = false;
    PointD dataPntD;
    RectF  dataRF;
    size_t dataID = DataItemFromMousePnt(point, dataPntD, dataRF);
    if (dataID < INT_MAX)
    {
        MAP_SELPNTSD::iterator it = m_mapSelCells.find(dataID);
        if (it != m_mapSelCells.end())
            m_mapSelCells.erase(it);
        else
        {
            m_mapSelCells.insert(MAP_SELPNTSD::value_type(dataID, dataPntD));
            bAdd = true;
        }

        CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
        pContainer->UpdateDataViewPnts(m_chartIdx, dataID, dataPntD, bAdd);

        // Redraw selected data rect
        dataRF.X /= m_dpiRatio;
        dataRF.Y /= m_dpiRatio;
        dataRF.Width /= m_dpiRatio;
        dataRF.Height /= m_dpiRatio;

        CRect r = RectFToCRect(dataRF);
        RedrawWindow(&r, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }

    CWnd::OnLButtonDown(nFlags, point);
}

void CChartDataView::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (!m_mapSelCells.empty())
    {
        if (IDYES ==
            AfxMessageBox(_T("Are you going to deselect all selected cells?(YES/NO"),
                MB_ICONQUESTION | MB_YESNO))
        {
            m_mapSelCells.clear();

            CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
            pContainer->UpdateDataViewPnts(-1, 0, PointD(0.0, 0.0), false);

            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
        }
    }

    CWnd::OnRButtonDown(nFlags, point);
}

void CChartDataView::OnBnClickedFirstPage(void)
{
    ChangeDataPage(INT_MIN);
}

void CChartDataView::OnBnClickedPrevPage(void)
{
    ChangeDataPage(-1);
}

void CChartDataView::OnBnClickedNextPage(void)
{
    ChangeDataPage(1);
}

void CChartDataView::OnBnClickedLastPage(void)
{
    ChangeDataPage(INT_MAX);
}

void CChartDataView::OnBnClickedClose(void)
{
    m_chartIdx = -1;
    m_mapSelCells.clear();
    CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
    pContainer->ClearDataViewChartIdx();
}

void CChartDataView::OnBnClickedPrint(void)
{
    CPrintDialog printDlg(FALSE, PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_HIDEPRINTTOFILE | PD_NOSELECTION | PD_RETURNDC);
    printDlg.m_pd.Flags &= ~PD_SHOWHELP;
    printDlg.m_pd.nFromPage = 1;
    printDlg.m_pd.nToPage = WORD(m_nPages);
    printDlg.m_pd.nMinPage = 0;
    printDlg.m_pd.nMaxPage = WORD(m_nPages);

    printDlg.DoModal();

    WORD nFromPage = printDlg.m_pd.nFromPage;
    WORD nToPage = printDlg.m_pd.nToPage;

    HDC printDC = printDlg.GetPrinterDC();
    if (printDC != NULL)
    {
        CDC* pDC = new CDC;
        pDC->Attach(printDC);
        Graphics* grPtr = new Graphics(printDC);
        grPtr->SetPageUnit(UnitDocument);  // 300 dpi/inch
        grPtr->SetSmoothingMode(SmoothingModeAntiAlias);
        grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        pDC->StartDoc(_T(""));
        for (WORD pageCnt = nFromPage; pageCnt <= nToPage; ++pageCnt)
        {
            pDC->StartPage();
            PrintData(pageCnt, grPtr);
            pDC->EndPage();
        }
        pDC->EndDoc();

        delete grPtr;
        pDC->Detach();
        delete pDC;
    }
    GlobalFree(printDlg.m_pd.hDevMode);
    GlobalFree(printDlg.m_pd.hDevNames);
}

void CChartDataView::OnClose()
{
    CChartContainer* pContainer = static_cast<CChartContainer*>(GetOwner());
    pContainer->ClearDataViewChartIdx();
}

