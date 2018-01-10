#include "res1.h"
#include "resource.h"
#include "stdafx.h"

#include "apputils.h"
#include "utils.h"

#include "ExternalHelper.h"
#include "FBE.h"

#define MENU_BASE 5000

struct Genre
{
	int groupid;
	CString id;
	CString text;
};

static CSimpleArray<CString> g_genre_groups;
static CSimpleArray<Genre> g_genres;

struct DescElement
{
	int groupid;
	CString text;
};

static CSimpleMap<CString, DescElement> g_desc_elements;

struct Lang
{
	CString id;
	CString text;
};

static CSimpleArray<CString> g_lang_groups;
static CSimpleArray<Lang> g_langs;

static void FillDescElements()
{
	g_desc_elements.RemoveAll();
	DescElement elem;
	elem.groupid = 1;
	wchar_t buf[MAX_LOAD_STRING + 1];
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_TI, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"ti_group", elem);
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_GENRE_M, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"ti_genre_match", elem);
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_KW, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"ti_kw", elem);
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_AUTHOR, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"ti_nic_mail_web", elem);
	elem.groupid = 2;
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_DI, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"di_group", elem);
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_ID, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"di_id", elem);
	elem.groupid = 0;
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_STI, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"sti_all", elem);
	if (::LoadString(_Module.GetResourceInstance(), IDS_DMS_CI, buf, MAX_LOAD_STRING))
		elem.text = buf;
	g_desc_elements.Add(L"ci_all", elem);
}

// genre list helper
static void LoadGenres()
{
	FILE * fp = nullptr;
	CString file_name = _Settings.GetLocalizedGenresFileName();
	// Modification by Pilgrim
	try
	{
		fp = _tfopen(U::GetProgDirFile(file_name), _T("rb"));
	}
	catch (...)
	{
	}

	if (!fp)
	{
		CString strMessage;
		strMessage.Format(IDS_GENRES_LIST_MSG, (LPCTSTR)file_name);
		AtlTaskDialog(::GetActiveWindow(), IDR_MAINFRAME, (LPCTSTR)strMessage, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		return;
	}

	g_genre_groups.RemoveAll();
	g_genres.RemoveAll();

	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), fp))
	{
		int l = strlen(buffer);
		if (l > 0 && buffer[l - 1] == '\n')
			buffer[--l] = '\0';
		if (l > 0 && buffer[l - 1] == '\r')
			buffer[--l] = '\0';

		if (buffer[0] && buffer[0] != ' ')
		{
			CA2W tmp(buffer, 65001);
			CString name(tmp);
			name.Replace(_T("&"), _T("&&"));
			g_genre_groups.Add(name);
		}
		else
		{
			char * p = strchr(buffer + 1, ' ');
			if (!p || p == buffer + 1)
				continue;
			*p++ = '\0';
			Genre g;
			g.groupid = g_genre_groups.GetSize() - 1;
			g.id = buffer + 1;
			CA2W tmp(p, 65001);
			g.text.SetString(tmp);
			g.text.Replace(_T("&"), _T("&&"));
			g_genres.Add(g);
		}
	}
	fclose(fp);
}

static CMenu MakeGenresMenu()
{
	CMenu ret;
	ret.CreatePopupMenu();

	CMenu cur;
	int g = -1;
	for (int i = 0; i < g_genres.GetSize(); ++i)
	{
		if (g_genres[i].groupid != g)
		{
			g = g_genres[i].groupid;
			cur.Detach();
			cur.CreatePopupMenu();
			ret.AppendMenu(MF_POPUP | MF_STRING, (UINT_PTR)(HMENU)cur, g_genre_groups[g]);
		}
		cur.AppendMenu(MF_STRING, i + MENU_BASE, g_genres[i].text);
	}
	cur.Detach();

	return ret.Detach();
}

