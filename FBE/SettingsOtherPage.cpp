// SettingsOtherDlg.cpp : Implementation of CSettingsOtherPage

#include "stdafx.h"
#include "SettingsOtherPage.h"
#include "utils.h"
#include "Settings.h"
#include "res1.h"

// {715BECF4-911B-457E-9766-78F273F25938}
static const GUID GUID_ScriptDialog =
{
	0x715becf4, 0x911b, 0x457e, { 0x97, 0x66, 0x78, 0xf2, 0x73, 0xf2, 0x59, 0x38 }
};

extern CSettings _Settings;

// CSettingsOtherPage

CSettingsOtherPage::CSettingsOtherPage()
{
	SetTitle(IDS_SETTINGS_OTHER_CAPTION);
	m_scripts_fld_dlg_msg.GetBufferSetLength(MAX_LOAD_STRING + 1);
}

LRESULT CSettingsOtherPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_keep = GetDlgItem(IDC_KEEP);
	m_def_enc = GetDlgItem(IDC_DEFAULT_ENC);
	m_restore_pos = GetDlgItem(IDC_RESTORE_POS);
	m_def_scripts_fld = GetDlgItem(IDC_DEFAULT_SCRIPTS_FOLDER);
	m_scripts_folder = GetDlgItem(IDC_SCRIPTS_FOLDER_PATH);
	m_scripts_folder_sel = GetDlgItem(IDC_SELECT_SCRIPTS_FOLDER_BUTTON);

	// added by SeNS
	m_nbsp_char = GetDlgItem(IDC_NBSP_CHAR);
	m_change_keyb = GetDlgItem(IDC_CHANGE_KEYB);

	m_image_type = GetDlgItem(IDC_IMAGETYPE);
	m_jpeg_quality = GetDlgItem(IDC_JPEGQUALITY);
	m_updown = GetDlgItem(IDC_JPEGSPIN);

	m_keyb_layout = GetDlgItem(IDC_KEYB_LAYOUT);

	::SendMessage(GetDlgItem(IDC_SETTINGS_ASKIMAGE), BM_SETCHECK, 
				_Settings.GetInsImageAsking() ? BST_CHECKED : BST_UNCHECKED, 0);

	::EnableWindow(GetDlgItem(IDC_OPTIONS_CLEARIMGS), !IsDlgButtonChecked(IDC_SETTINGS_ASKIMAGE));
	::SendMessage(GetDlgItem(IDC_OPTIONS_CLEARIMGS), BM_SETCHECK, 
				_Settings.GetIsInsClearImage() ? BST_CHECKED : BST_UNCHECKED, 0);
	
    wchar_t buf[MAX_LOAD_STRING + 1];
	if (::LoadString(_Module.GetResourceInstance(),IDS_ENCODINGS,buf,sizeof(buf)/sizeof(buf[0])))
	{
		TCHAR   *cp=buf;
		while (*cp) 
		{
			size_t len=_tcscspn(cp,_T(","));
			if (cp[len])
			cp[len++]=_T('\0');
			if (*cp)
			{
				m_def_enc.AddString(cp);
			}
			cp+=len;
		}
	}
	
	m_def_enc.SelectString(0, _Settings.GetDefaultEncoding());
	m_keep.SetCheck(_Settings.KeepEncoding() ? BST_CHECKED : BST_UNCHECKED);
	m_restore_pos.SetCheck(_Settings.RestoreFilePosition() ? BST_CHECKED : BST_UNCHECKED);

	_Settings.m_initial_scripts_folder = _Settings.GetScriptsFolder();
	m_def_scripts_fld.SetCheck(_Settings.IsDefaultScriptsFolder());
	m_scripts_folder.SetWindowText(_Settings.m_initial_scripts_folder);
	m_scripts_folder.SetReadOnly(_Settings.IsDefaultScriptsFolder());
	m_scripts_folder_sel.EnableWindow(!_Settings.IsDefaultScriptsFolder());
	m_scripts_switched = _Settings.IsDefaultScriptsFolder();

	// added by SeNS
	m_nbsp_char.AddString(L"□");  // \u25A1
	m_nbsp_char.AddString(L"▫");  // \u25AB
	m_nbsp_char.AddString(L"◦");  // \u26E6
	m_nbsp_char.AddString(L"\u00A0");  // original nbsp
	m_nbsp_char.SelectString (0, _Settings.GetNBSPChar());
	m_change_keyb.SetCheck(_Settings.GetChangeKeybLayout());

	m_image_type.AddString(L"PNG");
	m_image_type.AddString(L"JPEG");
	m_image_type.SetCurSel(_Settings.GetImageType());
	CString quality;
	quality.Format(L"%d", _Settings.GetJpegQuality());
	m_jpeg_quality.SetWindowText(quality);
	m_updown.SetRange(20, 100);

	// process keyboard layouts
	TCHAR name[255];
	HKL hLayouts[16];
	int nLayouts = GetKeyboardLayoutList(16, &hLayouts[0]);
	for (int i=0; i<nLayouts; i++)
	{
        // bottom 16 bit of HKL is LANGID
		LANGID language = (LANGID)(((UINT)hLayouts[i]) & 0x0000FFFF);
		LCID locale = MAKELCID(language, SORT_DEFAULT);
		GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, 255);
		CString layoutName(name);
		m_keyb_layout.AddString(layoutName);
		m_keyb_layout.SetItemData(i, locale);
	}
	m_keyb_layout.SetCurSel(0);
	for (int i=0; i<nLayouts; i++)
		if (m_keyb_layout.GetItemData(i) == _Settings.GetKeybLayout())
		{
			m_keyb_layout.SetCurSel(i);
			break;
		}
	
	::LoadString(_Module.GetResourceInstance(), IDS_CHOOSE_SCRIPTS_FLD, m_scripts_fld_dlg_msg.GetBuffer(), MAX_LOAD_STRING + 1);

	return 1;
}

