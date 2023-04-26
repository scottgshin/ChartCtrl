///////////////////////////////////////////////////////////////////////////////
//
// CChart.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ChartDef.h"
#include "DataLabel.h"
#include "Chart.h"
#include "ChartContainer.h"
#include "Util.h"


using namespace std;
using namespace Gdiplus;

//////////////////////////////////////////////////////////////////////////////
// Default global function to get label string

string_t __stdcall GetLabelValStr(double val, int precision, bool bAddEqSign)
{
    sstream_t stream_t;
    stream_t << std::setprecision(precision) << val;
    return bAddEqSign ? string_t(_T("= ")) + stream_t.str() : stream_t.str();
}

///////////////////////////////////////////////////////////////////////////////
// struct CChart

CChart::CChart(void) : 
m_pLabYValStrFn(&GetLabelValStr)
{
}

CChart::~CChart(void)
{
}

CChart* CChart::CloneChart(bool bCloneState)
{
    CChart* chartPtr = new CChart();

    chartPtr->m_nIdx = m_nIdx;
    if (bCloneState)
    {
        chartPtr->m_bVisible = m_bVisible;
        chartPtr->m_bSelected = m_bSelected;
        chartPtr->m_bShowPnts = m_bShowPnts;
        chartPtr->m_fLocScaleY = m_fLocScaleY;
    }
    else
    {
        chartPtr->m_bVisible = true;
        chartPtr->m_bSelected = false;
        chartPtr->m_bShowPnts = true;
        chartPtr->m_fLocScaleY = 1.0;
    }

    chartPtr->m_label = m_label;
    chartPtr->m_labelY = m_labelY;
    chartPtr->m_pLabYValStrFn = m_pLabYValStrFn;
    chartPtr->m_precisionY = m_precisionY;

    chartPtr->m_colChart = m_colChart;
    chartPtr->m_dashStyle = m_dashStyle;
    chartPtr->m_pntStyle = m_pntStyle;
    chartPtr->m_fPenWidth = m_fPenWidth;
    chartPtr->m_fTension = m_fTension;

    chartPtr->m_fMinValX = m_fMinValX;
    chartPtr->m_fMaxValX = m_fMaxValX;
    chartPtr->m_fMinValY = m_fMinValY;
    chartPtr->m_fMaxValY = m_fMaxValY;

    chartPtr->m_vDataPnts = m_vDataPnts;

    return chartPtr;
}

void CChart::SetChartAttr(bool bVisible, bool bShowPnts, int idx, string_t label, string_t labelY,
    int precisionY, DashStyle dashStyle, float penWidth, float tension, Color col, PointStyle pntStyle)
{
    ENSURE(!label.empty());
    m_bVisible = bVisible;
    m_bShowPnts = bShowPnts;
    m_nIdx = idx;
    m_label = label;
    m_labelY = labelY.empty() ? string_t(_T("Y")) : labelY;
    m_precisionY = precisionY;
    m_colChart = col;
    m_dashStyle = dashStyle;
    m_fPenWidth = penWidth;
    m_fTension = tension;
    m_pntStyle = pntStyle;
}

PAIR_ITS CChart::GetStartEndDataIterators(V_CHARTDATAD& vDataPnts, double startX, double endX)
{
    PAIR_ITS pair_its = find_border_pnts(vDataPnts.begin(), vDataPnts.end(),
        not_inside_range<double, false>(startX, endX));
    return pair_its;
}