static CMenu MakeDescComponentsMenu()
{
	CMenu ret;
	ret.CreatePopupMenu();

	CMenu cur;
	int g = -1;
	for (int i = 0; i < g_desc_elements.GetSize(); ++i)
	{
		if (g_desc_elements.GetValueAt(i).groupid == 0)
		{
			ret.AppendMenu(MF_STRING, i + MENU_BASE, g_desc_elements.GetValueAt(i).text);
			bool ext = _Settings.GetExtElementStyle(g_desc_elements.GetKeyAt(i));
			if (ext)
			{
				ret.CheckMenuItem(i + MENU_BASE, MF_CHECKED);
			}
			else
			{
				ret.CheckMenuItem(i + MENU_BASE, MF_UNCHECKED);
			}
			continue;
		}

		if (g_desc_elements.GetValueAt(i).groupid != g)
		{
			g = g_desc_elements.GetValueAt(i).groupid;
			cur.Detach();
			cur.CreatePopupMenu();
			ret.AppendMenu(MF_POPUP | MF_STRING, (UINT)(HMENU)cur, g_desc_elements.GetValueAt(i).text);
			continue;
		}
		cur.AppendMenu(MF_STRING, i + MENU_BASE, g_desc_elements.GetValueAt(i).text);
		bool ext = _Settings.GetExtElementStyle(g_desc_elements.GetKeyAt(i));
		if (ext)
		{
			cur.CheckMenuItem(i + MENU_BASE, MF_CHECKED);
		}
		else
		{
			cur.CheckMenuItem(i + MENU_BASE, MF_UNCHECKED);
		}
	}
	cur.Detach();

	return ret.Detach();
}

// IExternalHelper

STDMETHODIMP ExternalHelper::BeginUndoUnit(IDispatch * obj, BSTR name)
{
	MSHTML::IMarkupServices * srv;
	HRESULT hr = obj->QueryInterface(&srv);
	if (FAILED(hr))
		return hr;
	hr = srv->raw_BeginUndoUnit(name);
	srv->Release();
	return hr;
}

STDMETHODIMP ExternalHelper::EndUndoUnit(IDispatch * obj)
{
	MSHTML::IMarkupServices * srv;
	HRESULT hr = obj->QueryInterface(&srv);
	if (FAILED(hr))
		return hr;
	hr = srv->raw_EndUndoUnit();
	srv->Release();
	return hr;
}

STDMETHODIMP ExternalHelper::get_inflateBlock(IDispatch * obj, BOOL * ifb)
{
	MSHTML::IHTMLElement3 * elem;
	HRESULT hr = obj->QueryInterface(&elem);
	if (FAILED(hr))
		return hr;
	VARIANT_BOOL vb;
	hr = elem->get_inflateBlock(&vb);
	*ifb = SUCCEEDED(hr) && vb == VARIANT_TRUE ? TRUE : FALSE;
	elem->Release();
	return hr;
}

STDMETHODIMP ExternalHelper::put_inflateBlock(IDispatch * obj, BOOL ifb)
{
	MSHTML::IHTMLElement3 * elem;
	HRESULT hr = obj->QueryInterface(&elem);
	if (FAILED(hr))
		return hr;
	hr = elem->put_inflateBlock(ifb ? VARIANT_TRUE : VARIANT_FALSE);
	elem->Release();
	return hr;
}

HRESULT ExternalHelper::GenrePopup(IDispatch * /*obj*/, LONG x, LONG y, BSTR * name)
{
	LoadGenres();
	CMenu popup = MakeGenresMenu();
	if (popup)
	{
		UINT cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, x, y, ::GetActiveWindow());
		popup.DestroyMenu();
		cmd -= MENU_BASE;
		if (cmd < (UINT)g_genres.GetSize())
		{
			*name = g_genres[cmd].id.AllocSysString();
			return S_OK;
		}
	}
	*name = NULL;
	return S_OK;
}

// Modification by Pilgrim

