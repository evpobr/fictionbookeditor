#include "stdafx.h"

#include <exception>
#include <mlang.h>

#include "mainfrm.h"


#define	MAXARGS	32

#include <initguid.h>
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0,
   0xaa, 0x0, 0x4a, 0x55, 0xe8);
DEFINE_GUID(CLSID_JScript, 0xf414c260, 0x6ac0, 0x11cf, 0xb6, 0xd1, 0x00,
   0xaa, 0x00, 0xbb, 0xbb, 0x58);

static void	strlcatW(wchar_t *d, const wchar_t *s, int dl) {
  size_t l;

  if (dl<=0)
    return;

  l = wcslen(d);
  if (l>=dl)
    return;

  --dl;

  while (l < dl && *s)
    d[l++] = *s++;

  d[l] = 0;
}

static inline void  *::operator new(size_t amount) {
  return ::HeapAlloc(::GetProcessHeap(),0,amount);
}

static inline void  ::operator delete(void *p) {
  ::HeapFree(::GetProcessHeap(),0,p);
}

static bool EQ(REFIID i1,REFIID i2) {
  char	*c1 = (char *)&i1;
  char	*c2 = (char *)&i2;
  for (int i=0;i<sizeof(i1);++i)
    if (*c1++ != *c2++)
      return false;
  return true;
}

typedef HRESULT  (*GenFunc)(...);

class ScriptSite : public IActiveScriptSite {
  ULONG	m_refs;
  CMainFrame* m_frame;
public:
	ScriptSite(CMainFrame* mainframe) : m_refs(1), m_frame(mainframe) { }
  ~ScriptSite() { }

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid,void **ppv) {
    if (EQ(riid,IID_IUnknown) || EQ(riid,IID_IActiveScriptSite)) {
      *ppv = this;
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  STDMETHOD_(ULONG,AddRef)() {
    return ++m_refs;
  }

  STDMETHOD_(ULONG,Release)() {
    ULONG refs = --m_refs;
    if (refs == 0)
      delete this;
    return refs;
  }

  // IActiveScriptSite
  STDMETHOD(GetLCID)(LCID *) { return E_NOTIMPL; }
  STDMETHOD(GetItemInfo)(LPCOLESTR name, DWORD mask,IUnknown **ppUnk,ITypeInfo **ppTI) {
    if (ppTI)
      *ppTI = NULL; // we don't support this
    if (ppUnk) {
      *ppUnk = NULL;
      if (mask & SCRIPTINFO_IUNKNOWN) {

	if(lstrcmpW(name,L"document")==0)
	{
		IDispatch *doc;
		((m_frame->ActiveView()).Browser())->get_Document(&doc);
		*ppUnk = doc;//(m_frame->m_doc->m_desc).Document();		
		return S_OK;
	}
	if(lstrcmpW(name,L"window")==0)
	{
		IDispatch *window = ((m_frame->ActiveView()).Document())->parentWindow;		
		//IDispatch *window = ((m_frame->m_doc->m_desc).Document())->parentWindow;		
		*ppUnk = window;
		window->AddRef();
		return S_OK;
	}
      }
    }
    return TYPE_E_ELEMENTNOTFOUND;
  }
  STDMETHOD(GetDocVersionString)(BSTR *) { return E_NOTIMPL; }
  STDMETHOD(OnScriptTerminate)(const VARIANT *,const EXCEPINFO *) { return S_OK; }
  STDMETHOD(OnStateChange)(SCRIPTSTATE) { return S_OK; }
  STDMETHOD(OnScriptError)(IActiveScriptError *err)
  {
	  EXCEPINFO ei;
	  if (SUCCEEDED(err->GetExceptionInfo(&ei)))
	  {
		  DWORD ctx;
		  ULONG line = 0;
		  LONG  column = 0;
		  wchar_t *buf = NULL;

		  err->GetSourcePosition(&ctx, &line, &column);

		  CString strMessage;
		  if (ei.bstrDescription)
		  {
			  strMessage.Format(IDS_SCRIPT_ERRD_MSG, ei.bstrDescription, line + 1, column + 1);
			  AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, (LPCTSTR)strMessage, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		  }
		  else
		  {
			  strMessage.Format(IDS_SCRIPT_ERRX_MSG, ei.scode, line + 1, column + 1);
			  AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, (LPCTSTR)strMessage, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		  }
		  SysFreeString(ei.bstrSource);
		  SysFreeString(ei.bstrDescription);
		  SysFreeString(ei.bstrHelpFile);
		  return S_OK;
	  }
	  else
	  {
		  AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, IDS_SCRIPT_MSG, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		  return S_OK;
	  }
  }
  STDMETHOD(OnEnterScript)() { return S_OK; }
  STDMETHOD(OnLeaveScript)() { return S_OK; }
};

static ScriptSite     *g_site;
static IActiveScript  *g_script;

void	StopScript(void) {
  if (g_script) {
    g_script->Close();
    g_script->Release();
    g_script = NULL;
  }
  if (g_site) {
    g_site->Release();
    g_site = NULL;
  }
}

int	StartScript(CMainFrame* mainframe) {
  HRESULT hr;

  g_site = new ScriptSite(mainframe);
  if (g_site == NULL)
    return -1;

  if (FAILED(CoCreateInstance(CLSID_JScript, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript,(void**)&g_script))) {
    StopScript();
    return -1;
  }

  hr = g_script->SetScriptSite(g_site);
  hr = g_script->AddNamedItem(L"document",SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
  hr = g_script->AddNamedItem(L"window",SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);


  IActiveScriptParse	*pF;
  if (FAILED(g_script->QueryInterface(IID_IActiveScriptParse,(void**)&pF))) {
    StopScript();
    return -1;
  }

  hr = pF->InitNew();
  pF->Release();

  hr = g_script->SetScriptState(SCRIPTSTATE_STARTED);

  return 0;
}

HRESULT	ScriptCall(const wchar_t *func, VARIANT *arg, int argnum, VARIANT *ret) {
  HRESULT hr;

  if (g_script == NULL)
    return E_FAIL;

  IDispatch *pDisp;
  if (FAILED(hr = g_script->GetScriptDispatch(NULL,&pDisp)))
    return hr;

  DISPID    id;
  if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL,(wchar_t **)&func,1,LANG_USER_DEFAULT,&id))) {
    pDisp->Release();
    return hr;
  }

  DISPPARAMS  params;
  params.cNamedArgs = 0;
  params.rgdispidNamedArgs = NULL;
  params.cArgs = 0;
  params.rgvarg = NULL;

  if (arg) {
    params.cArgs = argnum;
    params.rgvarg = arg;
  }

  unsigned  argerr;
  hr = pDisp->Invoke(id,IID_NULL,LANG_USER_DEFAULT,DISPATCH_METHOD,&params, ret, NULL, &argerr);

  pDisp->Release();

  return hr;
}

