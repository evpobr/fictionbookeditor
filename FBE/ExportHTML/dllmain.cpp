#include "stdafx.h"
#include "resource.h"
#include "ExportHTML_i.h"
#include "dllmain.h"
#include "utils.h"

CExportHTMLModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);
		U::InitSettings();
	}
	return _AtlModule.DllMain(dwReason, lpReserved);
}
