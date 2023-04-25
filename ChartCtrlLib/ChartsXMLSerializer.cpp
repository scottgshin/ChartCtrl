
///////////////////////////////////////////////////////////////////////////////
// Class CChartsXML Serializer implementation

#include "stdafx.h"
#include <assert.h>
#include "ChartDef.h"
#include "Chart.h"
#include "ChartContainer.h"
#include "ChartsXMLSerializer.h"

using namespace std;
using namespace Gdiplus;
using namespace MSXML2;

_bstr_t CChartsXMLSerializer::m_bstr_wsn(_T("\n"));
_bstr_t CChartsXMLSerializer::m_bstr_wsnt(_T("\n\t"));
_bstr_t CChartsXMLSerializer::m_bstr_wsntt(_T("\n\t\t"));
_bstr_t CChartsXMLSerializer::m_bstr_wsnttt(_T("\n\t\t\t"));
_bstr_t CChartsXMLSerializer::m_bstr_wsntttt(_T("\n\t\t\t\t"));


CChartsXMLSerializer::CChartsXMLSerializer()
{
}

CChartsXMLSerializer::~CChartsXMLSerializer()
{
}

HRESULT  CChartsXMLSerializer::ChartDataToXML(const _TCHAR* fileName, 
                    const CChartContainer* pContainer, const string_t chartName, bool bAll)
{
  if (pContainer->IsContainerEmpty())
    return S_FALSE;

  HRESULT hr = S_OK;
  int chartIdx;
  if (!chartName.empty())
  {
    chartIdx = pContainer->GetChartIdx(chartName);
    if (chartIdx == -1)     // Has no chart with that name
      return S_FALSE;
  }
  else
    chartIdx = -1;          
// Has no visible charts or this particular chart is not exist or is not visible
  if (!bAll&&(!pContainer->IsChartVisible(chartIdx)))
    return S_FALSE;
// Sure the iterator does not point to the end of the map
  MAP_CHARTS::const_iterator itMap = chartIdx == -1 ? pContainer->GetFirstChart() :
                                                        pContainer->FindChart(chartIdx);
 
// Now begin to built XML file
  CoInitialize(NULL);
// Global code block for pxmlDoc
  {
    CComPtr<MSXML2::IXMLDOMDocument> pxmlDoc;
    try
    {
      hr = pxmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
      if (hr != S_OK)
        throw _T("Can't cocreate an instance of pxmlDoc");
    
      _variant_t var_t(false);
      hr = pxmlDoc->put_async(var_t);
      hr |= pxmlDoc->put_validateOnParse(var_t);
      hr |= pxmlDoc->put_resolveExternals(var_t);
      if (hr != S_OK)
        throw _T("Can't init DOM document");
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP);
      return S_FALSE;
    }

// Declaration of CComPtr
    CComPtr<MSXML2::IXMLDOMNode> pRoot;
    CComPtr<MSXML2::IXMLDOMElement> pe;
    CComPtr<MSXML2::IXMLDOMNode> pn, pn1, pnDummy;

// Code block for processing instruction
    {
      CComPtr<MSXML2::IXMLDOMProcessingInstruction> pi;
      _bstr_t bstr_t1(_T("xml")), bstr_t2(_T("version='1.0'"));
      hr = pxmlDoc->createProcessingInstruction( bstr_t1, bstr_t2, &pi);
      assert(hr == S_OK);
      pxmlDoc->appendChild(pi, &pn);
      pn.Release();   // To reuse it
    } // End of pi and pn for the processing instruction


    pxmlDoc->createElement(_bstr_t(_T("Charts")), &pe);
    pe->setAttribute(_bstr_t(_T("xmlns:xsi")),
         _variant_t(_T("http://www.w3.org/2001/XMLSchema-instance")));
    pe->setAttribute(_bstr_t(_T("xmlns:xsd")), 
      _variant_t(_T("http://www.w3.org/2001/XMLSchema")));
    pe->setAttribute(_bstr_t(_T("xmlns")), _variant_t(_T("http://tempuri.org/Network.xsd")));

    pxmlDoc->appendChild(pe, &pRoot);
    pe.Release(); // End of the "Charts" code block; pe is freed, pRoot stays active

// Add container name, name_x, and precision
    AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsnt, pRoot);
    hr = pxmlDoc->createElement(_bstr_t(_T("Container")), &pe);
    if (hr == S_OK)
    {
      pe->setAttribute(_bstr_t(_T("Cont_Name")), variant_t(pContainer->GetContainerName().c_str()));
      pe->setAttribute(_bstr_t(_T("Name_X")), variant_t(pContainer->GetAxisXName().c_str()));
      pe->setAttribute(_bstr_t(_T("Precision_X")), variant_t(pContainer->GetContainerPrecisionX()));
      pRoot->appendChild(pe, &pn);
      pn.Release();
    }
    pe.Release();
    if (hr != S_OK)
      throw _T("Can't create 'Container' node");

    _bstr_t bsLineName(_T("Pnt"));
    _bstr_t bsX(_T("X"));
    _bstr_t bsY(_T("Y"));
    _bstr_t bsPntIdx(_T("Idx"));

    int chartNmb = 0;

    for (; itMap != pContainer->GetChartsEnd(); ++ itMap)
    {
      CChart* chartPtr = itMap->second;
      if (!bAll && (false == chartPtr->IsChartVisible()))
        continue;   // This is to eliminate hidden charts while iterating over entire container
                    
      ++chartNmb;
// Get and save chart attributes
      string_t name = chartPtr->GetChartName();
      string_t nameY = chartPtr->GetAxisYName();
      Color chartColor = chartPtr->GetChartColor();
      bool bVisible  = chartPtr->IsChartVisible();
      DashStyle dashStyle = chartPtr->GetChartDashStyle();
      float penWidth = chartPtr->GetPenWidth();
      float tension  = chartPtr->GetTension();
      int precisionX = pContainer->GetContainerPrecisionX();
      int precisionY = chartPtr->GetPrecisionY();
      PointD pntD = chartPtr->HasData() ? chartPtr->m_vDataPnts.front() : PointD(1.1234567, 1.123456);
      string_t sampleDataX = pContainer->GetLabXValStrFnPtr()(pntD.X, precisionX, false);
      string_t sampleDataY = chartPtr->GetLabYValStrFnPtr()(pntD.Y, precisionY, false);
// Info parameters; will be ignored when this file is being loaded
      double minX    = chartPtr->GetMinValX();
      double maxX    = chartPtr->GetMaxValX();
      double minY    = chartPtr->GetMinValY();
      double maxY    = chartPtr->GetMaxValY();

      size_t pntsNmb = chartPtr->m_vDataPnts.size();
    
      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsnt, pRoot);
      hr |= pxmlDoc->createElement(_bstr_t(_T("Chart")), &pe);
      pe->setAttribute(_bstr_t(_T("Name")), variant_t(name.c_str()));
      pe->setAttribute(_bstr_t(_T("Precision_Y")), variant_t(precisionY));
      pe->setAttribute(_bstr_t(_T("Sample_X_Val")), variant_t(sampleDataX.c_str()));
      pe->setAttribute(_bstr_t(_T("Name_Y")), variant_t(nameY.c_str()));
      pe->setAttribute(_bstr_t(_T("Sample_Y_Val")), variant_t(sampleDataY.c_str()));
      pRoot->appendChild(pe, &pn);
      pe.Release();

      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsntt, pn);//pRoot);
      hr |= pxmlDoc->createElement(_bstr_t(_T("Chart_Colors")), &pe);
      pe->setAttribute(_bstr_t(_T("Alpha")), variant_t(chartColor.GetA()));
      pe->setAttribute(_bstr_t(_T("Red")), variant_t(chartColor.GetR()));
      pe->setAttribute(_bstr_t(_T("Green")), variant_t(chartColor.GetG()));
      pe->setAttribute(_bstr_t(_T("Blue")), variant_t(chartColor.GetB()));
      pn->appendChild(pe, &pn1);
      pe.Release();
      pn1.Release();

      _variant_t var_false(_T("false"));
      _variant_t var_true(_T("true"));

      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsntt, pn);//pRoot);
      hr |= pxmlDoc->createElement(_bstr_t(_T("Chart_Visuals")), &pe);
      pe->setAttribute(_bstr_t(_T("Visibility")), bVisible ? var_true : var_false); 
      pe->setAttribute(_bstr_t(_T("DashStyle")), variant_t(dashStyle));
      pe->setAttribute(_bstr_t(_T("PenWidth")), variant_t(penWidth));
      pe->setAttribute(_bstr_t(_T("Tension")), variant_t(tension));
      pn->appendChild(pe, &pn1);
      pe.Release();
      pn1.Release();

      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsntt, pn);//pRoot);
      hr |= pxmlDoc->createElement(_bstr_t(_T("Chart_Data")), &pe);
      pe->setAttribute(_bstr_t(_T("DataPnts_Nmb")), variant_t(pntsNmb));
      pe->setAttribute(_bstr_t(_T("MinX")), variant_t(minX));
      pe->setAttribute(_bstr_t(_T("MaxX")), variant_t(maxX));
      pe->setAttribute(_bstr_t(_T("MinY")), variant_t(minY));
      pe->setAttribute(_bstr_t(_T("MaxY")), variant_t(maxY));
      pn->appendChild(pe, &pn1);
      pe.Release();