// lang list helper
/*static void	    LoadLangs() {
	FILE	  *fp;
	try{
	  fp=_tfopen(U::GetProgDirFile(_T("languages.txt")),_T("rb"));
    }catch(...){
	}

	if(!fp){
		U::MessageBox(MB_OK|MB_ICONERROR,_T("FBE"),
			  _T("Не могу найти файл-список языков '%s'."),_T("languages.txt"));
		return;
	}

	g_lang_groups.RemoveAll();
	g_langs.RemoveAll();

	char	  buffer[1024];
	while (fgets(buffer,sizeof(buffer),fp)) {
		int	  l=strlen(buffer);
		if (l>0 && buffer[l-1]=='\n')
			buffer[--l]='\0';
		if (l>0 && buffer[l-1]=='\r')
			buffer[--l]='\0';

		char  *p=strchr(buffer+1,'|');
		if (!p || p==buffer+1)
			continue;
		*p++='\0';
		Lang   g;
		g.text=buffer;
		g.id=p;
		g.id.Replace(_T("&"),_T("&&"));
		g_langs.Add(g);
	}
	fclose(fp);
}*/

static CMenu MakeLangsMenu()
{
	CMenu cur;
	cur.CreatePopupMenu();

	for (int i = 0; i < g_langs.GetSize(); ++i)
	{
		cur.AppendMenu(MF_STRING, i + MENU_BASE, g_langs[i].text);
	}

	return cur.Detach();
}

static CMenu MakeExtendMenu()
{
	CMenu cur;
	cur.CreatePopupMenu();

	for (int i = 0; i < g_langs.GetSize(); ++i)
	{
		cur.AppendMenu(MF_STRING, i + MENU_BASE, g_langs[i].text);
	}

	return cur.Detach();
}

/*HRESULT	ExternalHelper::LangPopup(IDispatch *obj,LONG x,LONG y,BSTR *name) {
	LoadLangs();
	CMenu	  popup=MakeLangsMenu();
	if (popup) {
		UINT  cmd=popup.TrackPopupMenu(
			TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,
			x,y,::GetActiveWindow()
			);
		popup.DestroyMenu();
		cmd-=MENU_BASE;
		if (cmd<(UINT)g_langs.GetSize()) {
			*name=g_langs[cmd].id.AllocSysString();
			return S_OK;
		}
	}
	*name=NULL;
	return S_OK;
}

HRESULT	ExternalHelper::SrcLangPopup(IDispatch *obj,LONG x,LONG y,BSTR *name) {
	LoadLangs();
	CMenu	  popup=MakeLangsMenu();
	if (popup) {
		UINT  cmd=popup.TrackPopupMenu(
			TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,
			x,y,::GetActiveWindow()
			);
		popup.DestroyMenu();
		cmd-=MENU_BASE;
		if (cmd<(UINT)g_langs.GetSize()) {
			*name=g_langs[cmd].id.AllocSysString();
			return S_OK;
		}
	}
	*name=NULL;
	return S_OK;
}

HRESULT	ExternalHelper::STILangPopup(IDispatch *obj,LONG x,LONG y,BSTR *name) {
	LoadLangs();
	CMenu	  popup=MakeLangsMenu();
	if (popup) {
		UINT  cmd=popup.TrackPopupMenu(
			TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,
			x,y,::GetActiveWindow()
			);
		popup.DestroyMenu();
		cmd-=MENU_BASE;
		if (cmd<(UINT)g_langs.GetSize()) {
			*name=g_langs[cmd].id.AllocSysString();
			return S_OK;
		}
	}
	*name=NULL;
	return S_OK;
}

HRESULT	ExternalHelper::STISrcLangPopup(IDispatch *obj,LONG x,LONG y,BSTR *name) {
	LoadLangs();
	CMenu	  popup=MakeLangsMenu();
	if (popup) {
		UINT  cmd=popup.TrackPopupMenu(
			TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,
			x,y,::GetActiveWindow()
			);
		popup.DestroyMenu();
		cmd-=MENU_BASE;
		if (cmd<(UINT)g_langs.GetSize()) {
			*name=g_langs[cmd].id.AllocSysString();
			return S_OK;
		}
	}
	*name=NULL;
	return S_OK;
}*/

