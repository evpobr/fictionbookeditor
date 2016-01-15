#include "stdafx.h"

#include <windows.h>
#include <commctrl.h>
#include <assert.h>
#include <tchar.h>

#include "resource.h"

enum {
  INIT_TIMER=1
};

static HWND		g_dialog;
static HINSTANCE	g_instance;
static UINT_PTR		g_timer;
static bool		g_stop;
static bool		g_validating;
static TCHAR		g_schema_file[MAX_PATH];
static const wchar_t	*FBNS=L"http://www.gribuser.ru/xml/fictionbook/2.0";
static int		FBNS_len;
static const wchar_t	*XLINKNS=L"http://www.w3.org/1999/xlink";
static int		XLINKNS_len;

static void		DoEvents() {
  MSG	msg;

  while (::PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
    if (!::IsDialogMessage(g_dialog,&msg)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
  }
}

enum VState {
  UNCHECKED, VALID, INVALID
};

struct FileInfo {
  BSTR		  filename;
  BSTR		  errmsg;
  FILETIME	  timestamp;
  ULARGE_INTEGER  size;
  VState	  state;
};

static struct FileInfo	*g_file_list;
static int		g_list_items,g_list_max;

static void   ScanDirectory(const wchar_t *filename);
static void   ValidateFiles();

static void   AddFileInfo(const wchar_t *filename,DWORD attr,FILETIME ft,
			  DWORD szLow,DWORD szHigh)
{
  if (g_list_items>=g_list_max) {
    int	  nsize=g_list_max ? g_list_max<<1 : 128;
    void  *nmem=::realloc(g_file_list, nsize*sizeof(g_file_list[0]));
    if (!nmem)
      return;
    g_file_list=(FileInfo*)nmem;
    g_list_max=nsize;
  }

  FileInfo    *fi=&g_file_list[g_list_items++];

  fi->filename=::SysAllocString(filename);

  if (!fi->filename)
    return;

  fi->timestamp=ft;
  fi->size.LowPart=szLow;
  fi->size.HighPart=szHigh;
  fi->state=UNCHECKED;
  fi->errmsg=NULL;

  // add to list control
  LVITEM    lvi;
  memset(&lvi,0,sizeof(lvi));
  lvi.mask=LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT;
  lvi.iItem=g_list_items-1;
  lvi.pszText=LPSTR_TEXTCALLBACK;
  lvi.lParam=g_list_items-1;

  ::SendDlgItemMessage(g_dialog,IDC_FILELIST,LVM_INSERTITEM,0,(LPARAM)&lvi);
}

static void   AddFile(const wchar_t *filename) {
  DWORD	      attr=::GetFileAttributes(filename);

  if (attr == INVALID_FILE_ATTRIBUTES)
    return;

  if (attr & FILE_ATTRIBUTE_DIRECTORY) {
    ScanDirectory(filename);
    return;
  }

  HANDLE      hFile=::CreateFile(filename,FILE_READ_ATTRIBUTES,0,NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return;

  FILETIME	  ft;
  ULARGE_INTEGER  fs;

  ::GetFileTime(hFile,NULL,NULL,&ft);
  fs.LowPart=::GetFileSize(hFile,&fs.HighPart);

  ::CloseHandle(hFile);

  AddFileInfo(filename,attr,ft,fs.LowPart,fs.HighPart);
}

static void   ScanDirectory(const wchar_t *filename) {
  ::SendDlgItemMessage(g_dialog,IDC_STATUS,SB_SETTEXT,0,(LPARAM)filename);
  DoEvents(); // pump messages

  int		  namelen=lstrlen(filename);
  if (namelen<=0 || namelen>MAX_PATH-5)
    return;

  TCHAR		  buffer[MAX_PATH];
  lstrcpy(buffer,filename);

  if (buffer[namelen-1]!=_T('\\'))
    buffer[namelen++]=_T('\\');
  buffer[namelen]=_T('*');
  buffer[namelen+1]=0;

  HANDLE	  hFind;
  BOOL		  fNext=TRUE;
  WIN32_FIND_DATA fd;

  for (hFind=::FindFirstFile(buffer,&fd);
       hFind!=INVALID_HANDLE_VALUE && fNext && !g_stop;
       fNext=::FindNextFile(hFind,&fd))
  {
    if (lstrcmp(fd.cFileName,_T("."))==0 || lstrcmp(fd.cFileName,_T(".."))==0)
      continue;

    int	  curlen=lstrlen(fd.cFileName);
    if (curlen+namelen>=MAX_PATH)
      continue;

    lstrcpy(buffer+namelen,fd.cFileName);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      ScanDirectory(buffer);
      continue;
    }

    if (curlen>4 && lstrcmpi(fd.cFileName+curlen-4,_T(".fb2"))==0)
      AddFileInfo(buffer,fd.dwFileAttributes,fd.ftLastWriteTime,
	fd.nFileSizeLow,fd.nFileSizeHigh);
  }

  if (hFind!=INVALID_HANDLE_VALUE)
    ::FindClose(hFind);
}

static void   ProcessCommandLine() {
  int	    argc;
  wchar_t   **argv=::CommandLineToArgvW(::GetCommandLineW(),&argc);

  int	    base=0;
  if (argc>0) {
    int	  len=lstrlen(argv[0]);
    if (len>4 && lstrcmpiW(argv[0]+len-4,L".exe")==0)
      ++base;
  }

  for (int i=base;i<argc;++i)
    AddFile(argv[i]);

  ::GlobalFree((HGLOBAL)argv);

  if (!g_stop) {
    ::SendDlgItemMessage(g_dialog,IDC_STATUS,SB_SETTEXT,0,(LPARAM)_T("Done."));
    ::ShowWindow(::GetDlgItem(g_dialog,IDC_VALIDATE),SW_SHOW);
  }
}

static void   FreeFileData() {
  for (int i=0;i<g_list_items;++i) {
    ::SysFreeString(g_file_list[i].filename);
    ::SysFreeString(g_file_list[i].errmsg);
  }

  free(g_file_list);

  g_file_list=NULL;
  g_list_items=g_list_max=0;
}

static INT_PTR CALLBACK  DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

static bool   GetSchemaFile() {
  ::GetModuleFileName(NULL,g_schema_file,MAX_PATH);

  TCHAR	  *cp=wcsrchr(g_schema_file,_T('\\'));
  if (cp)
    ++cp;
  else
    cp=g_schema_file;

  *cp=0;

  int	len=lstrlen(g_schema_file);
  lstrcpyn(g_schema_file+len,_T("FictionBook.xsd"),MAX_PATH-len);

  if (::GetFileAttributes(g_schema_file)==INVALID_FILE_ATTRIBUTES) {
    ::MessageBox(NULL,_T("Can't load FictionBook schema."),_T("Error"),MB_OK|MB_ICONERROR);
    return false;
  }

  return true;
}

int __stdcall WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
		      LPSTR lpCmdLine,int nCmdShow)
{
  g_instance=hInstance;

  if (!GetSchemaFile())
    return 0;

  FBNS_len=lstrlenW(FBNS);
  XLINKNS_len=lstrlenW(XLINKNS);

  ::CoInitialize(NULL);

  ::InitCommonControls();

  ::DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN),NULL,DlgProc);

  FreeFileData();

  ::CoUninitialize();

  return 0;
}

