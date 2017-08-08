#include "stdafx.h"
#include "resource.h"
#include "res1.h"

#include "utils.h"
#include "apputils.h"

//#include <atlgdix.h>

#if _WIN32_WINNT>=0x0501
#include <atltheme.h>
#endif


#include "SettingsViewPage.h"
#include "Settings.h"

extern CSettings _Settings;


static int __stdcall EnumFontProc(const ENUMLOGFONTEX *lfe,
				 const NEWTEXTMETRICEX *ntm,
				 DWORD type,
				 LPARAM data)
{
  CSimpleArray<CString>	*stringList=(CSimpleArray<CString>*)data;
  stringList->Add(lfe->elfLogFont.lfFaceName);
  return TRUE;
}

static int  font_sizes[]={8,9,10,11,12,13,14,15,16,18,20,22,24,26,28,36,48,72};

CSettingsViewPage::CSettingsViewPage()
{
	SetTitle(IDS_SETTINGS_VIEW_CAPTION);
}

LRESULT CSettingsViewPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
  m_fg.SubclassWindow(GetDlgItem(IDC_FG));
  m_bg.SubclassWindow(GetDlgItem(IDC_BG));  
  m_fonts=GetDlgItem(IDC_FONT);
  m_srcfonts=GetDlgItem(IDC_SRCFONT);
  m_fontsize=GetDlgItem(IDC_FONT_SIZE);
  m_lang = GetDlgItem(IDC_LANG);
  // SeNS
  m_custom_dict = GetDlgItem(IDC_CUSTOM_DICT);

  // init color controls
  m_bg.SetDefaultColor(::GetSysColor(COLOR_WINDOW));
  m_fg.SetDefaultColor(::GetSysColor(COLOR_WINDOWTEXT));
  m_bg.SetColor(_Settings.GetColorBG());
  m_fg.SetColor(_Settings.GetColorFG());

  CString strLanguage;
  if(strLanguage.LoadString(IDS_LANG_ENGLISH))
 	m_lang.AddString(strLanguage);
  if(strLanguage.LoadString(IDS_LANG_RUSSIAN))
	m_lang.AddString(strLanguage);
  if(strLanguage.LoadString(IDS_LANG_UKRAINIAN))
	m_lang.AddString(strLanguage);

  if(LANG_RUSSIAN == _Settings.GetInterfaceLanguageID())
	m_lang.SetCurSel(1);
  else if(LANG_UKRAINIAN == _Settings.GetInterfaceLanguageID())
	m_lang.SetCurSel(2);
  else
	m_lang.SetCurSel(0);

  // get font list
  CSimpleArray<CString> installedFonts;
  CDC hdc;
  hdc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
  LOGFONT lf;
  memset(&lf,0,sizeof(lf));
  lf.lfCharSet=ANSI_CHARSET;
  ::EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)&EnumFontProc,(LPARAM)&installedFonts,0);

  for (int i=0; i<installedFonts.GetSize(); i++)
  {
	m_fonts.AddString(installedFonts[i]);
	m_srcfonts.AddString(installedFonts[i]);
  }

  // get body font name
  CString     fnt(_Settings.GetFont());
  int	      idx=m_fonts.FindStringExact(0,fnt);
  if (idx<0) idx=0;
  m_fonts.SetCurSel(idx);

  // get source font name
  fnt.SetString(_Settings.GetSrcFont());
  idx=m_srcfonts.FindStringExact(0,fnt);
  if (idx<0) idx=0;
  m_srcfonts.SetCurSel(idx);


  // init zoom
  int	      m_fsz_val = _Settings.GetFontSize();
  CString     szstr;
  szstr.Format(_T("%d"),m_fsz_val);
  m_fontsize.SetWindowText(szstr);
  for (int i=0;i<sizeof(font_sizes)/sizeof(font_sizes[0]);++i) {
    szstr.Format(_T("%d"),font_sizes[i]);
    m_fontsize.AddString(szstr);
  }

  // init controls
  
  // SeNS
  if (_Settings.m_usespell_check)
  {
	  m_highlight_check.EnableWindow(TRUE);
  }
  else
  {
	  m_highlight_check.EnableWindow(FALSE);
  }

  DoDataExchange(DDX_LOAD);

  return 0;
}

TCHAR FileNameBuffer[_MAX_PATH];

LRESULT CSettingsViewPage::OnShowFileDialog(WORD, WORD, HWND, BOOL&)
{
	CString strFileName;
	m_custom_dict.GetWindowText(strFileName);
	CFileDialog dlg(TRUE, L"dic", strFileName, OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
		L"Dictionaries (*.dic)\0*.dic\0All files (*.*)\0*.*\0\0");
	dlg.m_ofn.Flags &= ~OFN_ENABLEHOOK;
	dlg.m_ofn.lpfnHook = NULL;

	if (dlg.DoModal())
	{
		m_custom_dict.SetWindowText(dlg.m_szFileName);
	}
    return 0;	
}

int CSettingsViewPage::OnApply()
{
	// fetch zoom
	CString   szstr(U::GetWindowText(m_fontsize));
	if (_stscanf(szstr, _T("%d"), &m_fsz_val) != 1 ||
		m_fsz_val<6 || m_fsz_val>72)
	{
		MessageBeep(MB_ICONERROR);
		m_fontsize.SetFocus();
		return 0;
	}

	// save colors to registry
	_Settings.SetColorBG(m_bg.GetColor());
	_Settings.SetColorFG(m_fg.GetColor());

	// save source font face
	m_face = U::GetWindowText(m_srcfonts);
	_Settings.SetSrcFont(m_face);

	// save body font face
	m_face = U::GetWindowText(m_fonts);
	_Settings.SetFont(m_face);

	// save zoom
	_Settings.SetFontSize(m_fsz_val);

	DWORD new_lang;
	switch (m_lang.GetCurSel())
	{
	case 0: new_lang = LANG_ENGLISH; break;
	case 1: new_lang = LANG_RUSSIAN; break;
	case 2: new_lang = LANG_UKRAINIAN; break;
	}

	// если пользователь сменил язык интерфейса....
	if (new_lang != _Settings.GetInterfaceLanguageID())
	{
		// выдаем предупреждение, о том, что надо перезапустить программу.
		//...
		// выставляем флаг перезагрузки программы.
		_Settings.SetNeedRestart();
		_Settings.SetInterfaceLanguage(new_lang);
	}

	DoDataExchange(DDX_SAVE);

	return 0;
}