LRESULT CSettingsOtherPage::OnBnClickedDefaultScriptsFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!m_scripts_switched)
	{
		CString path;
		m_scripts_folder.GetWindowText(path);
		if(path != _Settings.GetDefaultScriptsFolder())
		{
			m_scripts_folder.SetWindowText(_Settings.GetDefaultScriptsFolder());
		}
		
		_Settings.SetScriptsFolder(_Settings.GetDefaultScriptsFolder(), true);
		m_scripts_folder.SetReadOnly(true);
		m_scripts_folder_sel.EnableWindow(false);
	}
	else
	{
		m_scripts_folder.SetReadOnly(false);
		m_scripts_folder_sel.EnableWindow(true);
	}

	m_scripts_switched = !m_scripts_switched;
	return 0;
}

LRESULT CSettingsOtherPage::OnBnClickedSelectScriptsFolderButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strPath;
	m_scripts_folder.GetWindowText(strPath);

	if (RunTimeHelper::IsVista())
	{
		CShellFileOpenDialog dlg(NULL, FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		dlg.GetPtr()->SetClientGuid(GUID_ScriptDialog);
		dlg.GetPtr()->SetTitle(m_scripts_fld_dlg_msg);
		CComPtr<IShellItem> spShellItem;
		PIDLIST_ABSOLUTE pidl = { 0 };
		HRESULT hr = SHParseDisplayName(strPath, NULL, &pidl, 0, NULL);
		if (SUCCEEDED(hr))
		{
			if (SUCCEEDED(SHCreateShellItem(NULL, NULL, pidl, &spShellItem)))
			{
				dlg.GetPtr()->SetFolder(spShellItem);
			}
			ILFree(pidl);
		}
		if (dlg.DoModal() == IDOK)
		{
			
			dlg.GetFilePath(strPath);
		}
	}
	else
	{
		CFolderDialog fldDlg(NULL, m_scripts_fld_dlg_msg, BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS);
		fldDlg.SetInitialFolder(strPath);
		if (fldDlg.DoModal(*this) == IDOK)
		{
			CString folderPath(fldDlg.m_szFolderPath);
			folderPath.MakeLower();
			if (!(folderPath.ReverseFind(L'\\') == (folderPath.GetLength() - 1)))
			{
				folderPath.Append(L"\\");
			}

			m_scripts_folder.SetWindowText(folderPath);
		}
	}
	return 0;
}
LRESULT CSettingsOtherPage::OnBnClickedSettingsAskimage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::EnableWindow(GetDlgItem(IDC_OPTIONS_CLEARIMGS), !IsDlgButtonChecked(IDC_SETTINGS_ASKIMAGE));
	if(IsDlgButtonChecked(IDC_SETTINGS_ASKIMAGE))
		::SendMessage(GetDlgItem(IDC_OPTIONS_CLEARIMGS), BM_SETCHECK, BST_UNCHECKED, 0);
	return 0;
}

int CSettingsOtherPage::OnApply()
{
	CString def_enc;
	m_def_enc.GetLBText(m_def_enc.GetCurSel(), def_enc);

	_Settings.SetDefaultEncoding(def_enc);
	_Settings.SetKeepEncoding(m_keep.GetState() != 0);
	_Settings.SetRestoreFilePosition(m_restore_pos.GetState() != 0);

	CString folderPath;
	m_scripts_folder.GetWindowText(folderPath);
	_Settings.SetScriptsFolder(folderPath == "" ? _Settings.GetDefaultScriptsFolder() : folderPath, true);

	if (_Settings.m_initial_scripts_folder != _Settings.GetScriptsFolder())
	{
		_Settings.SetNeedRestart();
	}

	_Settings.SetInsImageAsking(IsDlgButtonChecked(IDC_SETTINGS_ASKIMAGE) != 0);
	_Settings.SetIsInsClearImage(IsDlgButtonChecked(IDC_OPTIONS_CLEARIMGS) != 0);

	// Added by SeNS
	CString s;
	m_nbsp_char.GetWindowText(s);
	_Settings.SetNBSPChar(s);
	_Settings.SetChangeKeybLayout(IsDlgButtonChecked(IDC_CHANGE_KEYB) != 0);

	_Settings.SetImageType(m_image_type.GetCurSel());
	_Settings.SetJpegQuality(m_updown.GetPos());

	int n = m_keyb_layout.GetCurSel();
	_Settings.SetKeybLayout(m_keyb_layout.GetItemData(n));

	return 0;
}

void CSettingsOtherPage::OnReset()
{
	if (_Settings.m_initial_scripts_folder != _Settings.GetScriptsFolder())
	{
		_Settings.SetScriptsFolder(_Settings.m_initial_scripts_folder, true);
	}
}
