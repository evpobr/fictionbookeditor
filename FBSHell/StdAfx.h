#pragma once

#include "targetver.h"

// check for unicode
#ifndef UNICODE
#error This program requires unicode support to run
#endif

// we are MT by default
#define _ATL_MULTI_THREADED

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <atlbase.h>

extern CComModule _Module;

#include <atlapp.h>
#include <atlmisc.h>

#include <atlcom.h>

#include <comutil.h>
#include <comdef.h>

#import <msxml6.dll>

#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>

#include <initguid.h>

#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

#include "resource.h"
#include "ptr.h"
#include "Image.h"
#include "FBShell.h"

#include <wchar.h>

using namespace _com_util;