// dialog box resizing support
enum SizeMode {
  BL, BR, BS, BT=BL, BB=BR
};

struct ControlInfo {
  int	      id;
  SizeMode    borders[4]; // left, top, right, bottom
  int	      flags; // redraw flags
  RECT	      rc;
};

static ControlInfo  g_controls[]={
  { IDC_FILELIST, { BL, BT, BR, BB } },
  { IDC_MSG, { BL, BB, BR, BB }, TRUE },
  { IDC_VALIDATE, { BR, BB, BS, BS }, TRUE },
  { IDCANCEL, { BR, BB, BS, BS }, TRUE },
  { IDC_STATUS, { BL, BB, BR, BB }, TRUE },
};
#define NCONTROLS (sizeof(g_controls)/sizeof(g_controls[0]))

static void   ScreenToClient(HWND hWnd,RECT& rc) {
  POINT	  pt1={rc.left,rc.top};
  POINT	  pt2={rc.right,rc.bottom};
  ::ScreenToClient(hWnd,&pt1);
  ::ScreenToClient(hWnd,&pt2);
  rc.left=pt1.x; rc.top=pt1.y;
  rc.right=pt2.x; rc.bottom=pt2.y;
}

static void   SizeInit(HWND hDlg, ControlInfo *ii, int nitems) {
  RECT	cli;
  ::GetClientRect(hDlg,&cli);

  for (;nitems;--nitems,++ii) {
    RECT  rc;
    ::GetWindowRect(::GetDlgItem(hDlg,ii->id),&rc);
    ::ScreenToClient(hDlg,rc);

    // left
    switch (ii->borders[0]) {
    case BL: ii->rc.left=rc.left-cli.left; break;
    case BR: ii->rc.left=cli.right-rc.left; break;
    default: assert(0); break;
    }

    // top
    switch (ii->borders[1]) {
    case BT: ii->rc.top=rc.top-cli.top; break;
    case BB: ii->rc.top=cli.bottom-rc.top; break;
    default: assert(0); break;
    }

    // right
    switch (ii->borders[2]) {
    case BL: ii->rc.right=rc.right-cli.left; break;
    case BR: ii->rc.right=cli.right-rc.right; break;
    case BS: ii->rc.right=rc.right-rc.left; break;
    }

    // bottom
    switch (ii->borders[3]) {
    case BT: ii->rc.bottom=rc.bottom-cli.top; break;
    case BB: ii->rc.bottom=cli.bottom-rc.bottom; break;
    case BS: ii->rc.bottom=rc.bottom-rc.top; break;
    }
  }
}

