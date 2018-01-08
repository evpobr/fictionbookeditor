#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>

EXTERN_C const GUID CLSID_MemProtocol;

class ATL_NO_VTABLE CMemProtocol : public CComObjectRoot,
                                   public CComCoClass<CMemProtocol, &CLSID_MemProtocol>,
                                   public IInternetProtocol
{
	const char * m_buffer;
	DWORD m_bytes;
	_variant_t m_data;
	bool m_unaccess;

public:
	CMemProtocol();
	virtual ~CMemProtocol();

	DECLARE_NO_REGISTRY()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CMemProtocol)
		COM_INTERFACE_ENTRY(IInternetProtocolRoot)
		COM_INTERFACE_ENTRY(IInternetProtocol)
	END_COM_MAP()

	// IInternetProtocolRoot
	STDMETHODIMP Abort(HRESULT /*hrReason*/, DWORD /*dwOptions*/);
	STDMETHODIMP Continue(PROTOCOLDATA * pd);
	STDMETHODIMP Resume();
	STDMETHODIMP Start(const wchar_t * url, IInternetProtocolSink * sink, IInternetBindInfo * bi, DWORD flags, HANDLE_PTR dwReserved);
	STDMETHODIMP Suspend();
	STDMETHODIMP Terminate(DWORD dwOptions);

	// IInternetProtocol
	STDMETHODIMP LockRequest(DWORD dwOptions);
	STDMETHODIMP Read(void * pv, ULONG cb, ULONG * cbRead);
	STDMETHODIMP Seek(LARGE_INTEGER off, DWORD /*org*/, ULARGE_INTEGER * /*newoff*/);
	STDMETHODIMP UnlockRequest();
};
