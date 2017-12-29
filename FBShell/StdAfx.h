#pragma once

#include "targetver.h"

// we are MT by default
#define _ATL_APARTMENT_THREADED

#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlimage.h>

#include <msxml6.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>
#include <thumbcache.h>
#include <WinCrypt.h>

#include <wtl/atlapp.h>
#include <wtl/atldlgs.h>
