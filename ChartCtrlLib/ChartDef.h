///////////////////////////////////////////////////////////////////////////////
//
// Definitions for the ChartCtrl project
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
//
// Version
//

#define FILEVER            2,0,1,1
#define PRODUCTVER         2,0,1,1
#define STRFILEVER         _T("2.0.1.1")
#define STRPRODUCTVER      _T("2.0.1.1")
#define STRCOMPNAME        _T("geoyar")
#define STRFILEDESCRIPTION _T("ChartCtrlLib")
#define STRFILENAME        _T("ChartCtrlLib.lib")
#define STRPRODNAME        _T("ChartCtrlLib")

template <typename T> 
class PointT
{
public:
  PointT(T x = 0, T y = 0) : X(x), Y(y) {}
  PointT(const PointT &pntT) { X = pntT.X; Y = pntT.Y;}
  PointT(const Gdiplus::PointF& pntF) {X = static_cast<T>(pntF.X); Y = static_cast<T>(pntF.Y);}
  PointT(const CPoint point) {X = static_cast<T>(point.x); Y = static_cast<T>(point.y);}

  PointT operator=(const PointT& pntT) {X = pntT.X; Y = pntT.Y; return *this;}
  PointT operator+(const PointT& pntT) const {return PointT(X + pntT.X, Y + pntT.Y);}
  PointT operator-(const PointT& pntT) const {return PointT(X - pntT.X, Y - pntT.Y);}
  PointT operator*(const PointT& pntT) const {return PointT(X*pntT.X, Y*pntT.Y);}
  PointT operator/(const PointT& pntT) const {ASSERT((pntT.X != 0)&&(pntT.Y != 0));
                            return PointT(X/pntT.X, Y/pntT.Y);}
  bool operator ==(const PointT& pntT) {return (X == pntT.X)&&(Y == pntT.Y);}
// Conversion function
  operator Gdiplus::PointF() { return Gdiplus::PointF(float(X), float(Y));}

public:
    T X;
    T Y;
};

typedef PointT<double> PointD;

// Next could be a template, but it makes sense for floating-points only; Gdiplus has it for float
// This MatrixD is for translation and scaling of double numbers only; no rotations and like
typedef class MatrixD
{
public:
  double m_scX;
  double m_scY;
  double m_offsX;
  double m_offsY;

public:
  MatrixD(double scX = 1.0, double scY = 1.0, double offsX = 0.0, double offsY = 0.0):
                      m_scX(scX), m_scY(scY), m_offsX(offsX), m_offsY(offsY) {}

// Transforms
  void Translate(double offsX, double offsY)
                            { m_offsX += offsX*m_scX; m_offsY += offsY*m_scY; }
  void Scale(double scX, double scY) { m_scX *= scX; m_scY *= scY; }

// Operations on matrixD; if the matrix is not invertible, returns false  
  bool Invert(void) 
  { 
    if (m_scX*m_scY == 0.0)
      return false;

    m_offsX = -m_offsX/m_scX;
    m_offsY = -m_offsY/m_scY;  
    m_scX = 1.0/m_scX;
    m_scY = 1.0/m_scY;
    return true;
  }

  MatrixD* Clone(void)
  { 
    MatrixD* pMatrix = new MatrixD;
    pMatrix->m_scX = m_scX;
    pMatrix->m_scY = m_scY;
    pMatrix->m_offsX = m_offsX;
    pMatrix->m_offsY = m_offsY;
    return pMatrix;
  }

// Transforms PointD to PointF and PointD to pointF
  Gdiplus::PointF TransformToPntF(double locScY, const PointD& pntD)
  {
    double X = m_scX*pntD.X + m_offsX;
    double Y = pntD.Y*locScY;
    Y = m_scY*Y + m_offsY;
    return Gdiplus::PointF(static_cast<float>(X), static_cast<float>(Y));
  }

  PointD TransformToPntD(double locScY, const Gdiplus::PointF& pntF)
  {
    ENSURE(m_scX*m_scY != 0.0);
    MatrixD* matrixDI = Clone();
    matrixDI->Invert();

    double X = pntF.X*matrixDI->m_scX + matrixDI->m_offsX;
    double Y = pntF.Y*matrixDI->m_scY + matrixDI->m_offsY;
    Y /= locScY;

    delete matrixDI;
    return PointD(X, Y);
  }

private:
  MatrixD(const MatrixD& src);
  MatrixD operator =(const MatrixD& src);

} MATRIX_D;

// Predicate to use with the STL algorithm transform
typedef struct transform_and_cast_to_pntF
{
  double _locScY;
  MatrixD* _pMatrixD;

  transform_and_cast_to_pntF(double locScY, MatrixD* pMatrixD) : 
                              _locScY(locScY), _pMatrixD(pMatrixD) {}
  inline Gdiplus::PointF operator() (const PointD& pntD)
  {
    double X = _pMatrixD->m_scX*pntD.X + _pMatrixD->m_offsX;
    double Y = _locScY == 1.0 ? pntD.Y : _locScY*pntD.Y;
    Y = _pMatrixD->m_scY*Y + _pMatrixD->m_offsY;
    return Gdiplus::PointF(float(X), float(Y));
  }
} TRANSFORM_TO_PNTF;

