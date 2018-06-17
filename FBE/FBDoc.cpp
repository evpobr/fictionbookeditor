// Doc.cpp: implementation of the Doc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "res1.h"
#include "resource.h"

#include "apputils.h"
#include "utils.h"

#include "ElementDescMnr.h"
#include "FBDoc.h"
#include "Scintilla.h"
#include "Settings.h"

extern CElementDescMnr _EDMnr;

extern CSettings _Settings;

namespace FB
{

// namespaces
const _bstr_t FBNS(L"http://www.gribuser.ru/xml/fictionbook/2.0");
const _bstr_t XLINKNS(L"http://www.w3.org/1999/xlink");
const _bstr_t NEWLINE(L"\n");

// document list
CSimpleMap<Doc *, Doc *> Doc::m_active_docs;
Doc * FB::Doc::m_active_doc;
bool FB::Doc::m_fast_mode;

Doc * Doc::LocateDocument(const wchar_t * id)
{
	unsigned long * lv = nullptr;
	if (swscanf(id, L"%lu", lv) != 1)
		return NULL;
	return m_active_docs.Lookup((Doc *)lv);
}

// initialize a new Doc
Doc::Doc(HWND hWndFrame)
    : m_filename(_T("Untitled.fb2")), m_namevalid(false),
      m_body(hWndFrame, true),
      m_frame(hWndFrame),
      m_body_ver(-1),
      m_body_cp(-1),
      m_encoding(_T("utf-8"))
{
	m_active_docs.Add(this, this);
}

// destroy a Doc
Doc::~Doc()
{
	if (m_body.IsWindow())
		m_body.DestroyWindow();
	m_active_docs.Remove(this);
}

bool Doc::GetBinary(const wchar_t * id, _variant_t & vt)
{
	if (id && *id == L'#')
	{
		CComDispatchDriver body(m_body.Script());
		_variant_t vid(id + 1);
		body.Invoke1(L"apiGetBinary", &vid, &vt);
		return true;
	}
	return false;
}

struct ThreadArgs
{
	MSXML2::IXSLProcessor * proc;
	HANDLE hWr;
};

static DWORD __stdcall XMLTransformThread(LPVOID varg)
{
	ThreadArgs * arg = (ThreadArgs *)varg;

	arg->proc->put_output(_variant_t(U::NewStream(arg->hWr)));
	VARIANT_BOOL val;
	arg->proc->raw_transform(&val);
	arg->proc->Release();

	delete arg;

	return 0;
}

LPCWSTR Doc::MyID() const
{
	CString ret;
	ret.Format(_T("%lu"), reinterpret_cast<unsigned long>(this));
	return ret;
}

LPCWSTR Doc::MyURL(LPCWSTR pszPart) const
{
	CString ret;
	ret.Format(_T("fbw-internal:%lu:%s"), reinterpret_cast<unsigned long>(this), pszPart);
	return ret;
}

static MSXML2::IXSLTemplatePtr LoadXSL(const CString & path)
{
	MSXML2::IXMLDOMDocument2Ptr xsl;
	CheckError(U::CreateDocument(true, &xsl));
	HRESULT hr = U::LoadXml(xsl, U::GetProgDirFile(path));
	if (FAILED(hr) || (hr != S_OK))
		throw _com_error(E_FAIL);
	MSXML2::IXSLTemplatePtr tp;
	CheckError(U::CreateTemplate(&tp));
	tp->stylesheet = xsl;
	return tp;
}

HRESULT Doc::InvokeFunc(BSTR FuncName, CComVariant * params, int count, CComVariant & vtResult)
{
	LPDISPATCH pScript;
	IHTMLDocument2Ptr doc = m_body.Browser()->Document;
	doc->get_Script(&pScript);
	pScript->AddRef();

	BSTR szMember = FuncName;
	DISPID dispid;

	HRESULT hr = pScript->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);

	if (SUCCEEDED(hr))
	{
		CComDispatchDriver dispDriver(pScript);
		hr = dispDriver.InvokeN(dispid, params, count, &vtResult);
	}

	return hr;
}

void Doc::ShowDescription(bool Show)
{
	CComVariant vtResult;
	CComVariant params;
	V_VT(&params) = VT_BOOL;
	V_BOOL(&params) = Show;
	InvokeFunc(L"apiShowDesc", &params, 1, vtResult);
	return;
}

void Doc::RunScript(BSTR filePath)
{
	CComVariant vtResult;
	CComVariant params(filePath);
	InvokeFunc(L"apiRunCmd", &params, 1, vtResult);
}

VARIANT_BOOL Doc::CheckScript(BSTR filePath)
{
	CComVariant vtResult;
	CComVariant params(filePath);
	InvokeFunc(L"apiCheckScript", &params, 1, vtResult);

	return vtResult.boolVal;
}