static void   SizeDialog(HWND hDlg, ControlInfo *ii,int nitems) {
  RECT	cli;
  ::GetClientRect(hDlg,&cli);

  for (;nitems;--nitems,++ii) {
    RECT    rc;
    ::GetWindowRect(::GetDlgItem(hDlg,ii->id),&rc);
    int	    oldw=rc.right-rc.left;

    // left
    switch (ii->borders[0]) {
    case BL: rc.left=cli.left+ii->rc.left; break;
    case BR: rc.left=cli.right-ii->rc.left; break;
    case BS: assert(0); break;
    }

    // top
    switch (ii->borders[1]) {
    case BT: rc.top=cli.top+ii->rc.top; break;
    case BB: rc.top=cli.bottom-ii->rc.top; break;
    case BS: assert(0);
    }

    // right
    switch (ii->borders[2]) {
    case BL: rc.right=cli.left+ii->rc.right; break;
    case BR: rc.right=cli.right-ii->rc.right; break;
    case BS: rc.right=rc.left+ii->rc.right; break;
    }

    // bottom
    switch (ii->borders[3]) {
    case BT: rc.bottom=cli.top+ii->rc.bottom; break;
    case BB: rc.bottom=cli.bottom-ii->rc.bottom; break;
    case BS: rc.bottom=rc.top+ii->rc.bottom; break;
    }

    int	neww=rc.right-rc.left;

    if (!ii->flags && neww<oldw)
      ::SendDlgItemMessage(hDlg,ii->id,LVM_SETCOLUMNWIDTH,0,
	MAKELPARAM(neww-::GetSystemMetrics(SM_CXVSCROLL)-4,0));

    ::MoveWindow(::GetDlgItem(hDlg,ii->id),
      rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
      TRUE);

    if (!ii->flags && neww>oldw)
      ::SendDlgItemMessage(hDlg,ii->id,LVM_SETCOLUMNWIDTH,0,
	MAKELPARAM(neww-::GetSystemMetrics(SM_CXVSCROLL)-4,0));

    if (ii->flags)
      ::RedrawWindow(::GetDlgItem(hDlg,ii->id),NULL,NULL,
	RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);
  }
}

