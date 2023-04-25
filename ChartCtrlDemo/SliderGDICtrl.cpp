




////////////////////////////////////////////////////////////////////////////////
// 
// SlidrGDI.cpp : implementation file
// Implementation of the classes CTipWnd and CSliderGdiCtrl
// 
// Programmer: geoyar
// License: CPOL
// The code is provided "as is" without express or implied warranty.
//
// First version: 04/10/2011
// Last Update:   04/10/2011
//     
////////////////////////////////////////////////////////////////////////////////

// If you do not use a precompiled header write your own "stdafx.h" that includes 
// standard defines for WIN32 platform and necessary headers like 
// 
//#ifndef WINVER
//#define WINVER 0x601
//#endif
//
//#ifndef _SECURE_ATL
//#define _SECURE_ATL 1
//#endif
//
//#ifndef VC_EXTRALEAN
//#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
//#endif
//
//#define _AFX_ALL_WARNINGS
//
//#ifdef _UNICODE
//#if defined _M_IX86
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#elif defined _M_IA64
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#elif defined _M_X64
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#else
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#endif
//#endif
//
// Include files
//#include <afxext.h>         // MFC extensions
//#include <sstream>
//#include <iomanip>
//#include <cmath>
//#include <limits>
//#include <gdiplus.h>
//
// or uncomment lines 7 - 39 and make the line 44 a comment

#include "stdafx.h"

#include "SliderGdiCtrl.h"

using namespace std;
using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// Utility Functions

namespace SliderGdi
{
// Global helper functions 
  void CreateRoundedRect(GraphicsPath& grPath, RectF& rectF, float radius)
  {
    grPath.Reset();
    float d = radius * 2.0f;

    PointF pt1(rectF.X + radius, rectF.Y);                // Left end of top straight line
    PointF pt2(rectF.X + rectF.Width - radius, rectF.Y);  // Right end of top straight line  
    RectF r1(rectF.X, rectF.Y, d, d);                     // Left top arc bounding rect

    grPath.AddArc(r1, 180, 90);                           // Left top arc
    grPath.AddLine(pt1, pt2);                             // Top straight line

    SizeF sizeRectF;
    rectF.GetSize(&sizeRectF);                            // Get offset's base

    r1.Offset(sizeRectF.Width - d, 0);                    // Right top arc bounding rect
    grPath.AddArc(r1, 270, 90);                           // Right top arc
  
    pt1 = PointF(rectF.GetRight(), rectF.GetTop() + radius);    // Top end of right down line
    pt2 = PointF(rectF.GetRight(), rectF.GetBottom() - radius); // Bottom end
    grPath.AddLine(pt1, pt2);                             // Right line from top to bottom

    r1.Offset(0, sizeRectF.Height - d);                   // Move to the right bottom corner
    grPath.AddArc(r1, 0, 90);                             // Right bottom arc

    pt1 = PointF(rectF.GetRight() - radius, rectF.GetBottom());
    pt2 = PointF(rectF.GetLeft() + radius, rectF.GetBottom());
    grPath.AddLine(pt1, pt2);

    r1.Offset(-sizeRectF.Width + d,0);
    grPath.AddArc(r1, 90, 90);

    pt1 = PointF(rectF.GetLeft(), rectF.GetBottom() - radius);
    pt2 = PointF(rectF.GetLeft(), rectF.GetTop() + radius);
    grPath.AddLine(pt1, pt2);
  }

// Returns the center of Gdiplus::REctF rectangle
  PointF GetRectFCenter(const RectF& rectF)
  {
    PointF pntF;
    rectF.GetLocation(&pntF);
    return pntF + PointF(rectF.Width/2.0f, rectF.Height/2.0f);
  }

// Converts RectF to CRect
  CRect RectFToRect(const RectF& rectF)
  {
    int left = int(ceil(rectF.GetLeft()));
    int top  = int(ceil(rectF.GetTop()));
    int right = int(floor(rectF.GetRight()));
    int bottom = int(floor(rectF.GetBottom()));
    return CRect(left, top, right, bottom);
  }


// Converts MFC CRect to Gdiplus::RectF
  RectF RectToRectF(const CRect& rect)
  {
    return RectF(static_cast<float>(rect.left),
                 static_cast<float>(rect.top),
                 static_cast<float>(rect.Width()),
                 static_cast<float>(rect.Height()));
  }

// Converts Gdiplus::PointF to MFC CPoint
  CPoint PointFToPoint(const PointF& pntF)
  {
    int x = static_cast<int>(round_number(pntF.X));
    int y = static_cast<int>(round_number(pntF.Y));
    return CPoint(x, y);
  }

// Rounds a double val to precision        
  double round_number(const double val, const int precision /*= 0*/)
  {
    double roundBase = pow(10.0, -precision);
    double eps = roundBase /100.0; // To define accuracy
    if ((fabs(val) - 0.0) < eps)
      return 0.0;

    double tmp = val / roundBase;
    long long  tmpInt = static_cast<long long>(val < 0 ? ceil(tmp - 0.5) : floor(tmp + 0.5));
    double res = roundBase*tmpInt;
    return res;
  }

/////////////////////////////////////////////////////////////////////////////
// Implementation and Interfaces

///////////////////////////////////////////////////////////////////////////////
// CTipWnd

  IMPLEMENT_DYNAMIC(CTipWnd, CWnd)
  
  CTipWnd::CTipWnd() : m_fontFamilyStr(L"Verdana"), m_fontSize(8)
  {
    m_strFormat.SetAlignment(StringAlignmentCenter);
    m_strFormat.SetLineAlignment(StringAlignmentCenter);
  }
  
  CTipWnd::~CTipWnd()
  {
  }
  
///////////////////////////////////////////////////////////////////////////////
// CTipWnd Helpers
  
  SizeF CTipWnd::GetTextBoxSize(Graphics* grPtr, const float tipWndWidth, const wstring& maxStr, int strNmb)
  {
    int maxTextLen = maxStr.length();
  
    Gdiplus::Font fn(m_fontFamilyStr.c_str(), m_fontSize);
    float strHeight = fn.GetHeight(grPtr);
    float textH = 1.5f*strNmb*strHeight;
  
// Set Layout rect
    RectF layoutTextRectF(0.0f, 0.0f, tipWndWidth, textH);
// Get text bounding rect for one string
    int charFitted = 0;
    int lineFitted = 0;
    RectF boundRF;  
    while (true)
    {
      grPtr->MeasureString(maxStr.c_str(), -1, &fn, layoutTextRectF, &m_strFormat, &boundRF, &charFitted, &lineFitted); 
      if ((charFitted == maxTextLen)&&(lineFitted == 1))
        break;
      layoutTextRectF.Width += strHeight; // Assume char width is approx. font size
    }
// Empirical result:
    return SizeF(max(float(tipWndWidth),layoutTextRectF.Width), strNmb == 1 ? textH : 1.1f*strNmb*boundRF.Height);
  }