bool Doc::LoadFromHTML(HWND hWndParent, const CString & filename)
{
	HRESULT hr;
	wchar_t path[MAX_PATH + 1];
	GetModuleFileName(0, path, MAX_PATH);
	PathRemoveFileSpec(path);
	wcscat(path, L"\\main.html");
	CRect rcBody(0, 0, 500, 500);
	m_body.Create(hWndParent, &rcBody, _T("{8856F961-340A-11D0-A96B-00C04FD705A2}"));
	hr = m_body.Browser()->Navigate(path);
	MSG msg;
	while (!m_body.Loaded() && ::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	m_body.SetExternalDispatch(m_body.CreateHelper());

	m_body.Init();

	CComVariant params[2];
	params[1] = filename;
	params[0] = _Settings.GetInterfaceLanguageName();
	CComVariant res;

	ApplyConfChanges();
	hr = InvokeFunc(L"apiLoadFB2", params, 2, res);
	MarkSavePoint();

	if (res.vt == VT_BOOL)
	{
		m_encoding = _Settings.GetDefaultEncoding();
		return res.boolVal != VARIANT_FALSE;
	}

	if (res.vt == VT_BSTR)
	{
		m_encoding = res.bstrVal;
		return true;
	}

	if (res.vt == VT_EMPTY)
	{
		m_encoding = _Settings.GetDefaultEncoding();
		return true;
	}

	return false;
}

bool Doc::Load(HWND hWndParent, const CString & filename)
{

	try
	{
		CAutoPtr<CWaitCursor> spHourGlass(new CWaitCursor());
		if (!LoadFromHTML(hWndParent, filename))
		{
			return false;
		}

		m_filename = filename;
		m_namevalid = true;
	}
	catch (_com_error & e)
	{
		U::ReportError(e);
		return false;
	}

	return true;
}

void Doc::CreateBlank(HWND hWndParent)
{
	try
	{
		LoadFromHTML(hWndParent, L"blank.fb2");
	}
	catch (_com_error & e)
	{
		U::ReportError(e);
	}
}

// indent something
static void Indent(MSXML2::IXMLDOMNode * node, MSXML2::IXMLDOMDocument2 * xml, int len)
{
	// inefficient
	BSTR s = SysAllocStringLen(NULL, len + 2);
	if (s)
	{
		s[0] = L'\r';
		s[1] = L'\n';
		for (BSTR p = s + 2, q = s + 2 + len; p < q; ++p)
			*p = L' ';
		MSXML2::IXMLDOMTextPtr text;
		if (SUCCEEDED(xml->raw_createTextNode(s, &text)))
			node->raw_appendChild(text, NULL);
		SysFreeString(s);
	}
}

// set an attribute on the element
static void SetAttr(MSXML2::IXMLDOMElement * xe, const wchar_t * name, const wchar_t * ns, const _bstr_t & val, MSXML2::IXMLDOMDocument2 * doc)
{
	MSXML2::IXMLDOMAttributePtr attr(doc->createNode(2L, name, ns));
	attr->appendChild(doc->createTextNode(val));
	xe->setAttributeNode(attr);
}

// setup an ID for the element
static void SetID(MSHTML::IHTMLElement * he, MSXML2::IXMLDOMElement * xe, MSXML2::IXMLDOMDocument2 * doc)
{
	_bstr_t id(he->id);
	if (id.length() > 0)
		SetAttr(xe, L"id", FBNS, id, doc);
}

// copy text
static MSXML2::IXMLDOMTextPtr MkText(MSHTML::IHTMLDOMNode * hn, MSXML2::IXMLDOMDocument2 * xml)
{
	VARIANT vt;
	VariantInit(&vt);
	CheckError(hn->get_nodeValue(&vt));
	if (V_VT(&vt) != VT_BSTR)
	{
		VariantClear(&vt);
		return xml->createTextNode(_bstr_t());
	}
	MSXML2::IXMLDOMText * txt;
	HRESULT hr = xml->raw_createTextNode(V_BSTR(&vt), &txt);
	VariantClear(&vt);
	CheckError(hr);
	return MSXML2::IXMLDOMTextPtr(txt, FALSE);
}

// set an href attribute
static void SetHref(MSXML2::IXMLDOMElementPtr xe, MSXML2::IXMLDOMDocument2 * xml, const _bstr_t & href)
{
	SetAttr(xe, L"l:href", XLINKNS, href, xml);
}

static void SetTitle(MSXML2::IXMLDOMElementPtr xe, MSXML2::IXMLDOMDocument2 * xml, const _bstr_t & title)
{
	if (!title)
	{
		return;
	}
	SetAttr(xe, L"title", FB::FBNS, title, xml);
}

// handle inline formatting
static MSXML2::IXMLDOMNodePtr ProcessInline(MSHTML::IHTMLDOMNode * inl, MSXML2::IXMLDOMDocument2 * doc)
{
	// Source
	_bstr_t name(inl->nodeName);
	MSHTML::IHTMLElementPtr einl(inl);
	_bstr_t cls(einl->className);

	const wchar_t * xname = NULL;
	bool fA = false;
	bool fStyle = false;
	bool fUnk = false;
	bool fImg = false;

	// Modification by Pilgrim
	if (U::scmp(name, L"STRONG") == 0)
	{
		xname = L"strong";
	}
	else if (U::scmp(name, L"EM") == 0)
	{
		xname = L"emphasis";
	}
	else if (U::scmp(name, L"STRIKE") == 0)
	{
		xname = L"strikethrough";
	}
	else if (U::scmp(name, L"SUB") == 0)
	{
		xname = L"sub";
	}
	else if (U::scmp(name, L"SUP") == 0)
	{
		xname = L"sup";
	}
	else if (U::scmp(name, L"A") == 0)
	{
		xname = L"a";
		fA = true;
	}
	else if (U::scmp(name, L"SPAN") == 0)
	{
		if (U::scmp(cls, L"unknown_element") == 0)
		{
			_bstr_t realClassName = einl->getAttribute(L"source_class", 2);
			xname = realClassName;
			fUnk = true;
		}
		else if (U::scmp(cls, L"image") == 0)
		{
			fImg = true;
			xname = L"image";
		}
		else if (U::scmp(cls, L"code") == 0)
		{
			xname = L"code";
		}
		else
		{
			xname = L"style";
			fStyle = true;
		}
	}

	MSXML2::IXMLDOMElementPtr xinl(doc->createNode(1L, xname, FBNS));

	if (fImg)
		SetHref(xinl, doc, AU::GetAttrB(einl, L"href"));

	if (fA)
	{
		SetHref(xinl, doc, AU::GetAttrB(einl, L"href"));
		if (U::scmp(cls, L"note") == 0)
		{
			SetAttr(xinl, L"type", FBNS, cls, doc);
		}
	}
	if (fStyle)
		SetAttr(xinl, L"name", FBNS, cls, doc);

	if (fUnk)
	{
		MSHTML::IHTMLAttributeCollectionPtr col = inl->attributes;
		for (long i = 0; i < col->length; ++i)
		{
			VARIANT index;
			V_VT(&index) = VT_INT;
			index.intVal = i;
			_bstr_t attr_name = MSHTML::IHTMLDOMAttributePtr(col->item(&index))->nodeName;
			_bstr_t attr_value;
			wchar_t * real_attr_name = 0;
			const wchar_t * prefix = L"unknown_attribute_";
			if (wcsncmp(attr_name, prefix, wcslen(prefix)))
			{
				continue;
			}
			else
			{
				real_attr_name = (wchar_t *)attr_name + wcslen(prefix);
				attr_value = MSHTML::IHTMLDOMAttributePtr(col->item(&index))->nodeValue;
			}
			MSXML2::IXMLDOMAttributePtr attr(doc->createNode(2L, real_attr_name, FBNS));
			attr->appendChild(doc->createTextNode(attr_value));
			xinl->setAttributeNode(attr);
		}
	}

	MSHTML::IHTMLDOMNodePtr cn(inl->firstChild);

	// Modification by Pilgrim
	while ((bool)cn)
	{
		if (cn->nodeType == NODE_TEXT /*3*/)
		{
			xinl->appendChild(MkText(cn, doc));
		}
		else if ((cn->nodeType == NODE_ELEMENT /*1*/) && (!fImg)) // added by SeNS
		{
			xinl->appendChild(ProcessInline(cn, doc));
		}
		cn = cn->nextSibling;
	}

	return xinl;
}

// handle a paragraph element with subelements
static MSXML2::IXMLDOMNodePtr ProcessP(MSHTML::IHTMLElement * p, MSXML2::IXMLDOMDocument2 * doc, const wchar_t * baseName)
{
	_bstr_t cls(p->className);
	if (U::scmp(cls, L"text-author") == 0)
		baseName = L"text-author";
	else if (U::scmp(cls, L"subtitle") == 0)
		baseName = L"subtitle";
	// Modification by Pilgrim
	else if (U::scmp(cls, L"th") == 0)
		baseName = L"th";
	else if (U::scmp(cls, L"td") == 0)
		baseName = L"td";

	MSHTML::IHTMLDOMNodePtr hp(p);

	// check if it is an empty-line
	if (U::scmp(cls, L"td") && U::scmp(cls, L"th"))
	{
		if (hp->hasChildNodes() == VARIANT_FALSE || (!(bool)hp->firstChild->nextSibling && hp->firstChild->nodeType == 3 && U::is_whitespace(hp->firstChild->nodeValue.bstrVal)))
		{
			MSHTML::IHTMLElementPtr pParent = MSHTML::IHTMLElementPtr(p)->parentElement;
			if (MSHTML::IHTMLElement3Ptr(p)->inflateBlock == VARIANT_TRUE)
			{
				if (pParent && U::scmp(pParent->className, L"stanza") == 0)
				{
					MSXML2::IXMLDOMNodePtr vNode = doc->createNode(NODE_ELEMENT, L"v", FBNS);
					vNode->appendChild(doc->createTextNode(L" "));
					return vNode;
				}
				else
				{
					return doc->createNode(1L, L"empty-line", FBNS);
				}
			}
			return MSXML2::IXMLDOMNodePtr();
		}
	}

	MSXML2::IXMLDOMElementPtr xp(doc->createNode(1L, baseName, FBNS));

	SetID(p, xp, doc);

	_bstr_t style(AU::GetAttrB(p, L"fbstyle"));
	if (style.length() > 0)
		SetAttr(xp, L"style", FBNS, style, doc);

	// Modification by Pilgrim
	_bstr_t colspan(AU::GetAttrB(p, L"fbcolspan"));
	if (colspan.length() > 0)
		SetAttr(xp, L"colspan", FBNS, colspan, doc);

	_bstr_t rowspan(AU::GetAttrB(p, L"fbrowspan"));
	if (rowspan.length() > 0)
		SetAttr(xp, L"rowspan", FBNS, rowspan, doc);

	_bstr_t align(AU::GetAttrB(p, L"fbalign"));
	if (align.length() > 0)
		SetAttr(xp, L"align", FBNS, align, doc);

	_bstr_t valign(AU::GetAttrB(p, L"fbvalign"));
	if (valign.length() > 0)
		SetAttr(xp, L"valign", FBNS, valign, doc);

	hp = hp->firstChild;

	// Modification by Pilgrim
	while ((bool)hp)
	{
		if (hp->nodeType == NODE_TEXT /*3*/) // text segment
		{
			xp->appendChild(MkText(hp, doc));
		}
		else if (hp->nodeType == NODE_ELEMENT /*1*/)
		{
			xp->appendChild(ProcessInline(hp, doc));
		}
		hp = hp->nextSibling;
	}

	_bstr_t selected(AU::GetAttrB(p, L"fbe_selected"));
	if (selected.length() > 0)
		SetAttr(xp, L"selected", FBNS, selected, doc);

	return xp;
}

// handle a div element with subelements
static MSXML2::IXMLDOMNodePtr ProcessDiv(MSHTML::IHTMLElement * div, MSXML2::IXMLDOMDocument2 * doc, int indent)
{
	_bstr_t cls(div->className);

	MSXML2::IXMLDOMElementPtr xdiv(doc->createNode(1L, cls, FBNS));

	if (U::scmp(cls, L"image") == 0)
	{
		SetID(div, xdiv, doc);
		SetHref(xdiv, doc, AU::GetAttrB(div, L"href"));
		SetTitle(xdiv, doc, AU::GetAttrB(div, L"title"));
		return xdiv;
	}

	SetID(div, xdiv, doc);

	// Modification by Pilgrim
	if (U::scmp(cls, L"table") == 0)
	{
		_bstr_t style(AU::GetAttrB(div, L"fbstyle"));
		if (style.length() > 0)
		{
			SetAttr(xdiv, L"style", FBNS, style, doc);
		}
	}
	if (U::scmp(cls, L"tr") == 0)
	{
		_bstr_t align(AU::GetAttrB(div, L"fbalign"));
		if (align.length() > 0)
		{
			SetAttr(xdiv, L"align", FBNS, align, doc);
		}
	}

	MSHTML::IHTMLDOMNodePtr ndiv(div);
	MSHTML::IHTMLDOMNodePtr fc(ndiv->firstChild);

	LPCWSTR bn = U::scmp(cls, L"stanza") == 0 ? L"v" : L"p";

	while ((bool)fc)
	{
		_bstr_t name(fc->nodeName);
		MSHTML::IHTMLElementPtr efc(fc);
		// process empty lines
		MSHTML::IHTMLElement3Ptr(efc)->inflateBlock = VARIANT_TRUE;

		if (U::scmp(name, L"DIV") == 0)
		{
			Indent(xdiv, doc, indent + 1);
			MSXML2::IXMLDOMNodePtr nnp = ProcessDiv(efc, doc, indent + 1);
			xdiv->appendChild(nnp);
		}
		else if (U::scmp(name, L"P") == 0)
		{
			MSXML2::IXMLDOMNodePtr np;
			try
			{
				np = ProcessP(efc, doc, bn);
			}
			catch (...)
			{
				np = 0;
			}
			if (np)
			{
				Indent(xdiv, doc, indent + 1);
				xdiv->appendChild(np);
			}
		}

		fc = fc->nextSibling;
	}

	Indent(xdiv, doc, indent);

	return xdiv;
}

// find a first named DIV
static MSXML2::IXMLDOMNodePtr GetDiv(MSHTML::IHTMLElementPtr body, MSXML2::IXMLDOMDocument2 * xml, LPCWSTR name, int indent)
{
	MSHTML::IHTMLElementCollectionPtr children(body->children);
	long c_len = children->length;

	for (long i = 0; i < c_len; ++i)
	{
		MSHTML::IHTMLElementPtr div(children->item(i));
		if (!(bool)div)
		{
			continue;
		}
		if (U::scmp(div->tagName, L"DIV") == 0 && U::scmp(div->className, name) == 0)
		{
			return ProcessDiv(div, xml, indent);
		}
	}

	return MSXML2::IXMLDOMNodePtr();
}

// fetch bodies
static void GetBodies(MSHTML::IHTMLElementPtr body, MSXML2::IXMLDOMDocument2 * doc)
{
	MSHTML::IHTMLElementCollectionPtr children(body->children);
	long c_len = children->length;

	for (long i = 0; i < c_len; ++i)
	{
		MSHTML::IHTMLElementPtr div(children->item(i));

		if (!(bool)div)
		{
			continue;
		}

		if (U::scmp(div->tagName, L"DIV") == 0 && U::scmp(div->className, L"body") == 0)
		{
			MSXML2::IXMLDOMElementPtr xb(ProcessDiv(div, doc, 1));
			_bstr_t bn(AU::GetAttrB(div, L"fbname"));
			if (bn.length() > 0)
			{
				SetAttr(xb, L"name", FBNS, bn, doc);
			}
			Indent(doc->documentElement, doc, 1);
			doc->documentElement->appendChild(xb);
		}
	}
}

// validator object
class SAXErrorHandler : public CComObjectRoot, public MSXML2::ISAXErrorHandler
{
public:
	CString m_msg;
	int m_line, m_col;

	SAXErrorHandler()
	    : m_line(0), m_col(0)
	{
	}

	void SetMsg(MSXML2::ISAXLocator * loc, LPCWSTR msg, HRESULT /*hr*/)
	{
		if (!m_msg.IsEmpty())
			return;
		m_msg = msg;
		CString ns;
		ns.Format(_T("{%s}"), (LPCWSTR)FBNS);
		m_msg.Replace(ns, _T(""));
		ns.Format(_T("{%s}"), (LPCWSTR)XLINKNS);
		m_msg.Replace(ns, _T("xlink"));
		m_line = loc->getLineNumber();
		m_col = loc->getColumnNumber();
	}

	BEGIN_COM_MAP(SAXErrorHandler)
	COM_INTERFACE_ENTRY(MSXML2::ISAXErrorHandler)
	END_COM_MAP()

	STDMETHOD(raw_error)
	(MSXML2::ISAXLocator * loc, wchar_t * msg, HRESULT hr)
	{
		SetMsg(loc, msg, hr);
		return E_FAIL;
	}
	STDMETHOD(raw_fatalError)
	(MSXML2::ISAXLocator * loc, wchar_t * msg, HRESULT hr)
	{
		SetMsg(loc, msg, hr);
		return E_FAIL;
	}
	STDMETHOD(raw_ignorableWarning)
	(MSXML2::ISAXLocator * loc, wchar_t * msg, HRESULT hr)
	{
		SetMsg(loc, msg, hr);
		return E_FAIL;
	}
};

HRESULT Doc::CreateDOMImp(LPCWSTR pszEncoding, MSXML2::IXMLDOMDocument2 ** ppDoc)
{
	if (!ppDoc)
		return E_POINTER;

	*ppDoc = nullptr;

	HRESULT hr = S_OK;

	try
	{
		// normalize body first
		_EDMnr.CleanUpAll();
		m_body.Normalize(m_body.Document()->body);

		// create document
		MSXML2::IXMLDOMDocument2Ptr ndoc;
		CheckError(U::CreateDocument(false, &ndoc));
		ndoc->async = VARIANT_FALSE;

		bstr_t encoding(pszEncoding);
		// set encoding
		if (encoding.length() > 0)
			ndoc->appendChild(ndoc->createProcessingInstruction(L"xml", (const wchar_t *)(L"version=\"1.0\" encoding=\"" + encoding + L"\"")));

		// create document element
		MSXML2::IXMLDOMElementPtr root = ndoc->createNode(_variant_t(1L), L"FictionBook", FBNS);
		root->setAttribute(L"xmlns:l", XLINKNS);
		ndoc->documentElement = MSXML2::IXMLDOMElementPtr(root);

		// enable xpath queries
		ndoc->setProperty(L"SelectionLanguage", L"XPath");
		CString nsprop(L"xmlns:fb='");
		nsprop += (const wchar_t *)FBNS;
		nsprop += L"' xmlns:xlink='";
		nsprop += (const wchar_t *)XLINKNS;
		nsprop += L"'";
		ndoc->setProperty(L"SelectionNamespaces", (const TCHAR *)nsprop);

		// fetch annotation

		MSHTML::IHTMLElementCollectionPtr children(m_body.Document()->body->children);
		long c_len = children->length;

		MSHTML::IHTMLElementPtr fbw_body;

		for (long i = 0; i < c_len; ++i)
		{
			MSHTML::IHTMLElementPtr div(children->item(i));

			if (!(bool)div)
			{
				continue;
			}

			if (U::scmp(div->tagName, L"DIV") == 0 && U::scmp(div->id, L"fbw_body") == 0)
			{
				fbw_body = div;
				break;
			}
		}

		MSXML2::IXMLDOMNodePtr ann(GetDiv(fbw_body, ndoc, L"annotation", 3));

		// fetch history
		MSXML2::IXMLDOMNodePtr hist(GetDiv(fbw_body, ndoc, L"history", 3));

		// fetch description
		CComDispatchDriver body(m_body.Script());
		CComVariant args[3];
		if (hist)
		{
			args[0] = hist.GetInterfacePtr();
		}
		if (ann)
		{
			args[1] = ann.GetInterfacePtr();
		}
		args[2] = ndoc.GetInterfacePtr();
		CComVariant res;
		CheckError(body.InvokeN(L"GetDesc", &args[0], 3));

		// fetch body elements
		GetBodies(fbw_body, ndoc);

		// fetch binaries
		CheckError(body.Invoke1(L"GetBinaries", &args[2]));

		Indent(root, ndoc, 0);

		CheckError(ndoc.QueryInterface(IID_PPV_ARGS(ppDoc)));
	}
	catch (const _com_error & err)
	{
		hr = err.Error();
	}

	return hr;
}

HRESULT Doc::CreateDOM(_In_z_ LPCWSTR pszEncoding, _COM_Outptr_ MSXML2::IXMLDOMDocument2 ** ppDoc)
{
	if (!ppDoc)
		return E_POINTER;

	*ppDoc = nullptr;

	return CreateDOMImp(pszEncoding, ppDoc);
}

bool Doc::SaveToFile(const CString & filename, bool fValidateOnly, int * errline, int * errcol)
{
	try
	{
		// create a schema collection
		MSXML2::IXMLDOMSchemaCollection2Ptr scol;
		CheckError(scol.CreateInstance(L"Msxml2.XMLSchemaCache.6.0"));

		// load fictionbook schema
		scol->add(FBNS, (const wchar_t *)U::GetProgDirFile(L"FictionBook.xsd"));

		// create a SAX reader
		MSXML2::ISAXXMLReaderPtr rdr;
		CheckError(rdr.CreateInstance(L"Msxml2.SAXXMLReader.6.0"));

		// attach a schema
		rdr->putFeature(L"schema-validation", VARIANT_TRUE);
		rdr->putProperty(L"schemas", scol.GetInterfacePtr());
		rdr->putFeature(L"exhaustive-errors", VARIANT_TRUE);

		// create an error handler
		CComObject<SAXErrorHandler> * ehp;
		CheckError(CComObject<SAXErrorHandler>::CreateInstance(&ehp));
		CComPtr<CComObject<SAXErrorHandler>> eh(ehp);
		rdr->putErrorHandler(eh);

		// construct the document
		MSXML2::IXMLDOMDocument2Ptr ndoc;
		CheckError(CreateDOMImp(_Settings.m_keep_encoding ? m_encoding : _Settings.GetDefaultEncoding(), &ndoc));

		// reparse the document
		IStreamPtr isp(ndoc);
		HRESULT hr = rdr->raw_parse(_variant_t((IUnknown *)isp));
		bool bErrSave = false;
		if (FAILED(hr))
		{
			if (!eh->m_msg.IsEmpty())
			{
				// record error position
				if (errline)
				{
					*errline = eh->m_line;
				}
				if (errcol)
				{
					*errcol = eh->m_col;
				}
				if (fValidateOnly)
				{
					::MessageBeep(MB_ICONERROR);
				}
				else
				{
					CString strMessage;
					strMessage.Format(IDS_VALIDATION_FAIL_MSG, (LPCTSTR)eh->m_msg);

					CTaskDialog dlg;
					dlg.SetWindowTitle(IDS_VALIDATION_FAIL_CPT);
					dlg.SetMainInstructionText(strMessage);
					dlg.SetMainIcon(TD_ERROR_ICON);
					dlg.SetCommonButtons(TDCBF_YES_BUTTON | TDCBF_NO_BUTTON);
					dlg.SetDefaultButton(IDNO);
					int nButton;
					dlg.DoModal(::GetActiveWindow(), &nButton);
					if (IDYES == nButton)
					{
						bErrSave = true;
						goto forcesave;
					}
				}
				::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, (LPARAM)(const TCHAR *)eh->m_msg);
			}
			else
			{
				U::ReportError(hr);
			}
			return false;
		}

		if (fValidateOnly)
		{
			WCHAR buf[MAX_LOAD_STRING + 1];
			::LoadString(_Module.GetResourceInstance(), IDS_SB_NO_ERR, buf, MAX_LOAD_STRING);
			::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, (LPARAM)buf);
			::MessageBeep(MB_OK);
			return true;
		}

	forcesave:
		// now save it
		// create tmp filename
		CString path(filename);
		int cp = path.ReverseFind(_T('\\'));
		if (cp < 0)
		{
			path = _T(".\\");
		}
		else
		{
			path.Delete(cp, path.GetLength() - cp);
		}
		CString buf;

		LPWSTR bp = buf.GetBuffer(MAX_PATH);
		UINT uv = ::GetTempFileName(path, _T("fbe"), 0, bp);
		if (uv == 0)
		{
			throw _com_error(HRESULT_FROM_WIN32(::GetLastError()));
		}
		buf.ReleaseBuffer();

		// added by SeNS: replace all nbsp - non-breaking spaces
		if (_Settings.GetNBSPChar().Compare(L"\u00A0") != 0)
		{
			MSXML2::IXMLDOMNodePtr node = ndoc->firstChild;
			CString nbsp = _Settings.GetNBSPChar();
			while (node && node != ndoc)
			{
				if (node->nodeType == 3)
				{
					CString s = node->nodeValue;
					int n = s.Replace(_Settings.GetNBSPChar(), L"\u00A0");
					int k = s.Replace(L"<p>\u00A0</p>", L"<empty-line/>");
					if (n || k)
					{
						node->nodeValue = s.AllocSysString();
					}
				}
				if (node->firstChild)
				{
					node = node->firstChild;
				}
				else
				{
					while (node && node != ndoc && node->nextSibling == nullptr)
						node = node->parentNode;
					if (node && node != ndoc)
						node = node->nextSibling;
				}
			}
		}

		// try to save file
		hr = ndoc->raw_save(_variant_t((LPCWSTR)buf));
		if (FAILED(hr))
		{
			::DeleteFile(buf);
			_com_issue_errorex(hr, ndoc, __uuidof(ndoc));
		}
		// Modification by Pilgrim
		else
		{
			if (bErrSave)
			{
				CString mes("Document saved with errors: ");
				mes += (LPCWSTR)eh->m_msg;
				::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, (LPARAM)(LPCWSTR)mes);
				::MessageBeep(MB_OK);
			}
			else
			{
				CString strStatusText;
				strStatusText.LoadString(IDS_SB_SAVED_NO_ERR);
				::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(buf)));
				::MessageBeep(MB_OK);
			}
		}

		// rename tmp file to original filename
		::DeleteFile(filename);
		::MoveFile(buf, filename);
		m_encoding = _Settings.m_keep_encoding ? m_encoding : _Settings.GetDefaultEncoding();
	}
	catch (_com_error & e)
	{
		U::ReportError(e);
		return false;
	}

	return true;
}

