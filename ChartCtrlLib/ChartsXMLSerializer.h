///////////////////////////////////////////////////////////////////////////////
//
// ChartsXMLSerializer.h
//
///////////////////////////////////////////////////////////////////////////////

#include "ChartDef.h"
#pragma once;


#import <msxml6.dll> raw_interfaces_only

template<typename T>
inline void get_attrValue(_bstr_t attrName, MSXML2::IXMLDOMNamedNodeMap *pAttrMap, T& val)
{
  MSXML2::IXMLDOMNode *pNode = NULL;
  _variant_t value_t = 0;
  HRESULT hr = S_FALSE;

  hr = pAttrMap->getNamedItem(attrName, &pNode);   // Get attr
  if (hr == S_OK)
  {
    hr = pNode->get_nodeTypedValue(&value_t);
    if (hr == S_OK)
      val = T(value_t);
  }
  if (hr != S_OK)
    throw _T("Can't read attribute value");
}

template<>
inline void get_attrValue<string_t> (_bstr_t attrName, MSXML2::IXMLDOMNamedNodeMap *pAttrMap, string_t& val)
{
  MSXML2::IXMLDOMNode *pNode = NULL;
  _variant_t value_t;
  HRESULT hr = S_FALSE;

  hr = pAttrMap->getNamedItem(attrName, &pNode);   // Get attr
  if (hr == S_OK)
  {
    hr = pNode->get_nodeTypedValue(&value_t);
    if (hr == S_OK)
    {
      _bstr_t tmpStr = value_t.bstrVal;
      TCHAR* tPtr = tmpStr.operator TCHAR*();
      val = string_t(tPtr);
    }
  }
  if (hr != S_OK)
    throw _T("Can_t read attribute value");
}

class CChartsXMLSerializer
{
private:
  CChartsXMLSerializer();
  ~CChartsXMLSerializer();

public:
  static
    HRESULT ChartDataToXML(const _TCHAR* fileName, const CChartContainer* pContainer,
                                                          const string_t chartName, bool bAll = false);
  static
    HRESULT XMLToCharts(LPCTSTR fileName, CChartContainer* pContainer, const MAP_CHARTCOLS& mapCharts, bool bClearCharts);
  static
   HRESULT GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_CHARTCOLS& mapContent);
  static
   HRESULT GetChartNamesFromXMLFile(LPCTSTR fileName, MAP_NAMES& mapNames);
  static
    HRESULT ReplaceChartsFromXMLFile(LPCTSTR fileName, CChartContainer* pContainer);

private:
  static
    void AddWhiteSpaceToNode(CComPtr<MSXML2::IXMLDOMDocument> pDoc, _bstr_t strWs,
                                                      CComPtr<MSXML2::IXMLDOMNode> pNode)
    {
      CComPtr<MSXML2::IXMLDOMText> pws;     // By default pws.p = NULL
      CComPtr<MSXML2::IXMLDOMNode> pBuf;

      pDoc->createTextNode(strWs, &pws);
      pNode->appendChild(pws, &pBuf);
    }

  static
    void AddWhiteSpaceToElement(CComPtr<MSXML2::IXMLDOMDocument> pDoc, _bstr_t strWs,
                                                      CComPtr<MSXML2::IXMLDOMElement> pEl)
    {
      CComPtr<MSXML2::IXMLDOMText> pws;     // By default pws.p = NULL
      CComPtr<MSXML2::IXMLDOMNode> pBuf;

      pDoc->createTextNode(strWs, &pws);
      pEl->appendChild(pws, &pBuf);
    }
private:
    static _bstr_t m_bstr_wsn;
    static _bstr_t m_bstr_wsnt;
    static _bstr_t m_bstr_wsntt;
    static _bstr_t m_bstr_wsnttt;
    static _bstr_t m_bstr_wsntttt;
};
