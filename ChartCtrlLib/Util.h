///////////////////////////////////////////////////////////////////////////////
//
//  Util.h - misc. utility functios
//
//////////////////////////////////////////////////////////////////////////////


#pragma once
#include <string>

using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// Normalize string len

typedef std::basic_string<TCHAR> string_t;
typedef std::basic_stringstream<TCHAR> sstream_t;

string_t NormalizeString(string_t str, size_t maxLen, TCHAR delim);

/////////////////////////////////////////////////////////////////////////////

template <typename T>
bool in_range(const T& lL, const T& rL, const T& val)
{
  return ((val >= lL) && (val <= rL));
}

//////////////////////////////////////////////////////////////////////////////
// Gdiplus RectF conversion functions

inline Rect CRectToGdiRect(CRect& rect)
{
  return Rect(int(rect.left), int(rect.top), int(rect.Width()), int(rect.Height()));
}

inline RectF CRectToGdiRectF(CRect& rect)
{
  return
   RectF(float(rect.left), float(rect.top), float(rect.Width()), float(rect.Height()));
}

inline CRect RectFToCRect(RectF& rectF)  /// In client coord
{
  int left = int(max(floor(rectF.X), 0.0f));
  int top  = int(max(floor(rectF.Y), 0.0f));
  int right = int(ceil(rectF.GetRight()));
  int bottom = int(ceil(rectF.GetBottom()));
  return CRect(left, top, right, bottom);
}

inline RectF GdiRectToRectF(Rect rect)
{
  return
    RectF(float(rect.X), float(rect.Y), float(rect.Width), float(rect.Height));
}

inline RectF RectFFromCenterF(const PointF pntCenterF,
                                                   float fWidth, float fHeight)
{
  RectF rectF(pntCenterF.X, pntCenterF.Y, 0.0f, 0.0f);
  rectF.Inflate(fWidth/2.0f, fHeight/2.0f);
  return rectF;
}

inline PointF GetRectFCenterF(const RectF rectF)
{
  PointF pntF;
  pntF.X = rectF.X + rectF.Width/2.0f;
  pntF.Y = rectF.Y + rectF.Height/2.0f;
  return pntF;
}

inline CPoint CPointFromPntF(PointF pntF)
{
  CPoint pntC;
  float fX = pntF.X < 0.0f ? pntF.X - 0.5f : pntF.X + 0.5f;
  float fY = pntF.Y < 0.0f ? pntF.Y - 0.5f : pntF.Y + 0.5f;
  pntC.x = static_cast<int>(fX);
  pntC.y = static_cast<int>(fY);
  return pntC;
}

void CreateRoundedRect(GraphicsPath& grPath, RectF& rectF, float radius, bool bReset);

inline Color SetAlpha(const Color color, int alpha)
{
  ARGB locArgb = color.GetValue();
  locArgb &= ~Color::AlphaMask;
  locArgb |= (alpha << Color::AlphaShift);
  return Color(locArgb);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Predicates

template <typename T>
class PointT;

template <typename T, bool bY>
struct coord_in_range
{
  T _left;
  T _right;
  coord_in_range(T left, T right):_left(left), _right(right) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T val = pnt.Y;
    return ((val > _left)&&(val < _right));
  }
};

// Partial specialization
template <typename T>
struct coord_in_range<T, false>
{
  T _left;
  T _right;
  coord_in_range(T left, T right):_left(left), _right(right) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T val = pnt.X;
    return ((val > _left)&&(val < _right));
  }
};

template <typename T, bool bY>
struct less_pnt
{
  inline bool operator () (const PointT<T>& Left, const PointT<T>& Right)
  {
    return Left.Y < Right.Y;
  }
};

// Partial specialization
template <typename T>
struct less_pnt<T, false>
{
  inline bool operator () (const PointT<T>& Left, const PointT<T>& Right)
  {
    return Left.X < Right.X;
  }
};

template <typename T, bool bY>
struct equal_coord
{
  T _coord;
  equal_coord(T coord) : _coord(coord) {}
  inline bool operator () (const PointT<T>& pntT)
  {
    return (pntT.Y == _coord);
  }
};

// Partial specialization
template <typename T>
struct equal_coord<T, false>
{
  T _coord;
  equal_coord(T coord) : _coord(coord) {}
  inline bool operator () (const PointT<T>& pntT)
  {
    return (pntT.X == _coord);
  }
};

template <typename T, bool bY>
struct greater_or_equal
{
  T _val;
  greater_or_equal(T val): _val(val) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    return  _val : pnt.Y >= _val;
  }
};

// Partial specialization
template <typename T>
struct greater_or_equal<T, false>
{
  T _val;
  greater_or_equal(T val): _val(val) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    return pnt.X >= _val;
  }
};

