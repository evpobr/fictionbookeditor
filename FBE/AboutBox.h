#pragma once

#include "GLLogo.h"
#include "resource.h"

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum
	{
		IDD = IDD_ABOUTBOX
	};

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_RESIZE_OPENGL_WINDOW, OnResizeOpenGLWindow)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_HANDLER(IDC_SYSLINK_AB_LINKS, NM_CLICK, OnNMClickSyslinkAbLinks)
	END_MSG_MAP()

private:
	RECT m_SaveRect, m_LogoRect;
	CGLLogoView m_glLogo;
	CEdit m_Contributors;
	int m_AnimIdx;
	int m_retCode;
	BOOL m_SaveBtnState;
	bool m_bAllowResize;
	CString m_AboutCaption;

	CString m_sLogoCaption;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL &);
	LRESULT OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL &);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnCtlColor(UINT, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnNMClickSyslinkAbLinks(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL &);
	LRESULT OnResizeOpenGLWindow(UINT, WPARAM, LPARAM, BOOL &);
};
