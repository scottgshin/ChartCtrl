////////////////////////////////////////////////////////////////////////////////
// 
// SlidrGDI.h : header file
// Declaration of the classes CTipWnd and CSliderGdiCtrl
// Declaration of the template<typename T> class CSliderCtrlT
// 
// Programmer: geoyar
// License: CPOL
// The code is provided "as is" without express or implied warranty.
//
// First version: 04/10/2011
// Last Update:   04/10/2011
//     
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace Gdiplus;

#pragma once

namespace SliderGdi
{

// This is code from Andrei Alexandresku book "Modern C++ Design", ch. 2
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.

class NullType;

// Typelist
template <class T, class U>
struct TypeList
{
  typedef T Head;
  typedef U Tail;
};

#define TYPELIST_1(T1) TypeList<T1, NullType>
#define TYPELIST_2(T1, T2) TypeList<T1, TYPELIST_1(T2)>
#define TYPELIST_3(T1, T2, T3) TypeList<T1, TYPELIST_2(T2, T3)>
#define TYPELIST_4(T1, T2, T3, T4) TypeList<T1, TYPELIST_3(T2, T3, T4)>
#define TYPELIST_5(T1, T2, T3, T4, T5) TypeList<T1, TYPELIST_4(T2, T3, T4, T5)>
#define TYPELIST_6(T1, T2, T3, T4, T5, T6) TypeList<T1, TYPELIST_5(T2, T3, T4, T5, T6)>
#define TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) TypeList<T1, TYPELIST_6(T2, T3, T4, T5, T6, T7)>
#define TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) TypeList<T1, TYPELIST_7(T2, T3, T4, T5, T6, T7, T8)>
#define TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) \
                                                TypeList<T1, TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9)>

// IndexOf
template <class TList, typename T> struct IndexOf;

template <typename T> 
struct IndexOf<NullType, T>
{
  enum { value = -1};
};

template <class Tail, typename T>
struct IndexOf<TypeList<T, Tail>, T>
{
  enum {value = 0};
};

template <class Head, class Tail, typename T>
struct IndexOf<TypeList<Head, Tail>, T>
{
private:
  enum {temp = IndexOf<Tail, T>::value};
public:
  enum {value = temp == -1 ? -1 : 1 +temp};
};

// This static assert code is from Loki static_check.h (<int> changed to <bool>
template<bool> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define STATIC_CHECK(expr, msg) \
{ CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; } 



// End of Andrei's code

// List of 'lega'l PODs
typedef TYPELIST_9(byte, short, unsigned short, int, unsigned 
                    int, long, unsigned long, float, double) SL_TYPELIST;

// Global Helper Functions
  void CreateRoundedRect(GraphicsPath& grPath, RectF& rectF, float radius);
  CRect RectFToRect(const RectF& rectF);
  RectF RectToRectF(const CRect& rect);
  PointF GetRectFCenter(const RectF& rectF);
  CPoint PointFToPoint(const PointF& pntF);
  double round_number(const double val, const int precision = 0);

///////////////////////////////////////////////////////////////////////////////
// Class CTipWnd

class CTipWnd : public CWnd
{
  DECLARE_DYNAMIC(CTipWnd)

public:
	CTipWnd();
	virtual ~CTipWnd();

protected:
	DECLARE_MESSAGE_MAP()

public:
  BOOL CreateTipWnd(const CRect parentrRect, const std::wstring& maxStr, int strNmb);
  BOOL RecalcTipWnd(const CRect parentRect, const std::wstring& maxStr, int strNmb);
  void CaptionStr(const std::wstring& szCaption) { m_szCaption = szCaption; }
  void ValStr(const std::wstring& szValStr) { m_szValStr = szValStr;}
  void UpdateTipWnd(const std::wstring& szValStr, const float posX, bool bShow);
private:
  CRect SetTipWndRects(Gdiplus::Graphics* grPtr, const CRect& parentRect, const std::wstring& maxStr,
                                                                                           int strNmb);
  Gdiplus::SizeF GetTextBoxSize(Gdiplus::Graphics* grPtr, const float tipWndWidth, 
                                                               const std::wstring& maxStr, int strNmb);
  void DrawTipWnd(Gdiplus::Graphics* grPtr);
  void DrawTipText(Gdiplus::Graphics* grPtr);

private:
  bool m_bAbove;                        // Tip above or below the slider
  std::wstring m_szCaption;
  std::wstring m_szValStr;
  Gdiplus::RectF m_textRectF;
  Gdiplus::RectF m_tipWndRectF;
  Gdiplus::PointF m_tipPntF;            // Balloon tip point
  Gdiplus::StringFormat m_strFormat;
  std::wstring m_fontFamilyStr;
  float m_fontSize;                     // Tip point size, in points
  Gdiplus::SizeF m_textBoxSF;

public:
  afx_msg void OnPaint();
};

///////////////////////////////////////////////////////////////////////////////
// CSliderGdiCtrl

#define MIN_FONTH 8 
#define MAX_FONTH 18

