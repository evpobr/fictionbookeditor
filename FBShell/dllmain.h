#pragma once

class CFBShellModule : public ATL::CAtlDllModuleT< CFBShellModule >
{
public:
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FBSHELL, "{BE36E0D7-CE35-4586-A3F7-5883E21A9A34}")
};

extern class CFBShellModule _AtlModule;
