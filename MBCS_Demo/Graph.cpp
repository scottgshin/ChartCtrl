// Graph.cpp : implementation file
//

#include <Graph.h>
#include <Chart.h>
#include <DataLabel.h>
#include <algorithm>
#include <time.h>
#include <format>
#include <random>
#include <corecrt_math_defines.h>
#include <limits>


const int SET_BACK_SPACE = 29;

namespace CONVERT_AXIS_LABELS
{
    static double Y_AXIS_TEMP_SLOPE;
    static double Y_AXIS_TEMP_OFFSET;
    static string_t __stdcall format_Y_axis_temp_labels(double val, int precision, bool bAddEqSign)
    {
        sstream_t stream_t;
        stream_t << std::setprecision(precision) << (val-Y_AXIS_TEMP_OFFSET)/Y_AXIS_TEMP_SLOPE;
        return bAddEqSign ? string_t(_T("= ")) + stream_t.str() : stream_t.str();
    }

    static time_t X_AXIS_TIME_MIN;
    static time_t X_AXIS_TIME_MAX;
    static string_t __stdcall format_X_axis_time_labels(double val, int precision, bool bAddEqSign)
    {
        const __int64 length(X_AXIS_TIME_MAX - X_AXIS_TIME_MIN);
        const double num_graph_divisions(10.0f);
        const double division_step_size(length / num_graph_divisions);
        const double num_hours_per_division(division_step_size / 3600.0f);
        time_t time(static_cast<time_t>(X_AXIS_TIME_MIN + val));
        std::string printable_timestamp;
        tm local_time;
        // convert from timepoint to the local calendar time
        localtime_s(&local_time, &time);
        
        if ((num_hours_per_division >= local_time.tm_hour) || (division_step_size > val))
        {
            // Trend is 2.5 days = 60 hours and the graph is split into 10 lines so if the 
            // hour is less than 6 print the date
            printable_timestamp =
                std::format("{}/{}\n{}:{:02}:{:02}", local_time.tm_mon + 1, local_time.tm_mday,
                            local_time.tm_hour, local_time.tm_min, local_time.tm_sec);
        }
        else
        {
            printable_timestamp = std::format("{}:{:02}:{:02}", local_time.tm_hour,
                                              local_time.tm_min, local_time.tm_sec);
        }

        TRACE("FORMAT_X: %f\tTIME: %s\n", val, printable_timestamp.c_str());
        string_t label(printable_timestamp.begin(), printable_timestamp.end());
        return label;
    }
    static string_t __stdcall format_X_axis_time_labels_no_date(double val, int precision, bool bAddEqSign)
    {
        const __int64 length(X_AXIS_TIME_MAX - X_AXIS_TIME_MIN);
        const double num_graph_divisions(10.0f);
        const double division_step_size(length / num_graph_divisions);
        const double num_hours_per_division(division_step_size / 3600.0f);
        time_t time(static_cast<time_t>(X_AXIS_TIME_MIN + val));
        std::string printable_timestamp;
        tm local_time;
        // convert from timepoint to the local calendar time
        localtime_s(&local_time, &time);

        printable_timestamp = std::format("{}:{:02}:{:02}", local_time.tm_hour,
            local_time.tm_min, local_time.tm_sec);

        TRACE("FORMAT_X: %f\tTIME: %s\n", val, printable_timestamp.c_str());
        string_t label(printable_timestamp.begin(), printable_timestamp.end());
        return label;
    }
}


// CGraph dialog

IMPLEMENT_DYNAMIC(CGraph, CDialog)

CGraph::CGraph(CWnd* pParent) /*=NULL*/
    :
    CDialog(CGraph::IDD, pParent),
    m_timestamp(0)
{
    m_JPGFilename.clear();
    m_serial.clear();
    m_tag.clear();

    // ENABLE GDI+ for ChartCtrlLib
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_nGdiplusToken, &gdiplusStartupInput, NULL);
}

CGraph::~CGraph()
{
    Gdiplus::GdiplusShutdown(m_nGdiplusToken);
}

void CGraph::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STCHARTCONTAINER, m_chartContainer);
}


BEGIN_MESSAGE_MAP(CGraph, CDialog)
    ON_WM_SIZE()
    //ON_WM_CLOSE()
    //ON_BN_CLICKED(IDOK, &CGraph::OnBnClickedOk)
END_MESSAGE_MAP()



