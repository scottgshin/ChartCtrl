
// ChartCtrlDemo.h : main header file for the PROJECT_NAME application
//

#pragma once


#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CChartCtrlDemoApp:
// See ChartCtrlDemo.cpp for the implementation of this class
//

class CChartCtrlDemoApp : public CWinApp
{
public:
	CChartCtrlDemoApp();

// Overrides
public:
	virtual BOOL InitInstance();
  virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

private:
  ULONG_PTR m_nGdiplusToken;
};

extern CChartCtrlDemoApp theApp;