  CRect CTipWnd::SetTipWndRects(Graphics* grPtr, const CRect& parentRect, const wstring& maxStr, int strNmb)
  {
    SizeF textSize = GetTextBoxSize(grPtr, float(parentRect.Width()), maxStr, strNmb);
    int textBoxWidth = int(ceil(textSize.Width + 0.5f));
    int textBoxHeight = int(ceil(textSize.Height + 0.5f));
// Get main display working area 
    CRect workArea;
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &workArea, 0);
// Place tip window
    int leftX = parentRect.left;
    int rightX = leftX + textBoxWidth + 1; 
    if (rightX > workArea.right)
    {
      rightX = parentRect.right;
      leftX  = rightX - textBoxWidth - 1;
    }
  
    m_bAbove = true;
    int topY = parentRect.top - textBoxHeight - 5;
    int bottomY = parentRect.top + parentRect.Height()/2;
    if (topY < workArea.top)
    {
      m_bAbove = false;
      topY = bottomY;
      bottomY = parentRect.bottom + textBoxHeight + 5;
    }
  
    CRect tipWndRect(leftX, topY, rightX, bottomY);// In screen coordinates

    m_tipWndRectF = RectF(0.0f, 0.0f, float(tipWndRect.Width()), float(tipWndRect.Height()));
    m_textRectF = m_bAbove == true ? RectF(0.0f, 0.0f, m_tipWndRectF.Width, textSize.Height) :
    RectF(0.0f, m_tipWndRectF.GetBottom() - textSize.Height, m_tipWndRectF.Width, textSize.Height);
    tipWndRect.right += 1;
    tipWndRect.bottom += 1;
  
    return tipWndRect;
  }
          
  BOOL CTipWnd::CreateTipWnd(const CRect parentRect, const std::wstring& maxStr, int strNmb)
  {
    BOOL bRes = FALSE;
    bRes = CreateEx(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOPMOST|WS_EX_NOACTIVATE,
                                        AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS),
                                        NULL,         // Wnd Title (Caption string ?
                                        WS_POPUP,
                                        0, 0, 0, 0,
                                        NULL,
                                        NULL,


                                        0);
    if (bRes) // Calculate the tipWnd layout and set a transparent color
    {
      Graphics* grPtr = Graphics::FromHWND(m_hWnd);
      CRect tipWndRect = SetTipWndRects(grPtr, parentRect, maxStr, strNmb);
      Color bkgndCol((ARGB)Color::Azure);
      bRes = SetLayeredWindowAttributes(bkgndCol.ToCOLORREF(), 0, LWA_COLORKEY);
  // Important: use only SetWindowPos 
      SetWindowPos(&wndBottom, tipWndRect.left, tipWndRect.top, tipWndRect.Width(), 
                                                    tipWndRect.Height(), SWP_NOACTIVATE);
      delete grPtr;
    }

    return bRes;
  }

  BOOL CTipWnd::RecalcTipWnd(const CRect parentRect, const std::wstring& maxStr, int strNmb)
  {
    if (!IsWindow(m_hWnd))
      return FALSE;

    Graphics* grPtr = Graphics::FromHWND(m_hWnd);
    CRect tipWndRect = SetTipWndRects(grPtr, parentRect, maxStr, strNmb);
    SetWindowPos(&wndBottom, tipWndRect.left, tipWndRect.top, tipWndRect.Width(), 
                                                    tipWndRect.Height(), SWP_NOACTIVATE);
    delete grPtr;
    return TRUE;
  }

  void CTipWnd::UpdateTipWnd(const std::wstring& szValStr, const float posX, bool bShow)
  {
    m_szValStr = szValStr;
    m_tipPntF = m_bAbove ? PointF(posX, m_tipWndRectF.GetBottom()) :
                                PointF(posX, m_tipWndRectF.GetTop());
    if (bShow)
    {
      Invalidate(FALSE);
      CRect tipR;
      GetWindowRect(&tipR);
      SetWindowPos(&wndTopMost, tipR.left,tipR.top, tipR.Width(), tipR.Height(), 
                                                        SWP_SHOWWINDOW|SWP_NOACTIVATE);
    }
  }
  
  void CTipWnd::DrawTipWnd(Graphics* grPtr)
  {
    GraphicsPath grPath;
    CreateRoundedRect(grPath, m_textRectF, 5.0f);
    PointF balloonTipPntsF[3];
    balloonTipPntsF[0] = PointF(m_tipWndRectF.X + 0.48f*m_tipWndRectF.Width,
                          m_bAbove ? m_textRectF.GetBottom() : m_textRectF.GetTop());
    balloonTipPntsF[1] = m_tipPntF;
    balloonTipPntsF[2] = PointF(m_tipWndRectF.X + 0.52f*m_tipWndRectF.Width,
                          m_bAbove ? m_textRectF.GetBottom() : m_textRectF.GetTop());
  
    grPath.AddPolygon(balloonTipPntsF, 3);
    Pen pen((ARGB)Color::Black);
    grPtr->DrawPath(&pen, &grPath);
    SolidBrush br((ARGB)Color::LightYellow);
    grPtr->FillPath(&br, &grPath);
  }
  
  void CTipWnd::DrawTipText(Graphics* grPtr)
  {
    Gdiplus::Font fn(m_fontFamilyStr.c_str(), m_fontSize);
    float strHeight = fn.GetHeight(grPtr);
    RectF currTRectF(m_textRectF.X, m_textRectF.Y, m_textRectF.Width, 1.5f*strHeight);
    SolidBrush textBrush((ARGB)Color::Black);
    if (!m_szCaption.empty())
    {
      grPtr->DrawString(m_szCaption.c_str(), -1, &fn, currTRectF, &m_strFormat, &textBrush);
      currTRectF.Offset(0.0f, strHeight);
    }
    wstring valStr = wstring(L"Val = ") + m_szValStr;
    grPtr->DrawString(valStr.c_str(), -1, &fn, currTRectF, &m_strFormat, &textBrush);
  }
  
  BEGIN_MESSAGE_MAP(CTipWnd, CWnd)
    ON_WM_PAINT()
  END_MESSAGE_MAP()
  
///////////////////////////////////////////////////////////////////////////////
// CTipWnd message handlers
  
  void CTipWnd::OnPaint()
  {
    CPaintDC dc(this);                              // Device context for painting

    Graphics gr(dc.m_hDC);                          // Graphics to paint
    Rect rGdi;
    gr.GetVisibleClipBounds(&rGdi);                 // The same as the client rect

    Bitmap clBmp(rGdi.Width, rGdi.Height);          // Mem bitmap
    Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC: to draw onto memory 
    grPtr->SetSmoothingMode(SmoothingModeAntiAlias);
    grPtr->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    Rect rFill = rGdi;
    rFill.Inflate(1, 1);                            // To eliminate border effect
    SolidBrush bkBrush((ARGB)Color::Azure);         // Set as a transparent color
    grPtr->FillRectangle(&bkBrush, rFill);
    DrawTipWnd(grPtr);
    DrawTipText(grPtr);
  
    gr.DrawImage(&clBmp, rGdi);                     // Transfer onto display surface
    delete grPtr;
  }
 
