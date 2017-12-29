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
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject *obj, HKEY progid);

	// IContextMenu
	STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT idx, UINT cmdFirst, UINT cmdLast, UINT flags);
	STDMETHOD(GetCommandString)(UINT_PTR cmd, UINT flags, UINT *, LPSTR name, UINT namelen);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
protected:
	CSimpleArray<CString>	    m_files;
	bool			    m_folders;
};

OBJECT_ENTRY_AUTO(CLSID_ContextMenu, CContextMenu)