// Data Label values
#define EMSIZE_WW 0.3f
#define LAB_W 1.5f

#define LAB_INFLW 2.0f
#define LAB_INFLH 3.0f

class CSliderGdiCtrl : public CWnd
{
	DECLARE_DYNAMIC(CSliderGdiCtrl)

public:
  enum SLIDE_STATUS{ UNINIT, IDLE, LBTNDOWN, HOVERING};

public:
  CSliderGdiCtrl(int typeID = IndexOf<SL_TYPELIST, double>::value); 

	virtual ~CSliderGdiCtrl();

public:
  BOOL CreateSliderGdiCtrl(DWORD dwStyle, const CRect slRect, CWnd* pParent, UINT slID);

private: // Layout and drawing functions
  void InitSliderCtl(Gdiplus::Graphics& gr);
  void DrawBar(Gdiplus::Graphics* grPtr, Gdiplus::GraphicsPath& grPath);
  void DrawValLabel(Gdiplus::Graphics* grPtr, Gdiplus::GraphicsPath& grPath);
  void DrawMoveArea(Gdiplus::Graphics* grPtr, Gdiplus::GraphicsPath& grPath);
  void DrawThumb(Gdiplus::Graphics* grPtr, Gdiplus::GraphicsPath& grPath);

// Access functions
public:
  void SetBarColorDefault(bool bRedraw = false);
  Gdiplus::Color BarColor(void) const { return m_barFirstCol;}
  void BarColor(Gdiplus::Color barFirstCol, bool bRedraw = false); 

  Gdiplus::StringAlignment GetValStrAlignmentH() const { return m_valStrFormat.GetAlignment();}
  Gdiplus::StringAlignment GetValStrAlignmentV() const { return m_valStrFormat.GetLineAlignment();}
  void SetSliderStrFormat(Gdiplus::StringAlignment chAl, Gdiplus::StringAlignment lineAl, bool bRedraw = false);
  Gdiplus::FontStyle GetSliderFontStyle() const {return m_fontStyle;}
  std::wstring GetSliderFontFamilyName() const { return m_fontFamilyStr;}
  const WCHAR* GetSliderFontFamilyStr() const { return m_fontFamilyStr.c_str();}
  float GetSliderFontSize() const {return m_emSize;}
  void SetSliderFontAttributes(Gdiplus::FontStyle  fontStyle, std::wstring& strFontFamilyName, 
                                                                                  bool bRedraw = false);
  void SetSliderFontAttributes(Gdiplus::FontStyle  fontStyle, WCHAR* strFontFamilyName, bool bRedraw = false); 

  std::wstring ValString() const {return m_strValue;}   // Read-only member

private:
  bool IsSliderKeyDown(void); 

  bool ValToPos(double fVal, float& fPos);
  bool PosToVal(float fPos, double& fVal);              // To raw, not rounded val

  std::pair<int, int> GetFixedStrLen(double val, int precision);
  std::pair<int, int> GetSciStrLen(double val, int precision);

  void GetMaxValStr(std::wstring& maxLenStr);
  int GetMaxTipStr(std::wstring& maxString);
  void GetCombCapValString(std::wstring& comboStr);

  std::wstring SetValStr(double val, bool bRound = true);

  bool CheckWStrBounds(Gdiplus::Graphics& gr, const Gdiplus::RectF& rLayoutF, Gdiplus::RectF& rBoundF, 
    const float emSize, const Gdiplus::FontStyle fStyle, const int lines, const std::wstring& str);

// More public access functions
public:
  std::wstring GetCaption() { return m_strCaption;}
  void SetCaption(std::wstring& caption, bool bRedraw = false); 
  void SetCaption(WCHAR* caption, bool bRedraw = false); 
//                              { SetCaption(std::wstring(caption),bRedraw);}

  int Precision(void) const {return m_precision;}
  bool SetPrecision(int precision, bool bRedraw = false);
  double GetKeyStep (void) const;
  
// Access to thumb position (value)
  double GetCurrValue(void) const ;
  bool SetCurrValue(double value, bool bRedraw = false);

// Get min and max vals
  double GetMinVal(void) const;   // Output the rounded min and max values
  double GetMaxVal(void) const;

// Set Min and Max values
  bool SetMinMaxVal(double minVal, double maxVal, bool bAdjustCurrVal = false, bool bRedraw = false);

// Helpers
public:
// Set all init values and check for sanity
  bool SetInitVals(double minVal, double maxVal, int precision, double currVal, 
                                                                          bool bRedraw = false);
  int GetMaxPrecision(double value) const;
private:
  void SetEmSizeLims(Gdiplus::Graphics& grPtr, int minH, int maxH);
  float CalcEmSize(float rectH);
  float CalcDataLabAndMoveRectsF(Gdiplus::Graphics& gr, const Gdiplus::RectF& clRF, 
                                Gdiplus::RectF& labRF, Gdiplus::RectF& moveRF);
  bool IsThumbCenterVisible(void);
  bool RecalcTipWnd(void);
  void UpdateTipWnd(bool bShow = true);
  bool DestroyTipWnd(void);

public:
  bool IsSliderFullyVisible(int dx, int dy, int dW, int dH,CRect* visibleRectPtr);

protected:
	DECLARE_MESSAGE_MAP()

// Message Handlers
public:
  LRESULT SendNotification(UINT code = TB_THUMBPOSITION);

  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);

  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKillFocus(CWnd* pNewWnd);

  afx_msg void OnMove(int x, int y);
  afx_msg void OnSize(UINT nType, int cx, int cy);

  afx_msg void OnPaint();

