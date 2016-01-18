#include "stdafx.h"
#include "ExportHTMLPlugin.h"

#include "utils.h"
#include "FBE.h"
#include "CustomFileSaveDialog.h"

HRESULT	CExportHTMLPlugin::Export(long hWnd,BSTR filename,IDispatch *doc)
{
  HANDLE  hOut=INVALID_HANDLE_VALUE;

  try {
    // * construct doc pointer
    IXMLDOMDocument2Ptr	    source(doc);

    // * ask the user where he wants his html
    CCustomSaveDialog	    dlg(FALSE,_T("html"),filename,
      OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_OVERWRITEPROMPT|OFN_ENABLETEMPLATE,
      _T("Web Page, complete (*.html;*.htm)\0*.htm;*.html\0")
      _T("Web Archive, single file (*.mht)\0*.mht\0")
      _T("Web Page, HTML only (*.html;*.htm)\0*.htm;*.html\0")
    );
    dlg.m_ofn.nFilterIndex=1;
    if (dlg.DoModal((HWND)hWnd)!=IDOK)
      return S_FALSE;

    // * load template
    IXMLDOMDocument2Ptr	    tdoc(U::CreateDocument(true));
    if (!U::LoadXml(tdoc,dlg.m_template))
      return S_FALSE;
    IXSLTemplatePtr	    tmpl(U::CreateTemplate());
    CheckError(tmpl->putref_stylesheet(tdoc));

    // * create processor
	IXSLProcessorPtr	    proc;
	CheckError(tmpl->createProcessor(&proc));

    // * setup input
    CheckError(proc->put_input(variant_t(doc)));

    // * install template parameters
    CheckError(proc->addParameter(bstr_t(L"includedesc"), variant_t(dlg.m_includedesc),_bstr_t()));
    CheckError(proc->addParameter(bstr_t(L"tocdepth"),variant_t((long)dlg.m_tocdepth),_bstr_t()));

    bool    fImages=dlg.m_ofn.nFilterIndex<=2;
    bool    fMIME=dlg.m_ofn.nFilterIndex==2;

    CString dfile(dlg.m_szFileName);

    // * open the file
    hOut=::CreateFile(dlg.m_szFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
    if (hOut==INVALID_HANDLE_VALUE) {
      U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't open %s: %s"),
	dlg.m_szFileName,(const TCHAR *)U::Win32ErrMsg(::GetLastError()));
      return S_FALSE;
    }

    // * construct images directory
    int	  cp=dfile.ReverseFind(_T('.'));
    if (cp>=0)
      dfile.Delete(cp,dfile.GetLength()-cp);
    dfile+=_T("_files");
    if (fImages) {
      // construct a relative path
      CString	relpath(dfile);
      cp=relpath.ReverseFind(_T('\\'));
      if (cp>=0)
	relpath.Delete(0,cp+1);

      // see if it is ascii only
      bool fAscii=true;
      for (int i=0;i<relpath.GetLength();++i)
	if (relpath[i]<32 || relpath[i]>127) {
	  fAscii=false;
	  break;
	}

      if (fAscii && !fMIME) {
	relpath+=_T('/');
	CheckError(proc->addParameter(bstr_t(L"imgprefix"), variant_t((const TCHAR *)relpath),_bstr_t()));

	if (!::CreateDirectory(dfile,NULL) && ::GetLastError()!=ERROR_ALREADY_EXISTS) {
	  DWORD	de=::GetLastError();
	  CloseHandle(hOut);
	  ::DeleteFile(dlg.m_szFileName);
	  U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't create directory %s: %s"),
	    (const TCHAR *)dfile,(const TCHAR *)U::Win32ErrMsg(de));
	  return S_FALSE;
	}
      } else
	dfile.Delete(cp,dfile.GetLength()-cp);

      CheckError(proc->addParameter(bstr_t(L"saveimages"),variant_t(true),_bstr_t()));
    }

    char    boundary[256];

    // * write relevant MIME headers
    if (fMIME) {
      // format date
      char  date[256]; time_t  tt; time(&tt);
      strftime(date,sizeof(date),"%a, %d %b %Y %H:%M:%S +0000",gmtime(&tt));

      // construct some random mime boundary
      _snprintf_s(boundary,sizeof(boundary),"------NextPart---%08X.%08X",tt,rand());

      // construct mime header
      char  mime_hdr[2048];
      _snprintf_s(mime_hdr,sizeof(mime_hdr),
	"From: <Saved by Haali ExportHTML Plugin>\r\n"
	"Date: %s\r\n" // Thu, 17 Apr 2003 07:34:30 +0400
	"MIME-Version: 1.0\r\n"
	"Content-Type: multipart/related; boundary=\"%s\"; type=\"text/html\"\r\n"
	"\r\n"
	"This is a multi-part message in MIME format.\r\n"
	"\r\n"
	"%s\r\n"
	"Content-Type: text/html; charset=\"utf-8\"\r\n"
	"Content-Transfer-Encoding: 8bit\r\n"
	"\r\n",
	date,boundary+2,boundary);

      DWORD   len=strlen(mime_hdr);
      DWORD   nw;
      BOOL    fWr=WriteFile(hOut,mime_hdr,len,&nw,NULL);
      if (!fWr || nw!=len) {
	if (!fWr)
	  U::MessageBox(MB_OK|MB_ICONERROR,
	    _T("Export HTML"),_T("Can't write to %s: %s"),
	    dlg.m_szFileName,(const TCHAR *)U::Win32ErrMsg(::GetLastError()));
	else
	  U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't write to %s: short write."),
	    dlg.m_szFileName);
	::CloseHandle(hOut);
	::DeleteFile(dlg.m_szFileName);
	return S_FALSE;
      }
    }

    // * transform
    CheckError(proc->put_output(variant_t((IUnknown*)U::NewStream(hOut,!fMIME))));
	VARIANT_BOOL Done = VARIANT_FALSE;
    CheckError(proc->transform(&Done));

    // * save images
    if (fImages) {
      if (dfile.IsEmpty() || dfile[dfile.GetLength()-1]!=_T('\\'))
	dfile+=_T('\\');
	  IXMLDOMNodeListPtr      bins;
	  CheckError(source->selectNodes(bstr_t(L"/fb:FictionBook/fb:binary"), &bins));
	  long listLength = 0;
	  CheckError(bins->get_length(&listLength));
      for (long l=0;l<listLength;++l) {
	try {
		IXMLDOMNodePtr   be;
		CheckError(bins->get_item(l, &be));
		IXMLDOMElementPtr element;
		CheckError(be->QueryInterface(IID_PPV_ARGS(&element)));
		_variant_t	id;
		CheckError(element->getAttribute(bstr_t(L"id"), &id));
		_variant_t	ct;
		CheckError(element->getAttribute(bstr_t(L"content-type"), &ct));
		if (V_VT(&id)!=VT_BSTR || V_VT(&ct)!=VT_BSTR)
	    continue;

	  if (fMIME) {
	    // get base64 data
		  CComBSTR   data;
		  CheckError(be->get_text(&data));

	    // allocate buffer
	    char      *buffer=(char*)malloc(data.Length()+1024);
	    if (buffer==NULL)
	      continue;

	    // construct a MIME header
	    _snprintf(buffer,1024,
	      "\r\n"
	      "%s\r\n"
	      "Content-Type: %S\r\n"
	      "Content-Transfer-Encoding: base64\r\n"
	      "Content-Location: %S\r\n"
	      "\r\n",
	      boundary,V_BSTR(&ct),V_BSTR(&id));
	    DWORD     hlen=strlen(buffer);

	    // convert data to ascii
	    DWORD     mlen=WideCharToMultiByte(CP_ACP,0,
	      data,data.Length(),
	      buffer+hlen,data.Length(),
	      NULL,NULL);

	    // write a new mime header+data
	    DWORD   nw;
	    BOOL    fWr=WriteFile(hOut,buffer,hlen+mlen,&nw,NULL);
	    DWORD   de=::GetLastError();
	    free(buffer);

	    if (!fWr || nw!=hlen+mlen) {
	      if (!fWr)
		U::MessageBox(MB_OK|MB_ICONERROR,
		  _T("Export HTML"),_T("Can't write to %s: %s"),
		  dlg.m_szFileName,(const TCHAR *)U::Win32ErrMsg(de));
	      else
		U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't write to %s: short write."),
		  dlg.m_szFileName);
	      ::CloseHandle(hOut);
	      ::DeleteFile(dlg.m_szFileName);
	      return S_FALSE;
	    }
	  } else {
	    CheckError(be->put_dataType(bstr_t(L"bin.base64")));
		_variant_t	data;
		CheckError(be->get_nodeTypedValue(&data));
	    if (V_VT(&data)!=(VT_ARRAY|VT_UI1) || ::SafeArrayGetDim(V_ARRAY(&data))!=1)
	      continue;
	    DWORD len=V_ARRAY(&data)->rgsabound[0].cElements;
	    void	*buffer;
	    ::SafeArrayAccessData(V_ARRAY(&data),&buffer);
	    CString fname(dfile);
	    fname+=V_BSTR(&id);
	    HANDLE hFile=::CreateFile(fname,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
	    if (hFile==INVALID_HANDLE_VALUE && ::GetLastError()==ERROR_FILE_EXISTS) {
	      if (U::MessageBox(MB_YESNO|MB_ICONEXCLAMATION,_T("Export HTML"),
		  _T("%s already exists.\nDo you want to replace it?"),
		  (const TCHAR *)fname)!=IDYES)
		goto skip;
	      hFile=::CreateFile(fname,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	    }
	    if (hFile!=INVALID_HANDLE_VALUE) {
	      DWORD wr;
	      BOOL fWr=::WriteFile(hFile,buffer,len,&wr,NULL);
	      DWORD de=::GetLastError();
	      ::CloseHandle(hFile);
	      if (!fWr || wr!=len) {
		if (!fWr)
		  U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't write to %s: %s"),
		    (const TCHAR *)fname,(const TCHAR *)U::Win32ErrMsg(de));
		else
		  U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't write to %s: short write."),
		    (const TCHAR *)fname);
		::DeleteFile(fname);
	      }
	    } else {
	      U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't open %s: %s"),
		(const TCHAR *)fname,(const TCHAR *)U::Win32ErrMsg(::GetLastError()));
	    }
  skip:
	    ::SafeArrayUnaccessData(V_ARRAY(&data));
	  }
	}
	catch (_com_error&) { }
      }
    }

    // * write a final mime boundary
    if (fMIME) {
      char    mime_tmp[256];
      _snprintf_s(mime_tmp,sizeof(mime_tmp),"\r\n%s\r\n",boundary);
      DWORD   len=strlen(mime_tmp);
      DWORD   nw;
      BOOL    fWr=WriteFile(hOut,mime_tmp,len,&nw,NULL);
      if (!fWr || nw!=len) {
	if (!fWr)
	  U::MessageBox(MB_OK|MB_ICONERROR,
	    _T("Export HTML"),_T("Can't write to %s: %s"),
	    dlg.m_szFileName,(const TCHAR *)U::Win32ErrMsg(::GetLastError()));
	else
	  U::MessageBox(MB_OK|MB_ICONERROR,_T("Export HTML"),_T("Can't write to %s: short write."),
	    dlg.m_szFileName);
	::CloseHandle(hOut);
	::DeleteFile(dlg.m_szFileName);
	return S_FALSE;
      }
      ::CloseHandle(hOut);
    }
  }
  catch (_com_error& e) {
    if (hOut!=INVALID_HANDLE_VALUE)
      CloseHandle(hOut);
    U::ReportError(e);
    return S_FALSE;
  }
  return S_OK;
}