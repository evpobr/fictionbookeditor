#include "stdafx.h"
#include "OptionsPropertySheet.h"


COptionsPropertySheet::COptionsPropertySheet(_U_STRINGorID title, UINT uStartPage, HWND hWndParent)
	: CPropertySheetImpl<COptionsPropertySheet>(title, uStartPage, hWndParent)
{
	AddPage(m_pgView);
	AddPage(m_pgOther);
}
