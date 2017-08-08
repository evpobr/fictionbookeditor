#ifndef OPTDLG_H
#define OPTDLG_H

// color picker
#include <ColorButton.h>

#include "apputils.h"
#include "utils.h"
#include "FBEView.h"
#include "ModelessDialog.h"

class CSettingsViewPage: public CPropertyPageImpl<CSettingsViewPage>,
	public CWinDataExchange<CSettingsViewPage>
{
public:
  enum { IDD=IDD_SETTINGS_VIEW };

  CSettingsViewPage();

  CColorButton	m_fg,m_bg;
  CComboBox	    m_fonts;
  CComboBox	    m_srcfonts;
  CComboBox	    m_fontsize;
  CComboBox		m_lang;

  CString	    m_face;
  int		    m_fsz_val;

  CButton		m_highlight_check;	// SeNS
  CEdit			m_custom_dict;		// SeNS

  BEGIN_MSG_MAP(CSettingsViewPage)
	COMMAND_HANDLER(IDC_USESPELLCHECKER, BN_CLICKED, OnUseSpellChecker)
	COMMAND_HANDLER(IDC_DICTPATH, BN_CLICKED, OnShowFileDialog)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
 	CHAIN_MSG_MAP(CPropertyPageImpl<CSettingsViewPage>)
	REFLECT_NOTIFICATIONS()
  END_MSG_MAP()

  BEGIN_DDX_MAP(CSettingsViewPage)
	  DDX_CHECK(IDC_WRAP, _Settings.m_xml_src_wrap)
	  DDX_CHECK(IDC_SYNTAXHL, _Settings.m_xml_src_syntaxHL)
	  DDX_CHECK(IDC_SHOWEOL, _Settings.m_xml_src_showEOL)
	  DDX_CHECK(IDC_SHOWWHITESPACE, _Settings.m_xml_src_showSpace)
	  DDX_CHECK(IDC_SHOWLINENUMBERS, _Settings.m_show_line_numbers)
	  DDX_CHECK(IDC_TAGHL, _Settings.m_xml_src_tagHL)
	  DDX_CHECK(IDC_USESPELLCHECKER, _Settings.m_usespell_check)
	  DDX_CHECK(IDC_BACKGROUNDSPELLCHECK, _Settings.m_highlght_check)
	  DDX_TEXT(IDC_CUSTOM_DICT, _Settings.m_custom_dict)
	  DDX_CHECK(IDC_FAST_MODE, _Settings.m_fast_mode)
  END_DDX_MAP()

  LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);

  // SeNS
  LRESULT OnShowFileDialog(WORD, WORD, HWND, BOOL&);
  LRESULT OnUseSpellChecker(WORD, WORD, HWND, BOOL&)
  {
	  BOOL fChecked = (IsDlgButtonChecked(IDC_USESPELLCHECKER) == BST_CHECKED) ? TRUE : FALSE;
	  m_highlight_check.EnableWindow(fChecked);
	  m_custom_dict.EnableWindow(fChecked);

	  return 0;
  }

  int OnApply();

};
#endif