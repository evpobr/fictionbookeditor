#pragma once 

#include "targetver.h"

// check for unicode
#ifndef UNICODE
#error This program requires unicode support to run
#endif

// Insert your headers here
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _ATL_FREE_THREADED

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS
#pragma warning(disable : 4996)

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlstr.h>
#include <atlpath.h>
#include <atlcoll.h>
#include <atlfile.h>
#include <atlcomtime.h>
#include <atlimage.h>
#include <atlutil.h>
#include "atlimageex.h"

#include <shellapi.h>
#include <Wininet.h>
#include <Urlmon.h>

#include <atlapp.h>

extern CAppModule _Module;

#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#include <atlmisc.h>
#include <atluser.h>

#include <atlframe.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlsplit.h>
#include <atlddx.h>

#include <atltheme.h>

// C library
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>

// C++ library
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <deque>
#include <exception>

// Scintilla
#include "Scintilla.h"
#include "SciLexer.h"

// Hunspell
#include "hunspell.h"

// PCRE
#ifdef USE_PCRE
#include "pcre.h"
#endif

// MSXML
#import <msxml6.dll>

// vb regexps
#ifndef USE_PCRE
#import "progid:VBScript.RegExp"
#endif

// mshtml additional includes
#include <exdispid.h>
#include <mshtmdid.h>
#include <mshtmcid.h>
#import <shdocvw.dll> no_auto_exclude rename_namespace("SHD") rename("FindText","FindTextX")
#import <mshtml.tlb> no_auto_exclude rename("TranslateAccelerator","TranslateAcceleratorX") rename("min", "minX") rename("max", "maxX")

#include <gl\gl.h>
#include <gl\glu.h>

// use com utils
using namespace _com_util;

// extra defines
#ifndef I_IMAGENONE
#define	I_IMAGENONE -1
#endif
#ifndef BTNS_BUTTON
#define	BTNS_BUTTON TBSTYLE_BUTTON
#endif
#ifndef BTNS_AUTOSIZE
#define BTNS_AUTOSIZE TBSTYLE_AUTOSIZE
#endif
#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT 0x0040
#endif
#ifndef SPI_GETDROPSHADOW
#define	SPI_GETDROPSHADOW 0x1024
#endif

#define  UIS_WM_UPDATE_PROGRESS_UI	(WM_APP + 0x100)
#define  WM_RESIZE_OPENGL_WINDOW	(WM_APP + 0x101)

// scripting support
#include <activscp.h>

// XML serialization
#include "XMLSerializer/XMLSerializer.h"

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