bool ScriptFindFunc(const wchar_t* func)
{
	HRESULT hr;
	if (g_script == NULL)
    return false;

	IDispatch *pDisp;
	if (FAILED(hr = g_script->GetScriptDispatch(NULL,&pDisp)))
		return false;

	DISPID    id;
	if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL,(wchar_t **)&func,1,LANG_USER_DEFAULT,&id))) {
		pDisp->Release();
		return false;
	}

	return true;
}

static HANDLE TryOpen(bool pfx,const wchar_t *mid,const wchar_t *last) {
  wchar_t   xfilename[MAX_PATH];

  xfilename[0] = 0;

  if (pfx) {
    wchar_t *cp;
    GetModuleFileNameW(NULL,xfilename,sizeof(xfilename)/sizeof(xfilename[0]));
    for (cp = xfilename + lstrlenW(xfilename);cp>xfilename;--cp)
      if (cp[-1] == L'/' || cp[-1] == L'\\')
	break;
    *cp = 0;
  }

  if (mid)
    strlcatW(xfilename,mid,sizeof(xfilename)/sizeof(xfilename[0]));

  int len = lstrlenW(xfilename);
  if (len > 0 && (xfilename[len-1]==_T('/') || xfilename[len-1]==_T('\\')) &&
      (last && (*last==_T('/') || *last==_T('\\'))))
    ++last;

  strlcatW(xfilename,last,sizeof(xfilename)/sizeof(xfilename[0]));

  return CreateFile(xfilename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,0);
}

