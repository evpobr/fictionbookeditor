// SettingsHotkeysDlg.h : Declaration of the CSettingsHotkeysPage
#pragma once

#include "resource.h"
#include <vector>

class CHotkey;
class CHotkeysGroup;

class CAccelEdit : public CWindowImpl<CAccelEdit, CEdit, CControlWinTraits>, public CEditCommands<CAccelEdit>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())

	unsigned int virtkey;

	CAccelEdit()
	    : virtkey(0)
	{
	}

	BEGIN_MSG_MAP(CAccelEdit)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp) // Provides VK_ instead of FVIRT
		MESSAGE_HANDLER(WM_SYSKEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_PASTE, OnSkip)
		MESSAGE_HANDLER(WM_CHAR, OnSkip)
		CHAIN_MSG_MAP(CEditCommands<CAccelEdit>)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSkip(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
};

struct hkIndex
{
	int group;
	int hotkey;
};

// CSettingsHotkeysPage
class CSettingsHotkeysPage : public CPropertyPageImpl<CSettingsHotkeysPage>
{
public:
	CListBox m_hkGroups;
	CListBox m_hotkeys;
	CAccelEdit m_editHotkey;

	CSimpleMap<CString, CHotkey *> m_mapHotkeys;
	std::vector<CHotkeysGroup> m_initHkGroups;

	int m_count;
	ACCEL m_accel;
	CString m_wrongHkMsg;
	size_t m_selGr;
	size_t m_selHk;

	CSettingsHotkeysPage();

	enum
	{
		IDD = IDD_SETTINGS_HOTKEYS
	};

	BEGIN_MSG_MAP(CSettingsHotkeysPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_USER + 0x401, OnKeyPressed)
		MESSAGE_HANDLER(WM_USER + 0x402, OnKeyReleased)

		COMMAND_HANDLER(IDC_LIST_HOTKEYS_GROUPS, LBN_SELCHANGE, OnGroupsSelChange)
		COMMAND_HANDLER(IDC_LIST_HOTKEYS, LBN_SELCHANGE, OnHotkeysSelChange)
		COMMAND_HANDLER(IDC_EDIT_HOTKEY, EN_SETFOCUS, OnEditSetFocus)
		COMMAND_HANDLER(IDC_BUTTON_HOTKEY_DELETE, BN_CLICKED, OnBnClickedButtonHotkeyDelete)

		COMMAND_HANDLER(IDC_BUTTON_DEFAULT, BN_CLICKED, OnBnClickedButtonDefault)
		COMMAND_HANDLER(IDC_BUTTON_HOTKEY_ASSIGN, BN_CLICKED, OnBnClickedButtonHotkeyAssign)
		CHAIN_MSG_MAP(CPropertyPageImpl<CSettingsHotkeysPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnKeyPressed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnKeyReleased(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

	LRESULT OnGroupsSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
	LRESULT OnHotkeysSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
	LRESULT OnBnClickedButtonDefault(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonHotkeyDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonHotkeyAssign(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

	LRESULT OnEditSetFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

	hkIndex GetCollIndex(ACCEL);
	bool TestAndSet();
	int GetTextLen(CString text);
	bool Test();
	void ClearAndSet();

	int OnApply();
	void OnReset();
};