// Save chart data
      for (size_t pntCnt = 0; pntCnt < pntsNmb; ++pntCnt)
      {
        AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsnttt, pn1);

        hr |= pxmlDoc->createElement(bsLineName, &pe);
        hr |= pe->setAttribute(bsPntIdx, _variant_t(pntCnt));
        hr |= pe->setAttribute(bsX, _variant_t(chartPtr->m_vDataPnts[pntCnt].X));
        hr |= pe->setAttribute(bsY, _variant_t(chartPtr->m_vDataPnts[pntCnt].Y));
        pn1->appendChild(pe, &pnDummy);
        pe.Release();
        pnDummy.Release();
      }
      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsntt, pn1);
      AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsnt, pn);
      pn1.Release();
      pn.Release();
      if (chartIdx > -1)
        break;
    }
 
    AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsnt, pRoot);
    hr |= pxmlDoc->createElement(_bstr_t(_T("ChartNmb")), &pe);
    hr |= pe->setAttribute(_bstr_t(_T("Total")), variant_t(chartNmb));
    pRoot->appendChild(pe, &pnDummy);

    AddWhiteSpaceToNode(pxmlDoc, m_bstr_wsn, pRoot);

    pxmlDoc->save(_variant_t(fileName));
  }

  CoUninitialize();
  return hr;
}