BOOL CGraph::OnInitDialog()
{
    CDialog::OnInitDialog();

    switch (test_to_run)
    {
    case 1:
    default:
        TestGraph();
        GraphBuff();
        break;
    case 2:
        Sample2();
        break;
    }
    return TRUE;
}


void CGraph::TestGraph()
{
    // Create a random device
    std::random_device rd{};
    // Seed the random engine
    auto mtgen = std::mt19937{rd()};
    // Select the distribution
    auto ud510 = std::uniform_real_distribution<>{5, 10};      // Flow will be between 5 and 10 SMPS
    auto ud = std::uniform_real_distribution<>{15.0, 30.0};     // Temp will be between 15 and 30 °C

    time_t timer = time(nullptr);
    m_timestamp = timer;

    for (auto i = 0; i < 500; i++) 
    { 
        X.push_back(timer + i);
        if (250 != i)
        {
            Y_flow.push_back(ud510(mtgen));
            Y_temp.push_back(ud(mtgen));
        }
        else
        {
            // Create a point outside the distribution to allow the scales to be seen better
            Y_flow.push_back(2.0f);
            Y_temp.push_back(12.0f);
        }
    }
    m_serial = string_t("TEST GRAPH 1");
    m_tag = string_t("TEST 1");
    m_JPGFilename = string_t("C:\\tmp\\test1.jpeg");
}

void CGraph::Sample2()
{
    string_t name1{ "SineWave" };
    int func = 0;         // 0 - sin, 1 sin(x)/x, 2 - exp, 3 - rect, 4 - random, 5 - x = y^2.5
    V_CHARTDATAD vData;
    STRUCT_FUNCDATA funcData =
        GenerateData(func, -10, 10, 100, vData, 1);
    //_TCHAR buffer_t[64];
    //_itot_s(m_chartContainer.GetMaxChartIdx() + 1, buffer_t, 10);  // Chart idx to string
    //string_t idxStr(buffer_t);
    //name1 += string_t(buffer_t);
    int chartIdx = m_chartContainer.AddChart(true,
        true,
        name1, "Y", 3,
        Gdiplus::DashStyle::DashStyleSolid,
        2, float(0),
        Gdiplus::Color::Red,
        vData, PointStyle::Square, true);

    if (chartIdx == -1)
    {
        CString msgStr;
        msgStr.Format(_T("Can't add CChart ID=%d, Name=%s\n")
            _T("Check the data vector and/or\nchart name(should be unique)"),
            chartIdx, name1.c_str());
        AfxMessageBox(msgStr, MB_OK | MB_ICONERROR);
    }
    m_serial = string_t("TEST GRAPH 2");
    m_tag = string_t("TEST 2");
    m_JPGFilename = string_t("C:\\tmp\\test2.jpeg");

}

void CGraph::GraphBuff()
{
    if ( X.empty() || Y_flow.empty() || Y_temp.empty() )
    {
        return;
    }

    std::vector<double> Y_temp_scaled_to_flow_bounds;

    // Determine the max and min X values
    time_t min_x(*min_element(X.begin(), X.end()));
    time_t max_x(*max_element(X.begin(), X.end()));
    double min_y1(*min_element(Y_flow.begin(), Y_flow.end()));
    double max_y1(*max_element(Y_flow.begin(), Y_flow.end()));
    double min_y2(*min_element(Y_temp.begin(), Y_temp.end()));
    double max_y2(*max_element(Y_temp.begin(), Y_temp.end()));

    CONVERT_AXIS_LABELS::X_AXIS_TIME_MAX = max_x;
    CONVERT_AXIS_LABELS::X_AXIS_TIME_MIN = min_x;

    auto slope(( max_y1 - min_y1 )/( max_y2 - min_y2 ));
    auto offset( min_y1 - slope*min_y2 );

    for ( auto iter = Y_temp.begin(); iter != Y_temp.end(); ++iter )
    {
        Y_temp_scaled_to_flow_bounds.push_back( (*iter) * slope + offset );
    }

    CONVERT_AXIS_LABELS::Y_AXIS_TEMP_SLOPE = slope;
    CONVERT_AXIS_LABELS::Y_AXIS_TEMP_OFFSET = offset;

    TCHAR title[200] = {0};
    _stprintf_s( title, 200, _T("Volatile Trend       Tag: %s       Serial: %s"), m_tag.c_str(), m_serial.c_str() );
    SetDlgItemText(IDC_TITLE_STATIC, title);
    string_t temp(title);
    m_chartContainer.SetContainerName(temp);
    m_chartContainer.SetImagePath(m_JPGFilename);

    TCHAR time[200] = {0};
    _stprintf_s(time, 200, _T("Date and Time"));
    SetDlgItemText(IDC_TIME_STATIC, time );
    m_chartContainer.SetAxisXName(time);
    m_chartContainer.setYAxisLabel(TEXT("Flow"));
    m_chartContainer.ShowAxisCoordinates(true);
    int flow_id = m_chartContainer.AddChart(
        true, true, TEXT("Flow"), TEXT("Flow"), 3, Gdiplus::DashStyle::DashStyleSolid, 0, 0,
        Gdiplus::Color::Red, X, Y_flow, PointStyle::Ellipse, false);
    int temp_id = m_chartContainer.AddChart(
        true, true, TEXT("Temperature"), TEXT("Temperature"), 3, Gdiplus::DashStyle::DashStyleDash,
        0, 0, Gdiplus::Color::Blue, X, Y_temp_scaled_to_flow_bounds, PointStyle::Diamond, false);

    m_chartContainer.SetLabXValStrFn(CONVERT_AXIS_LABELS::format_X_axis_time_labels);
    m_chartContainer.SetLabYValStrFn(temp_id, CONVERT_AXIS_LABELS::format_Y_axis_temp_labels, true, false);

    // Causes an exception upon container destruction?
    m_chartContainer.setShowYAxisSecondScale(true);

    m_chartContainer.SetContainerPrecision(5);
    m_chartContainer.ShowChartPoints(flow_id, true, false);
    m_chartContainer.ShowChartPoints(temp_id, true, true);
    m_chartContainer.ShowAxisXBoundaries(false);
}


