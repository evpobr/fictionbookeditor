// SettingsOtherDlg.h : Declaration of the CSettingsOtherPage

#pragma once

#include "resource.h" 
#include <atlhost.h>


// CSettingsOtherPage

class CSettingsOtherPage : 
	public CPropertyPageImpl<CSettingsOtherPage>
{
	CButton		m_keep;
	CComboBox	m_def_enc;
	CButton		m_restore_pos;
	CButton		m_def_scripts_fld;
	CEdit		m_scripts_folder;
	CButton		m_scripts_folder_sel;

	// added by SeNS
	CComboBox   m_nbsp_char;
	CButton		m_change_keyb;
	CComboBox	m_image_type;
	CUpDownCtrl	m_updown;
	CEdit		m_jpeg_quality;
	CComboBox	m_keyb_layout;

	bool		m_scripts_switched;
	CString		m_scripts_fld_dlg_msg;

public:
	CSettingsOtherPage();

	enum { IDD = IDD_SETTINGS_OTHER };

BEGIN_MSG_MAP(CSettingsOtherPage)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_DEFAULT_SCRIPTS_FOLDER, BN_CLICKED, OnBnClickedDefaultScriptsFolder)
	COMMAND_HANDLER(IDC_SELECT_SCRIPTS_FOLDER_BUTTON, BN_CLICKED, OnBnClickedSelectScriptsFolderButton)
	COMMAND_HANDLER(IDC_SETTINGS_ASKIMAGE, BN_CLICKED, OnBnClickedSettingsAskimage)
	CHAIN_MSG_MAP(CPropertyPageImpl<CSettingsOtherPage>)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedDefaultScriptsFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectScriptsFolderButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSettingsAskimage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	int OnApply();
	void OnReset();
};


