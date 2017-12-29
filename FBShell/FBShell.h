#ifndef FBSHELL_H
#define FBSHELL_H

#include <windows.h>

extern class CFBShellModule _AtlModule;

// FDABCF3B-57BE-4110-94B5-4EF8EE3C6A62
DEFINE_GUID(CLSID_ContextMenu, 0xFDABCF3B, 0x57BE, 0x4110, 0x94, 0xB5, 0x4E, 0xF8, 0xEE, 0x3C, 0x6A, 0x62);
// 8CBB373E-693A-4bea-ADF3-D05EAE41684B
DEFINE_GUID(CLSID_ColumnProvider, 0x8CBB373E, 0x693A, 0x4bea, 0xAD, 0xF3, 0xD0, 0x5E, 0xAE, 0x41, 0x68, 0x4B);
// 585CFC85-7939-4004-9693-EB8C6F848B1F
DEFINE_GUID(CLSID_ThumbnailHandler, 0x585CFC85, 0x7939, 0x4004, 0x96, 0x93, 0xEB, 0x8C, 0x6F, 0x84, 0x8B, 0x1F);

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

#endif