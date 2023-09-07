///////////////////////////////////////////////////////////////////////////////
// Chart.h

#include "ChartDef.h"
#include "Util.h"

#pragma once

using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// Global default function to get val string

string_t __stdcall GetLabelValStr(double val, int precision, bool bAddEqSign);

///////////////////////////////////////////////////////////////////////////////
// Class CChart

class MatrixD;

class CChart
{
public:
    // Ctor and destructor
    CChart(void);
    ~CChart(void);
    // Operations and Implementation
    CChart* CloneChart(bool bCloneState = false);

    // Access to data members
    // Read only; software takes care of assigning chartIdx
    int GetChartIdx(void) const { return m_nIdx; }
    // Label
    string_t GetChartName(void) const { return m_label; }
    void SetChartName(string_t name) { m_label = name; }
    // YAxis
    string_t GetAxisYName(void) const { return m_labelY; }
    void SetAxisYName(string_t labelY) { m_labelY = labelY.empty() ? string_t(_T("Y")) : labelY; }

    val_label_str_fn GetLabYValStrFnPtr(void) const { return m_pLabYValStrFn; }
    void SetLabYValStrFn(val_label_str_fn pLabYValStrFn) { m_pLabYValStrFn = pLabYValStrFn; }


    // Visibility
    bool IsChartVisible() const { return m_bVisible; }
    void SetChartVisibility(bool bVisible) { m_bVisible = bVisible; }
    bool ToggleChartVisibility() { m_bVisible = !m_bVisible; return m_bVisible; }

    // Selection
    bool IsChartSelected() const { return m_bSelected; }
    void SelectChart(bool bSel) { m_bSelected = bSel; }

    // Show chart points
    bool ArePntsAllowed(void) const { return m_bShowPnts; }
    void AllowPnts(bool bAllowed) { m_bShowPnts = bAllowed; }

    // Color
    Color GetChartColor(void) const { return m_colChart; }
    void SetChartColor(Color colChart) { m_colChart = colChart; }

    // Dash style
    DashStyle GetChartDashStyle(void) const { return m_dashStyle; }
    void SetDashStyle(DashStyle dashStyle) { m_dashStyle = dashStyle; }

    // Point style
    PointStyle GetChartPointStyle(void) const { return m_pntStyle; }
    void SetPointStyle(PointStyle pntStyle) { m_pntStyle = pntStyle; }

    // Tension
    float GetTension(void) const { return m_fTension; }
    void SetTension(float fTension) { m_fTension = fTension; }

    // Pen width
    float GetPenWidth(void) const { return m_fPenWidth; }
    void SetPenWidth(float penWidth) { m_fPenWidth = penWidth; }

    // Minmax X and Y
    double GetMinValX(void) const { return m_fMinValX; }
    void SetMinValX(double fMinValX) { m_fMinValX = fMinValX; }

    double GetMaxValX(void) const { return m_fMaxValX; }
    void SetMaxValX(double fMaxValX) { m_fMaxValX = fMaxValX; }

    double GetMinValY(void) const { return m_fMinValY; }
    void SetMinValY(double fMinValY) { m_fMinValY = fMinValY; }

    double GetMaxValY(void) const { return m_fMaxValY; }
    void SetMaxValY(double fMaxValY) { m_fMaxValY = fMaxValY; }

    // Precision on Y-axis
    int GetPrecisionY(void) const { return m_precisionY; }
    void SetPrecisionY(int precisionY) { m_precisionY = precisionY; }

    // Loc scale Y
    double GetLocScaleY(void) const { return m_fLocScaleY; }
    void SetLocScaleY(double locScaleY) { m_fLocScaleY = locScaleY; }
    // Data
    size_t GetDataSize() const { return m_vDataPnts.size(); }
    // Get whether the chart is ready to be drawn
    bool HasData(void) const { return (m_vDataPnts.size() > 0); }

    void ResetChartData(void) { m_vDataPnts.clear(); }

    // Implementation
    void SetChartAttr(bool bVisible, bool bShowPnts, int idx, string_t label, string_t labelY,
        int precisionY, DashStyle dashStyle, float penWidth, float tension, Color col, PointStyle pntStyle);
//protected:
      // Returns iterators that points to the last point.X <= startX and first point.X >= endX
    PAIR_ITS GetStartEndDataIterators(V_CHARTDATAD& vDataPnts, double startX, double endX);
public:
    size_t AppendChartData(V_CHARTDATAD& vData);
    bool TruncateChartData(double startX, double endX);

    PAIR_ITNEAREST GetNearestPointD(const PointD& origPntD, double dist, PointD& selPnt);

    // Used to get chart name and visuals for data and names legends
    TUPLE_LABEL GetSelValString(const PointD selPntD, string_t nameX, int precision,
        val_label_str_fn pLabXValStrFnPtr);
    // Draws the curve and, if the distance between chart points allows, draws data points
    bool DrawChartCurve(V_CHARTDATAD& vDataPnts, double startX, double endX,
        MatrixD* pMatrixD, GraphicsPath* grPathPtr, Graphics* grPtr, float dpiRatio = 1.0f);
    // Casts points in the range startX, endX to PointsF that are positions of data pnts in
    // the ChartContainer window
    bool ConvertChartData(V_CHARTDATAD& vDataPnts, V_CHARTDATAF& vChDatF, MatrixD* pMatrixD,
        double startX, double endX);
    // Get visible chart name and color
    bool GetVisibleChartNameAndVisuals(TUPLE_LABEL& tuple_res);

    // Data members
protected:
    bool m_bVisible = true;
    bool m_bSelected = false;
    bool m_bShowPnts = true;

    int m_nIdx = -1;
    Color m_colChart = (ARGB)Color::Black;
    DashStyle m_dashStyle = DashStyleSolid;
    float m_fPenWidth = 2;
    float m_fTension = 0.0f;    // Smoothiness of the curve

    double m_fMinValX = DBL_MAX;    // Min X coordinate
    double m_fMaxValX = -DBL_MAX;   // Max X coordinate
    double m_fMinValY = DBL_MAX;    // Min Y coord; used to calculate scaleY
    double m_fMaxValY = -DBL_MAX;   // Max Y coord; used to calculate scaleY

    int m_precisionY = 3;   // Precision of Y-axis
    double m_fLocScaleY = 1.0;  // Additional PointD.Y multiplier; normally 1.0

    string_t m_label = string_t(_T(""));     // Chart's Name
    string_t m_labelY = string_t(_T("Y"));   // Y-axis unit name
    val_label_str_fn m_pLabYValStrFn;  // Pointer to the Y-value string generator
    PointStyle m_pntStyle = PointStyle::Ellipse;

public:
    V_CHARTDATAD m_vDataPnts;
};