private:
  float m_maxEmSize;      // Size of 18-pnt font for given screen
  float m_minEmSize;      // Size of 8-pnt font

  int m_typeID;           // Index of the slider data type: -1 if not supported, 
                          // 0 -6 for int, 7 for float, 8 for double
  SLIDE_STATUS m_slStat;  // For right painting
  std::wstring m_strCaption; 
  std::wstring  m_strValue; 

  double m_fPosScale;    // For thumb position to value and vice versa
  double m_fMinVal;
  double m_fMaxVal;
  double m_fCurrValue;

// Slider geometry  
  Gdiplus::RectF m_rBarF;       // Slider window rect
  Gdiplus::RectF m_rValLabelF;  // Data label
  Gdiplus::RectF m_rSelectF;    // Range of position of the thumb's center
  Gdiplus::RectF m_rThumbF;     // Thumb rect
  Gdiplus::RectF m_rMoveF;      // Range the thumb is moving in

  Gdiplus::Color m_barFirstCol; // First color of the bar's linear brush

// String related members
  Gdiplus::StringFormat m_valStrFormat;
  Gdiplus::FontStyle m_fontStyle;
  std::wstring m_fontFamilyStr;
  float m_emSize;

// Flag for WM_MOUSELEAVE
  BOOL m_bMouseTracking;
  int m_precision;              // Number of digits right of floating point
                                // if positive; left non-significant digit (zeros) if negative
  int m_digitNmb;               // Number of significant digits, defines range of precision 
  CTipWnd* m_tipWndPtr;         // Pointer to the popup tip window 

};

///////////////////////////////////////////////////////////////////////////////
// Class CSliderGdiCtrlT - templated wrapper for CSliderGdiCtrl

template <typename T>
class CSliderGdiCtrlT:public CSliderGdiCtrl
{
public:
  CSliderGdiCtrlT(void):CSliderGdiCtrl(IndexOf<SL_TYPELIST, T>::value) {}
  virtual ~CSliderGdiCtrlT() {}

public:
  template <typename U>
  bool IsRightSlType(void ) const {return (m_typeID == IndexOf<SL_TYPELIST, U>::value);}

// Access to m_fCurrValue of CSliderGdiCtlr
   T GetCurrValue(void) const {return static_cast<T>(CSliderGdiCtrl::GetCurrValue());}

// Set Curr Value - always clip value out of range max - min to max or min; return false if 
// value was clipped 
  template <typename T1>
  bool SetCurrValue(T1 value, bool bRedraw = false) 
  { 
    STATIC_CHECK((IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T1>::value), SetCurrValue);

    return CSliderGdiCtrl::SetCurrValue(value, bRedraw);
  }

// Get min and max vals
  T GetMinVal(void) const {return static_cast<T>(CSliderGdiCtrl::GetMinVal());}
  T GetMaxVal(void) const {return static_cast<T>(CSliderGdiCtrl::GetMaxVal());}

// Set Min and Max values (Changes nothing and returns false if constraints on min 
//                         and max are violated or bAdjustCurrVal = false
//                         and m_currValue is out of new selection range. Throws a 
//                         compile time assert if T1 and/or T2 are different from T) 
  template <typename T1, typename T2>
  bool SetMinMaxVal(T1 minVal, T2 maxVal, bool bAdjustCurrVal = false, bool bRedraw = false)
  { 
    STATIC_CHECK(((IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T1>::value)&&
        (IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T2>::value)), 
         Wrong_Argument_Types_In_SetMinMaxVal);

    return CSliderGdiCtrl::SetMinMaxVal(minVal, maxVal, bAdjustCurrVal, bRedraw);
  }
// Throws a compile time assert if any of T1, T2, and T3 types is not T.
//  Changes nothing and returns false if any of constraints on params are violated
  template <typename T1, typename T2, typename T3>
  bool SetInitVals(T1 minVal, T2 maxVal, int precision, T3 currVal, bool bRedraw = false)
  { 
    STATIC_CHECK(((IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T1>::value)&&
                       (IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T2>::value) &&
                       (IndexOf<SL_TYPELIST, T>::value == IndexOf<SL_TYPELIST, T3>::value)), 
                        Wrong_Argument_Types_In_SetInitVals);

    return CSliderGdiCtrl::SetInitVals(minVal, maxVal, precision, currVal, bRedraw);
  }
};

} // End namespace SliderGdi