bool Doc::Save()
{
	if (!m_namevalid)
	{
		return false;
	}

	AU::CPersistentWaitCursor wc;
	if (SaveToFile(m_filename))
	{
		MarkSavePoint();
		return true;
	}
	return false;
}

// changes

bool Doc::DocChanged()
{
	return m_body_ver != m_body.GetVersionNumber() || m_body.IsFormChanged();
}

// added by SeNS

void Doc::AdvanceDocVersion(int delta)
{
	m_body_ver += delta;
}

void Doc::MarkSavePoint()
{
	m_body_ver = m_body.GetVersionNumber();
	m_body.ResetFormChanged();
}

void Doc::ResetSavePoint()
{
	m_body_ver = -1;
}

void Doc::MarkDocCP()
{
	m_body_cp = m_body.GetVersionNumber();
	m_body.ResetFormCP();
}

bool Doc::DocRelChanged()
{
	return m_body_cp != m_body.GetVersionNumber() || m_body.IsFormCP();
}

bool Doc::Save(const CString & filename)
{
	AU::CPersistentWaitCursor wc;
	if (SaveToFile(filename))
	{
		MarkSavePoint();
		m_filename = filename;
		WCHAR str[MAX_PATH];
		wcscpy(str, (LPCWSTR)filename);
		PathRemoveFileSpec(str);
		SetCurrentDirectory(str);
		m_namevalid = true;
		return true;
	}
	return false;
}

