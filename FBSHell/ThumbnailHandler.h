// ThumbnailHandler.h : Declaration of the CThumbnailHandler

#pragma once
#include "resource.h"       // main symbols

#include "FBShell_h.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

// CThumbnailHandler

class ATL_NO_VTABLE CThumbnailHandler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CThumbnailHandler, &CLSID_ThumbnailHandler>,
	public IInitializeWithStream,
	public IThumbnailProvider
{
public:
	CThumbnailHandler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_THUMBNAILHANDLER)

DECLARE_NOT_AGGREGATABLE(CThumbnailHandler)

BEGIN_COM_MAP(CThumbnailHandler)
	COM_INTERFACE_ENTRY(IInitializeWithStream)
	COM_INTERFACE_ENTRY(IThumbnailProvider)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{

	}

private:
	CImage m_imgCoverpage;

	HRESULT LoadFB2Document(_In_ IStream *pStream, _COM_Outptr_ IXMLDOMDocument2 **pDoc);
	HRESULT DecodeBase64StringToStream(_In_ CString strBase64, _COM_Outptr_ IStream **pStream);
	HRESULT GetCoverpageHRef(_In_ IXMLDOMDocument2 *pDoc, _Out_ CString &strHRef);
	HRESULT GetBinaryNodeById(_In_ IXMLDOMDocument2 *pDoc, _In_z_ LPCWSTR pszId, _COM_Outptr_ IXMLDOMNode **pNode);
	HRESULT GetBinaryBase64String(_In_ IXMLDOMNode *pNode, _Out_ CString &strBase64);

public:

	// Inherited via IInitializeWithStream
	STDMETHODIMP Initialize(IStream * pstream, DWORD grfMode);


	// Inherited via IThumbnailProvider
	STDMETHODIMP GetThumbnail(UINT cx, HBITMAP * phbmp, WTS_ALPHATYPE * pdwAlpha);

};

OBJECT_ENTRY_AUTO(__uuidof(ThumbnailHandler), CThumbnailHandler)