size_t CChart::AppendChartData(V_CHARTDATAD& vData)
{
    if (vData.empty())
        return 0;

    // Find first element to append
    V_CHARTDATAD::iterator itStart = vData.begin();
    if (!m_vDataPnts.empty())
    {
        PointD oldEndPntD = m_vDataPnts.back();
        double oldEndX = oldEndPntD.X;
        itStart = find_if(itStart, vData.end(), greater_or_equal<double, false>(oldEndX));
        if (itStart != vData.end()) // Find the first not a dublicate
        {
            while ((itStart != vData.end()) && (oldEndPntD.X == itStart->X))
            {
                if (oldEndPntD.Y != itStart->Y)
                    break;
                ++itStart;
            }
        }
        if (itStart == vData.end())
            return 0;       // All data points are duplicates
    }

    // Resize and copy
    size_t oldSize = m_vDataPnts.size();
    size_t sizeNew = oldSize + distance(itStart, vData.end());
    m_vDataPnts.resize(sizeNew);
    copy(itStart, vData.end(), m_vDataPnts.begin() + oldSize);

    // Correct minmaxes
    m_fMinValX = m_vDataPnts.front().X; // Just in case; it was not changed
    m_fMaxValX = m_vDataPnts.back().X;
    std::pair<V_CHARTDATAD::iterator, V_CHARTDATAD::iterator> pair_minmaxY =
        minmax_element(m_vDataPnts.begin(), m_vDataPnts.end(), less_pnt<double, true>());
    m_fMinValY = pair_minmaxY.first->Y;
    m_fMaxValY = pair_minmaxY.second->Y;
    return sizeNew - oldSize;
}

bool CChart::TruncateChartData(double startX, double endX)
{
    if (startX >= endX)           // Wrong parameters
        return false;
    if (!HasData())
        return false;
    if ((m_fMinValX >= startX) && (m_fMaxValX <= endX))
        return false;               // Nothing to truncate

    PAIR_ITS pair_its = GetStartEndDataIterators(m_vDataPnts, startX, endX);
    // We need this last point to draw the right end of the chart
    if (pair_its.second != m_vDataPnts.end())
        ++pair_its.second;

    size_t dataSize = distance(pair_its.first, pair_its.second);
    if (dataSize == 0)
    {
        m_vDataPnts.clear();
        m_fMinValX = DBL_MAX;
        m_fMaxValX = -DBL_MAX;
        m_fMinValY = DBL_MAX;
        m_fMaxValY = -DBL_MAX;
    }
    else
    {
        V_CHARTDATAD vDataD(dataSize);
        copy(pair_its.first, pair_its.second, vDataD.begin());

        m_vDataPnts.swap(vDataD);

        m_fMinValX = m_vDataPnts.front().X;
        m_fMaxValX = m_vDataPnts.back().X;
        std::pair<V_CHARTDATAD::iterator, V_CHARTDATAD::iterator> pair_minmaxY =
            minmax_element(m_vDataPnts.begin(), m_vDataPnts.end(), less_pnt<double, true>());
        m_fMinValY = pair_minmaxY.first->Y;
        m_fMaxValY = pair_minmaxY.second->Y;
    }
    return true;
}

PAIR_ITNEAREST CChart::GetNearestPointD(const PointD& origPntD, double dist, PointD& selPntD)
{
    V_CHARTDATAD::iterator it = m_vDataPnts.begin(), itE = m_vDataPnts.end();
    int nmbMultPntsD = 0;
    double leftX = origPntD.X - dist / 2.0;
    double rightX = origPntD.X + dist / 2.0;
    // Find the first point in distance range from the origPntsD.X, if it exists
    it = find_if(it, itE, coord_in_range<double, false>(leftX, rightX));
    if (it != itE)  // Find closest to origPntD.X
    {
        it = find_nearest(it, itE, nearest_to<double, false>(origPntD, dist));
        if (it != itE)          // Impossible; will return found_if at least
        {
            selPntD = *it;        // Get number of multivalued points (the same X's, different Y's)
            nmbMultPntsD = count_if(m_vDataPnts.begin(), it, count_in_range<double, false>(selPntD.X, selPntD.X));
            return make_pair(it, nmbMultPntsD + 1);
        }
    }
    return make_pair(itE, 0);
}