bool Doc::Validate(int & errline, int & errcol)
{
	AU::CPersistentWaitCursor wc;
	return SaveToFile(CString(), true, &errline, &errcol);
}

// IDs
static LPCWSTR AddHash(CString & tmp, const _bstr_t & id)
{
	LPWSTR cp = tmp.GetBuffer(id.length() + 1);
	*cp++ = L'#';
	memcpy(cp, (LPCWSTR)id, id.length() * sizeof(wchar_t));
	tmp.ReleaseBuffer(id.length() + 1);
	return tmp;
}

static void GrabIDs(CString & tmp, CComboBox & box, MSHTML::IHTMLDOMNode * node)
{
	if (node->nodeType != 1)
		return;

	_bstr_t name(node->nodeName);
	if (U::scmp(name, L"P") && U::scmp(name, L"DIV") && U::scmp(name, L"BODY"))
		return;

	MSHTML::IHTMLElementPtr elem(node);
	_bstr_t id(elem->id);
	if (id.length() > 0)
		box.AddString(AddHash(tmp, id));

	MSHTML::IHTMLDOMNodePtr cn(node->firstChild);
	while ((bool)cn)
	{
		GrabIDs(tmp, box, cn);
		cn = cn->nextSibling;
	}
}

void Doc::ParaIDsToComboBox(CComboBox & box)
{
	try
	{
		CString tmp;
		MSHTML::IHTMLDOMNodePtr body(m_body.Document()->body);
		GrabIDs(tmp, box, body);
	}
	catch (_com_error &)
	{
	}
}

