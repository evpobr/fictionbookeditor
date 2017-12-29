#pragma once

#include "resource.h"
#include "FBShell.h"

using namespace ATL;

///////////////////////////////////////////////////////////
// Validation context menu

class ATL_NO_VTABLE CContextMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CContextMenu, &CLSID_ContextMenu>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CContextMenu()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTMENU)

	DECLARE_NOT_AGGREGATABLE(CContextMenu)

	BEGIN_COM_MAP(CContextMenu)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IShellExtInit
	STDMETHODIMP Initialize(_In_opt_ PCIDLIST_ABSOLUTE pidlFolder,
							_In_opt_ IDataObject *pdtobj,
							_In_opt_ HKEY hkeyProgID);

	// IContextMenu
	STDMETHODIMP QueryContextMenu(_In_ HMENU hmenu,
								  _In_ UINT indexMenu,
								  _In_ UINT idCmdFirst,
								  _In_ UINT idCmdLast,
								  _In_ UINT uFlags);

	STDMETHODIMP GetCommandString(_In_ UINT_PTR idCmd,
								  _In_ UINT uType,
								  _Reserved_ UINT *pReserved,
								  _Out_writes_bytes_((uType & GCS_UNICODE) ? (cchMax * sizeof(wchar_t)) : cchMax) _When_(!(uType & (GCS_VALIDATEA | GCS_VALIDATEW)), _Null_terminated_) CHAR *pszName,
								  _In_ UINT cchMax);

	STDMETHODIMP InvokeCommand(_In_ CMINVOKECOMMANDINFO *pici);
protected:
	CSimpleArray<CString>	    m_files;
	bool			    m_folders;
};

OBJECT_ENTRY_AUTO(CLSID_ContextMenu, CContextMenu)