HRESULT ExternalHelper::DescShowMenu(IDispatch * /*obj*/, LONG x, LONG y, BSTR * element_id)
{
	FillDescElements();
	CMenu popup = MakeDescComponentsMenu();
	if (popup)
	{
		UINT cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, x, y, ::GetActiveWindow());
		if (!cmd)
		{
			return S_OK;
		}

		popup.DestroyMenu();
		cmd -= MENU_BASE;
		if (cmd < (UINT)g_desc_elements.GetSize())
		{
			DescElement elem = g_desc_elements.GetValueAt(cmd);
			*element_id = g_desc_elements.GetKeyAt(cmd).AllocSysString();
			return S_OK;
		}
	}

	return S_OK;
}

STDMETHODIMP ExternalHelper::GetStylePath(BSTR * name)
{
	wchar_t path[MAX_PATH + 1];
	GetModuleFileName(0, path, MAX_PATH);
	PathRemoveFileSpec(path);
	CString us(path);
	*name = us.AllocSysString();

	return S_OK;
}

STDMETHODIMP ExternalHelper::GetBinarySize(BSTR data, int * length)
{
	*length = SysStringByteLen(data);
	return S_OK;
}

STDMETHODIMP ExternalHelper::InflateParagraphs(IDispatch * elem)
{
	MSHTML::IHTMLElement2Ptr el;
	elem->QueryInterface(IID_IHTMLElement2, (void **)&el);
	MSHTML::IHTMLElementCollectionPtr pp(el->getElementsByTagName(L"P"));
	for (long l = 0; l < pp->length; ++l)
	{
		MSHTML::IHTMLElement3Ptr(pp->item(l))->inflateBlock = VARIANT_TRUE;
	}
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetUUID(BSTR * uid)
{
	UUID uuid;
	unsigned char * str;
	if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringA(&uuid, &str) == RPC_S_OK)
	{
		CString us(str);
		RpcStringFreeA(&str);
		us.MakeUpper();
		*uid = us.AllocSysString();
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP ExternalHelper::GetNBSP(BSTR * nbsp)
{
	CString s_nbsp = _Settings.GetNBSPChar();
	*nbsp = s_nbsp.AllocSysString();
	return S_OK;
}

STDMETHODIMP ExternalHelper::MsgBox(BSTR message)
{
	AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, (LPCTSTR)message, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_INFORMATION_ICON);
	return S_OK;
}

STDMETHODIMP ExternalHelper::AskYesNo(BSTR message, BOOL * pVal)
{
	if (IDYES == AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, (LPCTSTR)message, (LPCTSTR)NULL, TDCBF_YES_BUTTON | TDCBF_YES_BUTTON, TD_WARNING_ICON))
	{
		*pVal = true;
	}
	else
	{
		*pVal = false;
	}
	return S_OK;
}