///////////////////////////////////////////////////////////////////////////////
// CSliderGdiCtrl

  IMPLEMENT_DYNAMIC(CSliderGdiCtrl, CWnd)

// Constructor
CSliderGdiCtrl::CSliderGdiCtrl(int typeID) : 
     m_typeID(typeID), m_slStat(UNINIT),/* m_strValLen(10),*/ m_fMaxVal(255.0), 
     m_fMinVal(0.0),  m_fCurrValue(0.0), m_fontStyle(FontStyleRegular), 
     m_fontFamilyStr(L"Palatino Linotype"), m_bMouseTracking(false), m_precision(1), 
     m_maxEmSize(10.0f), m_minEmSize(24.0f), m_tipWndPtr(NULL)
  {
    m_precision = typeID < IndexOf<SL_TYPELIST, float>::value ? 0 : 1;
    if (typeID == IndexOf<SL_TYPELIST, float>::value)
      m_digitNmb = numeric_limits<float>::digits10;
    else
      m_digitNmb = numeric_limits<double>::digits10;
    SetBarColorDefault();
// Set the value string aligment
    m_valStrFormat.SetAlignment(StringAlignmentCenter);
    m_valStrFormat.SetLineAlignment(StringAlignmentCenter);
// Set val string
    m_strValue = typeID < IndexOf<SL_TYPELIST, float>::value ? wstring(L"0") : wstring(L"0.0");
  }
  
  CSliderGdiCtrl::~CSliderGdiCtrl()
  {
    DestroyTipWnd();
  }

  BOOL CSliderGdiCtrl::CreateSliderGdiCtrl(DWORD dwStyle, const CRect slRect, CWnd* pParent, UINT slID)
  {
    return CWnd::Create(AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS),
                 NULL,         // Wnd Title 
                 dwStyle,
                 slRect,
                 pParent,
                 slID
                 );
  }

///////////////////////////////////////////////////////////////////////////////
// Helpers

  void CSliderGdiCtrl::InitSliderCtl(Graphics& gr)
  {
// Bar
    RectF rectF;
    gr.GetVisibleClipBounds(&rectF);  // Same size and position as the client rect
    rectF.Width -= 1.0f;              // Otherway right and bottom borders are invisible
    rectF.Height -= 1.0f;
    m_rBarF = rectF;
// Data label area and thumb moving area
    SetEmSizeLims(gr, MIN_FONTH, MAX_FONTH);  // TODO: from #define change to memberds?
    m_emSize = CalcEmSize(rectF.Height);
    RectF valLabRectF;
// Calculate data label width
    CalcDataLabAndMoveRectsF(gr, rectF, m_rValLabelF, m_rMoveF);
// Select area (thumb center's position range, subtract the thumb width)
    float thumbWidth = min(m_rMoveF.Height, 0.2f*m_rMoveF.Width);
    RectF moveAreaRectF = m_rMoveF;
    moveAreaRectF.Inflate(-thumbWidth/2.0f, 0.0f);
    m_rSelectF = moveAreaRectF;
// Range on screen and the scale to calculate the thumb offset
    double selOffsetPosRange = m_rSelectF.Width;
    double fValRange = m_fMaxVal - m_fMinVal;
    m_fPosScale = selOffsetPosRange/fValRange;
// Calculate current thumb offset X
    float fCurrPosOffs = static_cast<float>((m_fCurrValue - m_fMinVal) * m_fPosScale); 
// Thumb (button) rect
    RectF thumbRectF(m_rMoveF.GetLeft(), m_rMoveF.GetTop(), thumbWidth, m_rMoveF.Height);
    thumbRectF.Offset(fCurrPosOffs, 0.0f);
    m_rThumbF = thumbRectF;
  }