void Doc::BinIDsToComboBox(CComboBox & box)
{
	try
	{
		IDispatchPtr bo(m_body.Document()->all->item(L"id"));
		if (!(bool)bo)
			return;
		CString tmp;
		MSHTML::IHTMLElementCollectionPtr sbo(bo);
		if ((bool)sbo)
		{
			long l = sbo->length;
			for (long i = 0; i < l; ++i)
			{
				MSHTML::IHTMLElementPtr elem = sbo->item(i);
				CString value = elem->getAttribute(L"value", 0);
				if (!value.IsEmpty())
				{
					box.AddString(AddHash(tmp, value.AllocSysString()));
				}
			}
		}
		else
		{
			MSHTML::IHTMLInputTextElementPtr ebo(bo);
			if ((bool)ebo)
			{
				box.AddString(AddHash(tmp, ebo->value));
			}
		}
	}
	catch (_com_error &)
	{
	}
}

BSTR Doc::PrepareDefaultId(const CString & filename)
{

	CString _filename = U::Transliterate(filename);
	// prepare a default id
	int cp = _filename.ReverseFind(_T('\\'));
	if (cp < 0)
		cp = 0;
	else
		++cp;
	CString newid;
	LPWSTR ncp = newid.GetBuffer(_filename.GetLength() - cp);
	int newlen = 0;
	while (cp < _filename.GetLength())
	{
		TCHAR c = _filename[cp];
		if ((c >= _T('0') && c <= _T('9')) || (c >= _T('A') && c <= _T('Z')) || (c >= _T('a') && c <= _T('z')) || c == _T('_') || c == _T('.'))
			ncp[newlen++] = c;
		++cp;
	}
	newid.ReleaseBuffer(newlen);
	if (!newid.IsEmpty() && !((newid[0] >= _T('A') && newid[0] <= _T('Z')) || (newid[0] >= _T('a') && newid[0] <= _T('z')) || newid[0] == _T('_')))
		newid.Insert(0, _T('_'));
	return newid.AllocSysString();
}