HRESULT CChartsXMLSerializer::XMLToCharts(LPCTSTR fileName, CChartContainer* pContainer, 
                                                         const MAP_CHARTCOLS& mapCharts, bool bClearCharts)
{
  size_t selChartsNmb = mapCharts.size();
  if (selChartsNmb == 0)
    return S_FALSE;

  CoInitialize(NULL);
  HRESULT hr = S_OK;

  {
    CComPtr<MSXML2::IXMLDOMDocument> pxmlDoc;
// Create doc, load source
    try
    {
      hr = pxmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
      if (hr != S_OK)
        throw _T("Can't cocreate an instance of pxmlDoc");
    
     _variant_t var_t(false);
      hr = pxmlDoc->put_async(var_t);
      hr |= pxmlDoc->put_validateOnParse(var_t);
      hr |= pxmlDoc->put_resolveExternals(var_t);
      hr |= pxmlDoc->put_preserveWhiteSpace(var_t);
      if (hr != S_OK)
        throw _T("Can't initialize the DOM document");

      _variant_t fvar_t(fileName);

      hr = pxmlDoc->load(fvar_t.GetVARIANT(), &(var_t.boolVal));
      if (hr != S_OK)
        throw _T("Can't load file ");
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return hr;
    }

// Get doc element
    MSXML2::IXMLDOMElement *docElement = NULL;
    MSXML2::IXMLDOMNode *pNode = NULL, *pChildNode = NULL;
    MSXML2::IXMLDOMNodeList *pNodeList = NULL, *pChartNodeList = NULL, *pPntNodeList = NULL;
    MSXML2::IXMLDOMNamedNodeMap *pAttrMap = NULL;

    try
    {
      hr = pxmlDoc->get_documentElement(&docElement);
      if (hr != S_OK)
        throw _T("Can't get document element");
// The doc baseName must be "Charts"
      _bstr_t nameStr;
      docElement->get_baseName(nameStr.GetAddress());
      if (nameStr != _bstr_t(_T("Charts")))
        throw _T("It is not a chart file");

// Get number of charts in this file
      long nodeListLen = 0;
      long chartsNmb = 0;

// Get chart elements
      _variant_t vname_t1(_T("Name"));
      _variant_t vname_t2(_T("Alpha"));
      _variant_t vname_t3(_T("Red"));
      _variant_t vname_t4(_T("Green"));
      _variant_t vname_t5(_T("Blue"));
      _variant_t vname_t6(_T("DashStyle"));
      _variant_t vname_t7(_T("PenWidth"));
      _variant_t vname_t8(_T("Tension"));
      _variant_t vname_t9(_T("X"));
      _variant_t vname_t10(_T("Y"));
      _variant_t vname_t11(_T("Name_Y"));
      _variant_t vname_t12(_T("Precision_Y"));
      _variant_t vname_t13(_T("Visibility"));
      _variant_t vname_t14(_T("Name_X"));
      _variant_t vname_t15(_T("Precision_X"));

      long pntsNmb = 0;
      
      string_t chName;
      string_t nameX;
      string_t nameY;
      Color chColor;

      bool bVisible = true;
      int dashStyle = DashStyleSolid;
      float penWidth = 2.0f;
      float tension = 0.0f;
      int precisionX = 3;
      int precisionY = 3;
      double X = 0.0, Y = 0.0;

// Get precision and nameX
      if (bClearCharts)
      {
        pxmlDoc->getElementsByTagName(_bstr_t(_T("Container")), &pNodeList);
        pNodeList->get_length(&nodeListLen);
        if (nodeListLen == 0)
          throw _T("File has no node 'Container'");
      
        hr = pNodeList->get_item(0, &pNode);
        pNodeList->Release();
       if (hr == S_OK)
       {
         hr = pNode->get_attributes(&pAttrMap);
         pNode->Release();
         if (hr == S_OK)
         {
           get_attrValue(vname_t14.bstrVal, pAttrMap, nameX);
           get_attrValue(vname_t15.bstrVal, pAttrMap, precisionX);
         }
       }
       pAttrMap->Release();
      
       if (hr != S_OK)
         throw _T("Can't read axis_X name");
      }

      pxmlDoc->getElementsByTagName(_bstr_t(_T("Chart")), &pNodeList);
      pNodeList->get_length(&nodeListLen);
      if (nodeListLen == 0)
        throw _T("File has no charts");
      else
      {
        chartsNmb = nodeListLen;
        if (bClearCharts)
          pContainer->GetMapCharts()->clear();
      }

      V_CHARTDATAD vData;
      typedef std::map<string_t, Color> MAP_XML;
      MAP_XML::const_iterator itNm, itNmB = mapCharts.cbegin();;
      MAP_XML::const_iterator itNmE = mapCharts.cend();

      for (int chartCnt = 0; chartCnt < chartsNmb; ++chartCnt)
      {
        hr = pNodeList->get_item(chartCnt, &pNode);
        if (hr == S_OK)
        {
          hr = pNode->get_attributes(&pAttrMap);
          if (hr == S_OK)
          {
            get_attrValue(vname_t1.bstrVal, pAttrMap, chName);
            get_attrValue(vname_t11.bstrVal, pAttrMap, nameY);
            get_attrValue(vname_t12.bstrVal, pAttrMap, precisionY);
            pAttrMap->Release();
            itNm = mapCharts.find(chName);
            if (itNm == itNmE)
              continue;
            chColor = itNm->second;
          }
        }
        if (hr != S_OK)
          throw _T("Can't read the chart's name");

        hr = pNode->get_childNodes(&pChartNodeList);
        pNode->Release();
// Use colors from the map, dont read them

// GetVisuals
          _bstr_t res;
          hr = pChartNodeList->get_item(1, &pChildNode);
          if (hr == S_OK)
          {
            hr = pChildNode->get_attributes(&pAttrMap);
            pChildNode->Release();
            if (hr == S_OK)
            {
              get_attrValue(vname_t13.bstrVal, pAttrMap, res); 
              bVisible = res == _bstr_t(_T("false")) ? false : true;
              get_attrValue(vname_t6.bstrVal, pAttrMap, dashStyle);    
              get_attrValue(vname_t7.bstrVal, pAttrMap, penWidth);
              get_attrValue(vname_t8.bstrVal, pAttrMap, tension);   
              pAttrMap->Release();
            }
          }
          if (hr != S_OK)
            throw _T("Can't read the chart visial attributes");

// Get the chart data points
          hr = pChartNodeList->get_item(2, &pChildNode);
          pChartNodeList->Release();
          if (hr == S_OK)
          {
            pChildNode->get_childNodes(&pPntNodeList);
            pChildNode->Release();
            pPntNodeList->get_length(&pntsNmb);

            vData.clear();
            if (pntsNmb > 0)
            {
              vData.resize(pntsNmb);
              for (long pntsCnt = 0; pntsCnt < pntsNmb; ++pntsCnt)
              {
                hr = pPntNodeList->get_item(pntsCnt, &pChildNode);
                if (hr == S_OK)
                {
                  hr = pChildNode->get_attributes(&pAttrMap);
                  pChildNode->Release();
                  if (hr == S_OK)
                  {
                    get_attrValue(vname_t9.bstrVal, pAttrMap, X);
                    get_attrValue(vname_t10.bstrVal, pAttrMap, Y);    
                    if (hr == S_OK)
                      vData[pntsCnt] = PointD(X, Y);
                  }
                  pAttrMap->Release();
               }
             }
             pPntNodeList->Release();
            }
          }
          if (hr != S_OK)
            throw _T("Can't read data point");

          if (bClearCharts)
          {
            pContainer->SetContainerPrecision(precisionX);
            pContainer->SetAxisXName(nameX);
            pContainer->SetContainerPrecision(precisionX);
          }
          pContainer->AddChart(true, true, chName, nameY, precisionY, DashStyle(dashStyle), 
                                                              penWidth, tension, chColor, vData);
      }
      pNodeList->Release();
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return S_FALSE;
    }
  }
  CoUninitialize();
  return hr;
}