///////////////////////////////////////////////////////////////////////////////
// Drawing functions
  
  void CSliderGdiCtrl::DrawBar(Graphics* grPtr, GraphicsPath& grPath)
  {
    RectF rectF = m_rBarF;
    grPath.Reset();
    CreateRoundedRect(grPath, rectF, 5.0f);  
// Use a linear brush
    LinearGradientBrush lgbr(rectF, m_barFirstCol, (ARGB)Color::White, 90);
    lgbr.SetBlendTriangularShape(0.35f, 1.0f);
    grPtr->FillPath(&lgbr, &grPath);
// Draw contours  
    Pen bPen(Color(255, 0, 0, 0));
    grPtr->DrawPath(&bPen, &grPath);
  }

  void CSliderGdiCtrl::DrawValLabel(Graphics* grPtr, GraphicsPath& grPath)
  {
    RectF rectF = m_rValLabelF;
    grPath.Reset();
    CreateRoundedRect(grPath, rectF, 5.0f);
  
    SolidBrush sbr(Color(180, 255, 255, 255));
    grPtr->FillPath(&sbr, &grPath); // To whitewash colors
    Pen bPen(Color(255, 0, 0, 0));
    grPtr->DrawPath(&bPen, &grPath);
  
    SolidBrush fntBr((ARGB)Color::Black);
    Gdiplus::Font valFont(m_fontFamilyStr.c_str(), m_emSize, m_fontStyle, UnitWorld);
    grPtr->DrawString(m_strValue.c_str(), -1, &valFont, rectF, &m_valStrFormat, &fntBr);
  }
  
  void CSliderGdiCtrl::DrawMoveArea(Graphics* grPtr, GraphicsPath& grPath)
  {
    RectF rectF = m_rMoveF;
    grPath.Reset();
    CreateRoundedRect(grPath, rectF, 5.0f);
  
    SolidBrush sbr1(Color(180, 255, 255, 255));
    grPtr->FillPath(&sbr1, &grPath); // To whitewash
    Pen bPen(Color(255, 0, 0, 0));
    grPtr->DrawPath(&bPen, &grPath);
  
    wstring wStr;
    std::wstring temp;
    RectF boundRF;
    if (m_rValLabelF.Width == 0) // The value does not fit in data label area
    {  
      GetCombCapValString(temp); // Name + current value
      bool bVal = CheckWStrBounds(*grPtr, rectF, boundRF, m_emSize, m_fontStyle, 1, temp);
      if (bVal)  // Fits in the bar
        wStr = temp;
      else       // Try the name only
      {
        bVal = CheckWStrBounds(*grPtr, rectF, boundRF, m_emSize, m_fontStyle, 1, m_strCaption);
        if (bVal)
          wStr = m_strCaption;
      }
    }
    else        // All is OK, prepare to write the name
    {
      temp = m_strCaption;
      bool bVal = CheckWStrBounds(*grPtr, rectF, boundRF, m_emSize, m_fontStyle, 1, temp);
      if (bVal)
        wStr = m_strCaption;
    }
  
    if (!wStr.empty()) // Draw the string into thumb movement's area
    {
      SolidBrush fntBr((ARGB)Color::Black);
      Gdiplus::Font valFont(m_fontFamilyStr.c_str(), m_emSize, m_fontStyle, UnitWorld);
      grPtr->DrawString(wStr.c_str(), -1, &valFont, rectF, &m_valStrFormat, &fntBr);
    }
  }
  
  void CSliderGdiCtrl::DrawThumb(Graphics* grPtr, GraphicsPath& grPath)
  {
// Calc new thumb position and move thumb rect
    float thumbPos = GetRectFCenter(m_rThumbF).X;
    float selNewPos;
    ValToPos(m_fCurrValue, selNewPos);
    float thumbOffset = selNewPos - thumbPos;
    m_rThumbF.Offset(thumbOffset, 0);
    RectF rectF = m_rThumbF;
  
// Draw
    grPath.Reset();
    CreateRoundedRect(grPath, rectF, 5.0f);
  
    PathGradientBrush pathBr(&grPath);
    INT colCnt = 4;
    Color cols[4] = 
    {
      Color(200, 227, 243, 255),
      m_slStat == LBTNDOWN ? Color(140, 0, 255, 0) :
      (m_slStat == HOVERING) ? Color(140, 255,100, 0) : Color(140, 189, 205, 233),
      Color(200, 103, 150, 161),
      (IsWindowEnabled()) ? Color(200, 76, 118, 146): Color(55, 76, 118, 146)
    };
  
    if (m_slStat == LBTNDOWN)
      pathBr.SetCenterColor(Color(155, 255, 0, 0));   // Red
    else if ((m_slStat == HOVERING)|| (GetFocus() == this))
      pathBr.SetCenterColor(Color(155, 0, 255, 0));   //Green
    else if (!IsWindowEnabled())
      pathBr.SetCenterColor(Color(0, 255, 255, 255));  // White
    else
     pathBr.SetCenterColor(Color(155, 255, 255, 255));  // Default: Bluish Gray
    pathBr.SetSurroundColors(cols, &colCnt);
    grPtr->FillPath(&pathBr, &grPath);
  
    Pen bPen(Color(255, 0, 0, 0));
    grPtr->DrawPath(&bPen, &grPath);
  
    if (GetFocus() == this)   // Draw a focus rectangle 
    {
      Pen fPen((ARGB)Color::Black);
      rectF.Inflate(-3.0f, -3.0f);
      if ((rectF.Width >= 5.0f)&&(rectF.Height >= 5.0f))
      {
        CreateRoundedRect(grPath, rectF, 5.0f);
        fPen.SetDashStyle(DashStyleDash);
        grPtr->DrawPath(&fPen, &grPath);
      }
      else
      {
        fPen.SetWidth(2.0f);
        fPen.SetAlignment(PenAlignmentInset);
        grPtr->DrawPath(&fPen, &grPath);
      }

    }
  }
  
///////////////////////////////////////////////////////////////////////////////
// Access Functions
  
// Colors:
  void CSliderGdiCtrl::SetBarColorDefault(bool bRedraw /* = false*/) 
  {
    m_barFirstCol = Color(255, 91, 122, 202);
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
  }
  
//Set the first color of the bar linear brush; the second color is white
  void CSliderGdiCtrl::BarColor(Color barFirstCol, bool bRedraw /* = false*/)
  {
    m_barFirstCol = barFirstCol;

    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
  }

// Slider strings' format: alignment horiz. and vertical   
  void CSliderGdiCtrl::SetSliderStrFormat(StringAlignment chAl, StringAlignment lineAl, 
                                                                          bool bRedraw/*= false*/)
  {
    m_valStrFormat.SetAlignment(chAl);
    m_valStrFormat.SetLineAlignment(lineAl);
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
  }

// Set slider font style
  void CSliderGdiCtrl::SetSliderFontAttributes(FontStyle fontStyle, wstring& strFontFamilyName,
                                                                          bool bRedraw/*= false*/)
  {
    m_fontStyle = fontStyle;
    m_fontFamilyStr = strFontFamilyName;
    m_slStat = UNINIT; // The string bound rects may change
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
  }
   
  void CSliderGdiCtrl::SetSliderFontAttributes(FontStyle  fontStyle, WCHAR* strFontFamilyName,
                                                                          bool bRedraw/*= false*/)
  {
    wstring famName(strFontFamilyName);
    SetSliderFontAttributes(fontStyle, famName, bRedraw);
  }

// Returns true if arrow key is dowm
  bool CSliderGdiCtrl::IsSliderKeyDown(void)
  {
    short keyState = GetAsyncKeyState(VK_RIGHT)|GetAsyncKeyState(VK_LEFT)|
      GetAsyncKeyState(VK_NEXT)|GetAsyncKeyState(VK_PRIOR)|GetAsyncKeyState(VK_HOME)| 
      GetAsyncKeyState(VK_END);
    return  (keyState & 0x8000) ? true : false;
  }
  
// From value to the thumb's position and back
  bool CSliderGdiCtrl::ValToPos(double fVal, float& fPos)
  {
    if (m_fPosScale == 0)
      return false;
  
    double currValOffset = fVal - m_fMinVal;
    float fPosOffset = static_cast<float>(currValOffset * m_fPosScale);
    fPos = m_rSelectF.X + fPosOffset;
    return true;
  }
  
  bool CSliderGdiCtrl::PosToVal(float fPos, double& fVal)
  {
    if (m_fPosScale == 0)
      return false;
  
    float fPosMin = m_rSelectF.GetLeft();
    float fPosOffset = fPos - fPosMin;
    fVal = m_fMinVal + fPosOffset / m_fPosScale;
    return true;
  }

