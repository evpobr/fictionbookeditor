#pragma once

#include <algorithm>
#include <map>
#include <vector>

#include "XMLSerializer\Serializable.h"
#include "res1.h"
#include "resource.h"
#include "utils.h"

/*const int ILANG_ENGLISH = 0;
const int ILANG_RUSSIAN = 1;*/

class WordsItem : public ISerializable, public IObjectFactory
{
public:
	CString m_word;

	int m_count;
	CString m_sCount;

	int m_percent;
	int m_prc_idx;

	WordsItem();
	WordsItem(CString word, int count);

	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & property);
	bool SetPropertyValue(const CString & sProperty, CProperty & property);
	bool HasMultipleInstances();
	CString GetClassName();
	CString GetID();

	ISerializable * Create();
	void Destroy(ISerializable * obj);

	bool operator==(const WordsItem & wi)
	{
		return m_word.CompareNoCase(wi.m_word) == 0;
	}
};

class CHotkey : public ISerializable, public IObjectFactory
{
public:
	CString m_name;
	CString m_reg_name;
	ACCEL m_accel;
	ACCEL m_def_accel;
	CString m_desc;
	wchar_t m_char_val; // value for symbol hotkey

	CHotkey();
	CHotkey(CString reg_name, int IDS_CMD_NAME, BYTE fVirt, WORD cmd, WORD key, CString descr = L"");
	CHotkey(CString reg_name, int IDS_CMD_NAME, CString uchar, BYTE fVirt, WORD cmd, WORD key, CString descr = L"");
	CHotkey(CString reg_name, CString name, wchar_t symbol, BYTE fVirt, WORD cmd, WORD key, CString descr = L"");
	CHotkey(CString reg_name, CString cmd_name, BYTE fVirt, WORD cmd, WORD key, CString descr = L"");

	bool operator<(const CHotkey & other) const
	{
		return (m_name.CompareNoCase(other.m_name) < 0);
	}

	// ISerializable interface
	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & property);
	bool SetPropertyValue(const CString & sProperty, CProperty & sValue);

	bool HasMultipleInstances();

	CString GetClassName();
	CString GetID();

	ISerializable * Create();
	void Destroy(ISerializable * obj);
};

class CHotkeysGroup : public ISerializable, public IObjectFactory
{
public:
	CString m_name;
	CString m_reg_name;
	std::vector<CHotkey> m_hotkeys;

	CHotkey m_hotkey_factory;
	std::vector<void *> m_ptr_hotkeys;

	CHotkeysGroup();
	CHotkeysGroup(CString reg_name, int IDS_GROUP_NAME);

	bool operator<(const CHotkeysGroup & other) const
	{
		return (m_name.CompareNoCase(other.m_name) < 0);
	}

	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & property);
	bool SetPropertyValue(const CString & sProperty, CProperty & sValue);

	bool HasMultipleInstances();

	CString GetClassName();
	CString GetID();

	ISerializable * Create();
	void Destroy(ISerializable * obj);
};

class DESCSHOWINFO : public ISerializable, public IObjectFactory
{
public:
	std::map<CString, bool> elements;

	DESCSHOWINFO();

	// Default fields showing in description
	void SetDefaults();

	// ISerializable interface
	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool SetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool HasMultipleInstances();
	CString GetClassName();
	CString GetID();

	// IObjectFactory
	ISerializable * Create();
	void Destroy(ISerializable *);
};

class TREEITEMSHOWINFO : public ISerializable, public IObjectFactory
{
public:
	std::map<CString, bool> items;

	TREEITEMSHOWINFO();
	void SetDefaults();

	// ISerializable interface
	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool SetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool HasMultipleInstances();
	CString GetClassName();
	CString GetID();

	// IObjectFactory
	ISerializable * Create();
	void Destroy(ISerializable *);
};

class CSettings : public ISerializable, public IObjectFactory
{
	CRegKey m_key;
	CString m_key_path;

	CString m_default_encoding;

	DWORD m_search_options;

	DWORD m_collorBG;
	DWORD m_collorFG;
	DWORD m_font_size;
	CString m_font;
	CString m_srcfont;

	bool m_view_status_bar;
	bool m_view_doc_tree;

	// added by SeNS
	DWORD m_custom_dict_codepage;
	CString m_nbsp_char;
	CString m_old_nbsp;
	DWORD_PTR m_keyb_layout;
	DWORD m_image_type;
	///

	DWORD m_splitter_pos;
	CString m_toolbars_settings;

	DWORD m_interface_lang_id;

	bool m_need_restart;

	CString m_scripts_folder;

	bool m_show_words_excls;

	WINDOWPLACEMENT m_words_dlg_placement;
	WINDOWPLACEMENT m_wnd_placement;

	DESCSHOWINFO m_desc;
	TREEITEMSHOWINFO m_tree_items;

	// added by SeNS: view dimensions for external helper
	int m_viewWidth, m_viewHeight;
	HWND m_hMainWindow;

public:
	std::vector<CHotkeysGroup> m_hotkey_groups;
	int keycodes; // total number of accelerators
	std::vector<WordsItem> m_words;

	bool m_xml_src_wrap;
	bool m_xml_src_syntaxHL;
	bool m_xml_src_showEOL;
	bool m_xml_src_showSpace;
	bool m_show_line_numbers;
	bool m_xml_src_tagHL;
	bool m_usespell_check;
	bool m_highlght_check;
	CString m_custom_dict;
	bool m_fast_mode;