// Formats string and prepares chart visuals for the screen; for the print use the container helper
TUPLE_LABEL CChart::GetSelValString(const PointD selPntD, string_t nameX,
    int precision, val_label_str_fn pLabValXStrFnPtr)
{
    TUPLE_LABEL tuple_label;
    get<IDX_LNAME>(tuple_label) = m_label;
    get<IDX_LNAMEX>(tuple_label) = nameX;
    bool bAddEqSign = nameX.empty() ? false : true;
    get<IDX_LX>(tuple_label) = pLabValXStrFnPtr(selPntD.X, precision, bAddEqSign);
    get<IDX_LNAMEY>(tuple_label) = m_labelY;
    bAddEqSign = m_labelY.empty() ? false : true;
    get<IDX_LY>(tuple_label) = m_pLabYValStrFn(selPntD.Y, m_precisionY, bAddEqSign);

    int alpha = max(m_colChart.GetAlpha(), ALPHA_MINFORLABEL);
    Color labCol = SetAlpha(m_colChart, alpha);
    get<IDX_LCOLOR>(tuple_label) = labCol;

    get<IDX_LDASH>(tuple_label) = m_dashStyle;
    get<IDX_LPEN>(tuple_label) = m_fPenWidth;
    get<IDX_LPOINT>(tuple_label) = m_pntStyle;

    return tuple_label;
}

bool CChart::DrawChartCurve(V_CHARTDATAD& vDataPntsD, double startX, double endX,
    MatrixD* pMatrixD, GraphicsPath* grPathPtr, Graphics* grPtr, float dpiRatio)
{
    if (vDataPntsD.size() == 0)    // Just for safe programming; the function is never called on count zero
        return false;

    V_CHARTDATAF vDataPntsF;
    // Convert the pntsD to the screen pntsF
    if (!ConvertChartData(vDataPntsD, vDataPntsF, pMatrixD, startX, endX))
        return false;

    V_CHARTDATAF::iterator itF = vDataPntsF.begin();
    V_CHARTDATAF::pointer ptrDataPntsF = vDataPntsF.data();
    size_t vSize = vDataPntsF.size();

    // Add the curve to grPath
    Pen pen(m_colChart, m_fPenWidth * dpiRatio);
    pen.SetDashStyle(m_dashStyle);
    if (!m_bShowPnts && (vSize == 2))   // Are outside or at boundaries of clipping area
    {                                 // Make special semi-transparent dash pen
        Color col(SetAlpha(m_colChart, ALPHA_NOPNT));
        pen.SetColor(col);
    }

    if (m_dashStyle != DashStyleCustom)
    {
        if (vSize > 1)
        {
            grPtr->DrawCurve(&pen, ptrDataPntsF, vSize, m_fTension);

            if (m_bSelected && (dpiRatio == 1.0f))  // Mark the chart as selected on screen only
            {
                Pen selPen(Color(SetAlpha(m_colChart, ALPHA_SELECT)), (m_fPenWidth + PEN_SELWIDTH) * dpiRatio);
                grPtr->DrawCurve(&selPen, ptrDataPntsF, vSize, m_fTension);
            }
        }

        // Now add the points
        if (m_bShowPnts || (vSize == 1))
        {
            // Check to make sure that the data points are not too close together and trim any affending points.
            V_CHARTDATAF trimmed_set;
            trimmed_set.push_back(*vDataPntsF.begin()); // Add the first point
            for (auto it = vDataPntsF.begin();
                    it != vDataPntsF.end() && next(it) != vDataPntsF.end(); ++it) 
            {
                // check the distance between adjacent points.
                //distance.push_back(next(it)->X - it->X);
                //toss.push_back((next(it)->X - it->X) < (dpiRatio * CHART_PNTSTRSH));
                if ((next(it)->X - it->X) >= (dpiRatio * CHART_PNTSTRSH)) 
                {
                    trimmed_set.push_back(*next(it));
                }
            }

            for (const auto & n : trimmed_set)
            {
                // SGS - Add the ability to show the points based on the given style
                std::vector<Gdiplus::PointF> vec;
                RectF rPntF = RectFFromCenterF(n, dpiRatio * CHART_DTPNTSZ,
                                                dpiRatio * CHART_DTPNTSZ);

                switch (m_pntStyle)
                {
                    case PointStyle::Triangle_Up:
                        TriangleUpFFromCenterF(n, dpiRatio * CHART_DTPNTSZ,
                                                dpiRatio * CHART_DTPNTSZ, vec);
                        if (3 == vec.size())
                        {
                            auto ptr = vec.data();
                            grPathPtr->AddPolygon(ptr, vec.size());
                        }
                        break;
                    case PointStyle::Square:
                        grPathPtr->AddRectangle(rPntF);
                        break;
                    case PointStyle::Diamond:
                        DiamondFFromCenterF(n, dpiRatio * CHART_DTPNTSZ,
                                            dpiRatio * CHART_DTPNTSZ, vec);
                        if (4 == vec.size())
                        {
                            auto ptr = vec.data();
                            grPathPtr->AddPolygon(ptr, vec.size());
                        }
                        break;
                    case PointStyle::Triangle_Down:
                        TriangleDownFFromCenterF(n, dpiRatio * CHART_DTPNTSZ,
                                                    dpiRatio * CHART_DTPNTSZ, vec);
                        if (3 == vec.size())
                        {
                            auto ptr = vec.data();
                            grPathPtr->AddPolygon(ptr, vec.size());
                        }
                        break;
                    case PointStyle::Ellipse:
                    default:
                        grPathPtr->AddEllipse(rPntF);
                        break;
                }
            }
        }
    }
    else
    {
        PointF pntF;
        PointF pntFX(dpiRatio * CHART_DTPNTSZ / 2, 0.0f);
        PointF pntFY(0.0f, dpiRatio * CHART_DTPNTSZ / 2);

        for (; itF != vDataPntsF.end(); ++itF)
        {
            pntF = *itF;
            grPathPtr->StartFigure();
            grPathPtr->AddLine(pntF - pntFX, pntF + pntFX);
            grPathPtr->StartFigure();
            grPathPtr->AddLine(pntF - pntFY, pntF + pntFY);
        }
        if (vSize == 1)
        {
            grPathPtr->StartFigure();
            grPathPtr->AddEllipse(RectFFromCenterF(pntF, 2.0f * pntFX.X, 2.0f * pntFY.Y));
        }
    }

    if (grPathPtr->GetPointCount() > 0)          // Has points to draw
    {
        pen.SetWidth(1.0f * dpiRatio);
        pen.SetDashStyle(DashStyleSolid);
        grPtr->DrawPath(&pen, grPathPtr);
        if (((m_dashStyle == DashStyleCustom) || (vSize == 1)) && m_bSelected && (dpiRatio == 1.0f))
        {
            pen.SetColor(Color(SetAlpha(m_colChart, ALPHA_SELECT)));
            pen.SetWidth(m_fPenWidth + PEN_SELWIDTH);
            grPtr->DrawPath(&pen, grPathPtr);
        }
        grPathPtr->Reset();
    }
    return true;
}

