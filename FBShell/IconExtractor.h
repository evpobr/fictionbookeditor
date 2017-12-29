#pragma once

#include "resource.h"
#include "FBShell.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

///////////////////////////////////////////////////////////
// Image extractor

class ATL_NO_VTABLE CIconExtractor :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIconExtractor, &CLSID_IconExtractor>,
	public IPersistFile,
	public IExtractImage2
{
public:
	CIconExtractor()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_ICONEXTRACTOR)

	DECLARE_NOT_AGGREGATABLE(CIconExtractor)


	BEGIN_COM_MAP(CIconExtractor)
		COM_INTERFACE_ENTRY(IPersist)
		COM_INTERFACE_ENTRY(IPersistFile)
		COM_INTERFACE_ENTRY(IExtractImage)
		COM_INTERFACE_ENTRY(IExtractImage2)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct() {
		m_filename.Empty();
		m_desired_size.cx = m_desired_size.cy = 32;
		m_desired_depth = 24;

		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IPersist
	STDMETHOD(GetClassID)(CLSID *id) { *id = CLSID_IconExtractor; return S_OK; }

	// IPersistFile
	STDMETHOD(IsDirty)() { return E_NOTIMPL; }
	STDMETHOD(Load)(const wchar_t *name, DWORD mode) { m_filename = name; return S_OK; }
	STDMETHOD(Save)(const wchar_t *name, BOOL remember) { return E_NOTIMPL; }
	STDMETHOD(SaveCompleted)(const wchar_t *name) { return E_NOTIMPL; }
	STDMETHOD(GetCurFile)(wchar_t **name) { return E_NOTIMPL; }

	// IExtractImage
	STDMETHOD(GetLocation)(wchar_t *file, DWORD filelen, DWORD *prio, const SIZE *sz, DWORD depth, DWORD *flags);
	STDMETHOD(Extract)(HBITMAP *hBmp);

	// IExtractImage2
	STDMETHOD(GetDateStamp)(FILETIME *tm);

protected:
	CString		      m_filename;
	SIZE			  m_desired_size;
	int			      m_desired_depth;

	class ContentHandlerImpl;
	typedef CComObject<ContentHandlerImpl>	ContentHandler;
	typedef CComPtr<ContentHandler>		ContentHandlerPtr;

	// common functions
	static bool		LoadObject(const wchar_t *filename, CString& type,
		void *&data, int& datalen);
};

OBJECT_ENTRY_AUTO(CLSID_IconExtractor, CIconExtractor)
