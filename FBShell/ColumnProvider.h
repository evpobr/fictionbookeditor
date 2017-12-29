#pragma once

#include "resource.h"
#include "FBShell.h"
#include <InitGuid.h>

using namespace ATL;

// {69EA815C-7D5E-486e-85D7-433B19127467}
DEFINE_GUID(FMTID_FB,
	0x69ea815c, 0x7d5e, 0x486e, 0x85, 0xd7, 0x43, 0x3b, 0x19, 0x12, 0x74, 0x67);

///////////////////////////////////////////////////////////
// Column provider

class ATL_NO_VTABLE CColumnProvider :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CColumnProvider, &CLSID_ColumnProvider>,
	public IColumnProvider
{
public:
	CColumnProvider()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_COLUMNPROVIDER)

	DECLARE_NOT_AGGREGATABLE(CColumnProvider)

	BEGIN_COM_MAP(CColumnProvider)
		COM_INTERFACE_ENTRY(IColumnProvider)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		::InitializeCriticalSection(&m_lock);
		return S_OK;
	}

	void FinalRelease()
	{
		::DeleteCriticalSection(&m_lock);
	}

	// IColumnProvider
	STDMETHOD(Initialize)(LPCSHCOLUMNINIT psci) { return S_OK; }
	STDMETHOD(GetColumnInfo)(DWORD dwIndex, SHCOLUMNINFO *psci);
	STDMETHOD(GetItemData)(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData);

protected:
	enum {
		PIDFB_LANG,
		PIDFB_SRCLANG,
		PIDFB_SEQ,
		PIDFB_DOCAUTH,
		PIDFB_DOCDATE,
		PIDFB_VER,
		PIDFB_ID,
		PIDFB_DOCDV,
		PIDFB_DV
	};

	struct FBInfo {
		CString	    filename;

		struct title_info {
			CString	    genres;
			CString	    authors;
			CString	    title;
			CString	    date;
			CString	    dateval;
			CString	    lang;
			CString	    srclang;
			CString	    seq;
		} title;
		struct document_info {
			CString	    authors;
			CString	    date;
			CString	    dateval;
			CString	    id;
			CString	    ver;
		} doc;

		HRESULT     Init(const wchar_t *filename);
		void	      Clear();

		static HRESULT    GetVariant(const CString& str, VARIANT *vt);

		// field accessors
#define	FIELD(cat,name) static HRESULT get_##cat##_##name(const FBInfo& fbi,VARIANT *vt) { return GetVariant(fbi.cat.name,vt); }
		FIELD(title, genres)
			FIELD(title, authors)
			FIELD(title, title)
			FIELD(title, date)
			FIELD(title, lang)
			FIELD(title, srclang)
			FIELD(title, seq)
			FIELD(title, dateval)
			FIELD(doc, authors)
			FIELD(doc, date)
			FIELD(doc, id)
			FIELD(doc, ver)
			FIELD(doc, dateval)
#undef FIELD
	};

	struct ColumnInfo {
		const wchar_t     *name;
		int		      width;
		DWORD	      col;
		const GUID	      *fmtid;
		DWORD	      pid;
		HRESULT(*handler)(const FBInfo& fbi, VARIANT *ret);
	};

	static ColumnInfo g_columns[];
	FBInfo	    m_cache;
	CRITICAL_SECTION  m_lock;

	class ContentHandlerImpl;
	typedef CComObject<ContentHandlerImpl>	ContentHandler;
	typedef CComPtr<ContentHandler>		ContentHandlerPtr;
};

OBJECT_ENTRY_AUTO(CLSID_ColumnProvider, CColumnProvider)