HRESULT CChartsXMLSerializer::GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_CHARTCOLS& mapContent)
{
  mapContent.clear();

  CoInitialize(NULL);
  HRESULT hr = S_OK;

 {
    CComPtr<MSXML2::IXMLDOMDocument> pxmlDoc;
// Create doc, load source
    try
    {
      hr = pxmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
      if (hr != S_OK)
        throw _T("Can't cocreate an instance of pxmlDoc");
    
     _variant_t var_t(false);
      hr = pxmlDoc->put_async(var_t);
      hr |= pxmlDoc->put_validateOnParse(var_t);
      hr |= pxmlDoc->put_resolveExternals(var_t);
      hr |= pxmlDoc->put_preserveWhiteSpace(var_t);
      if (hr != S_OK)
        throw _T("Can't initialize the DOM document");

      _variant_t fvar_t(fileName);

      hr = pxmlDoc->load(fvar_t.GetVARIANT(), &(var_t.boolVal));
      if (hr != S_OK)
        throw _T("Can't load chart file ");
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return hr;
    }

// Get doc element
    MSXML2::IXMLDOMElement *docElement = NULL;
    MSXML2::IXMLDOMNode *pNode = NULL, *pChildNode = NULL;
    MSXML2::IXMLDOMNodeList *pNodeList = NULL, *pChartNodeList = NULL;
    MSXML2::IXMLDOMNamedNodeMap *pAttrMap = NULL;

    try
    {
      hr = pxmlDoc->get_documentElement(&docElement);
      if (hr != S_OK)
        throw _T("Can't get document element");
// The doc baseName must be "Charts"
      _bstr_t nameStr;
      docElement->get_baseName(nameStr.GetAddress());
      if (nameStr != _bstr_t(_T("Charts")))
        throw _T("It is not a chart file");

// Get chart elements
      long nodeListLen = 0;
      _variant_t vname_t1(_T("Name"));
      _variant_t vname_t2(_T("Alpha"));
      _variant_t vname_t3(_T("Red"));
      _variant_t vname_t4(_T("Green"));
      _variant_t vname_t5(_T("Blue"));

      string_t chName;
      BYTE nA = 0xFF, nR = 0, nG = 0, nB = 0;
      Color chColor;

      pxmlDoc->getElementsByTagName(_bstr_t(_T("Chart")), &pNodeList);
      pNodeList->get_length(&nodeListLen);
      if (nodeListLen == 0)
        throw _T("File has no charts");

// Get chart names
      for (long chartCnt = 0; chartCnt < nodeListLen; ++chartCnt)
      {
        hr = pNodeList->get_item(chartCnt, &pNode);
        if (hr == S_OK)
        {
          hr = pNode->get_attributes(&pAttrMap);
          if (hr == S_OK)
          {
            get_attrValue(vname_t1.bstrVal, pAttrMap, chName);
            pAttrMap->Release();
          }
        }
        if (hr != S_OK)
          throw _T("Can't read chart name");

        hr = pNode->get_childNodes(&pChartNodeList);
        pNode->Release();

        if (hr == S_OK)
        {                             // Get chart color
            hr = pChartNodeList->get_item(0, &pChildNode);
            if (hr == S_OK)
            {
              hr = pChildNode->get_attributes(&pAttrMap);
              pChildNode->Release();
              if (hr == S_OK)
              {
                get_attrValue(vname_t2.bstrVal, pAttrMap, nA);    
                get_attrValue(vname_t3.bstrVal, pAttrMap, nR);    
                get_attrValue(vname_t4.bstrVal, pAttrMap, nG);
                get_attrValue(vname_t5.bstrVal, pAttrMap, nB);    
              }
              pAttrMap->Release();
              if (hr == S_OK)
                chColor = Color(nA, nR, nG, nB);
            }
          }
          if (hr != S_OK)
            throw _T("Can't read chart color");
          mapContent.insert(std::map<string_t, Color>::value_type(chName, chColor));
      }
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return S_FALSE;
    }
 }
  CoUninitialize();
  return hr;
}