bool CChart::ConvertChartData(V_CHARTDATAD& vDataPnts, V_CHARTDATAF& vDataPntsF,
    MatrixD* pMatrixD, double startX, double endX)
{
    ENSURE(vDataPnts.size() > 0);
    V_CHARTDATAD::iterator itFirst, itLast;
    if (vDataPnts.size() == 1)
    {
        itFirst = vDataPnts.begin();
        itLast = vDataPnts.end();
    }
    else
        std::tie(itFirst, itLast) = GetStartEndDataIterators(vDataPnts, startX, endX);
    if (itLast != vDataPnts.end())    // Last iterator points after the last point
        ++itLast;

    int vSize = distance(itFirst, itLast);
    if (vSize == 0)
        return false;
    // Transform to screen pointsF
    vDataPntsF.resize(vSize);
    transform(itFirst, itLast, vDataPntsF.begin(), TRANSFORM_TO_PNTF(m_fLocScaleY, pMatrixD));

    return true;
}

bool CChart::GetVisibleChartNameAndVisuals(TUPLE_LABEL& tuple_res)
{
    if (m_bVisible && HasData())
    {
        int alpha = m_colChart.GetAlpha();
        alpha = max(alpha, 128);
        string_t emptyStr(_T(""));
        tuple_res = make_tuple(m_label, emptyStr, emptyStr, emptyStr, emptyStr, SetAlpha(m_colChart, alpha),
            m_dashStyle, m_fPenWidth, m_pntStyle);
        return true;
    }
    // Does not change the tuple_res
    return false;
}
