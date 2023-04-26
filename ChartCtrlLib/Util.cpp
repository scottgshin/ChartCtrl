///////////////////////////////////////////////////////////////////////////////
//
//  Util.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "util.h"

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

  r1.Offset(-sizeRectF.Width + d,0);
  grPath.AddArc(r1, 90, 90);

  pt1 = PointF(rectF.GetLeft(), rectF.GetBottom() - radius);
  pt2 = PointF(rectF.GetLeft(), rectF.GetTop() + radius);
  grPath.AddLine(pt1, pt2);
}