static INT_PTR CALLBACK  DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  NMHDR	      *nmh;

  switch (uMsg) {
  case WM_INITDIALOG: {
    RECT	      rcmsg,rcstatus;

    g_dialog=hDlg;
    // adjust the edit control height to avoid overlapping the status line
    ::GetWindowRect(::GetDlgItem(hDlg,IDC_STATUS),&rcstatus);
    ::GetWindowRect(::GetDlgItem(hDlg,IDC_MSG),&rcmsg);
    rcmsg.bottom=rcstatus.top;
    ScreenToClient(hDlg,rcmsg);
    ::MoveWindow(::GetDlgItem(hDlg,IDC_MSG),rcmsg.left,rcmsg.top,
      rcmsg.right-rcmsg.left,rcmsg.bottom-rcmsg.top,FALSE);
    // initialize resizing structures
    SizeInit(hDlg,g_controls,NCONTROLS);
    ::SendMessage(hDlg,WM_SETICON,ICON_BIG,
      (LPARAM)::LoadIcon(g_instance,MAKEINTRESOURCE(IDI_MAIN)));
    // set list view imagelist
    ::SendDlgItemMessage(hDlg,IDC_FILELIST,LVM_SETIMAGELIST,LVSIL_SMALL,
      (LPARAM)::ImageList_LoadBitmap(g_instance,MAKEINTRESOURCE(IDB_VICONS),
      16,0,RGB(255,0,255)));
    // add one column to list view
    ::GetClientRect(::GetDlgItem(hDlg,IDC_FILELIST),&rcmsg);
    LVCOLUMN  lvc;
    memset(&lvc,0,sizeof(lvc));
    lvc.mask=LVCF_FMT|LVCF_WIDTH;
    lvc.fmt=LVCFMT_LEFT;
    lvc.cx=rcmsg.right-rcmsg.left-::GetSystemMetrics(SM_CXVSCROLL)-4;
    lvc.pszText=_T("File name");
    ::SendDlgItemMessage(hDlg,IDC_FILELIST,LVM_INSERTCOLUMN,0,(LPARAM)&lvc);
    // set startup timer
    g_timer=::SetTimer(hDlg,INIT_TIMER,100,NULL);
    return TRUE; }

  case WM_COMMAND:
    if (HIWORD(wParam)==0 || HIWORD(wParam)==1) {
      switch (LOWORD(wParam)) {
      case IDCANCEL:
	g_stop=true;
	::EndDialog(hDlg,IDCANCEL);
	break;
      case IDC_VALIDATE:
	if (g_validating)
	  g_stop=true;
	else
	  ValidateFiles();
	break;
      }
    }
    break;

  case WM_SIZE:
    SizeDialog(hDlg,g_controls,NCONTROLS);
    break;

  case WM_TIMER:
    if (wParam==g_timer) {
      ::KillTimer(hDlg,g_timer);
      ProcessCommandLine();
      if (g_list_items==0)
	::EndDialog(hDlg,IDCANCEL);
      else
	ValidateFiles();
    } else
      return FALSE;
    break;

  case WM_NOTIFY:
    nmh=(NMHDR *)lParam;
    if (nmh->idFrom==IDC_FILELIST && nmh->code==LVN_GETDISPINFO) {
      NMLVDISPINFO  *lvi=(NMLVDISPINFO*)lParam;

      if (lvi->item.mask & LVIF_TEXT && lvi->item.lParam>=0 && lvi->item.lParam<g_list_items)
      {
	// extract a file title
	const wchar_t	*name=g_file_list[lvi->item.lParam].filename;
	const wchar_t	*slash=wcsrchr(name,_T('\\'));
	if (slash)
	  name=slash+1;

	int		len=lstrlenW(name);
	if (len>4 && lstrcmpiW(name+len-4,L".fb2")==0)
	  len-=4;

	// copy filename
	if (len>lvi->item.cchTextMax-1)
	  len=lvi->item.cchTextMax-1;
	memcpy(lvi->item.pszText,name,len*sizeof(wchar_t));
	lvi->item.pszText[len]=0;
      }
    } else if (nmh->idFrom==IDC_FILELIST && nmh->code==LVN_ITEMCHANGED) {
      NMLISTVIEW  *lvi=(NMLISTVIEW *)lParam;

      if (lvi->iItem>=0 && lvi->lParam>=0 && lvi->lParam<g_list_items &&
	  lvi->uNewState & LVIS_SELECTED)
      {
	FileInfo  *fi=&g_file_list[lvi->lParam];

	if (fi->errmsg && fi->errmsg[0])
	  ::SetDlgItemText(hDlg,IDC_MSG,fi->errmsg);
	else
	  ::SetDlgItemText(hDlg,IDC_MSG,_T("No errors."));

	::SendDlgItemMessage(g_dialog,IDC_STATUS,SB_SETTEXT,0,(LPARAM)fi->filename);
      }
    }
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

// SAX error handler
class SAXEH : public MSXML2::ISAXErrorHandler {
public:
  SAXEH() : m_error_msg(0) { }
  ~SAXEH() { ::SysFreeString(m_error_msg); }

  // dummy IUnknown
  STDMETHOD(QueryInterface)(REFIID iid,void **ppvObject) {
    if (iid==IID_IUnknown || iid==__uuidof(MSXML2::ISAXErrorHandler)) {
      *ppvObject=this;
      return S_OK;
    }
    return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG,AddRef)() { return 1; }
  STDMETHOD_(ULONG,Release)() { return 1; }

  // ISAXErrorHandler
  STDMETHOD(raw_error)(MSXML2::ISAXLocator *loc, USHORT *msg, HRESULT hr) {
    SetMsg(loc,msg,hr);
    return E_FAIL;
  }
  STDMETHOD(raw_fatalError)(MSXML2::ISAXLocator *loc, USHORT *msg, HRESULT hr) {
    SetMsg(loc,msg,hr);
    return E_FAIL;
  }
  STDMETHOD(raw_ignorableWarning)(MSXML2::ISAXLocator *loc, USHORT *msg, HRESULT hr) {
    SetMsg(loc,msg,hr);
    return E_FAIL;
  }

  BSTR	    m_error_msg;

  void	  SetMsg(MSXML2::ISAXLocator *loc,const USHORT *msg,HRESULT hr) {
    if (m_error_msg)
      return;

    int	  line=loc->getLineNumber();
    int	  col=loc->getColumnNumber();

    if (line>0 && col>0) {
      wchar_t	buffer[2048];

      _snwprintf(buffer,sizeof(buffer)/sizeof(buffer[0]),L"At line %d, column %d:\r\n%s",
	  line,col,msg);

      // delete namespace references
      for (wchar_t *cp=buffer;*cp;++cp)
	if (*cp==L'{') {
	  if (wcsncmp(cp+1,FBNS,FBNS_len)==0 && cp[1+FBNS_len]==L'}') {
	    int	  len=lstrlen(cp+FBNS_len+2);
	    memmove(cp,cp+FBNS_len+2,len*sizeof(wchar_t));
	    cp[len]=0;
	    --cp; // restart from copied char
	  } else if (wcsncmp(cp+1,XLINKNS,XLINKNS_len)==0 && cp[1+XLINKNS_len]==L'}') {
	    int	  len=lstrlen(cp+XLINKNS_len+2);
	    memmove(cp,cp+XLINKNS_len+2,len*sizeof(wchar_t));
	    cp[len]=0;
	    --cp; // restart from copied char
	  }
	}

      m_error_msg=::SysAllocString(buffer);
    } else
      m_error_msg=::SysAllocString((const OLECHAR *)msg);
  }
};

static void   SetItemState(int idx,FileInfo *fi,VState state) {
  if (fi->state==state)
    return;

  fi->state=state;

  LVITEM	  lvi;
  memset(&lvi,0,sizeof(lvi));

  lvi.iItem=idx; // XXX assumes index==param, should really use LVM_FINDITEM
  lvi.mask=LVIF_IMAGE;
  lvi.iImage=fi->state;

  ::SendDlgItemMessage(g_dialog,IDC_FILELIST,LVM_SETITEM,0,(LPARAM)&lvi);
}

static void   SetCOMError(int idx,FileInfo *fi,_com_error& e) {
  wchar_t    buffer[1024];

  _snwprintf(buffer,sizeof(buffer)/sizeof(buffer[0]),L"COM Error: %x [%s]",
      e.Error(),(const wchar_t *)e.Description());

  fi->errmsg=::SysAllocString(buffer);
  SetItemState(idx,fi,INVALID);
}

static void   ValidateFiles() {
  g_validating=true;
  g_stop=false;
  ::SetDlgItemText(g_dialog,IDC_VALIDATE,_T("&Stop"));

  // create an error handler
  SAXEH     eh;

  try {
    MSXML2::IXMLDOMSchemaCollection2Ptr scol;
    CheckError(scol.CreateInstance(L"Msxml2.XMLSchemaCache.6.0"));

    // load fictionbook schema
    scol->add(FBNS,g_schema_file);

    // create a SAX reader
    MSXML2::ISAXXMLReaderPtr	  rdr;
    CheckError(rdr.CreateInstance(L"Msxml2.SAXXMLReader.6.0"));

    // attach a schema
    rdr->putFeature((USHORT*)L"schema-validation",VARIANT_TRUE);
    rdr->putProperty((USHORT*)L"schemas",scol.GetInterfacePtr());
    rdr->putFeature((USHORT*)L"exhaustive-errors",VARIANT_TRUE);

    rdr->putErrorHandler(&eh);

    for (int i=0;!g_stop && i<g_list_items;++i) {
      FileInfo  *fi=&g_file_list[i];

      bool  fDV=fi->state==UNCHECKED;

      if (!fDV) {
	HANDLE  hFile=::CreateFile(fi->filename,FILE_READ_ATTRIBUTES,0,NULL,OPEN_EXISTING,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE) {
	  FILETIME	tm;
	  ULARGE_INTEGER	sz;
	  ::GetFileTime(hFile,NULL,NULL,&tm);
	  sz.LowPart=::GetFileSize(hFile,&sz.HighPart);
	  ::CloseHandle(hFile);

	  fDV=tm.dwHighDateTime!=fi->timestamp.dwHighDateTime ||
	      tm.dwLowDateTime!=fi->timestamp.dwLowDateTime ||
	      sz.QuadPart!=fi->size.QuadPart;

	  fi->timestamp=tm;
	  fi->size=sz;
	}
      }

      if (fDV) {
	::SendDlgItemMessage(g_dialog,IDC_STATUS,SB_SETTEXT,0,(LPARAM)fi->filename);
	try {
	  rdr->parseURL((USHORT*)fi->filename);
	  SetItemState(i,fi,VALID);
	}
	catch (_com_error& e) {
	  if (eh.m_error_msg) {
	    fi->errmsg=eh.m_error_msg;
	    eh.m_error_msg=0;
	    SetItemState(i,fi,INVALID);
	  } else
	    SetCOMError(i,fi,e);
	}
	DoEvents();
      }
    }
  }
  catch (_com_error& e) {
    wchar_t	buffer[1024];
    _snwprintf(buffer,sizeof(buffer)/sizeof(buffer[0]),L"COM Error: %x [%s]",
	e.Error(),(const wchar_t *)e.Description());
    ::MessageBox(g_dialog,buffer,_T("Error"),MB_OK|MB_ICONERROR);
  }

  ::SendDlgItemMessage(g_dialog,IDC_STATUS,SB_SETTEXT,0,(LPARAM)_T("Done."));
  ::SetDlgItemText(g_dialog,IDC_VALIDATE,_T("Re&validate"));
  g_validating=false;
}