// Revers transformation, is not used for now
typedef struct reverse_transform_and_cast_to_pntD
{
  double _locScY;
  MatrixD* _pMatrixDI;

  reverse_transform_and_cast_to_pntD(double locScY, MatrixD* pMatrixD) : _locScY(locScY)
  {
    _pMatrixDI = pMatrixD->Clone();
    _pMatrixDI->Invert();
  }

  ~reverse_transform_and_cast_to_pntD() {delete _pMatrixDI;}

  inline PointD operator() (const Gdiplus::PointF& pntF)
  {
    double X = _pMatrixDI->m_scX*pntF.X + _pMatrixDI->m_offsX;
    double Y = _pMatrixDI->m_scY*pntF.Y + _pMatrixDI->m_offsY;
    Y /= _locScY;
    return PointD(X, Y);
  }
} REVERSE_TRANSFORM_TO_PNTD;

// typedefs and #declares
typedef std::basic_string<TCHAR> string_t;
typedef std::basic_stringstream<TCHAR> sstream_t;

// Typedefs for CChart and CChartContainer
typedef std::pair<double, double> PAIR_DBLS;

// Typedefs for data vectors and other STL containers
typedef std::vector<PointD> V_CHARTDATAD;
typedef std::vector<string_t> V_VALSTRINGS;
typedef std::vector<Gdiplus::PointF> V_CHARTDATAF;
typedef std::multimap<int, PointD> MAP_SELPNTSD;

// Used to count multiple Y values for the same X: 
// the first member is the iterator to the first occurence, the second is the count
typedef std::pair<V_CHARTDATAD::iterator, int> PAIR_ITNEAREST;

typedef std::pair<V_CHARTDATAD::iterator, V_CHARTDATAD::iterator> PAIR_ITS;
typedef std::vector<string_t> V_CHARTNAMES;

// Typedefs for History
typedef std::pair <double, double> PAIR_POS;
//typedef std::deque<PAIR_POS> DQ_HIST;
typedef std::vector<PAIR_POS> V_HIST;
// Typedefs for container chart map
class CChart;
typedef std::map<int, CChart*> MAP_CHARTS;

// For data view
typedef std::vector<size_t>  V_ROWS;
// For load from file
typedef std::map<string_t, Gdiplus::Color> MAP_CHARTCOLS;
enum NAMES {NAME_AXX, NAME_SAMLEX, NAME_AXY, NAME_SAMPLEY};
typedef std::tuple<string_t, string_t, string_t, string_t> TUPLE_NAMES;
typedef std::map<string_t, TUPLE_NAMES> MAP_NAMES;

// For notification
typedef struct tagNMCHART
{
  NMHDR hdr;
  int chartIdx;
  bool bState;
  double minX;
  double maxX;
  double minY;
  double maxY;
} NMCHART, *PNMCHART;

// Codes: Toggle Visibility
#define CODE_VISIBILITY  1U
// Show points
#define CODE_SHOWPNTS    2U
// Ext X was changed
#define CODE_EXTX        3U
#define CODE_EXTY        4u
#define CODE_REFRESH     5U
// Save (to use in multithreading)
#define CODE_SAVEIMAGE   6U
#define CODE_SAVEDIMAGE  7U
#define CODE_SAVECHARTS  8U
#define CODE_SAVEDCHARTS 9U
// Printing
#define CODE_PRINTING   10U
#define CODE_PRINTED    11U
// Scale change
#define CODE_SCY        12U
// For enabling tracking from popup menu
#define CODE_TRACKING   14U

// Screen values of search X, Y range around a mouse point
#define CHART_DTPNTSZ  6.0f
#define CHART_PNTSTRSH 6.0f

// Defs for CChart container
enum CH_MODE {MODE_FULLX,   MODE_FULLY, MODE_ZOOMINGX, MODE_ZOOMINGY, 
              MODE_ZOOMX,   MODE_ZOOMY, MODE_MOVEY,    MODE_ZOOMEDX,  
              MODE_ZOOMEDY, MODE_MOVEDY}; 

// Data view: Data Window size 7.4" x 9.1" for resolution 96 dpi 
#define DV_RECTW                 710
#define DV_RECTH                 874
#define DV_SPACE                  50

#define IDC_DV_LEFTENDARR       37600
#define IDC_DV_LEFTARR          37601
#define IDC_DV_RIGHTENDARR      37602
#define IDC_DV_RIGHTARR         37603
#define IDC_DV_BTNPRINT         37604
#define IDC_DV_BTNCLOSE         37605     