template <typename T>
struct in_vicinity
{
  PointT<T> _origPntT, _epsPntT;
  in_vicinity(PointT<T> origPntT, PointT<T> epsPntT): _origPntT(origPntT),
                              _epsPntT(PointT<T>(abs(epsPntT.X), abs(epsPntT.Y))) {}
  bool operator ()(const PointT<T>& pntT)
  {
      PointT<T> vicT = pntT - _origPntT;
      return ((abs(vicT.X) < _epsPntT.X)&& (abs(vicT.Y) < _epsPntT.Y));
  }
};

template <class Pnt, bool bY>
struct lesser_adjacent_interval
{
  Pnt _epsPnt;
  lesser_adjacent_interval(Pnt epsPnt):_epsPnt(epsPnt) {}
  inline bool operator () (const Pnt& leftPnt, const Pnt& rightPnt)
  {
    bool bRes;
    if (rightPnt.Y == leftPnt.Y)
      bRes = false;
    else
      bRes = (rightPnt.Y - leftPnt.Y) < _epsPnt.Y;
    return bRes;
  }
};

// Partial Specialization for false
template <class Pnt>
struct lesser_adjacent_interval<Pnt, false>
{
  Pnt _epsPnt;
  lesser_adjacent_interval(Pnt epsPnt):_epsPnt(epsPnt) {}
  inline bool operator () (const Pnt& leftPnt, const Pnt& rightPnt)
  {
    bool bRes;
    if (rightPnt.X == leftPnt.X)
      bRes = false;
    else
      bRes = (rightPnt.X - leftPnt.X) < _epsPnt.X;
    return bRes;
  }
};

template <typename T>
struct time_series_to_pnt
{
  size_t _idx;
  T _startX;
  T _stepX;
  time_series_to_pnt(T startX, T stepX) :
              _startX(startX), _stepX(stepX), _idx(0) {}
  inline PointT<T> operator () (const T& valY)
  {
    PointT<T> pnt(_startX + _idx*_stepX, valY);
    ++_idx;
    return pnt;
  }
};


template <typename T>
struct xy_to_pnt
{
  xy_to_pnt() {}
  inline PointT<T> operator () (const std::pair<T, T>& XY)
  {
    PointT<T> pnt(XY.first, XY.second);
    return pnt;
  }
};

// Receives dereferenced map iterator and returns the second of the pair key-value
template <typename T1, typename T2>
struct get_map_value
{
  inline T2 operator () (const std::pair<T1, T2>& val_pair)
  {
    return val_pair.second;
  }
};

template <typename T, bool bY>
struct count_in_range
{
  T _left;
  T _right;
  count_in_range(T left, T right):_left(left), _right(right) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T compVal = pnt.Y;
    return ((compVal >= _left) && (compVal <= _right));
  }
};

// Partial specialization
template <typename T>
struct count_in_range<T, false>
{
  T _left;
  T _right;
  count_in_range(T left, T right):_left(left), _right(right) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T compVal = pnt.X;
    return ((compVal >= _left) && (compVal <= _right));
  }
};

template <typename T, bool bY>
struct nearest_to     // Apply to sorted sequence only
{
  PointT<T> _origPnt;
  T _distance;
  nearest_to(PointT<T> origPnt, T distance):_origPnt(origPnt), _distance(distance) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T value = pnt.Y;
    T  orig = _origPnt.Y;
    T dist = orig - value;
    if (dist < T(0)) dist = -dist;
    if (dist > _distance)
      return true;
    _distance = dist;
    return false;
  }
};

// Partial specialization
template <typename T>
struct nearest_to<T, false>     // Apply to sorted sequence only
{
  PointT<T> _origPnt;
  T _distance;
  nearest_to(PointT<T> origPnt, T distance):_origPnt(origPnt), _distance(distance) {}
  inline bool operator () (const PointT<T>& pnt)
  {
    T value = pnt.X;
    T  orig = _origPnt.X;
    T dist = orig - value;
    if (dist < T(0)) dist = -dist;
    if (dist > _distance)
      return true;
    _distance = dist;
    return false;
  }
};

template <typename T, bool bY>
struct nmb_to_string
{
  typedef std::basic_string<_TCHAR> string_t;
  typedef std::basic_stringstream<TCHAR> sstream_t;
  typedef string_t (__stdcall *val_label_str_fn)(double val, int precision, bool bAddEqSign);

  int _prec;
  val_label_str_fn _pStrFn;
  nmb_to_string(int prec, val_label_str_fn pStrFn) : _prec(prec), _pStrFn(pStrFn) {}
  string_t operator()(const PointT<T>& pnt)
  {
    T val = pnt.Y;
    string_t res = _pStrFn(val, _prec, false);
    return res;
  }
};

// Partial specialization for pntD.X
template <typename T>
struct nmb_to_string <T, false>
{
  typedef std::basic_string<_TCHAR> string_t;
  typedef std::basic_stringstream<TCHAR> sstream_t;
  typedef string_t (__stdcall *val_label_str_fn)(double val, int precision, bool bAddEqSign);

