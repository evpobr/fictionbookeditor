// SettingsDlg.h : Declaration of the CSettingsDlg

#pragma once

#include "resource.h"
#include "SettingsViewPage.h"
#include "SettingsOtherPage.h"
#include "SettingsHotkeysPage.h"
#include "SettingsWordsPage.h"

// CSettingsDlg

class CSettingsDlg : public CPropertySheetImpl<CSettingsDlg>
{
public:
	CSettingsDlg(_U_STRINGorID title = (LPCTSTR)NULL,
		UINT uStartPage = 0, HWND hWndParent = NULL);

	BEGIN_MSG_MAP(CSettingsDlg)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CSettingsDlg>)
	END_MSG_MAP()

	void OnShowWindow(BOOL bShowing, int nReason);

private:
	bool m_bCentered;

	CSettingsViewPage m_pgView;
	CSettingsOtherPage m_pgOther;
	CSettingsHotkeysPage m_pgHotkeys;
	CSettingsWordsDlg m_pgWords;
};
