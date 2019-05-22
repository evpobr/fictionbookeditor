#include "stdafx.h"

#include "MemProtocol.h"
#include "FBDoc.h"

CMemProtocol::CMemProtocol()
    : m_buffer(0), m_bytes(0), m_unaccess(false)
{
}

CMemProtocol::~CMemProtocol()
{
	if (m_unaccess)
		::SafeArrayUnaccessData(V_ARRAY(&m_data));
}

// IInternetProtocolRoot

STDMETHODIMP CMemProtocol::Abort(HRESULT, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMemProtocol::Continue(PROTOCOLDATA * /*pd*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMemProtocol::Resume()
{
	return E_NOTIMPL;
}

STDMETHODIMP CMemProtocol::Start(const wchar_t * url, IInternetProtocolSink * sink, IInternetBindInfo *, DWORD, HANDLE_PTR)
{
	// check if this an embedded image
	const wchar_t * col = wcschr(url, L':');
	FB::Doc * doc;
	if (col)
	{
		//doc=FB::Doc::LocateDocument(col+1);
		doc = FB::Doc::m_active_doc;
		if (doc)
		{
			//col=wcschr(col+1,L':');
			if (col && doc->GetBinary(col + 1, m_data))
			{
				if (V_VT(&m_data) == (VT_UI1 | VT_ARRAY) && ::SafeArrayGetDim(V_ARRAY(&m_data)) == 1)
				{
				gotit:
					m_bytes = V_ARRAY(&m_data)->rgsabound[0].cElements;
					m_unaccess = true;
					::SafeArrayAccessData(V_ARRAY(&m_data), (void **)&m_buffer);
				okreport:
					sink->ReportData(BSCF_DATAFULLYAVAILABLE, 1, 1);
					sink->ReportResult(S_OK, 0, NULL);
					return S_OK;
				}
				else if (V_VT(&m_data) == VT_BSTR)
				{
					m_buffer = (const char *)V_BSTR(&m_data);
					m_bytes = ::SysStringByteLen(V_BSTR(&m_data));
					m_unaccess = false;
					sink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"text/html");
					goto okreport;
				}
			}
			m_data.Clear();
		}
	}

	// fallback to a builtin image
	HRESULT hr = U::LoadFile(U::GetProgDirFile(_T("imgph.png")), &m_data);
	if (SUCCEEDED(hr))
		goto gotit;

	// or fail
	m_bytes = 0;
	return hr;
}

STDMETHODIMP CMemProtocol::Suspend()
{
	return E_NOTIMPL;
}

STDMETHODIMP CMemProtocol::Terminate(DWORD /*dwOptions*/)
{
	if (m_unaccess)
	{
		::SafeArrayUnaccessData(V_ARRAY(&m_data));
		m_unaccess = false;
	}
	if (V_VT(&m_data) != VT_EMPTY)
		m_data.Clear();
	m_bytes = 0;
	return S_OK;
}

// IInternetProtocol

STDMETHODIMP CMemProtocol::LockRequest(DWORD /*dwOptions*/)
{
	return S_OK;
}

STDMETHODIMP CMemProtocol::Read(void * pv, ULONG cb, ULONG * cbRead)
{
	if (m_bytes == 0)
		return S_FALSE;
	if (cb > m_bytes)
		cb = m_bytes;
	memcpy(pv, m_buffer, cb);
	*cbRead = cb;
	m_buffer += cb;
	m_bytes -= cb;
	return S_OK;
}

STDMETHODIMP CMemProtocol::Seek(LARGE_INTEGER /*off*/, DWORD, ULARGE_INTEGER *)
{
	return E_FAIL;
}

STDMETHODIMP CMemProtocol::UnlockRequest()
{
	return S_OK;
}
