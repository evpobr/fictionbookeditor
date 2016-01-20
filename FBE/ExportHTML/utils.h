#ifndef UTILS_H
#define	UTILS_H

namespace U { // place all utilities into their own namespace

  // loading data into array
  HRESULT   LoadFile(const TCHAR *filename,VARIANT *vt);

  // a generic stream
  class HandleStreamImpl: public CComObjectRoot, public IStream {
  private:
    HANDLE    m_handle;
    bool      m_close;

  public:
    typedef IStream Interface;

    HandleStreamImpl() : m_handle(INVALID_HANDLE_VALUE), m_close(true) { }
    virtual ~HandleStreamImpl() { if (m_close) CloseHandle(m_handle); }

    void  SetHandle(HANDLE hf,bool cl=true) { m_handle=hf; m_close=cl; }

    BEGIN_COM_MAP(HandleStreamImpl)
      COM_INTERFACE_ENTRY(ISequentialStream)
      COM_INTERFACE_ENTRY(IStream)
    END_COM_MAP()

    // ISequentialStream
    STDMETHOD(Read)(void *pv,ULONG cb,ULONG *pcbRead) {
      DWORD   nrd;
      if (ReadFile(m_handle,pv,cb,&nrd,NULL)) {
	*pcbRead=nrd;
	return S_OK;
      }
      if (GetLastError()==ERROR_BROKEN_PIPE) { // treat as eof
	*pcbRead=0;
	return S_OK;
      }
      return HRESULT_FROM_WIN32(GetLastError());
    }
    STDMETHOD(Write)(const void *pv,ULONG cb,ULONG *pcbWr) {
      DWORD   nwr;
      if (WriteFile(m_handle,pv,cb,&nwr,NULL)) {
	*pcbWr=nwr;
	return S_OK;
      }
      return HRESULT_FROM_WIN32(GetLastError());
    }

    // IStream
    STDMETHOD(Seek)(LARGE_INTEGER,DWORD,ULARGE_INTEGER*) { return E_NOTIMPL; }
    STDMETHOD(SetSize)(ULARGE_INTEGER) { return E_NOTIMPL; }
    STDMETHOD(CopyTo)(IStream*,ULARGE_INTEGER,ULARGE_INTEGER*,ULARGE_INTEGER*) { return E_NOTIMPL; }
    STDMETHOD(Commit)(DWORD) { return E_NOTIMPL; }
    STDMETHOD(Revert)() { return E_NOTIMPL; }
    STDMETHOD(LockRegion)(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) { return E_NOTIMPL; }
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) { return E_NOTIMPL; }
    STDMETHOD(Stat)(STATSTG*,DWORD) { return E_NOTIMPL; }
    STDMETHOD(Clone)(IStream**) { return E_NOTIMPL; }
  };

  typedef CComObject<HandleStreamImpl>	  HandleStream;
  typedef CComPtr<HandleStream>		  HandleStreamPtr;
  HandleStreamPtr			  NewStream(HANDLE& hf,bool fClose=true);

  // strings
  int		scmp(const wchar_t *s1,const wchar_t *s2);
  CString	GetMimeType(const CString& filename);
  bool		is_whitespace(const wchar_t *spc);
  void		NormalizeInplace(CString& s);
  void		RemoveSpaces(wchar_t *zstr);
  extern inline CString	Normalize(const CString& s) { CString p(s); NormalizeInplace(p); return p; }
  CString	GetFileTitle(const TCHAR *filename);
  extern inline void	StrAppend(CString& s1,const CString& s2) {
    if (!s2.IsEmpty()) {
      s1+=_T(' ');
      s1+=s2;
    }
  }
  CString	UrlFromPath(const CString& path);

  // settings in the registry
  CString	QuerySV(HKEY hKey,const TCHAR *name,const TCHAR *def=NULL);
  DWORD		QueryIV(HKEY hKey,const TCHAR *name,DWORD defval=0);
  void		InitSettings();
  extern inline DWORD	GetSettingI(const TCHAR *name,DWORD defval=0) {
    return U::QueryIV(_Settings,name,defval);
  }

  extern inline CString	GetSettingS(const TCHAR *name,const TCHAR *def=NULL) {
    return U::QuerySV(_Settings,name,def);
  }

  // windows api
  HFONT	  CreatePtFont(int sizept,const TCHAR *facename,bool fBold=false,bool fItalic=false);
  CString GetFullPathName(const CString& filename);
  CString Win32ErrMsg(DWORD code);
  CString GetWindowText(HWND hWnd);
  void	  ReportError(HRESULT hr);
  void	  ReportError(_com_error& e);
  UINT	  MessageBox(UINT type,const TCHAR *title,const TCHAR *msg,...);
  CString GetProgDir();
  CString GetProgDirFile(const CString& filename);
  CString GetCBString(HWND hCB,int idx);

  // unicode char names (win2k/xp only)
  CString GetCharName(int ch);

  // msxml support
  IXMLDOMDocument2Ptr CreateDocument(bool fFreeThreaded=false);
  void			      ReportParseError(IXMLDOMDocument2Ptr doc);
  bool			      LoadXml(IXMLDOMDocument2Ptr doc,
				      const CString& url);
  IXSLTemplatePtr     CreateTemplate();

} // namespace

#endif