// binaries
void Doc::AddBinary(const CString & filename)
{
	_variant_t args[4];
	HRESULT hr;

	V_BSTR(&args[3]) = filename.AllocSysString();
	V_VT(&args[3]) = VT_BSTR;

	if (FAILED(hr = U::LoadFile(filename, &args[0])))
	{
		U::ReportError(hr);
		return;
	}

	V_BSTR(&args[2]) = PrepareDefaultId(filename);
	V_VT(&args[2]) = VT_BSTR;

	// Try to find out mime type
	V_BSTR(&args[1]) = U::GetMimeType(filename).AllocSysString();
	V_VT(&args[1]) = VT_BSTR;

	// Stuff the thing into JavaScript
	CComDispatchDriver body(m_body.Script());
	hr = body.InvokeN(L"apiAddBinary", args, 4);
	if (FAILED(hr))
		U::ReportError(hr);

	hr = body.Invoke0(L"FillCoverList");
	if (FAILED(hr))
		U::ReportError(hr);
}

void Doc::ApplyConfChanges()
{
	try
	{
		MSHTML::IHTMLStylePtr hs(m_body.Document()->body->style);

		CString fss(_Settings.GetFont());
		if (!fss.IsEmpty())
			hs->fontFamily = (LPCWSTR)fss;

		DWORD fs = _Settings.GetFontSize();
		if (fs > 1)
		{
			fss.Format(_T("%dpt"), fs);
			hs->fontSize = (const wchar_t *)fss;
		}

		fs = _Settings.GetColorFG();
		if (fs == CLR_DEFAULT)
			fs = ::GetSysColor(COLOR_WINDOWTEXT);
		fss.Format(_T("rgb(%d,%d,%d)"), GetRValue(fs), GetGValue(fs), GetBValue(fs));
		hs->color = (LPCWSTR)fss;

		fs = _Settings.GetColorBG();
		if (fs == CLR_DEFAULT)
			fs = ::GetSysColor(COLOR_WINDOW);
		fss.Format(_T("rgb(%d,%d,%d)"), GetRValue(fs), GetGValue(fs), GetBValue(fs));
		hs->backgroundColor = (const wchar_t *)fss;

		bool mode = _Settings.m_fast_mode;
		SetFastMode(mode);
		::SendMessage(m_frame, WM_COMMAND, MAKELONG(mode, IDN_FAST_MODE_CHANGE), (LPARAM)0);
	}
	catch (_com_error &)
	{
	}
}

static int compare_nocase(const void * v1, const void * v2)
{
	CString * s1 = (CString *)v1;
	CString * s2 = (CString *)v2;

	int cv = s1->CompareNoCase(*s2);
	if (cv != 0)
		return cv;

	return s1->Compare(*s2);
}

static int compare_counts(const void * v1, const void * v2)
{
	const Doc::Word * w1 = (const Doc::Word *)v1;
	const Doc::Word * w2 = (const Doc::Word *)v2;
	int diff = w1->count - w2->count;
	return diff ? diff : w1->word.CompareNoCase(w2->word);
}