HRESULT CChartsXMLSerializer::GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_NAMES& mapNames)
{
  mapNames.clear();

  CoInitialize(NULL);
  HRESULT hr = S_OK;

 {
    CComPtr<MSXML2::IXMLDOMDocument> pxmlDoc;
// Create doc, load source
    try
    {
      hr = pxmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
      if (hr != S_OK)
        throw _T("Can't cocreate an instance of pxmlDoc");
    
     _variant_t var_t(false);
      hr = pxmlDoc->put_async(var_t);
      hr |= pxmlDoc->put_validateOnParse(var_t);
      hr |= pxmlDoc->put_resolveExternals(var_t);
      hr |= pxmlDoc->put_preserveWhiteSpace(var_t);
      if (hr != S_OK)
        throw _T("Can't initialize the DOM document");

      _variant_t fvar_t(fileName);

      hr = pxmlDoc->load(fvar_t.GetVARIANT(), &(var_t.boolVal));
      if (hr != S_OK)
        throw _T("Can't load chart file ");
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return hr;
    }

// Get doc element
    MSXML2::IXMLDOMElement *docElement = NULL;
    MSXML2::IXMLDOMNode *pNode = NULL;
    MSXML2::IXMLDOMNodeList *pNodeList = NULL;
    MSXML2::IXMLDOMNamedNodeMap *pAttrMap = NULL;

    try
    {
      hr = pxmlDoc->get_documentElement(&docElement);
      if (hr != S_OK)
        throw _T("Can't get document element");
// The doc baseName must be "Charts"
      _bstr_t nameStr;
      docElement->get_baseName(nameStr.GetAddress());
      if (nameStr != _bstr_t(_T("Charts")))
        throw _T("It is not a chart file");

// Get chart elements
      long nodeListLen = 0;
      _variant_t vname_t1(_T("Name"));
      _variant_t vname_t2(_T("Name_X"));
      _variant_t vname_t3(_T("Sample_X_Val"));
      _variant_t vname_t4(_T("Name_Y"));
      _variant_t vname_t5(_T("Sample_Y_Val"));

      string_t chName, nameX, sampleX, nameY, sampleY;

      pxmlDoc->getElementsByTagName(_bstr_t(_T("Container")), &pNodeList);
      pNodeList->get_length(&nodeListLen);
      if (nodeListLen == 0)
        throw _T("File has no node 'Container'");
      
      hr = pNodeList->get_item(0, &pNode);
      pNodeList->Release();
      if (hr == S_OK)
      {
        hr = pNode->get_attributes(&pAttrMap);
        pNode->Release();
        if (hr == S_OK)
          get_attrValue(vname_t2.bstrVal, pAttrMap, nameX);
      }
      pAttrMap->Release();
      
      if (hr != S_OK)
        throw _T("Can't read axis_X name");


      pxmlDoc->getElementsByTagName(_bstr_t(_T("Chart")), &pNodeList);
      pNodeList->get_length(&nodeListLen);
      if (nodeListLen == 0)
        throw _T("File has no charts");

// Get chart names
      for (long chartCnt = 0; chartCnt < nodeListLen; ++chartCnt)
      {
        hr = pNodeList->get_item(chartCnt, &pNode);
        if (hr == S_OK)
        {
          hr = pNode->get_attributes(&pAttrMap);
          if (hr == S_OK)
          {
            get_attrValue(vname_t1.bstrVal, pAttrMap, chName);
            get_attrValue(vname_t3.bstrVal, pAttrMap, sampleX);
            get_attrValue(vname_t4.bstrVal, pAttrMap, nameY);
            get_attrValue(vname_t5.bstrVal, pAttrMap, sampleY);
            pAttrMap->Release();
          }
        }
        if (hr != S_OK)
          throw _T("Can't read chart name");

        TUPLE_NAMES tuple_names = make_tuple(nameX, sampleX, nameY, sampleY);

        mapNames.insert(MAP_NAMES::value_type(chName, tuple_names));
      }
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return S_FALSE;
    }
 }
  CoUninitialize();
  return hr;
}

