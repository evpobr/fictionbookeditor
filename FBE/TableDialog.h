#pragma once

#include "resource.h"

class CTableDlg :
	public CDialogImpl<CTableDlg>,
	public CWinDataExchange<CTableDlg>
{
public:
	enum { IDD = IDD_TABLE };

	CButton	m_chekTitle;
	CEdit m_eRows;
	CUpDownCtrl m_udRows;

	int m_nRows;
	bool m_bTitle;

	BEGIN_MSG_MAP(CTableDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDIT_TABLE_ROWS, IDC_EDIT_TABLE_ROWS, EN_CHANGE, OnEditChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(CTableDlg)
		DDX_INT(IDC_EDIT_TABLE_ROWS, m_nRows)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditChange(UINT uNotifyCode, int nID, CWindow wndCtl);
};