// For chartContainer context menu
#define IDM_CHART_LEGEND        33771
#define IDM_CHART_BOUNDARIES    33772
#define IDM_CHART_VIEWDATA      33773
#define IDM_CHART_SAVE          33774
#define IDM_CHART_SAVEIMAGE     33775
#define IDM_CHART_PRINT         33776
#define IDM_CHART_ZOOMINX       33777
#define IDM_CHART_MOVERIGHT     33778
#define IDM_CHART_MOVELEFT      33779
#define IDM_CHART_UNDOZOOMMOVEX 33780
#define IDM_CHART_ZOOMINY       33781
#define IDM_CHART_MOVEUP        33782
#define IDM_CHART_MOVEDOWN      33783
#define IDM_CHART_UNDOZOOMMOVEY 33784
#define IDM_CHART_REFRESH       33785
#define IDM_CHART_TRACK         33786
#define IDM_CHART_LOCSCALEY     33787
#define IDM_CHART_SHOWPNTS      33788
#define IDM_CHART_TEST          33789
#define IDM_CHART_FIRSTCHART    33790
#define IDM_CHART_LASTCHART     IDM_CHART_FIRSTCHART + 25

// CChart and Labels Drawing
#define LABLEN_MAX              16
#define DR_SPACE                3.0f
#define AXIS_PENWIDTH           2.0f
#define AXIS_OFFSET             0.0f
#define DELTA_LOCY              0.1
#define MOVE_DELTAX             0.01
#define MOVE_DELTAY             0.01

#define ALPHA_NOPNT             64
#define ALPHA_SELECT            64
#define ALPHA_MINFORLABEL      128

#define PEN_SELWIDTH            6.0f

#define LABEL_DATAHIDDEN        0x01
#define LABEL_NAMESHIDDEN       0x02

// DataView rects offset
#define DATA_ENDOFFS            6.0f

// Limit on chart name, nameY, container nameX
#define STR_MAXLEN              18
#define STR_NORMSIGN            _T('^')

// Value string function typedef
typedef string_t (__stdcall *val_label_str_fn)(double val, int precision, bool bAddEqSign);

enum TUPLE_LIDX {IDX_LNAME, IDX_LNAMEX, IDX_LX, IDX_LNAMEY, IDX_LY, IDX_LCOLOR, IDX_LDASH, IDX_LPEN};
typedef std::tuple<string_t, string_t,string_t, string_t, string_t, Gdiplus::Color, 
                                 Gdiplus::DashStyle, float> TUPLE_LABEL;
typedef std::multimap<int, TUPLE_LABEL> MAP_LABSTR;
typedef std::vector<TUPLE_LABEL> V_LABSTR;

// Container axes
enum AX_XPOS{LEFT, BETWEEN, RIGHT};
enum AX_YPOS{TOP, MIDDLE, BOTTOM};
typedef std::pair<AX_YPOS, float> PAIR_XAXPOS;
typedef std::pair<AX_XPOS, float> PAIR_YAXPOS;

enum DATAVIEW_FLAGS{F_NODATACHANGE = 0, F_NAME = 0x0001, F_NAMEX = 0x0002, F_NAMEY = 0x0004, F_VALX = 0x0008, 
   F_VALY = 0x0010, F_DSIZE = 0x0020, F_APPEND = 0x0040, F_TRUNCATE = 0x0080, F_REPLACE = 0x0100, F_HASCELLSMAP = 0x0200}; 


// Enum and typedefs for chart printing
enum STR_IDX {IDX_NAME, IDX_SCY, IDX_VAL, IDX_NAMEX,IDX_X, IDX_NAMEY, 
                                   IDX_Y, IDX_COL, IDX_DSTYLE, IDX_PWIDTH, IDX_STRTOTAL};
enum PRNLAY_IDX {IDX_BULLETSTART = IDX_Y + 1, IDX_STEPSY};

#define STEPY_VALSTR 0x0001
#define STEPY_VALY   0x0002

typedef std::tuple<float, float, float, float, float, 
                                                  float, float, float, int> TUPLE_PRNLEGLAYOUT; 

typedef std::tuple<string_t, string_t, string_t, string_t, string_t, string_t, string_t, 
                                                 Gdiplus::Color, Gdiplus::DashStyle, float> TUPLE_PRINT;
typedef std::multimap<int, TUPLE_PRINT> MAP_PRNDATA;


// Retrieve version info
inline string_t GetLibFileVersion(void){return string_t(STRFILEVER);}
inline string_t GetLibProductVersion(void) {return string_t(STRPRODUCTVER);}
inline string_t GetLibCompName(void) {return string_t(STRCOMPNAME);}
inline string_t GetLibFileDescr(void) {return string_t(STRFILEDESCRIPTION);}
inline string_t GetLibFileName(void) {return string_t(STRFILENAME );}
inline string_t GetLibProdName(void) {return string_t(STRPRODNAME);}
