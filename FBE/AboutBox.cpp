#include "stdafx.h"
#include "AboutBox.h"
#include "Utils.h"

LRESULT CAboutDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL &)
{
	m_bAllowResize = false;

	SetDlgItemText(IDC_STATIC_AB_APPNAMEVER, U::GetProductInfo());

	SetIcon(LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME)));

	m_Contributors = GetDlgItem(IDC_CONTRIBS);
	HRSRC hres = ::FindResource(NULL, L"ABOUT_FILE", L"ABOUT_FILE");
	HGLOBAL hbytes = ::LoadResource(NULL, hres);
	CA2CT contribs((char *)::LockResource(hbytes), 65001); // UTF-8
	CString s(contribs);
	m_Contributors.SetWindowText(s.Left(s.ReverseFind(L'\n') - 1));

	// create OpenGL logo window
	m_glLogo.SubclassWindow(GetDlgItem(IDC_AB_BANNER));
	if (m_glLogo.OpenGLError())
	{
		m_glLogo.UnsubclassWindow(TRUE);
		GetDlgItem(IDC_AB_BANNER).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AB_STATIC_BANNER).ShowWindow(SW_SHOW);
	}
	else
		GetDlgItem(IDC_AB_BANNER).ShowWindow(SW_SHOW);

	::ShowWindow(GetDlgItem(IDC_PIC_UPDATE), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_TEXT_STATUS), SW_HIDE);

	return 0;
}

LRESULT CAboutDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL &)
{
	EndDialog(wID);
	return 0;
}

LRESULT CAboutDlg::OnCtlColor(UINT, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HWND hwndEdit = (HWND)lParam;
	if (hwndEdit == GetDlgItem(IDC_CONTRIBS))
	{
		HDC hdc = (HDC)wParam;
		::SetBkColor(hdc, RGB(255, 255, 255));
		return (LRESULT)::GetStockObject(WHITE_BRUSH);
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CAboutDlg::OnNMClickSyslinkAbLinks(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL &)
{
	PNMLINK pNMLink = (PNMLINK)pNMHDR;
	CString args;

	args.Format(L"url.dll, FileProtocolHandler %s", pNMLink->item.szUrl);
	ShellExecute(NULL, L"open", L"rundll32.exe", args, NULL, SW_SHOW);

	return 0;
}

LRESULT CAboutDlg::OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL &)
{
	if (!m_bAllowResize)
	{
		RECT rect;
		GetWindowRect(&rect);
		LPMINMAXINFO pMMI = (LPMINMAXINFO)lParam;
		pMMI->ptMaxTrackSize.x = rect.right - rect.left;
		pMMI->ptMaxTrackSize.y = rect.bottom - rect.top;
		pMMI->ptMinTrackSize.x = rect.right - rect.left;
		pMMI->ptMinTrackSize.y = rect.bottom - rect.top;
	}
	return TRUE;
}

LRESULT CAboutDlg::OnSize(UINT, WPARAM, LPARAM, BOOL &)
{
	if (m_bAllowResize)
	{
		RECT rect;
		GetClientRect(&rect);
		m_glLogo.SetWindowPos(0, &rect, 0);
	}
	return FALSE;
}

LRESULT CAboutDlg::OnResizeOpenGLWindow(UINT, WPARAM, LPARAM, BOOL &)
{
	CButton btn = GetDlgItem(IDOK);
	// switch glLogo to full client area
	if (btn.IsWindowVisible())
	{
		// hide controls
		btn.ShowWindow(SW_HIDE);
		btn.EnableWindow(FALSE);

		m_SaveBtnState = GetDlgItem(IDC_UPDATE).IsWindowVisible();
		if (m_SaveBtnState)
		{
			GetDlgItem(IDC_UPDATE).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_UPDATE).EnableWindow(FALSE);
		}

		GetDlgItem(IDC_PIC_UPDATE).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEXT_STATUS).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CONTRIBS).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_AB_APPICON).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_AB_APPNAMEVER).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUILDSTAMP).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SYSLINK_AB_LINKS).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_BUILD).ShowWindow(SW_HIDE);

		// save dialog size & position
		GetWindowRect(&m_SaveRect);

		// save control size & position
		m_glLogo.GetWindowRect(&m_LogoRect);
		ScreenToClient(&m_LogoRect);

		RECT rect;
		GetClientRect(&rect);
		m_glLogo.SetWindowPos(0, &rect, 0);
		m_glLogo.SetFocus();

		GetWindowText(m_AboutCaption);
		SetWindowText(m_sLogoCaption);
		ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);
		UpdateWindow();

		m_bAllowResize = true;
	}
	else
	{
		// restore dialog size and position
		SetWindowPos(0, &m_SaveRect, 0);

		m_glLogo.SetWindowPos(0, &m_LogoRect, 0);

		btn.ShowWindow(SW_SHOW);
		btn.EnableWindow(TRUE);

		if (m_SaveBtnState)
		{
			GetDlgItem(IDC_UPDATE).ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UPDATE).EnableWindow(TRUE);
		}

		GetDlgItem(IDC_PIC_UPDATE).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TEXT_STATUS).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CONTRIBS).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_AB_APPICON).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_AB_APPNAMEVER).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BUILDSTAMP).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SYSLINK_AB_LINKS).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_BUILD).ShowWindow(SW_SHOW);

		SetWindowText(m_AboutCaption);
		ModifyStyle(WS_MAXIMIZEBOX, 0, SWP_FRAMECHANGED);

		m_bAllowResize = false;
	}
	return TRUE;
}
