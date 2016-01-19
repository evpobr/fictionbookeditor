#include "stdafx.h"
#include "resource.h"
#include "FBShell_h.h"
#include "dllmain.h"


/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
  return _AtlModule.DllCanUnloadNow();
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// misc global utilities
const wchar_t 	*FBNS=L"http://www.gribuser.ru/xml/fictionbook/2.0";
const wchar_t 	*XLINKNS=L"http://www.w3.org/1999/xlink";

bool   StrEQ(const wchar_t *zstr,const wchar_t *wstr,int wlen) {
  while (*zstr && wlen--)
    if (*zstr++!=*wstr++)
      return false;

  return wlen==0 && !*zstr;
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

CString	GetAttr(ISAXAttributes *attr,const wchar_t *name,const wchar_t *ns) {
  int nslen=ns ? lstrlenW(ns) : 0;
  int nlen=lstrlenW(name);

  int vlen;
  const wchar_t *val;

  if (FAILED(attr->getValueFromName(ns,nslen,name,nlen, &val,&vlen)))
    return CString();

  CString ret(val,vlen);
  NormalizeInplace(ret);
  return ret;
}

void  AppendText(CString& str,const TCHAR *text,int len) {
  int	  off=str.GetLength();
  int	  total=off+len;

  TCHAR   *buf=str.GetBuffer(total);

  memcpy(buf+off,text,len*sizeof(TCHAR));

  str.ReleaseBuffer(total);
}