void CGraph::setJPGFilename(const TCHAR * const cFilename)
{
    //_tcscpy_s(m_JPGFilename, MAX_PATH, cFilename);
    m_JPGFilename = string_t(cFilename);
}

void CGraph::OnSize(UINT nType, int cx, int cy)
{
    // Min size 400 x 400
    if (cx < 400)
    {
        cx = 400;
    }

    if (cy < 400)
    {
        cy = 400;
    }

    CDialog::OnSize(nType, cx, cy);
    if (IsWindow(m_chartContainer.m_hWnd) && (nType == SIZE_RESTORED))
    {
        m_chartContainer.OnChangedSize(PointT<int>(SET_BACK_SPACE, SET_BACK_SPACE), cx - 2*SET_BACK_SPACE, cy-2*SET_BACK_SPACE);
        // Structure used to store the button control's dimensions.
        RECT rect;
        CWnd* pWindow = GetDlgItem(IDC_TIME_STATIC);
        ASSERT(pWindow);
        // Get button control's dimensions with respect to the screen.
        pWindow->GetWindowRect(&rect);
        // Convert the button control's dimensions with respect to its parent window.
        ScreenToClient(&rect);
        // Adjust button control's dimensions -17 pixels up from the bottom of the Y axis.
        rect.top = cy - 25;
        rect.bottom = rect.top + 15;
        rect.left = cx/2 -100;
        if (rect.left < 0 )
        {
            rect.left = 0;
        }
        rect.right = rect.left + 200;
        // Move the button control 26 pixels down the Y axis.
        pWindow->MoveWindow(&rect);

        pWindow = GetDlgItem(IDC_TITLE_STATIC);
        ASSERT(pWindow);
        // Get button control's dimensions with respect to the screen.
        pWindow->GetWindowRect(&rect);
        // Convert the button control's dimensions with respect to its parent window.
        ScreenToClient(&rect);
        // Adjust button control's dimensions -17 pixels up from the bottom of the Y axis.
        rect.left = cx/2 -150;
        if (rect.left < 0 )
        {
            rect.left = 0;
        }
        rect.right = rect.left + 400;
        // Move the button control 26 pixels down the Y axis.
        pWindow->MoveWindow(&rect);
    }
}


void CGraph::OnCancel()
{
    m_chartContainer.SaveContainerImage(m_JPGFilename);
    // Because the chart data view is owned by the container
    m_chartContainer.ClearDataViewChartIdx();
    CDialog::OnCancel();
}