// Two functions below evaluate str len and precision for fixed and sci float pnt string
  std::pair<int, int> CSliderGdiCtrl::GetFixedStrLen(double val, int precision)
  {
    double fAbsVal = fabs(val);
    if ((fAbsVal - 0.0) < pow(10.0, -precision - 2))
      fAbsVal = pow(10.0, -precision);

    int intDigNmb;      // Number of digits before floating point
    if (fAbsVal <= 1.0)
      intDigNmb = 1;
    else
    {
      double dblIntPart;
      modf(log10(fAbsVal), &dblIntPart);
      intDigNmb = static_cast<int>(dblIntPart) + 1;
    }
    int fixStrLen = intDigNmb;    // Will keep total number of digits + sign + float pnt
    if (precision > 0)
      fixStrLen += precision + 1; // Digits after floating point + the point itself
    else
      precision = 0;              // No floating part
    if (val < 0)
      fixStrLen += 1;

    return make_pair(fixStrLen, precision);// This precision is for value string only
  }

  std::pair<int, int> CSliderGdiCtrl::GetSciStrLen(double val, int precision)
  {
    if (m_typeID < IndexOf<SL_TYPELIST, float>::value)
      return make_pair(INT_MAX, 0);         // No sci presentation for integer types

    double fAbsVal = fabs(val);
    if ((fAbsVal - 0.0) < pow(10.0, -precision - 2))
      fAbsVal = pow(10.0, -precision);

    double dblIntPart;
    modf(log10(fAbsVal), &dblIntPart);
    int intDigNmb = static_cast<int>(dblIntPart) + 1;
    int sciStrLen = intDigNmb + 1 + precision;
    if (val < 0)
      sciStrLen += 1;
    precision += intDigNmb - 1;
    if (precision < 1)
    {
      precision = 1;
      sciStrLen = 3;
    }
    return make_pair(sciStrLen + 5, precision);
  }

// Get lengthier string of min or max values
  void CSliderGdiCtrl::GetMaxValStr(wstring& maxLenStr)
  {
    wstring minValStr = SetValStr(m_fMinVal);
    wstring maxValStr = SetValStr(m_fMaxVal);

    maxLenStr = (minValStr.length() > maxValStr.length()) ? minValStr : maxValStr;
  }
  
// String of max length of minVal, maxVal, or caption - for tipWnd
// Returns number of string to display.
  int CSliderGdiCtrl::GetMaxTipStr(wstring& maxString)
  {
    wstring prefix(L"Val = ");
    wstring maxValStr;
    GetMaxValStr(maxValStr); 
    maxValStr = prefix + maxValStr;
    if (m_strCaption.empty())
    {
      maxString = maxValStr;
      return 1;
    }

    maxString = (maxValStr.length() < m_strCaption.length()) ? m_strCaption : maxValStr;
    return 2;
  } 
  
// Combo string: Caption + ", Val = " + wstring(m_fCurrValue); m_strValue keeps the rounded curr value
  void CSliderGdiCtrl::GetCombCapValString(wstring& comboStr)
  {
    if (m_strCaption.empty())
      comboStr = wstring(L"Val = ") + m_strValue;
    else
      comboStr = m_strCaption + wstring(L", Val = ") + m_strValue;
  }

  wstring CSliderGdiCtrl::SetValStr(double val, bool bRound /*=true*/)
  {
// Round it
    if (bRound)
      val = round_number(val, m_precision);

// Get str len for the fixed and sci format
    std::pair<int, int> fix_lenprec = GetFixedStrLen(val, m_precision);
    std::pair<int, int> sci_lenprec = GetSciStrLen(val, m_precision);
    
    basic_stringstream<WCHAR> wws_t;
    if (fix_lenprec.first < sci_lenprec.first)
      wws_t << fixed << setprecision(fix_lenprec.second) << val;
    else
      wws_t << scientific << setprecision(sci_lenprec.second) << val;
    return wws_t.str();
  }

// Measures string to calculate layout
  bool CSliderGdiCtrl::CheckWStrBounds(Graphics& gr, const RectF& rLayoutF, RectF& rBoundF, const float emSize,
                  const FontStyle fStyle, const int lines, const wstring& str)
  {
    RectF strBoundF;
    int codePntFitted = 0, linesFitted = 0;
    int strlen = (int)str.length();
  
    Gdiplus::Font valFont(m_fontFamilyStr.c_str(), emSize, fStyle, UnitPixel);
    gr.MeasureString(str.c_str(), -1, &valFont, rLayoutF, &m_valStrFormat, &strBoundF, 
      &codePntFitted, &linesFitted);
    rBoundF = strBoundF;
  
    return ((linesFitted > lines) || (codePntFitted < strlen)) ? false : true;
  }
  
  void CSliderGdiCtrl::SetCaption(std::wstring& caption, bool bRedraw /*= false*/) 
  { 
    m_strCaption = caption;
    m_slStat = UNINIT;        // Slider layout to change. TipWnd should be changed too

    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
  }

  void CSliderGdiCtrl::SetCaption(WCHAR* caption, bool bRedraw /* = false*/) 
  { 
    wstring wstrCaption(caption);
    SetCaption(wstrCaption, bRedraw);
  }

  bool CSliderGdiCtrl::SetPrecision(int precision,bool bRedraw /*= false*/)
  {
    int maxPrecision = GetMaxPrecision(max(abs(m_fMinVal), abs(m_fMaxVal)));
    if (precision > maxPrecision)
      return false;

    double step = pow(10.0, -precision); // Increment
// Calculate range; m_fMaxVal and m_fMinVal are the range bounddaries    
    double fRange = m_fMaxVal - m_fMinVal;
    if (step > fRange)
      return false;
    m_precision = precision;              // Set data member
    m_strValue = SetValStr(m_fCurrValue);

    m_slStat = UNINIT;
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
    return true;
  }

// Not used in demos, added just in case...
  double CSliderGdiCtrl::GetKeyStep (void) const
  {
    return pow(10.0, -m_precision);
  }

  double CSliderGdiCtrl::GetCurrValue(void) const
  { 
    double val = round_number(m_fCurrValue, m_precision); 
    return val; 
  }

// This function sets m_fCurrValue to the rounded value
  bool CSliderGdiCtrl::SetCurrValue(double value, bool bRedraw /*= false*/)
  { 
    bool bRes = true;

    double fValue = round_number(value, m_precision); 
    double val = (fValue < m_fMinVal) ? m_fMinVal : (fValue > m_fMaxVal) ? m_fMaxVal : fValue;
    if (val != fValue)
      bRes = false;                        // Signals value was out of range
 
    m_fCurrValue = val;
    m_strValue = SetValStr(val, false);    // Already rounded

    if (bRedraw)  // Changes the number only, no need to recalculate layout
    {
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
      UpdateTipWnd();
    }
    return bRes;
  }

  double CSliderGdiCtrl::GetMinVal(void) const
  {
    double val = round_number(m_fMinVal, m_precision);
    return val;
  }

  double CSliderGdiCtrl::GetMaxVal(void) const
  {
      double val = round_number(m_fMaxVal, m_precision);
      return val;
   }