HRESULT CChartsXMLSerializer::ReplaceChartsFromXMLFile(LPCTSTR fileName, CChartContainer* pContainer)
{
  pContainer->ResetChartContainer();

  CoInitialize(NULL);
  HRESULT hr = S_OK;

  {
    CComPtr<MSXML2::IXMLDOMDocument> pxmlDoc;
// Create doc, load source
    try
    {
      hr = pxmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
      if (hr != S_OK)
        throw _T("Can't cocreate an instance of pxmlDoc");
    
      _variant_t var_t(false);
      hr = pxmlDoc->put_async(var_t);
      hr |= pxmlDoc->put_validateOnParse(var_t);
      hr |= pxmlDoc->put_resolveExternals(var_t);
      hr |= pxmlDoc->put_preserveWhiteSpace(var_t);
      if (hr != S_OK)
        throw _T("Can't initialize the DOM document");

      _variant_t fvar_t(fileName);

      hr = pxmlDoc->load(fvar_t.GetVARIANT(), &(var_t.boolVal));
      if (hr != S_OK)
        throw _T("Can't load chart file ");
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return hr;
    }

// Get doc element
    MSXML2::IXMLDOMElement *docElement = NULL;
    MSXML2::IXMLDOMNode *pNode = NULL, *pChildNode = NULL;
    MSXML2::IXMLDOMNodeList *pNodeList = NULL, *pChartNodeList = NULL, *pPntNodeList = NULL;
    MSXML2::IXMLDOMNamedNodeMap *pAttrMap = NULL;

    try
    {
      hr = pxmlDoc->get_documentElement(&docElement);
      if (hr != S_OK)
        throw _T("Can't get document element");
// The doc baseName must be "Charts"
      _bstr_t nameStr;
      docElement->get_baseName(nameStr.GetAddress());
      if (nameStr != _bstr_t(_T("Charts")))
        throw _T("It is not a chart file");


// Get number of charts in this file
      long nodeListLen = 0;
      long chartsNmb = 0;


// Get container data
      string_t contName;
      string_t nameX;
      int precisionX = 3;

      hr = pxmlDoc->getElementsByTagName(_bstr_t(_T("Container")), &pNodeList);
      if (hr == S_OK)
      {
        hr = pNodeList->get_item(0, &pNode);
        pNodeList->Release();
        if (hr ==S_OK)
        {
          hr = pNode->get_attributes(&pAttrMap);
          pNode->Release();
          if (hr == S_OK)
          {
            get_attrValue(_bstr_t(_T("Cont_Name")), pAttrMap, contName);
            get_attrValue(_bstr_t(_T("Name_X")), pAttrMap, nameX);
            get_attrValue(_bstr_t(_T("Precision_X")), pAttrMap, precisionX);
            pAttrMap->Release();
          }
        }
      }
      if (hr != S_OK)
        throw _T("Can't get container params");
 
      hr = pxmlDoc->getElementsByTagName(_bstr_t(_T("Chart")), &pNodeList);
      if (hr == S_OK)
        pNodeList->get_length(&nodeListLen);
      if (nodeListLen == 0)
        throw _T("File has no charts");
      else
        chartsNmb = nodeListLen;
// Get chart elements
      _variant_t vname_t1(_T("Name"));
      _variant_t vname_t2(_T("Alpha"));
      _variant_t vname_t3(_T("Red"));
      _variant_t vname_t4(_T("Green"));
      _variant_t vname_t5(_T("Blue"));
      _variant_t vname_t6(_T("DashStyle"));
      _variant_t vname_t7(_T("PenWidth"));
      _variant_t vname_t8(_T("Tension"));
      _variant_t vname_t9(_T("X"));
      _variant_t vname_t10(_T("Y"));
      _variant_t vname_t11(_T("Name_Y"));
      _variant_t vname_t12(_T("Precision_Y"));
      _variant_t vname_t13(_T("Visibility"));
      _variant_t vname_t14(_T("Name_S"));

      long pntsNmb = 0;
      
      string_t chName;
      string_t name_X;
      string_t nameY;
      BYTE alphaC =255, redC = 0, greenC = 0, blueC = 0;
      Color chColor;

      bool bVisible = true;
      int dashStyle = DashStyleSolid;
      float penWidth = 2.0f;
      float tension = 0.0f;
      int precisionY = 3;
      double X = 0.0, Y = 0.0;

      V_CHARTDATAD vData;

      for (int chartCnt = 0; chartCnt < chartsNmb; ++chartCnt)
      {
        hr = pNodeList->get_item(chartCnt, &pNode);
        if (hr == S_OK)
        {
          hr = pNode->get_attributes(&pAttrMap);
          if (hr == S_OK)
          {
            get_attrValue(vname_t1.bstrVal, pAttrMap, chName);
            get_attrValue(vname_t11.bstrVal, pAttrMap, nameY);
            pAttrMap->Release();
          }
        }
        if (hr != S_OK)
          throw _T("Can't read the chart's name");

        hr = pNode->get_childNodes(&pChartNodeList);
        pNode->Release();
        if (hr != S_OK)
          throw _T("Can't get chart children nodes");

// Use colors from the map, dont read them
        hr = pChartNodeList->get_item(0, &pChildNode);
        if (hr == S_OK)
        {
          hr = pChildNode->get_attributes(&pAttrMap);
          pChildNode->Release();
          if (hr == S_OK)
          {
            alphaC = 255;
            redC   = 0;
            greenC = 0;
            blueC  = 0;             // Default - black
            get_attrValue(vname_t2.bstrVal, pAttrMap, alphaC);
            get_attrValue(vname_t3.bstrVal, pAttrMap, redC);
            get_attrValue(vname_t4.bstrVal, pAttrMap, greenC);
            get_attrValue(vname_t5.bstrVal, pAttrMap, blueC);
            pAttrMap->Release();
          }
          if (hr == S_OK)
            chColor = Color(alphaC, redC, greenC, blueC); 
          else
            throw _T("Can't get chart color");

// GetVisuals
          _bstr_t res;
          hr = pChartNodeList->get_item(1, &pChildNode);
          if (hr == S_OK)
          {
            hr = pChildNode->get_attributes(&pAttrMap);
            pChildNode->Release();
            if (hr == S_OK)
            {
              get_attrValue(vname_t13.bstrVal, pAttrMap, res); 
              bVisible = res == _bstr_t(_T("false")) ? false : true;
              get_attrValue(vname_t6.bstrVal, pAttrMap, dashStyle);    
              get_attrValue(vname_t7.bstrVal, pAttrMap, penWidth);
              get_attrValue(vname_t8.bstrVal, pAttrMap, tension);   
              pAttrMap->Release();
            }
          }
          if (hr != S_OK)
            throw _T("Can't read the chart visial attributes");

// Get the chart data points
          vData.clear();
          hr = pChartNodeList->get_item(2, &pChildNode);
          pChartNodeList->Release();
          if (hr == S_OK)
          {
            pChildNode->get_childNodes(&pPntNodeList);
            pChildNode->Release();
            pPntNodeList->get_length(&pntsNmb);

            if (pntsNmb > 0)
            {
              vData.resize(pntsNmb);
              for (long pntsCnt = 0; pntsCnt < pntsNmb; ++pntsCnt)
              {
                hr = pPntNodeList->get_item(pntsCnt, &pChildNode);
                if (hr == S_OK)
                {
                  hr = pChildNode->get_attributes(&pAttrMap);
                  pChildNode->Release();
                  if (hr == S_OK)
                  {
                    get_attrValue(vname_t9.bstrVal, pAttrMap, X);
                    get_attrValue(vname_t10.bstrVal, pAttrMap, Y);    
                    vData[pntsCnt] = PointD(X, Y);
                  }
                  pAttrMap->Release();
                }
              }
              pPntNodeList->Release();
            }
          }
          if (hr != S_OK)
            throw _T("Can't read data point");
        
          pContainer->AddChart(bVisible, true, chName, nameY, precisionY, DashStyle(dashStyle), 
                                                              penWidth, tension, chColor, vData);
        }
      }
      pNodeList->Release();
      pContainer->SetContainerName(contName);
      pContainer->SetAxisXName(name_X);
      pContainer->SetContainerPrecision(precisionX);
    }
    catch(_TCHAR* str)
    {
      CoUninitialize();
      AfxMessageBox(str, MB_OK|MB_ICONSTOP); 
      return S_FALSE;
    }
  }
  CoUninitialize();
  return hr;
}

