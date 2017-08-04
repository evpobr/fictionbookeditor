// SettingsDlg.cpp : Implementation of CSettingsDlg

#include "stdafx.h"
#include "Settings.h"
#include "SettingsDlg.h"
#include "SettingsOtherPage.h"
#include "SettingsHotkeysDlg.h"
#include "SettingsWordsPage.h"
#include "res1.h"

extern CSettings _Settings;

// CSettingsDlg

CSettingsDlg::CSettingsDlg(_U_STRINGorID title, UINT uStartPage, HWND hWndParent)
	: CPropertySheetImpl<CSettingsDlg>(title, uStartPage, hWndParent)
{
	m_bCentered = false;

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;

	AddPage(m_pgView);
	AddPage(m_pgOther);
	AddPage(m_pgHotkeys);
	AddPage(m_pgWords);
}

void CSettingsDlg::OnShowWindow(BOOL bShowing, int nReason)
{
	if (bShowing && !m_bCentered)
	{
		m_bCentered = true;
		CenterWindow(m_psh.hwndParent);
	}
}