// Might change the curr value to fit in max-min range and key step to 10% of the range
  bool CSliderGdiCtrl::SetMinMaxVal(double minVal, double maxVal, bool bAdjustCurrVal /*= false*/,
                                                                                        bool bRedraw /*false*/)
  {
    double dbRange = maxVal - minVal;
    if (!_finite(dbRange))
      return false;
    double roundMinVal = round_number(minVal, m_precision);
    double roundMaxVal = round_number(maxVal, m_precision);
    if (roundMaxVal <= roundMinVal)
      return false;
    
    if (m_precision > GetMaxPrecision(max(fabs(minVal), fabs(maxVal))))
      return false;

    double step = pow(10.0, -m_precision);
    if ((roundMaxVal - roundMinVal) < step)
      return false;
    if (!bAdjustCurrVal &&((m_fCurrValue < roundMinVal)||(m_fCurrValue > roundMaxVal)))
      return false;
    
    m_fMinVal = roundMinVal;
    m_fMaxVal = roundMaxVal;

// Now adjust the curr value and key steps
// Set or clip curr value
    m_fCurrValue = (m_fCurrValue < m_fMinVal) ? m_fMinVal : 
                                      (m_fCurrValue > m_fMaxVal) ? m_fMaxVal : m_fCurrValue;
    m_strValue = SetValStr(m_fCurrValue);
 
    m_slStat = UNINIT;   // Probably needs to recalculate layout, scales, and thumb pos
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
    return true;
  }

// Checks all constraints and sets all  essential data members
  bool CSliderGdiCtrl::SetInitVals(double minVal, double maxVal, int precision, 
                                          double currVal, bool bRedraw /*= false*/)
  {
    double dbRange = maxVal - minVal;
    if (!_finite(dbRange))
      return false;
    if (precision > GetMaxPrecision(max(fabs(minVal), fabs(maxVal))))
      return false;

    minVal = round_number(minVal, precision);
    maxVal = round_number(maxVal, precision);

    if (minVal >= maxVal)
      return false;
    double step = pow(10.0, -precision);
    if (step > (maxVal - minVal))
      return false;
    currVal = round_number(currVal, precision);
    if ((currVal < minVal)||(currVal > maxVal))
      return false;

    m_precision = precision;
    m_fMinVal = minVal;
    m_fMaxVal = maxVal;
    m_fCurrValue = currVal;
    m_strValue = SetValStr(m_fCurrValue, false);    // Already rounded

    m_slStat = UNINIT;                              // Might need to recalculate the slider layout
    if (bRedraw)
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);

    return true; 
  }

  int CSliderGdiCtrl::GetMaxPrecision(double value) const
  {
// For integer types
    if (m_typeID < IndexOf<SL_TYPELIST, float>::value)
      return 0;
// For floating-point types float and double
    double absVal = fabs(value);
    double lgAbsVal = log10(absVal);
    double maxDig; 
    modf(lgAbsVal, &maxDig); // Can't use ceil(lgAbsVal) because ceil(log10(1.0)) = 0 (we need 1) 
    int nMaxDig = static_cast<int>(maxDig); 
    if (lgAbsVal >= 0) 
      nMaxDig += 1;
    int maxPrec = m_digitNmb - nMaxDig;
    int minExp = m_typeID == IndexOf<SL_TYPELIST, double>::value ?
      -numeric_limits<double>::min_exponent10 : -numeric_limits<float>::min_exponent10;
    return min(maxPrec, minExp);
  }

// Translates limits on font size from points to pixels. Font size should be from 8 to 18 points
  void CSliderGdiCtrl::SetEmSizeLims(Graphics& grPtr, int minH, int maxH)
  {
    float resY = grPtr.GetDpiY();
    m_minEmSize = resY*(float)minH/72.0f;   // Points to Pixels
    m_maxEmSize = resY*(float)maxH/72.0f;
  }
  
// Calculates font size from the data label area
  float CSliderGdiCtrl::CalcEmSize(float rectH)
  {
    float emSize = EMSIZE_WW*(LAB_W*rectH - 2.0f*LAB_INFLH);
    emSize = min(emSize, m_maxEmSize);
    emSize = max(emSize, m_minEmSize);

    return emSize;
  }
  
// Used in initial layout
  float CSliderGdiCtrl::CalcDataLabAndMoveRectsF(Graphics& gr, const RectF& clRF, RectF& labRF, RectF& moveRF)
  {
    wstring strToMeasure;
    GetMaxValStr(strToMeasure);
  
    labRF = RectF(0, 0, 0, 0);  // Just preparing for worst

    RectF bRF = clRF;
    bRF.Inflate(-LAB_INFLW, -LAB_INFLH);
    float valLabWH = LAB_W*clRF.Height;   // 150% of ctrl height
    float valLabWW = EMSIZE_WW*bRF.Width; // 30% of ctrl width
    float valLabW = 0.0f;
    RectF strBoundRF;
    bool bVal = CheckWStrBounds(gr, bRF, strBoundRF, m_emSize, m_fontStyle, 1, strToMeasure);
    if (bVal) // Fit  in bRF (Only 'lines' matter)
    {
      if (valLabWH > valLabWW)    // Layout: Should fit in 30% of bar width
      {
        if (strBoundRF.Width >= (valLabWW -3.0f*LAB_INFLW))//maxLimW)
          bVal = false;
        else
          valLabW = valLabWW; 
      }
      else
      {
        if (strBoundRF.Width <= (valLabWH - 3.0f*LAB_INFLW))
          valLabW = valLabWH;
        else if (strBoundRF.Width <= (valLabWW - 3.0f*LAB_INFLW))
          valLabW = valLabWW;
        else
          bVal = false;
      }
      if (bVal)
      {
        labRF = RectF(clRF.GetRight() - valLabW, clRF.Y, valLabW, clRF.Height);
        labRF.Inflate(-LAB_INFLW, -LAB_INFLH);
        labRF.X -= LAB_INFLW;
      }
      else
        valLabW = -1;
    }
    else
      valLabW = -1;
  
    if (valLabW > 0)
    {
      moveRF = RectF(clRF.X, clRF.Y , clRF.Width - valLabW + LAB_INFLW, 
                                                                      clRF.Height);
    }
    else              // m_rValLabelF is empty (constructed by default c'tor)
      moveRF = clRF;
    moveRF.Inflate(-LAB_INFLW, -LAB_INFLH);

    return valLabW;
  }