void Doc::GetWordList(int flags, CSimpleArray<Word> & words, CString tagName)
{
	CWaitCursor hourglass;

	MSHTML::IHTMLElementPtr fbw_body = MSHTML::IHTMLDocument3Ptr(m_body.Document())->getElementById(L"fbw_body");
	MSHTML::IHTMLElementCollectionPtr paras = MSHTML::IHTMLElement2Ptr(fbw_body)->getElementsByTagName(L"P");
	if (!paras->length)
		return;

	int iNextElem = 0;

	// Construct a word list
	CSimpleArray<CString> wl;

	while (iNextElem < paras->length)
	{
		MSHTML::IHTMLElementPtr currElem(paras->item(iNextElem));
		CString innerText = currElem->innerText;

		MSHTML::IHTMLDOMNodePtr currNode(currElem);
		if (MSHTML::IHTMLElementPtr siblElem = currNode->nextSibling)
		{
			int jNextElem = iNextElem + 1;
			for (int i = jNextElem; i < paras->length; ++i)
			{
				MSHTML::IHTMLElementPtr nextElem = paras->item(i);
				if (siblElem == nextElem)
				{
					innerText += CString(L"\r\n") + siblElem->innerText.GetBSTR();
					iNextElem++;
					siblElem = MSHTML::IHTMLDOMNodePtr(nextElem)->nextSibling;
				}
				else
				{
					break;
				}
			}
		}

		_bstr_t bb(innerText.AllocSysString());

		if (bb.length() == 0)
		{
			iNextElem++;
			continue;
		}

		// iterate over bb using a primitive fsm
		wchar_t *p = bb, *e = p + bb.length() + 1; // include trailing 0!
		wchar_t * wstart = nullptr;
		wchar_t * wend = nullptr;

		enum
		{
			INITIAL,
			INWORD1,
			INWORD2,
			HYPH1,
			HYPH2
		} state = INITIAL;

		while (p < e)
		{
			int letter = iswalpha(*p);
			switch (state)
			{
			case INITIAL:
			initial:
				if (letter)
				{
					wstart = p;
					state = INWORD1;
				}
				break;
			case INWORD1:
				if (!letter)
				{
					if (flags & GW_INCLUDE_HYPHENS)
					{
						if (iswspace(*p))
						{
							wend = p;
							state = HYPH1;
							break;
						}
						else if (*p == L'-')
						{
							wend = p;
							state = HYPH2;
							break;
						}
					}
					if (!(flags & GW_HYPHENS_ONLY))
					{
						*p = L'\0';
						wl.Add(wstart);
					}
					state = INITIAL;
				}
				break;
			case INWORD2:
				if (!letter)
				{
					*p = L'\0';
					U::RemoveSpaces(wstart);
					wl.Add(wstart);
					state = INITIAL;
				}
				break;
			case HYPH1:
				if (*p == L'-')
					state = HYPH2;
				else if (!iswspace(*p))
				{
					if (!(flags & GW_HYPHENS_ONLY))
					{
						*wend = L'\0';
						wl.Add(wstart);
					}
					state = INITIAL;
					goto initial;
				}
				break;
			case HYPH2:
				if (letter)
				{
					state = INWORD2;
				}
				else if (!iswspace(*p))
				{
					if (!(flags & GW_HYPHENS_ONLY))
					{
						*wend = L'\0';
						wl.Add(wstart);
					}
					state = INITIAL;
					goto initial;
				}
				break;
			}
			++p;
		}

		iNextElem++;
	}

	if (wl.GetSize() == 0)
		return;

	// now sort the list
	qsort(wl.GetData(), wl.GetSize(), sizeof(CString), compare_nocase);

	int wlSize = wl.GetSize();
	for (int i = 0; i < wlSize; ++i)
	{
		int count = 1, k = 0;
		for (int j = i + 1; j < wlSize; ++j)
		{
			if (wl[i] == wl[j])
			{
				count++;
			}
			else
			{
				k = --j;
				break;
			}

			k = j;
		}
		words.Add(Word(wl[i], count));
		if (k)
		{
			i = k;
		}
	}

	// Sort by count now
	if (flags & GW_SORT_BY_COUNT)
	{
		qsort(words.GetData(), words.GetSize(), sizeof(Word), compare_counts);
	}
}

// source editing
bool Doc::SetXMLAndValidate(HWND sci, bool fValidateOnly, int & errline, int & errcol)
{
	errline = errcol = 0;

	// validate it first
	try
	{
		// create a schema collection
		MSXML2::IXMLDOMSchemaCollection2Ptr scol;
		CheckError(scol.CreateInstance(L"Msxml2.XMLSchemaCache.6.0"));

		// load fictionbook schema
		scol->add(FBNS, (LPCWSTR)U::GetProgDirFile(L"FictionBook.xsd"));

		// create a SAX reader
		MSXML2::ISAXXMLReaderPtr rdr;
		CheckError(rdr.CreateInstance(L"Msxml2.SAXXMLReader.6.0"));

		// attach a schema
		rdr->putFeature(L"schema-validation", VARIANT_TRUE);
		rdr->putProperty(L"schemas", scol.GetInterfacePtr());
		rdr->putFeature(L"exhaustive-errors", VARIANT_TRUE);

		// create an error handler
		CComObject<SAXErrorHandler> * ehp;
		CheckError(CComObject<SAXErrorHandler>::CreateInstance(&ehp));
		CComPtr<CComObject<SAXErrorHandler>> eh(ehp);
		rdr->putErrorHandler(eh);

		// construct a document
		MSXML2::IXMLDOMDocument2Ptr dom;

		if (!fValidateOnly)
		{
			CheckError(U::CreateDocument(true, &dom));

			// construct an xml writer
			MSXML2::IMXWriterPtr wrt;
			CheckError(wrt.CreateInstance(L"Msxml2.MXXMLWriter.6.0"));

			// connect document to the writer
			wrt->output = dom.GetInterfacePtr();

			// connect the writer to the reader
			rdr->putContentHandler(MSXML2::ISAXContentHandlerPtr(wrt));
		}

		// now parse it!
		// oh well, let's waste more memory
		CComBSTR ustr;
		int textlen = ::SendMessage(sci, SCI_GETLENGTH, 0, 0);
		try
		{
			CStringA buffer;

			::SendMessage(sci, SCI_GETTEXT, textlen + 1, reinterpret_cast<LPARAM>(buffer.GetBuffer(textlen + 1)));
			buffer.ReleaseBuffer();
			ustr = CA2W(buffer, CP_UTF8);
		}
		catch (...)
		{
			AtlTaskDialog(::GetActiveWindow(), IDR_MAINFRAME, IDS_OUT_OF_MEM_MSG, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
			return false;
		}

		CComVariant vt(ustr);
		HRESULT hr = rdr->raw_parse(vt);
		vt.Clear();

		if (FAILED(hr))
		{
			if (!eh->m_msg.IsEmpty())
			{
				// record error position
				errline = eh->m_line;
				errcol = eh->m_col;
				::MessageBeep(MB_ICONERROR);
				::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0,
				              (LPARAM)(const TCHAR *)eh->m_msg);
			}
			else
			{
				U::ReportError(hr);
			}
			return false;
		}

		if (fValidateOnly)
		{
			CString buf;
			buf.LoadString(IDS_SB_NO_ERR);
			::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(buf)));
			::MessageBeep(MB_OK);
			return true;
		}

		// ok, it seems valid, put it int6o document then
		dom->setProperty(L"SelectionLanguage", L"XPath");
		CString nsprop(L"xmlns:fb='");
		nsprop += (const wchar_t *)FBNS;
		nsprop += L"' xmlns:xlink='";
		nsprop += (const wchar_t *)XLINKNS;
		nsprop += L"'";
		dom->setProperty(L"SelectionNamespaces", (const TCHAR *)nsprop);

		// transform to html
		CComDispatchDriver body(m_body.Script());
		CComVariant args[2];
		args[1] = dom.GetInterfacePtr();
		args[0] = _Settings.GetInterfaceLanguageName();
		CheckError(body.InvokeN(L"LoadFromDOM", args, 2));
		m_body.Init();

		// mark unchanged
		MarkSavePoint();
	}
	catch (_com_error & e)
	{
		U::ReportError(e);
		return false;
	}

	return true;
}