HRESULT	ScriptLoad(const wchar_t *filename) {
  if (!g_script)
    return E_FAIL;

  HRESULT   hr;

  // open file and try to load it in memory
  HANDLE    hFile;
  if ((hFile = TryOpen(false,NULL,filename)) == INVALID_HANDLE_VALUE &&
      (hFile = TryOpen(true,NULL,filename)) == INVALID_HANDLE_VALUE &&
      (hFile = TryOpen(true,L"..\\",filename)) == INVALID_HANDLE_VALUE)
  {
	  DWORD   code = GetLastError();
	  wchar_t em[256];
	  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, em, sizeof(em) / sizeof(em[0]), 0);
	  CString strMessage;
	  strMessage.Format(IDS_SCRIPT_LOAD_ERR_MSG, filename, em);
	  AtlTaskDialog(::GetActiveWindow(), IDS_SCRIPT_MSG_CPT, (LPCTSTR)strMessage, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
	  return HRESULT_FROM_WIN32(GetLastError());
  }

  DWORD length = SetFilePointer(hFile, 0, NULL, FILE_END);
  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

  BYTE *tmp = new BYTE[length + 2];
  if (tmp == NULL) {
    CloseHandle(hFile);
    return E_OUTOFMEMORY;
  }

  DWORD	nrd;
  BOOL	ok = ReadFile(hFile, tmp, length, &nrd, NULL);
  tmp[length] = tmp[length + 1] = 0;
  CloseHandle(hFile);

  if (!ok) {
    delete[] tmp;
    return HRESULT_FROM_WIN32(GetLastError());
  }
  if (nrd!=length) {
    delete[] tmp;
    return E_FAIL;
  }

  IActiveScriptParse	*pF;
  if (FAILED(hr = g_script->QueryInterface(IID_IActiveScriptParse, (void**)&pF))) {
	delete[] tmp;
	return hr;
  }

  EXCEPINFO ei;
  ZeroMemory(&ei, sizeof(ei));

  INT nos = 1;
  DetectEncodingInfo pencode;
  ZeroMemory(&pencode, sizeof(pencode));

  IMultiLanguage2 *pimlang2;
  CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_ALL, IID_IMultiLanguage2, (void **)&pimlang2);

  // 1КБ текста вполне достаточно для определения кодировки. Работает гораздо быстрее, чем с целым файлом.
  INT len = min(1024, length);
  HRESULT hCP = pimlang2->DetectInputCodepage(MLDETECTCP_8BIT, 0, (CHAR*)tmp, &len, &pencode, &nos);

  if(hCP == S_OK)
  {
	  if(pencode.nCodePage != 1200 && pencode.nCodePage != 1201)
	  {
		wchar_t* buffer = new wchar_t[length + 1];
		DWORD cvt = MultiByteToWideChar(pencode.nCodePage, 0 , (LPCSTR)tmp, length, buffer, length);
		buffer[cvt] = 0;
		hr = pF->ParseScriptText(buffer, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE | SCRIPTTEXT_ISPERSISTENT, NULL, &ei);
		delete[] buffer;
	  }
      else
	  {
		  if(pencode.nCodePage == 1201)
		  {
			  DWORD pdwMode = 0;
			  UINT inlen, outlen;
			  inlen = outlen = length + 2;
			  BYTE* converted = new BYTE[length + 2];
			  converted[length] = converted[length + 1] = 0;
			  pimlang2->ConvertString(&pdwMode, pencode.nCodePage, 1200, tmp, &inlen, converted, &outlen);
			  memcpy(tmp, converted, length + 2);
			  delete[] converted;
		  }
		  hr = pF->ParseScriptText((wchar_t*)tmp, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE | SCRIPTTEXT_ISPERSISTENT, NULL, &ei);
	  }
  }
  else hr = 0; 

  SysFreeString(ei.bstrSource);
  SysFreeString(ei.bstrDescription);
  SysFreeString(ei.bstrHelpFile);

  pimlang2->Release();
  pF->Release();
  delete[] tmp;

  return hr;
}