// If the slider size and position are not limited, only part of the slider thumb might be 
// visible; it is responsibility of the programmer to limit the moves and/or resizing of 
// the slider. If the center of the thumb is not visible, the tip window sill be destroyed 
  bool CSliderGdiCtrl::IsThumbCenterVisible()
  {
    PointF thumbPntF = GetRectFCenter(m_rThumbF); 
    CPoint thumbPnt = PointFToPoint(thumbPntF);
    ClientToScreen(&thumbPnt);
    CWnd* pParent = GetParent();
    CRect parentRect;
    pParent->GetClientRect(&parentRect);
    pParent->ClientToScreen(&parentRect);
    return parentRect.PtInRect(thumbPnt) == TRUE ? true : false;    // true - it is visible
  }

  bool CSliderGdiCtrl::RecalcTipWnd()
  {
    if (m_tipWndPtr == NULL)
      return false;

    CRect cR = RectFToRect(m_rMoveF);
    ClientToScreen(&cR);
    wstring maxStr;
    int strNmb = GetMaxTipStr(maxStr);
    if (m_tipWndPtr->RecalcTipWnd(cR, maxStr, strNmb) == TRUE)
    {
      UpdateTipWnd();
      return true;
    }
    return false;
  }

  void CSliderGdiCtrl::UpdateTipWnd(bool bShow)
  {
    if (m_tipWndPtr == NULL)
      return;

    if (!IsThumbCenterVisible())
    {
        DestroyTipWnd();
        return;
     }
  
    float fPosX;
    ValToPos(m_fCurrValue, fPosX);    // Where the center of the thumb is
    CRect slRect;
    GetWindowRect(&slRect);
    CRect tipRect;
    m_tipWndPtr->GetWindowRect(&tipRect);
    fPosX += float(slRect.left - tipRect.left);
    wstring valStr = SetValStr(m_fCurrValue);
    m_tipWndPtr->CaptionStr(m_strCaption);
    m_tipWndPtr->UpdateTipWnd(valStr, fPosX, bShow);
  }

  bool CSliderGdiCtrl::DestroyTipWnd()
  {
    if (m_tipWndPtr != NULL)
    {
      if (IsWindow(m_tipWndPtr->m_hWnd))
        m_tipWndPtr->DestroyWindow();
      delete m_tipWndPtr;
      m_tipWndPtr = NULL;
      return true;
    }
    return false;
  }

// If the slider size and position are not limited, only part of the slidermight be 
// visible; it is responsibility of the programmer to limit the moves and/or resizing of 
// the slider. THe function is not used in the demos 
  bool CSliderGdiCtrl::IsSliderFullyVisible(int dx, int dy, int dW, int dH, CRect* visibleRectPtr)
  {
    CWnd* pParent = GetParent();
    if (pParent == NULL)
    {
      if (visibleRectPtr)
        visibleRectPtr->SetRectEmpty();
      return false;
    }

    CRect slRect;
    GetWindowRect(&slRect);
    int W = slRect.Width() + dW;;
    int H = slRect.Height() + dH;
    slRect.left += dx;
    slRect.top += dy;
    slRect.right = slRect.left + W;
    slRect.bottom = slRect.top + H;

    CRect parentRect;
    pParent->GetClientRect(&parentRect);
    pParent->ClientToScreen(&parentRect);
    parentRect.IntersectRect(&parentRect, &slRect);

    if (visibleRectPtr != NULL)
      *visibleRectPtr = parentRect;
    
    return parentRect.EqualRect(&slRect) == TRUE ? true : false;
  }

  BEGIN_MESSAGE_MAP(CSliderGdiCtrl, CWnd) 
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_KEYDOWN()
    ON_WM_KILLFOCUS()
    ON_WM_KEYUP()
    ON_WM_SIZE()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOVE()
  END_MESSAGE_MAP()
 
///////////////////////////////////////////////////////////////////////////////
// CSliderGdiCtrl message handlers
  
  void CSliderGdiCtrl::OnPaint()
  {
    CPaintDC dc(this);                              // device context for painting
    Graphics gr(dc.m_hDC);                          // Graphics to paint
  
    Rect rGdi;
    gr.GetVisibleClipBounds(&rGdi);                 // The same as the clip rect
// Calculate layout, set rectangles 
    if (m_slStat == UNINIT)
    {
      InitSliderCtl(gr);
      CPoint pnt;                                   // Highlite the thumb if mouse
      GetCursorPos(&pnt);                           // is hovering over moving area
      ScreenToClient(&pnt);
      PointF pntF(float(pnt.x), float( pnt.y));
      if (m_rMoveF.Contains(pntF))
      {
        m_slStat = HOVERING;
        if (!m_bMouseTracking)                      // To catch WM_MOUSELEAVE
        {
          TRACKMOUSEEVENT trMouseEvent;
          trMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
          trMouseEvent.dwFlags = TME_LEAVE;
          trMouseEvent.hwndTrack = this->m_hWnd;
          m_bMouseTracking = TrackMouseEvent(&trMouseEvent);
        }
      }
      else
        m_slStat = IDLE;
      RecalcTipWnd();                               // If it is visible
    }

    Bitmap clBmp(rGdi.Width, rGdi.Height);          // Mem bitmap
    Graphics* grPtr = Graphics::FromImage(&clBmp);  // As memDC
    grPtr->SetSmoothingMode(SmoothingModeAntiAlias);

    GraphicsPath grPath;
  
    DrawBar(grPtr, grPath);
    if (!m_rValLabelF.IsEmptyArea())
      DrawValLabel(grPtr, grPath);
    DrawMoveArea(grPtr, grPath);
    DrawThumb(grPtr, grPath);

    gr.DrawImage(&clBmp, rGdi);                     // Transfer onto screen
    delete grPtr;
  }
  
///////////////////////////////////////////////////////////////////////////////
// Mouse and Key events
   
  void CSliderGdiCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
  {
    if (IsSliderKeyDown()) 
      return;
    if (m_slStat != HOVERING) 
      return;

    SetFocus();
    bool bMoving = false;
    PointF pntF(float(point.x), float(point.y));
    if (m_rMoveF.Contains(pntF))    // In the MoveArea
    {
      SetCapture();                 // Capture the mouse
      CRect boundRect = RectFToRect(m_rMoveF);
      ClientToScreen(&boundRect);
      ClipCursor(&boundRect);       // Restrict the cursor

      if (!m_rThumbF.Contains(pntF))  // Not in thumb, move button
      {
        pntF.X = min(max(pntF.X, m_rSelectF.GetLeft()), m_rSelectF.GetRight());
        PosToVal(pntF.X, m_fCurrValue);
        m_strValue = SetValStr(m_fCurrValue); // Rounded value
        bMoving = true;
      }
      else                          // Move the cursor to the center of the thumb
      {
        PointF pntCF = GetRectFCenter(m_rThumbF);
        CPoint pnt(int(pntCF.X), int(pntCF.Y));
        ClientToScreen(&pnt);
        SetCursorPos(pnt.x, pnt.y);
      }
      m_slStat = LBTNDOWN;
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
   }
  
    if (bMoving)
    {
      SendNotification(TB_THUMBTRACK);
      UpdateTipWnd();
    }
  }
  
// Just sends the TB_THUMBPOSITION and releases mouse
  void CSliderGdiCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint point)
  {
    if (IsSliderKeyDown()) 
      return;
    
    if (m_slStat == LBTNDOWN) // Send TB_THUMBPOSITION
      SendNotification();  
  
    ReleaseCapture();
    ClipCursor(NULL);
  
    PointF pntF(float(point.x), float(point.y));
    if (m_rMoveF.Contains(pntF))
     m_slStat = HOVERING;
    else
      m_slStat = IDLE;
  
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
  }
  