void Doc::SaveSelectedPos()
{
	MSHTML::IHTMLElementPtr selected = m_body.SelectionStructCon();

	//  UUID
	UUID uuid;
	wchar_t * str;
	if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringW(&uuid, &str) == RPC_S_OK)
	{
		m_save_marker = str;
	}
	else
	{
		return;
	}
	selected->setAttribute(L"fbe_selected", m_save_marker, 0);
	m_saved_element = selected;
}

long Doc::GetSavedPos(bstr_t & xml, bool deleteMarker)
{
	bstr_t searchString = L" selected=\"" + m_save_marker + L"\"";
	const wchar_t * wpos = wcsstr(xml.operator const wchar_t *(), searchString);
	if (!wpos)
	{
		return 0;
	}
	int pos = wpos - (wchar_t *)xml;

	if (deleteMarker)
	{
		wchar_t * Buf = new wchar_t[xml.length() + 1];
		wcsncpy(Buf, xml, pos);
		wcscpy(Buf + pos, (wchar_t *)xml + pos + searchString.length());
		Buf[xml.length() - searchString.length()] = 0;
		xml = Buf;
		delete[] Buf;
	}
	return pos;
}

void Doc::DeleteSaveMarker()
{
	m_saved_element->removeAttribute(L"fbe_selected", 1);
}

bool Doc::TextToXML(BSTR text, MSXML2::IXMLDOMDocument2 ** ppXml)
{
	if (!ppXml)
		return false;

	*ppXml = nullptr;

	MSXML2::IXMLDOMSchemaCollection2Ptr scol;
	CheckError(scol.CreateInstance(L"Msxml2.XMLSchemaCache.6.0"));

	// load fictionbook schema
	scol->add(FBNS, (const wchar_t *)U::GetProgDirFile(L"FictionBook.xsd"));

	// create a SAX reader
	MSXML2::ISAXXMLReaderPtr rdr;
	CheckError(rdr.CreateInstance(L"Msxml2.SAXXMLReader.6.0"));

	// attach a schema
	rdr->putFeature(L"schema-validation", VARIANT_TRUE);
	rdr->putProperty(L"schemas", scol.GetInterfacePtr());
	rdr->putFeature(L"exhaustive-errors", VARIANT_TRUE);

	// create an error handler
	CComObject<SAXErrorHandler> * ehp;
	CheckError(CComObject<SAXErrorHandler>::CreateInstance(&ehp));
	CComPtr<CComObject<SAXErrorHandler>> eh(ehp);
	rdr->putErrorHandler(eh);

	MSXML2::IXMLDOMDocument2Ptr xml;
	CheckError(U::CreateDocument(true, &xml));

	// construct an xml writer
	MSXML2::IMXWriterPtr wrt;
	CheckError(wrt.CreateInstance(L"Msxml2.MXXMLWriter.6.0"));

	// connect document to the writer
	wrt->output = _variant_t(xml.GetInterfacePtr());

	// connect the writer to the reader
	rdr->putContentHandler(MSXML2::ISAXContentHandlerPtr(wrt));

	// now parse it!
	// oh well, let's waste more memory

	VARIANT vt;
	V_VT(&vt) = VT_BSTR;
	V_BSTR(&vt) = text;
	HRESULT hr = rdr->raw_parse(vt);

	bstr_t msg = eh->m_msg;
	if (FAILED(hr))
	{
		if (!eh->m_msg.IsEmpty())
		{
			// record error position
			::MessageBeep(MB_ICONERROR);
			::SendMessage(m_frame, AU::WM_SETSTATUSTEXT, 0, (LPARAM)(const TCHAR *)eh->m_msg);
		}
		else
		{
			U::ReportError(hr);
		}
		return false;
	}

	// ok, it seems valid, put it into document then
	xml->setProperty(L"SelectionLanguage", L"XPath");
	CString nsprop(L"xmlns:fb='");
	nsprop += (const wchar_t *)FBNS;
	nsprop += L"' xmlns:xlink='";
	nsprop += (const wchar_t *)XLINKNS;
	nsprop += L"'";
	xml->setProperty(L"SelectionNamespaces", (const TCHAR *)nsprop);

	CheckError(xml.QueryInterface(IID_PPV_ARGS(ppXml)));

	return true;
}

MSHTML::IHTMLDOMNodePtr Doc::MoveNode(MSHTML::IHTMLDOMNodePtr from, MSHTML::IHTMLDOMNodePtr to, MSHTML::IHTMLDOMNodePtr insertBefore)
{
	VARIANT disp;
	MSHTML::IHTMLElementPtr to_elem = (MSHTML::IHTMLElementPtr)to;
	bstr_t text = to_elem->innerHTML;

	//  title
	if ((bool)insertBefore)
	{
		while (1)
		{
			MSHTML::IHTMLElementPtr before_elem = (MSHTML::IHTMLElementPtr)insertBefore;
			_bstr_t class_name(before_elem->className);
			if ((0 == U::scmp(class_name, L"title")) ||
			    (0 == U::scmp(class_name, L"epigraph")) ||
			    (0 == U::scmp(class_name, L"annotation")) ||
			    (0 == U::scmp(class_name, L"image")))
			{
				insertBefore = insertBefore->nextSibling;
				continue;
			}
			break;
		}
	}

	MSHTML::IHTMLDOMNodePtr ret;
	if ((bool)insertBefore)
	{
		disp.pdispVal = insertBefore;
		disp.vt = VT_DISPATCH;
		ret = to->insertBefore(from, disp);
	}
	else
	{
		ret = to->appendChild(from);
	}

	return ret;
}

void Doc::FastMode()
{
	CComDispatchDriver body(m_body.Script());
	CComVariant args[1];
	args[0] = m_fast_mode;
	CheckError(body.Invoke1(L"apiSetFastMode", &args[0]));
	return;
}

void Doc::SetFastMode(bool fast)
{
	m_fast_mode = fast;
	if (m_body != 0)
		FastMode();
}

bool Doc::GetFastMode() const
{
	return m_fast_mode;
}

// TODO (by SeNS): should be fixed!
int Doc::GetSelectedPos()
{
	int len = 0;
	const long delta = -100000;
	CComPtr<IDispatch> spDisp;
	HRESULT hr = m_body.Document()->selection->raw_createRange(&spDisp);
	if (SUCCEEDED(hr))
	{
		CComPtr<MSHTML::IHTMLTxtRange> spRange;
		hr = spDisp.QueryInterface(&spRange);
		if (SUCCEEDED(hr))
		{
			while (1)
			{
				long moved = 0;
				hr = spRange->raw_move(CComBSTR(L"character"), delta, &moved);
				if (SUCCEEDED(hr))
				{
					len -= moved;
					if (moved != delta)
					{
						return len - 21;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return len;
}

CString Doc::GetOpenFileName() const
{
	if (m_filename == L"Untitled.fb2")
		return L"";
	else
		return m_filename;
}

} // namespace FB
