#pragma once

#include "OptionsViewPage.h"

class COptionsPropertySheet: public CPropertySheetImpl<COptionsPropertySheet>
{
public:
	COptionsPropertySheet(_U_STRINGorID title = (LPCTSTR)NULL,
		UINT uStartPage = 0, HWND hWndParent = NULL);

	BEGIN_MSG_MAP(COptionsPropertySheet)
		CHAIN_MSG_MAP(CPropertySheetImpl<COptionsPropertySheet>)
	END_MSG_MAP()

private:
	COptionsViewPage m_pgView;
	COptionsOtherPage m_pgOther;
};