  int _prec;
  val_label_str_fn _pStrFn;
  nmb_to_string(int prec, val_label_str_fn pStrFn) : _prec(prec), _pStrFn(pStrFn) {}
  string_t operator()(const PointT<T>& pnt)
  {
    T val = pnt.X;
    string_t res = _pStrFn(val, _prec, false);
    return res;
  }
};

template <typename T, bool bY>
struct not_inside_range
{
  T _lhs;
  T _rhs;
  bool _bFnd;
  not_inside_range(T lhs, T rhs) : _lhs(lhs), _rhs(rhs), _bFnd(false) {}

  inline std::pair<bool, bool> operator () (const PointT<T>& pntT)
  {
    bool bLeft = false;
    bool bRight = false;
    if (pntT.Y < _lhs)
      bLeft = true;
    else if (!_bFnd && (pntT.Y == _lhs))
    {
      bLeft = true;
      _bFnd = true;
    }
    else if (pntT.Y >= _rhs)
      bRight = true;
    std::pair<bool, bool> pair_res(bLeft, bRight);
    return pair_res;
  }
};

// Partial specialization
template <typename T>
struct not_inside_range<T, false>
{
  T _lhs;
  T _rhs;
  bool _bFnd;
  not_inside_range(T lhs, T rhs) : _lhs(lhs), _rhs(rhs), _bFnd(false) {}

  inline std::pair<bool, bool> operator () (const PointT<T>& pntT)
  {
    bool bLeft = false;
    bool bRight = false;
    if (pntT.X < _lhs)
      bLeft = true;
    else if (!_bFnd && (pntT.X == _lhs))
    {
      bLeft = true;
      _bFnd = true;
    }
    else if (pntT.X >= _rhs)
      bRight = true;
    std::pair<bool, bool> pair_res(bLeft, bRight);
    return pair_res;
  }
};

template <typename T, int Idx>
struct get_max_str
{
  Gdiplus::Font* _pFont;
  Graphics* _grPtr;
  RectF     _maxRF;
  get_max_str(Gdiplus::Font *pFont, Graphics* grPtr): _pFont(pFont), _grPtr(grPtr) {}
  void operator () (const T& t)
  {
    RectF rF;
    PointF pntF(0.0f, 0.0f);
    string_t str = get<Idx>(t);
#ifdef _UNICODE
    _grPtr->MeasureString(str.c_str(),-1, _pFont, pntF, &rF);
#else if _MBCS
    USES_CONVERSION;
    _grPtr->MeasureString(CA2W(str.c_str()),-1, _pFont, pntF, &rF);
#endif
    if (rF.Width > _maxRF.Width)
      _maxRF = rF;
  }
};

// Partial specialization
template <int Idx>
struct get_max_str <string_t, Idx>
{
  Gdiplus::Font* _pFont;
  Graphics* _grPtr;
  RectF     _maxRF;
  get_max_str(Gdiplus::Font *pFont, Graphics* grPtr): _pFont(pFont), _grPtr(grPtr) {}
  void operator () (const string_t& str)
  {
    RectF rF;
    PointF pntF(0.0f, 0.0f);
#ifdef _UNICODE
    _grPtr->MeasureString(str.c_str(),-1, _pFont, pntF, &rF);
#else if _MBCS
    USES_CONVERSION;
    _grPtr->MeasureString(CA2W(str.c_str()),-1, _pFont, pntF, &rF);
#endif
    if (rF.Width > _maxRF.Width)
      _maxRF = rF;
  }
};

///////////////////////////////////////////////////////////////////////////////
// Algorithms

//Find closest to X or Y coord of some origin point; apply to sorted sequences only
template<class _InIt, class _Pr>
inline _InIt find_nearest(_InIt _First, _InIt _Last, _Pr _Pred)
{
    //_DEBUG_RANGE(_First, _Last);
    //_DEBUG_POINTER(_Pred);

  _InIt _NearestIt = _First; // Find first satisfying _Pred
    for (; _First != _Last; ++_First)
  {
        if (_Pred(*_First))
            break;
    _NearestIt = _First;
  }

    return (_NearestIt);
}

// Search for the points with X coordinates closest to, but no inside some X range.
// Apply to sorted sequeces only
template <class _InIt, class _Pr>
inline std::pair<_InIt, _InIt> find_border_pnts(_InIt _First, _InIt _Last, _Pr _Pred)
{
    //_DEBUG_RANGE(_First, _Last);
    //_DEBUG_POINTER(_Pred);

  std::pair<_InIt, _InIt> pair_res_alg = make_pair(_First, _Last);
  for (; _First != _Last; ++_First )
  {
    std::pair<bool, bool> pair_pred_res = _Pred(*_First);
    if (pair_pred_res.first == true)
      pair_res_alg.first = _First;
    if (pair_pred_res.second == true)
    {
      pair_res_alg.second = _First;
      break;
    }
  }
  return pair_res_alg;
}






