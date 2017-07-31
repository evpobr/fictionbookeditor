#pragma once

#include "resource.h"

class COptionsViewPage: public CPropertyPageImpl<COptionsViewPage>
{
public:
	enum { IDD = IDD_OPTIONS };

	COptionsViewPage();
	~COptionsViewPage();

	BEGIN_MSG_MAP(COptionsViewPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_DICTPATH, OnDictPath)
		CHAIN_MSG_MAP(CPropertyPageImpl<COptionsViewPage>)
	END_MSG_MAP()

	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);

	LRESULT OnDictPath(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		MessageBox(L"Hello!");
		return 0;
	}

	int OnApply();
};

class COptionsOtherPage : public CPropertyPageImpl<COptionsOtherPage>
{
public:
	enum { IDD = IDD_SETTING_OTHER };

	BEGIN_MSG_MAP(COptionsViewPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<COptionsOtherPage>)
	END_MSG_MAP()

	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		return TRUE;
	}

	int OnApply()
	{
		return PSNRET_NOERROR;
	}
};