STDMETHODIMP ExternalHelper::SaveBinary(BSTR path, BSTR data, BOOL prompt, BOOL * ret)
{
	INT_PTR modalResult = IDOK;
	*ret = false;
	CString file_name = CString(path);

	if (prompt)
	{
		CString fname = ATLPath::FindFileName(file_name);
		CString fpath(file_name);
		fpath = fpath.Left(file_name.GetLength() - fname.GetLength());
		CFileDialog imgSaveDlg(FALSE, NULL, fname);
		imgSaveDlg.m_ofn.lpstrInitialDir = fpath;

		// add file types
		imgSaveDlg.m_ofn.lpstrFilter = L"JPEG files (*.jpg)\0*.jpg\0PNG files (*.png)\0*.png\0All files (*.*)\0*.*\0\0";
		imgSaveDlg.m_ofn.nFilterIndex = 0;
		imgSaveDlg.m_ofn.lpstrDefExt = L"jpg";
		imgSaveDlg.m_ofn.Flags &= ~OFN_ENABLEHOOK;
		imgSaveDlg.m_ofn.lpfnHook = NULL;

		modalResult = imgSaveDlg.DoModal(NULL);
		file_name.SetString(imgSaveDlg.m_szFileName);
	}

	if (modalResult == IDOK)
	{
		int len = SysStringByteLen(data);
		void * buf = (void *)data;
		HANDLE file = CreateFile(file_name, GENERIC_WRITE | FILE_WRITE_DATA, FILE_SHARE_WRITE, 0, CREATE_NEW, 0, 0);
		if (INVALID_HANDLE_VALUE == file)
		{
			return S_OK;
		}
		DWORD writen = 0;
		WriteFile(file, buf, len, &writen, 0);
		CloseHandle(file);
		*ret = true;
	}
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetExtendedStyle(BSTR elem, BOOL * ext)
{
	*ext = _Settings.GetExtElementStyle(elem);
	return S_OK;
}

STDMETHODIMP ExternalHelper::IsFastMode(BOOL * ext)
{
	*ext = _Settings.m_fast_mode;
	return S_OK;
}

STDMETHODIMP ExternalHelper::DescShowElement(BSTR elem, BOOL show)
{
	_Settings.SetExtElementStyle(elem, show != 0);
	return S_OK;
}

STDMETHODIMP ExternalHelper::SetStyleEx(IDispatch *, IDispatch * elem, BSTR style)
{
	MSHTML::IHTMLElementPtr el = elem;
	U::ChangeAttribute(el, L"class", style);
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetImageDimsByPath(BSTR path, BSTR * dims)
{
	int nWidth, nHeight;

	if (U::GetImageDimsByPath(path, &nWidth, &nHeight))
	{
		CString format;
		format.Format(L"%dx%d", nWidth, nHeight);
		*dims = format.AllocSysString();
	}
	else
		*dims = L"";

	return S_OK;
}

STDMETHODIMP ExternalHelper::GetImageDimsByData(VARIANT * data, BSTR * dims)
{
	int nWidth, nHeight;

	SAFEARRAY * psa = data->parray;
	long lUbound;

	if (SafeArrayGetUBound(psa, 1, &lUbound) == S_OK && U::GetImageDimsByData(psa, lUbound, &nWidth, &nHeight))
	{
		CString format;
		format.Format(L"%dx%d", nWidth, nHeight);
		*dims = format.AllocSysString();
	}
	else
		*dims = L"";

	return S_OK;
}

STDMETHODIMP ExternalHelper::GetViewWidth(int * width)
{
	*width = _Settings.GetViewWidth();
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetViewHeight(int * height)
{
	*height = _Settings.GetViewHeight();
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetProgramVersion(BSTR * ver)
{
	CString strVersion = U::GetProductInfo();
	*ver = strVersion.AllocSysString();
	return S_OK;
}

STDMETHODIMP ExternalHelper::InputBox(BSTR prompt, BSTR title, BSTR value, BSTR * input)
{
	CString sPrompt(prompt);
	CString sTitle(title);
	CString sInput(value);

	modalResultCode = AU::InputBox(sInput, sTitle, sPrompt);

	if (modalResultCode != IDYES)
		sInput.SetString(L"");
	*input = sInput.AllocSysString();
	return S_OK;
}

STDMETHODIMP ExternalHelper::GetModalResult(int * modalResult)
{
	*modalResult = modalResultCode;
	return S_OK;
}

STDMETHODIMP ExternalHelper::SetStatusBarText(BSTR text)
{
	CString sbtext(text);
	::SendMessage(_Settings.GetMainWindow(), AU::WM_SETSTATUSTEXT, 0, (LPARAM)(LPCTSTR)sbtext.GetBuffer());
	return S_OK;
}