// Taken from DlgAddChart.cpp
STRUCT_FUNCDATA CGraph::GenerateData(int funcID, double fMinX,
    double fMaxX, size_t pntsNmb, V_CHARTDATAD& vData, double multY)
{
    if (fMinX >= fMaxX)
        return STRUCT_FUNCDATA();
    double fDeltaX = (fMaxX - fMinX) / pntsNmb;
    double alpha = 0.0;
    size_t period = funcID == 3 ? pntsNmb / 2 : 0;  // Used to append RectWave only


    switch (funcID)
    {
    case 0: alpha = GenerateSinWave(pntsNmb, fMinX, fDeltaX, vData, multY);  break;
    case 1: alpha = GenerateSincWave(pntsNmb, fMinX, fDeltaX, vData, multY); break;
    case 2: alpha = GenerateExp(pntsNmb, fMinX, fDeltaX, vData, multY);      break;
    case 3: alpha = GenerateRectWave(pntsNmb, fMinX, fDeltaX, vData, multY); break;
    case 4: alpha = GenerateRandomNmbs(pntsNmb, fMinX, fDeltaX, vData, multY); break;
    }

    STRUCT_FUNCDATA funcData = { funcID, vData.back().X, fDeltaX, alpha, multY, pntsNmb, period };
    return funcData;
}

double CGraph::GenerateSinWave(size_t nPntNmb, double fMinX, double fDeltaX,
    V_CHARTDATAD& vData, double multY)
{
    // Calculate period: one for range
    double alpha = 2 * M_PI / (fDeltaX * nPntNmb);
    double lim = pow(10.0, -std::numeric_limits<double>::digits10);

    // Fill the vector
    vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
    for (size_t i = 0; i <= nPntNmb; ++i)
    {
        double pntX = fMinX + i * fDeltaX;
        double pntY = sin(alpha * pntX);
        if (fabs(pntY) < lim)
            pntY = 0.0;
        vData[i] = PointD(pntX, multY * pntY);
    }
    return alpha;
}

double CGraph::GenerateSincWave(size_t nPntNmb, double fMinX, double fDeltaX,
    V_CHARTDATAD& vData, double multY)
{
    // Calculate period: four for nPntNmb
    double alpha = 4 * M_PI / (fDeltaX * nPntNmb);;
    double lim = pow(10.0, -std::numeric_limits<double>::digits10);
    // Fill the vector
    vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
    for (size_t i = 0; i <= nPntNmb; ++i)
    {
        double pntX = fMinX + i * fDeltaX;
        double sinArg = alpha * pntX;
        if ((pntX > -0.00001) && (pntX < 0.00001))
            vData[i] = PointD(pntX, multY);
        else
        {
            double pntY = sin(sinArg) / sinArg;
            if (fabs(pntY) < lim)
                pntY = 0.0;
            vData[i] = PointD(pntX, multY * pntY);
        }
    }
    return alpha;
}

double CGraph::GenerateExp(size_t nPntNmb, double fMinX, double fDeltaX,
    V_CHARTDATAD& vData, double multY)
{
    // Exp argument is between 0 and 4 for range [0, fMax]
    double alpha = 4.0 / (fDeltaX * nPntNmb);
    double lim = pow(10.0, -std::numeric_limits<double>::digits10);
    // Fill the vector
    vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
    for (size_t i = 0; i <= nPntNmb; ++i)
    {
        double pntX = fMinX + i * fDeltaX;
        double expX = fabs(alpha * pntX);
        double pntY = exp(-expX);
        if (pntY < lim)
            pntY = 0.0;
        vData[i] = PointD(pntX, multY * pntY);
    }
    return alpha;
}

double CGraph::GenerateRectWave(size_t nPntNmb, double fMinX, double fDeltaX,
    V_CHARTDATAD& vData, double multY)
{
    // Two periods
    size_t nTau = nPntNmb / 2; // Two periods
    size_t nPositive = nTau / 2;
    size_t nNegative = nTau - nPositive;
    double alpha = 0.5 * multY;

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

double CGraph::GenerateRandomNmbs(size_t nPntNmb, double fMinX, double fDeltaX,
    V_CHARTDATAD& vData, double multY)
{
    std::mt19937 m_rndGenNmbs;
    m_rndGenNmbs.seed(_time32(NULL));

    double alpha = multY;
    std::uniform_real<double> uniDistrib(-alpha, alpha);

    // Fill the vector
    vData.resize(nPntNmb + 1, PointD(0.0, 0.0));
    for (size_t i = 0; i <= nPntNmb; ++i)
        vData[i] = PointD(fMinX + i * fDeltaX, uniDistrib(m_rndGenNmbs));
    return 1.0;
}
