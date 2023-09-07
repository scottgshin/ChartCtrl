///////////////////////////////////////////////////////////////////////////////
//
//  Util.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "util.h"
#include <sstream>

using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// Normalize string len

string_t NormalizeString(string_t str, size_t maxLen, TCHAR delim)
{
    size_t strLen = str.length();
    if (strLen > maxLen)
    {
        size_t position = maxLen - 2;
        str[position++] = delim;
        str[position++] = str[strLen - 1];
        str.erase(position, strLen);
    }
    return str;
}


///////////////////////////////////////////////////////////////////////////////
// Create rounded rect

void CreateRoundedRect(GraphicsPath& grPath, RectF& rectF, float radius, bool bReset)
{
    if (bReset)
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

    r1.Offset(-sizeRectF.Width + d, 0);
    grPath.AddArc(r1, 90, 90);

    pt1 = PointF(rectF.GetLeft(), rectF.GetBottom() - radius);
    pt2 = PointF(rectF.GetLeft(), rectF.GetTop() + radius);
    grPath.AddLine(pt1, pt2);
}

//
// Methods to add functionality to the GDI+ functionality
//
// This method will check to see if the coord string has \n's and correct for that.
Status MeasureString(Graphics* grPtr, IN const string_t string, IN INT,
                     IN const Gdiplus::Font* font, IN const RectF& layoutRect,
                     OUT RectF* boundingBox)
{
    RectF box;
    RectF maximum;
    std::basic_stringstream<TCHAR> ss(string);
    TCHAR line[50];
    auto num_lines(0);
    Status retval{};
    while (ss.getline(line, 50))
    {
        num_lines++;
#ifdef _UNICODE
        retval = grPtr->MeasureString(line, -1, font, layoutRect, &box);
#else if _MBCS
        retval = grPtr->MeasureString(CA2W(line), -1, font, layoutRect, &box);
#endif
        if (maximum.Width < box.Width)
        {
            maximum.Width = box.Width;
        }
        if (maximum.Height < box.Height)
        {
            maximum.Height = box.Height;
        }
    }
    boundingBox->Width = maximum.Width;
    boundingBox->Height = maximum.Height * num_lines;
    return retval;
}

// This method will check to see if the coord string has \n's and correct for that.
Status MeasureString(Graphics* grPtr, IN const string_t string, IN INT,
                     IN const Gdiplus::Font* font, IN const PointF& layoutPoint,
                     OUT RectF* boundingBox)
{
    RectF box;
    RectF maximum;
    std::basic_stringstream<TCHAR> ss(string);
    TCHAR line[50];
    auto num_lines(0);
    Status retval{};
    while (ss.getline(line, 50))
    {
        num_lines++;
#ifdef _UNICODE
        retval = grPtr->MeasureString(line, -1, font, layoutPoint, &box);
#else if _MBCS
        retval = grPtr->MeasureString(CA2W(line), -1, font, layoutPoint, &box);
#endif
        if (maximum.Width < box.Width)
        {
            maximum.Width = box.Width;
        }
        if (maximum.Height < box.Height)
        {
            maximum.Height = box.Height;
        }
    }
    boundingBox->Width = maximum.Width;
    boundingBox->Height = maximum.Height * num_lines;
    return retval;
}

// This method will check to see if the coord string has \n's and correct for that.
size_t StringLength(const string_t str)
{
    std::basic_stringstream<TCHAR> ss(str);
    TCHAR line[500];
    size_t maximum{0};
    while (ss.getline(line, 500))
    {
        auto sz = _tcslen(line);

        if (maximum < sz)
        {
            maximum = sz;
        }
    }
    return maximum;
}

// Method to remove the #ifdefs everywhere prior to DrawString
Status DrawString(Graphics* grPtr, const string_t string, INT length, const Gdiplus::Font* font,
                  const PointF& origin, const Brush* brush)
{
#ifdef _UNICODE
    return grPtr->DrawString(string.c_str(), length, font, origin, brush);
#else if _MBCS
    return grPtr->DrawString(CA2W(string.c_str()), length, font, origin, brush);
#endif
}

// Method to remove the #ifdefs everywhere prior to DrawString
Status DrawString(Graphics* grPtr, IN const string_t string, IN INT length,
                                   IN const Gdiplus::Font* font, IN const RectF& layoutRect,
                                   IN const StringFormat* stringFormat, IN const Brush* brush)
{
#ifdef _UNICODE
    return grPtr->DrawString(string.c_str(), length, font, layoutRect, stringFormat, brush);
#else if _MBCS
    return grPtr->DrawString(CA2W(string.c_str()), length, font, layoutRect, stringFormat, brush);
#endif
}

void BoundsCheckRectangle(Gdiplus::RectF& rect, const double x_min, const double x_max,
                          const double y_min, const double y_max)
{
    if (x_min > rect.X)
    {
        rect.X = static_cast<Gdiplus::REAL>(x_min);
    }
    if ((x_max - x_min) < rect.Width)
    {
        rect.Width = static_cast<Gdiplus::REAL>(x_max - x_min);
    }
    if (y_min > rect.Y)
    {
        rect.Y = static_cast<Gdiplus::REAL>(y_min);
    }
    if ((y_max - y_min) < rect.Height)
    {
        rect.Height = static_cast<Gdiplus::REAL>(y_max - y_min);
    }
}
