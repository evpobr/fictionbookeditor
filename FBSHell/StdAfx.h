// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__AA787312_D02C_4332_A4DD_1B1AA8B9E8BF__INCLUDED_)
#define AFX_STDAFX_H__AA787312_D02C_4332_A4DD_1B1AA8B9E8BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// check for unicode
#ifndef UNICODE
#error This program requires unicode support to run
#endif

#include "targetver.h"

// we are MT by default
#define _ATL_APARTMENT_THREADED

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <atlbase.h>
#include <atlstr.h>

#include <atlcom.h>

#include <atlapp.h>
#include <atlgdi.h>

#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include <atlxml.h>

#include <msxml6.h>

_COM_SMARTPTR_TYPEDEF(ISAXXMLReader, __uuidof(ISAXXMLReader));
#define SHARED_HANDLERS 

#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>

#include <atlimage.h>
#include <WinCrypt.h>

using namespace _com_util;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AA787312_D02C_4332_A4DD_1B1AA8B9E8BF__INCLUDED_)

