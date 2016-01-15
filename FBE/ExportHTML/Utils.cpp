#include "stdafx.h"

#include "utils.h"

namespace U {

HandleStreamPtr	  NewStream(HANDLE& hf,bool fClose) {
  HandleStream  *hs;
  CheckError(HandleStream::CreateInstance(&hs));
  hs->SetHandle(hf,fClose);
  if (fClose)
    hf=INVALID_HANDLE_VALUE;
  return hs;
}

void  NormalizeInplace(CString& s) {
  int	  len=s.GetLength();
  TCHAR	  *p=s.GetBuffer(len);
  TCHAR	  *r=p;
  TCHAR	  *q=p;
  TCHAR	  *e=p+len;
  int	  state=0;

  while (p<e) {
    switch (state) {
    case 0:
      if ((unsigned)*p > 32) {
	*q++=*p;
	state=1;
      }
      break;
    case 1:
      if ((unsigned)*p > 32)
	*q++=*p;
      else
	state=2;
      break;
    case 2:
      if ((unsigned)*p > 32) {
	*q++=_T(' ');
	*q++=*p;
	state=1;
      }
      break;
    }
    ++p;
  }
  s.ReleaseBuffer(q-r);
}

void  RemoveSpaces(wchar_t *zstr) {
  wchar_t  *q=zstr;

  while (*zstr) {
    if (*zstr > 32)
      *q++=*zstr;
    ++zstr;
  }
  *q=L'\0';
}

int	scmp(const wchar_t *s1,const wchar_t *s2) {
  bool f1=!s1 || !*s1;
  bool f2=!s2 || !*s2;
  if (f1 && f2)
    return 0;
  if (f1)
    return -1;
  if (f2)
    return 1;
  return wcscmp(s1,s2);
}

CString	GetMimeType(const CString& filename) {
  CString   fn(filename);
  int	cp=fn.ReverseFind(_T('.'));
  if (cp<0)
os:
    return _T("application/octet-stream");
  fn.Delete(0,cp);
  CRegKey   rk;
  if (rk.Open(HKEY_CLASSES_ROOT,fn,KEY_READ)!=ERROR_SUCCESS)
    goto os;
  CString   ret;
  ULONG	    len=128;
  TCHAR	    *rbuf=ret.GetBuffer(len);
  rbuf[0]=_T('\0');
  LONG	    rv=rk.QueryStringValue(_T("Content Type"),rbuf,&len);
  ret.ReleaseBuffer();
  if (rv!=ERROR_SUCCESS)
    goto os;
  return ret;
}

bool is_whitespace(const wchar_t *spc) {
  while (*spc) {
    if (!iswspace(*spc) && *spc!=160)
      return false;
    ++spc;
  }
  return true;
}

CString	GetFileTitle(const TCHAR *filename) {
  CString   ret;
  TCHAR	    *buf=ret.GetBuffer(MAX_PATH);
  if (::GetFileTitle(filename,buf,MAX_PATH))
    ret.ReleaseBuffer(0);
  else
    ret.ReleaseBuffer();
  return ret;
}

DWORD QueryIV(HKEY hKey,const TCHAR *name,DWORD defval) {
  DWORD	dw;
  DWORD	len=sizeof(DWORD);
  DWORD	type=REG_DWORD;
  if (::RegQueryValueEx(hKey,name,NULL,&type,(BYTE*)&dw,&len)!=ERROR_SUCCESS ||
      type!=REG_DWORD || len!=sizeof(DWORD))
    return defval;
  return dw;
}

CString	QuerySV(HKEY hKey,const TCHAR *name,const TCHAR *def) {
  CString   ret;
  DWORD	    type=REG_SZ;
  DWORD	    len=0;
  if (::RegQueryValueEx(hKey,name,NULL,&type,NULL,&len)!=ERROR_SUCCESS ||
      (type!=REG_SZ && type!=REG_EXPAND_SZ))
    return def ? def : CString();
  TCHAR	    *cp=ret.GetBuffer(len/sizeof(TCHAR));
  if (::RegQueryValueEx(hKey,name,NULL,&type,(BYTE*)cp,&len)!=ERROR_SUCCESS) {
    ret.ReleaseBuffer(0);
    return def ? def : CString();
  }
  ret.ReleaseBuffer(len/sizeof(TCHAR));
  return ret;
}

HFONT	  CreatePtFont(int sizept,const TCHAR *facename,bool fBold,bool fItalic)
{
  HDC	  hDC=CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
  HFONT	  hFont=::CreateFont(
    -MulDiv(sizept, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0,
    0,
    0,
    fBold ? FW_BOLD : FW_NORMAL,
    fItalic,
    0,
    0,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY,
    DEFAULT_PITCH|FF_DONTCARE,
    facename);
  DeleteDC(hDC);
  return hFont;
}

CString	GetCharName(int ch) {
  CString   num;
  num.Format(_T("U+%04X"),ch);

#if 0
  static bool	  fTriedDll;
  static HMODULE  hDll;
  if (!hDll) {
    if (fTriedDll)
      return num;
    hDll=LoadLibrary(_T("getuname.dll"));
    fTriedDll=true;
    if (!hDll)
      return num;
  }
  int	    cur=num.GetLength();
  TCHAR	    *buf=num.GetBuffer(cur+101);
  int	    nch=LoadString(hDll,ch,buf+cur+1,100);
  if (nch) {
    buf[cur]=' ';
    num.ReleaseBuffer(cur+1+nch);
  } else
    num.ReleaseBuffer(cur);
#endif
  return num;
}

// path names
CString	GetFullPathName(const CString& filename) {
  CString   ret;
  int	    len=MAX_PATH;
  for (;;) {
    TCHAR	    *buf=ret.GetBuffer(len);
    TCHAR	    *final;
    int nlen=::GetFullPathName(filename,len,buf,&final);
    if (nlen==0) // failed
      return filename;
    if (nlen>len) {
      ret.ReleaseBuffer(0);
      len=nlen;
    } else {
      ret.ReleaseBuffer(nlen);
      break;
    }
  }

  typedef DWORD	(__stdcall *GLPN)(LPCTSTR,LPTSTR,DWORD);
  static bool	checked=false;
  static GLPN	glpn;

  if (!checked) {
    HMODULE hDll=::GetModuleHandle(_T("kernel32.dll"));
    if (hDll) {
      glpn=(GLPN)::GetProcAddress(hDll,"GetLongPathName"
#ifdef UNICODE
	"W"
#else
	"A"
#endif
	);
    }
    checked=true;
  }

  if (glpn) {
    CString   r2;
    for (;;) {
      TCHAR   *buf=r2.GetBuffer(len);
      int	nlen=glpn(ret,buf,len);
      if (nlen==0)
	return ret;
      if (nlen>len) {
	r2.ReleaseBuffer(0);
	len=nlen;
      } else {
	r2.ReleaseBuffer(nlen);
	return r2;
      }
    }
  } else
    return ret;
}

CString	UrlFromPath(const CString& path) {
  CString   ret;
  DWORD	    size=MAX_PATH*3;
  TCHAR	    *cp=ret.GetBuffer(size);
  if (FAILED(UrlCreateFromPath(path,cp,&size,0)))
    return CString();
  ret.ReleaseBuffer(size);
  ret.FreeExtra();
  return ret;
}

CString	Win32ErrMsg(DWORD code) {
  CString ret;
  TCHAR	  *buf=ret.GetBuffer(1024);
  int len=::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,code,0,buf,1024,NULL);
  ret.ReleaseBuffer(len);
  if (len==0)
    ret.Format(_T("Unknown error %x"),code);
  return ret;
}

CString	GetProgDir() {
  CString     exedir;
  TCHAR	      *cp=exedir.GetBuffer(MAX_PATH);
  DWORD len=::GetModuleFileName(_Module.GetModuleInstance(),cp,MAX_PATH);
  exedir.ReleaseBuffer(len);
  int	      p=len;
  while (p>0 && exedir[p-1]!=_T('\\'))
    --p;
  exedir.Delete(p,len-p);
  return exedir;
}

CString	GetProgDirFile(const CString& filename) {
  CString     exedir;
  TCHAR	      *cp=exedir.GetBuffer(MAX_PATH);
  DWORD len=::GetModuleFileName(_Module.GetModuleInstance(),cp,MAX_PATH);
  exedir.ReleaseBuffer(len);
  int	      p=len;
  while (p>0 && exedir[p-1]!=_T('\\'))
    --p;
  exedir.Delete(p,len-p);
  CString     tryname(exedir+filename);
  DWORD	      attr=::GetFileAttributes(tryname);
  if (attr!=INVALID_FILE_ATTRIBUTES)
    goto ok;
  tryname=exedir+_T("..\\")+filename;
  attr=::GetFileAttributes(tryname);
  if (attr!=INVALID_FILE_ATTRIBUTES)
    goto ok;
  tryname=filename;
ok:
  return GetFullPathName(tryname);
}

void  ReportError(HRESULT hr) {
  CString   err(Win32ErrMsg(hr));
  ::MessageBox(::GetActiveWindow(),err,_T("Error"),MB_OK|MB_ICONERROR);
}

void  ReportError(_com_error& e) {
  CString   err;
  err.Format(_T("Code: %08lx [%s]"),e.Error(),e.ErrorMessage());
  _bstr_t src(e.Source());
  if (src.length()>0) {
    err+=_T("\nSource: ");
    err+=(const wchar_t *)src;
  }
  src=e.Description();
  if (src.length()>0) {
    err+=_T("\nDescription: ");
    err+=(const wchar_t *)src;
  }
  ::MessageBox(::GetActiveWindow(),err,_T("COM Error"),MB_OK|MB_ICONERROR);
}

UINT  MessageBox(UINT type,const TCHAR *title,const TCHAR *msg,...) {
  CString   str;
  va_list   ap;
  va_start(ap,msg);
  str.FormatV(msg,ap);
  va_end(ap);
  return ::MessageBox(::GetActiveWindow(),str,title,type);
}

CString	GetWindowText(HWND hWnd) {
  CString ret;
  int	  tl=::GetWindowTextLength(hWnd);
  TCHAR	  *cp=ret.GetBuffer(tl+1);
  ::GetWindowText(hWnd,cp,tl+1);
  ret.ReleaseBuffer();
  return ret;
}

CString	GetCBString(HWND hWnd,int idx) {
  LRESULT len=::SendMessage(hWnd,CB_GETLBTEXTLEN,idx,0);
  if (len==CB_ERR)
    return CString();
  CString ret;
  TCHAR	  *cp=ret.GetBuffer(len+1);
  len=::SendMessage(hWnd,CB_GETLBTEXT,idx,(LPARAM)cp);
  if (len==CB_ERR)
    ret.ReleaseBuffer(0);
  else
    ret.ReleaseBuffer(len);
  return ret;
}

MSXML2::IXMLDOMDocument2Ptr  CreateDocument(bool fFreeThreaded)
{
  MSXML2::IXMLDOMDocument2Ptr  doc;
  wchar_t		      *cls=fFreeThreaded ?
    L"Msxml2.FreeThreadedDOMDocument.6.0" : L"Msxml2.DOMDocument.6.0";
  CheckError(doc.CreateInstance(cls));
  return doc;
}

MSXML2::IXSLTemplatePtr    CreateTemplate() {
  MSXML2::IXSLTemplatePtr    tp;
  CheckError(tp.CreateInstance(L"Msxml2.XSLTemplate.6.0"));
  return tp;
}

void  ReportParseError(MSXML2::IXMLDOMDocument2Ptr doc)
{
  try {
    MSXML2::IXMLDOMParseErrorPtr err(doc->parseError);
    long	  line=err->line;
    long	  col=err->linepos;
    _bstr_t	  url(err->url);
    _bstr_t	  reason(err->reason);
    CString   msg;
    if (line && col)
      U::MessageBox(MB_OK|MB_ICONERROR,_T("XML Parse Error"),
	_T("At %s, line %d, column %d: %s"),(const TCHAR *)url,
	line,col,(const TCHAR *)reason);
    else
      U::MessageBox(MB_OK|MB_ICONERROR,_T("XML Parse Error"),
	_T("At %s: %s"),(const TCHAR *)url,(const TCHAR *)reason);
  }
  catch (_com_error& e) {
    ReportError(e);
  }
}

bool  LoadXml(MSXML2::IXMLDOMDocument2Ptr doc,const CString& url)
{
  doc->put_async(VARIANT_FALSE);
  _variant_t  vturl((const TCHAR *)url);
  VARIANT_BOOL	flag;
  HRESULT   hr=doc->raw_load(vturl,&flag);
  if (FAILED(hr)) {
    ReportError(hr);
    return false;
  }
  if (flag!=VARIANT_TRUE) {
    ReportParseError(doc);
    return false;
  }
  return true;
}

HRESULT	  LoadFile(const TCHAR *filename,VARIANT *vt) {
  HRESULT   hr;

  HANDLE    hFile=::CreateFile(filename,
    GENERIC_READ,
    FILE_SHARE_READ,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_SEQUENTIAL_SCAN,
    NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return HRESULT_FROM_WIN32(::GetLastError());

  DWORD	  fsz=::GetFileSize(hFile,NULL);
  if (fsz==INVALID_FILE_SIZE) {
    hr=HRESULT_FROM_WIN32(::GetLastError());
cfexit:
    CloseHandle(hFile);
    return hr;
  }

  SAFEARRAY   *sa=::SafeArrayCreateVector(VT_UI1,0,fsz);
  if (sa==NULL) {
    hr=E_OUTOFMEMORY;
    goto cfexit;
  }

  void		  *pv;
  if (FAILED(hr=::SafeArrayAccessData(sa,&pv))) {
    ::SafeArrayDestroy(sa);
    goto cfexit;
  }

  DWORD	  nrd;
  BOOL	  fRd=::ReadFile(hFile,pv,fsz,&nrd,NULL);
  DWORD	  err=::GetLastError();

  ::SafeArrayUnaccessData(sa);
  ::CloseHandle(hFile);

  if (!fRd) {
    ::SafeArrayDestroy(sa);
    return HRESULT_FROM_WIN32(err);
  }

  if (nrd!=fsz) {
    ::SafeArrayDestroy(sa);
    return E_FAIL;
  }

  V_VT(vt)=VT_ARRAY|VT_UI1;
  V_ARRAY(vt)=sa;

  return S_OK;
}

void InitSettings() {
  TCHAR	  filepath[MAX_PATH];
  DWORD	  pathlen=::GetModuleFileName(_Module.GetModuleInstance(),filepath,MAX_PATH);
  TCHAR	  *appname;
  if (pathlen==0)
    appname=_T("FBE"); // fallback
  else {
    CString   tmp=GetFullPathName(filepath);
    int	      pos=tmp.ReverseFind(_T('\\'));
    if (pos>=0)
      tmp.Delete(0,pos+1);
    pos=tmp.ReverseFind(_T('.'));
    if (pos>=0) {
      const TCHAR *cp=tmp;
      cp+=pos;
      if (_tcsicmp(cp,_T(".exe"))==0 || _tcsicmp(cp,_T(".dll"))==0)
	tmp.Delete(pos,tmp.GetLength()-pos);
    }
    if (tmp.IsEmpty())
      appname=_T("FBE");
    else {
      lstrcpyn(filepath,tmp,MAX_PATH);
      appname=filepath;
    }
  }
  _SettingsPath=_T("Software\\Haali\\");
  _SettingsPath+=appname;
  _Settings.Create(HKEY_CURRENT_USER,_SettingsPath);
}

}