	bool m_keep_encoding; // save with opened encoding
	bool m_restore_file_position;
	bool m_insimage_ask;
	bool m_ins_clear_image;
	DWORD m_jpeg_quality;
	bool m_change_kbd_layout_check;

public:
	CSettings();
	~CSettings();

	void Init();
	void InitHotkeyGroups();
	void Close();

	// ISerializable interface
	int GetProperties(std::vector<CString> & properties);
	bool GetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool SetPropertyValue(const CString & sProperty, CProperty & sValue);
	bool HasMultipleInstances();
	CString GetClassName();
	CString GetID();

	// IObjectFactory
	ISerializable * Create();
	void Destroy(ISerializable *);
	void SetDefaults();

	void Load();
	void Save();

	void LoadHotkeyGroups();
	void SaveHotkeyGroups();

	CHotkeysGroup * GetGroupByName(const CString & name);
	CHotkey * GetHotkeyByName(const CString & name, CHotkeysGroup & group);

	void LoadWords();
	void SaveWords();

	bool NeedRestart() const;

	bool ViewStatusBar() const;
	bool ViewDocumentTree() const;

	CString GetKeyPath() const;

	CString GetDefaultEncoding() const;
	DWORD GetSearchOptions() const;
	DWORD GetColorBG() const;
	DWORD GetColorFG() const;
	DWORD GetFontSize() const;
	CString GetFont() const;
	CString GetSrcFont() const;
	DWORD GetSplitterPos() const;
	CString GetToolbarsSettings() const;
	const CRegKey & GetKey() const;

	// added by SeNS
	DWORD GetCustomDictCodepage() const;
	CString GetNBSPChar() const;
	CString GetOldNBSPChar() const;
	DWORD GetKeybLayout() const;
	DWORD GetImageType() const;

	bool GetExtElementStyle(const CString & elem) const;
	bool GetWindowPosition(WINDOWPLACEMENT & wpl) const;
	DWORD GetInterfaceLanguageID() const;
	CString GetInterfaceLanguageDllName() const;
	CString GetLocalizedGenresFileName() const;
	CString GetInterfaceLanguageName() const;
	CString GetScriptsFolder() const;
	CString GetDefaultScriptsFolder();
	bool IsDefaultScriptsFolder();
	bool GetShowWordsExcls() const;
	bool GetWordsDlgPosition(WINDOWPLACEMENT & wpl) const;

	bool GetDocTreeItemState(const CString & item, bool default_state);

	void SetKeepEncoding(bool keep, bool apply = false);
	void SetDefaultEncoding(const CString & encoding, bool apply = false);
	void SetSearchOptions(DWORD opt, bool apply = false);
	void SetColorBG(DWORD color, bool apply = false);
	void SetColorFG(DWORD color, bool apply = false);
	void SetFontSize(DWORD size, bool apply = false);
	void SetXmlSrcWrap(bool wrap, bool apply = false);
	void SetXmlSrcSyntaxHL(bool hl, bool apply = false);
	void SetXmlSrcTagHL(bool hl, bool apply = false);
	void SetXmlSrcShowEOL(bool eol, bool apply = false);
	void SetXmlSrcShowSpace(bool eol, bool apply = false);
	void SetFastMode(bool mode, bool apply = false);
	void SetFont(const CString & font, bool apply = false);
	void SetSrcFont(const CString & font, bool apply = false);
	void SetViewStatusBar(bool view, bool apply = false);
	void SetViewDocumentTree(bool view, bool apply = false);
	void SetSplitterPos(DWORD pos, bool apply = false);
	void SetToolbarsSettings(CString & settings, bool apply = false);
	void SetExtElementStyle(const CString & elem, bool ext, bool apply = false);
	void SetWindowPosition(const WINDOWPLACEMENT & wpl, bool apply = false);
	void SetRestoreFilePosition(bool restore, bool apply = false);
	void SetInterfaceLanguage(DWORD Language, bool apply = false);
	void SetScriptsFolder(const CString fullpath, bool apply = false);
	void SetInsImageAsking(const bool value, bool apply = false);
	void SetIsInsClearImage(const bool value, bool apply = false);
	void SetDocTreeItemState(const CString & item, bool state);
	void SetShowWordsExcls(const bool value, bool apply = false);
	void SetWordsDlgPosition(const WINDOWPLACEMENT & wpl, bool apply = false);

	void SetNeedRestart();

	CString m_initial_scripts_folder;

	// added by SeNS
	void SetUseSpellChecker(const bool value, bool apply = false);
	void SetHighlightMisspells(const bool value, bool apply = false);
	void SetCustomDict(const ATL::CString & value, bool apply = false);
	void SetCustomDictCodepage(const DWORD value, bool apply = false);
	void SetNBSPChar(const ATL::CString & value, bool apply = false);
	void SetChangeKeybLayout(const bool value, bool apply = false);
	void SetKeybLayout(const DWORD_PTR value, bool apply = false);
	void SetXMLSrcShowLineNumbers(const bool value, bool apply = false);
	void SetImageType(const DWORD value, bool apply = false);
	void SetJpegQuality(const DWORD value, bool apply = false);

	void SetViewWidth(int width);
	void SetViewHeight(int height);
	int GetViewWidth();
	int GetViewHeight();
	void SetMainWindow(HWND hwnd);
	HWND GetMainWindow();
};
