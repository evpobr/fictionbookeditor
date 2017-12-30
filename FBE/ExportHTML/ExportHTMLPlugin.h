#pragma once
#include "resource.h"
#include "ExportHTML.h"

class ATL_NO_VTABLE CExportHTMLPlugin :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CExportHTMLPlugin, &CLSID_ExportHTMLPlugin>,
	public FBELib::IFBEExportPlugin
{
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_EXPORTHTML)
	DECLARE_NOT_AGGREGATABLE(CExportHTMLPlugin)

	BEGIN_COM_MAP(CExportHTMLPlugin)
		COM_INTERFACE_ENTRY(IFBEExportPlugin)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// IFBEExportPlugin
	STDMETHODIMP raw_Export(long hWnd,BSTR filename,IDispatch *doc);
};

OBJECT_ENTRY_AUTO(CLSID_ExportHTMLPlugin, CExportHTMLPlugin)
