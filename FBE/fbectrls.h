#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlframe.h>
#include <wtl/atlctrls.h>

typedef CWinTraits<WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT, WS_EX_CLIENTEDGE> CCustomEditWinTraits;

class CCustomEdit : public CWindowImpl<CCustomEdit, CEdit, CCustomEditWinTraits>, public CEditCommands<CCustomEdit>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())

	CCustomEdit() { }

	BEGIN_MSG_MAP(CCustomEdit)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		CHAIN_MSG_MAP_ALT(CEditCommands<CCustomEdit>, 1)
	END_MSG_MAP()

	LRESULT OnChar(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
	{
		if (wParam == VK_RETURN)
			::PostMessage(::GetParent(GetParent()), WM_COMMAND, MAKELONG(GetDlgCtrlID(), IDN_ED_RETURN), (LPARAM)m_hWnd);

		bHandled = FALSE;
		return 0;
	}
};

class CCustomStatic : public CWindowImpl<CCustomStatic, CStatic/*,CCustomStaticWinTraits*/>
{
private:
	HFONT m_font;
	bool m_enabled;
public:
	CCustomStatic() :m_font(0), m_enabled(0) {}

	void DoPaint(CDCHandle dc)
	{
		RECT rc;
		GetClientRect(&rc);
		/*HBRUSH hBr = GetSysColorBrush(COLOR_3DFACE);
		HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
		HBRUSH oldBrush = (HBRUSH)SelectObject(dc, hBr);
		HPEN oldPen = (HPEN)SelectObject(dc, pen);
		Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(dc, oldBrush);
		SelectObject(dc, oldPen);*/
		HFONT oldFont = (HFONT)SelectObject(dc, m_font);

		UINT iFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;

		int len = GetWindowTextLength();
		wchar_t* text = new wchar_t[len + 1];
		GetWindowText(text, len + 1);

		dc.SetBkMode(TRANSPARENT);
		if (m_enabled)
		{
			dc.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		}
		else
		{
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		}
		dc.DrawText(text, -1, &rc, iFlags);
		SelectObject(dc, oldFont);
		delete[]text;
	}

	LRESULT OnPaint(UINT, WPARAM wParam, LPARAM, BOOL&)
	{
		if (wParam != NULL) {
			DoPaint((HDC)wParam);
		}
		else {
			CPaintDC dc(m_hWnd);
			DoPaint(dc.m_hDC);
		}
		return 0;
	}

	void SetFont(HFONT pFont)
	{
		m_font = pFont;
	}

	void SetEnabled(bool Enabled = true)
	{
		m_enabled = Enabled;
		Invalidate();
	}

	BEGIN_MSG_MAP(CCustomStatic)
		//MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
};

class CTableToolbarsWindow : public CFrameWindowImpl<CTableToolbarsWindow>,
	public CUpdateUI<CTableToolbarsWindow>
{
public:
	BEGIN_UPDATE_UI_MAP(CTableToolbarsWindow)

	END_UPDATE_UI_MAP()
};
