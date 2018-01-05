#pragma once

class CCustomSaveDialog : public CFileDialogImpl<CCustomSaveDialog>
{
public:
	HWND	      m_hDlg;
	CString     m_template;
	bool	      m_includedesc;
	int	      m_tocdepth;

	CCustomSaveDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialogImpl<CCustomSaveDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent),
		m_hDlg(NULL), m_includedesc(true), m_tocdepth(1)
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_CUSTOMSAVEDLG);
	}

	BEGIN_MSG_MAP(CCustomSaveDialog)
		if (uMsg == WM_INITDIALOG)
			return OnInitDialog(hWnd, uMsg, wParam, lParam);

	MESSAGE_HANDLER(WM_SIZE, OnSize)

		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse);

	CHAIN_MSG_MAP(CFileDialogImpl<CCustomSaveDialog>)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
		// save window handles
		m_hDlg = hWnd;

		// read saved template name
		m_template = U::QuerySV(_Settings, _T("Template"), U::GetProgDirFile(_T("html.xsl")));
		SetDlgItemText(IDC_TEMPLATE, m_template);
		m_includedesc = U::QueryIV(_Settings, _T("IncludeDesc"), 1) != 0;
		SendDlgItemMessage(IDC_DOCINFO, BM_SETCHECK,
			m_includedesc ? BST_CHECKED : BST_UNCHECKED, 0);
		m_tocdepth = U::QueryIV(_Settings, _T("TOCDepth"), 1);
		SetDlgItemInt(IDC_TOCDEPTH, m_tocdepth, FALSE);
		return TRUE;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		// make combobox the same size as std controls
		RECT    rc_std, rc_my;
		HWND    hCB = ::GetDlgItem(m_hDlg, IDC_TEMPLATE);
		::GetWindowRect(hCB, &rc_my);
		::GetWindowRect(GetFileDialogWindow().GetDlgItem(cmb1), &rc_std);
		POINT   pt = { rc_my.left,rc_my.top };
		::ScreenToClient(m_hDlg, &pt);
		::MoveWindow(hCB, pt.x, pt.y, rc_std.right - rc_std.left, rc_my.bottom - rc_my.top, TRUE);
		hCB = ::GetDlgItem(m_hDlg, IDC_BROWSE);
		::GetWindowRect(hCB, &rc_my);
		::MoveWindow(hCB,
			pt.x + rc_std.right - rc_std.left + 10, pt.y,
			rc_my.right - rc_my.left, rc_my.bottom - rc_my.top, TRUE);

		return 0;
	}

	LRESULT OnBrowse(WORD, WORD, HWND, BOOL&) {
		CFileDialog	dlg(
			TRUE,
			_T("xsl"),
			NULL,
			OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
			_T("XSL Templates (*.xsl;*.xslt)\0*.xsl;*.xslt\0All files (*.*)\0*.*\0\0")
			);

		if (dlg.DoModal(*this) == IDOK)
			::SetDlgItemText(m_hDlg, IDC_TEMPLATE, dlg.m_szFileName);

		return 0;
	}

	BOOL OnFileOK(LPOFNOTIFY /*on*/) {
		m_template = U::GetWindowText(::GetDlgItem(m_hDlg, IDC_TEMPLATE));
		_Settings.SetStringValue(_T("Template"), m_template);
		m_includedesc = ::SendDlgItemMessage(m_hDlg, IDC_DOCINFO, BM_GETCHECK, 0, 0) == BST_CHECKED;
		_Settings.SetDWORDValue(_T("IncludeDesc"), m_includedesc);
		m_tocdepth = ::GetDlgItemInt(m_hDlg, IDC_TOCDEPTH, NULL, FALSE);
		if (m_tocdepth<0)
			m_tocdepth = 0;
		if (m_tocdepth>10)
			m_tocdepth = 10;
		_Settings.SetDWORDValue(_T("TOCDepth"), m_tocdepth);
		return TRUE;
	}
};