// Show/hide tip window if thumb center is visible
  void CSliderGdiCtrl::OnRButtonDown(UINT nFlags, CPoint point)
  {
    bool bTipDestroyed = DestroyTipWnd(); 

    if (!bTipDestroyed) // m_nTipWnd == NULL already
    {
 // Define whether the thumb is visible     
      if (IsThumbCenterVisible())    // It is visible
      {
        CRect cR = RectFToRect(m_rMoveF);
        ClientToScreen(&cR);
        m_tipWndPtr = new CTipWnd;
        m_tipWndPtr->SetOwner(this);
        wstring maxStr;
        int strNmb = GetMaxTipStr(maxStr);
        BOOL bRes = m_tipWndPtr->CreateTipWnd(cR, maxStr, strNmb);
        if (!bRes)
          DestroyTipWnd();
        else
        {
          UpdateTipWnd();
          SetFocus();
        }
      }
    }
   CWnd::OnRButtonDown(nFlags, point);
  }

// Mouse Movements
  void CSliderGdiCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
  {
    if (IsSliderKeyDown()) 
      return;
    bool bChange = false;
    PointF pntF(float(point.x), float(point.y));
    if (m_slStat != LBTNDOWN)
    {                               // Free to travel everywhere
      if (m_rBarF.Contains(pntF))   // Mouse is over the client window
      {
        if (m_slStat != HOVERING)   // Come from outside
        {
          m_slStat = HOVERING;      
          bChange  = true;
// Prepare to catch WM_MOUSELEAVE; if the mouse is 
// too quick, without this hovering colors will persist
          if (!m_bMouseTracking)
          {
            TRACKMOUSEEVENT trMouseEvent;
            trMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
            trMouseEvent.dwFlags = TME_LEAVE;
            trMouseEvent.hwndTrack = this->m_hWnd;
            m_bMouseTracking = TrackMouseEvent(&trMouseEvent);
          }
        }
      }
      else
      {
        m_slStat = IDLE; 
        bChange = true;
      }
    }
    else if (m_rMoveF.Contains(pntF))  
    {
      bChange = true;
// Mouse might be outside the select rect, captured in the move rect
      if (!m_rSelectF.Contains(pntF)) // Set the thumb to the left or right
      {
        if (pntF.X < m_rSelectF.GetLeft())
          pntF.X = m_rSelectF.GetLeft();
        else if (pntF.X > m_rSelectF.GetRight())
          pntF.X = m_rSelectF.GetRight();
      }
   
      PosToVal(pntF.X, m_fCurrValue);
      m_strValue = SetValStr(m_fCurrValue);
    }
    if (bChange == true)
    {
      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);      
      UpdateTipWnd();
      SendNotification(TB_THUMBTRACK);
    }
  }
  
  LRESULT CSliderGdiCtrl::OnMouseLeave(WPARAM, LPARAM)
  {
    DestroyTipWnd();
  
    m_bMouseTracking = FALSE;  
    m_slStat = IDLE;
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
  
    return 0;
  }

// Key events
  void CSliderGdiCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
  {
    double keyStep = pow(10.0, -m_precision);
    double fCurrValue = m_fCurrValue;

    PointF currPosF = GetRectFCenter(m_rThumbF);
    switch (nChar)
    {
    case VK_RIGHT:
//    case VK_DOWN:
      fCurrValue = min(fCurrValue + keyStep, m_fMaxVal);         break; 
    case VK_LEFT:
//    case VK_UP:
      fCurrValue = max(fCurrValue - keyStep, m_fMinVal);         break; 
    case VK_NEXT:
      fCurrValue = min(fCurrValue + 10.0f*keyStep, m_fMaxVal);   break; 
    case VK_PRIOR:
      fCurrValue = max(fCurrValue - 10.0f*keyStep, m_fMinVal); break; 
    case VK_END:
      fCurrValue = m_fMaxVal;                                    break;
    case VK_HOME:
      fCurrValue = m_fMinVal;                                    break;
    default:
      CWnd::OnKeyDown(nChar, nRepCnt, nFlags);                   return;
    }

    m_fCurrValue = round_number(fCurrValue, m_precision);
// Calculate new thump position
    float target = 0;
    ValToPos(m_fCurrValue, target);

    m_strValue = SetValStr(m_fCurrValue, false);
  
// Get new cursor position and move cursor; do not need capture because mouse handlers disabled
    CPoint cursorPnt = CPoint(int(target), int(currPosF.Y + 0.5f));
    ClientToScreen(&cursorPnt);
    SetCursorPos(cursorPnt.x, cursorPnt.y);

    m_slStat = LBTNDOWN;
  
//Update window
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
  
    SendNotification(TB_THUMBTRACK);
    UpdateTipWnd();
  
// To send notification messages
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
  }
  
  
// Sends TB_THUMBPOSITION
  void CSliderGdiCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
  {
    if (m_slStat == LBTNDOWN)
      SendNotification();
    CPoint pnt;
    GetCursorPos(&pnt);
    ScreenToClient(&pnt);
    PointF pntF(float(pnt.x), float(pnt.y));
    m_slStat = HOVERING;        // OnKeyDown moved cursor to the center of the slider's rect 
    if (!m_bMouseTracking)
    {
      TRACKMOUSEEVENT trMouseEvent;
      trMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
      trMouseEvent.dwFlags = TME_LEAVE;
      trMouseEvent.hwndTrack = this->m_hWnd;
      m_bMouseTracking = TrackMouseEvent(&trMouseEvent);
    }
  
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
  }
  
// Windows messages
// Corrects the slider state according to focus  
  void CSliderGdiCtrl::OnKillFocus(CWnd* pNewWnd)
  {
    CWnd::OnKillFocus(pNewWnd);
   
    CPoint cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(&cursorPos);

    PointF cursorPosF(float(cursorPos.x), float(cursorPos.y));
    if (m_rSelectF.Contains(cursorPosF))
      m_slStat = HOVERING;
    else
      m_slStat = IDLE;
  
    if (GetCapture() == this)
    {
      ClipCursor(NULL);
      ReleaseCapture();
    }
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE); 
  }
    
  void CSliderGdiCtrl::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/) // To suppress Level four warnings
  {
    m_slStat = UNINIT;   // Will recalculate layout
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);   
  }

  void CSliderGdiCtrl::OnMove(int /*x*/, int /*y*/)   // To suppress Level four warnings
  {
    m_slStat = UNINIT;
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);   
  }
  
// Sends notifications to parent
  
  LRESULT CSliderGdiCtrl::SendNotification(UINT code)
  {
    NMHDR nmhdr;
    nmhdr.hwndFrom = m_hWnd;
    nmhdr.idFrom = GetDlgCtrlID();
    nmhdr.code = code;
  
    CWnd* parentPtr = (CWnd*)GetParent();
    if (parentPtr != NULL)
      return parentPtr->SendMessage(WM_NOTIFY, WPARAM(nmhdr.idFrom), LPARAM(&nmhdr));
    return 0;
  }
  
}
  
  
  
  
  
  
