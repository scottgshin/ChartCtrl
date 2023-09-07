#ifndef Graph_h
#define Graph_h

#include <winsock2.h>    // Needs to be included before windows.h for some reason.
#include <Windows.h>
//#include <winsock2.h>

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>
#include <afxdtctl.h>
#include <afxdlgs.h>

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

//#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxole.h>         // MFC OLE classes
//#include <afxodlgs.h>       // MFC OLE dialog classes
//#include <afxdisp.h>        // MFC Automation classes
//#endif // _AFX_NO_OLE_SUPPORT
//
//#ifndef _AFX_NO_DB_SUPPORT
//#include <afxdb.h>          // MFC ODBC database classes
//#endif // _AFX_NO_DB_SUPPORT
//
//#ifndef _AFX_NO_DAO_SUPPORT
//#include <afxdao.h>         // MFC DAO database classes
//#endif // _AFX_NO_DAO_SUPPORT
//
//#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
//#endif
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>         // MFC support for Windows Common Controls
//#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>        // MFC socket extensions
#include <afxcontrolbars.h>



#include <ChartContainer.h>
#include <Resource.h>


// Taken from DlgAddChart.h
typedef struct
{
    int funcID;
    double lastX;
    double deltaX;
    double alpha;
    double multY;
    size_t pntsNmb;
    size_t period;
} STRUCT_FUNCDATA;



// CGraph dialog

class CGraph : public CDialog
{
    DECLARE_DYNAMIC(CGraph)

public:
    CGraph(CWnd* pParent = nullptr);   // standard constructor
    CGraph(CGraph &) = delete;
    virtual ~CGraph();
    virtual BOOL OnInitDialog();
    void setTest(int test_num) { test_to_run = test_num; };

    // Dialog Data
    enum
    {
        IDD = IDD_GRAPH
    };

    void TestGraph();
    void setJPGFilename(const TCHAR * const cFilename);
    void Sample2();

protected:
    DECLARE_MESSAGE_MAP()

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual void OnCancel();

    // Taken from DlgAddChart.h
    STRUCT_FUNCDATA GenerateData(int funcID, double fMinX, double fMaxX, size_t pntsNmb,
        V_CHARTDATAD& vData, double multY);
    double GenerateSinWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
        double multY);
    double GenerateSincWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
        double multY);
    double GenerateExp(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
        double multY);
    double GenerateRectWave(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
        double multY);
    double GenerateRandomNmbs(size_t nPntNmb, double fMinX, double fDeltaX, V_CHARTDATAD& vData,
        double multY);


private:
    string_t m_JPGFilename;
    time_t m_timestamp;
    string_t m_serial;
    string_t m_tag;

    void GraphBuff();

    CChartContainer m_chartContainer;
    std::vector<time_t> X;
    std::vector<double> Y_flow;
    std::vector<double> Y_temp;
    ULONG_PTR m_nGdiplusToken;
    int test_to_run{ 1 };
};

#endif // Graph_h