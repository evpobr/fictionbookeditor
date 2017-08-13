#include "stdafx.h"
#include "TableDialog.h"

LRESULT CTableDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	DoDataExchange(FALSE, IDC_EDIT_TABLE_ROWS);

	m_bTitle = true;

	m_chekTitle = GetDlgItem(IDC_CHECK_TABLE_TITLE);
	m_eRows = GetDlgItem(IDC_EDIT_TABLE_ROWS);
	m_udRows = GetDlgItem(IDC_SPIN_TABLE_ROWS);

	m_chekTitle.SetCheck(1);
	m_eRows.SetWindowText(_T("1"));
	m_eRows.SetSelAll(TRUE);
	m_eRows.SetFocus();

	m_udRows.SetRange(1, 1000);
	m_udRows.SetPos(1);

	return 0;
}

LRESULT CTableDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{

	m_bTitle = false;
	if (m_chekTitle.GetCheck() == BST_CHECKED)
	{
		m_bTitle = true;
	}

	EndDialog(wID);
	return IDOK;
}

LRESULT CTableDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	EndDialog(wID);
	return IDCANCEL;
}

LRESULT CTableDlg::OnEditChange(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	static BOOL bAlreadyThere = FALSE;

	if (!bAlreadyThere)
	{
		bAlreadyThere = TRUE;
		DoDataExchange(TRUE, nID);

		static int IDs = IDC_EDIT_TABLE_ROWS;
		if (IDs != nID)
			DoDataExchange(FALSE, IDs);
		bAlreadyThere = FALSE;
	}

	return 0;
}
