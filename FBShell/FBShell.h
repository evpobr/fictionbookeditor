#ifndef FBSHELL_H
#define FBSHELL_H

#include "FBShell_h.h"

extern class CFBShellModule _AtlModule;

///////////////////////////////////////////////////////////
// Misc globals

extern const wchar_t 	*FBNS;
extern const wchar_t 	*XLINKNS;
bool	StrEQ(const wchar_t *zstr,const wchar_t *wstr,int wlen);
void	NormalizeInplace(CString& s);
CString	GetAttr(ISAXAttributes *attr,const wchar_t *name,const wchar_t *ns=NULL);
template<class T>
extern inline HRESULT CreateObject(CComPtr<T>& ptr) {
  T	  *obj;
  HRESULT hr=T::CreateInstance(&obj);
  if (FAILED(hr))
    return hr;
  ptr=obj;
  return S_OK;
}
void	AppendText(CString& str,const TCHAR *text,int textlen);

extern CRITICAL_SECTION	g_Lock;

///////////////////////////////////////////////////////////
// missing shell guids
struct __declspec(uuid("E8025004-1C42-11d2-BE2C-00A0C9A83DA1")) IColumnProvider;
struct __declspec(uuid("85788d00-6807-11d0-b810-00c04fd706ec")) IRunnableTask;

#endif