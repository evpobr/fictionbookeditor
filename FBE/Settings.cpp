#include "stdafx.h"
#include "XMLSerializer/XMLSerializer.h"

enum KEY_TYPE
{
	KEY_INT,
	KEY_UINT,
	KEY_ULONG,
	KEY_BOOL,
	KEY_STRUCT
};

CString GetStringedProperty(void * member, KEY_TYPE type)
{
	switch (type)
	{
	case KEY_INT:
	{
		CString temp;
		temp.Format(L"%d", *(int *)member);
		return temp;
	}
	case KEY_UINT:
	case KEY_ULONG:
	{
		CString temp;
		temp.Format(L"%u", *(unsigned long *)member);
		return temp;
	}
	case KEY_BOOL:
	{
		CString temp;
		temp.Format(L"%s", *(bool *)member ? L"true" : L"false");
		return temp;
	}
	default:
		return CString();
	}
}

bool StrToBool(CString sValue)
{
	return sValue == L"true";
}

// Settings XML nodes
const wchar_t KEEP_ENCODING_KEY[] = L"KeepEncoding";
const wchar_t DEFAULT_ENCODING_KEY[] = L"DefaultSaveEncoding";
const wchar_t SEARCH_OPTIONS_KEY[] = L"SearchOptions";
const wchar_t COLOR_BG_KEY[] = L"ColorBG";
const wchar_t COLOR_FG_KEY[] = L"ColorFG";
const wchar_t FONT_SIZE_KEY[] = L"FontSize";
const wchar_t XML_SRC_WRAP_KEY[] = L"XMLSrcWrap";
const wchar_t XML_SRC_SYNTAX_HL_KEY[] = L"XMLSrcSyntaxHL";
const wchar_t XML_SRC_TAG_HL_KEY[] = L"XMLSrcTagHL";
const wchar_t XML_SRC_SHOW_EOL_KEY[] = L"XMLSrcShowEOL";
const wchar_t XML_SRC_SHOW_SPACE_KEY[] = L"XMLSrcShowSpace";
const wchar_t FAST_MODE_KEY[] = L"FastMode";
const wchar_t FONT_KEY[] = L"Font";
const wchar_t SRC_FONT_KEY[] = L"SrcFont";
const wchar_t VIEW_STATUS_BAR_KEY[] = L"ViewStatusBar";
const wchar_t VIEW_DOCUMENT_TREE_KEY[] = L"ViewDocumentTree";
const wchar_t SPLITTER_POS_KEY[] = L"SplitterPos";
const wchar_t TOOLBARS_SETTINGS_KEY[] = L"Toolbars";
const wchar_t RESTORE_FILE_POS_KEY[] = L"RestoreFilePosition";
const wchar_t INTERFACE_LANG_KEY[] = L"IntefaceLangID";
const wchar_t SCRIPTS_FOLDER_KEY[] = L"ScriptsFolder";

// Added by SeNS
const wchar_t USESPELLER_CHECK_KEY[] = L"UseSpellChecker";
const wchar_t HIGHLIGHT_CHECK_KEY[] = L"HighlightMisspells";
const wchar_t CUSTOM_DICT_KEY[] = L"CustomDict";
const wchar_t CUSTOM_DICT_CODEPAGE_KEY[] = L"CustomDictCodePage";
const wchar_t NBSPCHAR_KEY[] = L"NBSPChar";
const wchar_t CHANGE_KEYBD_CHECK_KEY[] = L"ChangeKeybLayout";
const wchar_t KEYB_LAYOUT_KEY[] = L"KeyboardLayout";
const wchar_t SHOW_LINE_NUMBERS_KEY[] = L"XMLSrcShowLineNumbers";
const wchar_t IMAGE_TYPE_KEY[] = L"PasteImageType";
const wchar_t JPEG_QUALITY_KEY[] = L"JpegQuality";
//

const wchar_t INSIMAGE_ASKING[] = L"InsImageDialog";
const wchar_t SCRIPTS_HKEY_ERR_NTF[] = L"ScrHkErrDialog";
const wchar_t INS_CLEAR_IMAGE[] = L"InsClearImage";
const wchar_t WINDOW_POSITION[] = L"WindowPosition";
const wchar_t WORDS_DLG_POSITION[] = L"WordsDlgPosition";
const wchar_t SHOW_WORDS_EXCLUSIONS[] = L"ShowWordsExclusions";

// Default values for string settings
const wchar_t DEFAULT_ENCODING[] = L"utf-8";
const wchar_t DEFAULT_FONT[] = L"Trebuchet MS";
const wchar_t DEFAULT_SRCFONT[] = L"Lucida Console";
const wchar_t DEFAULT_SCRIPTS_FOLDER[] = L"Scripts";

// XML serialization filenames
const wchar_t SETTINGS_XML_FILE[] = L"Settings.xml";
const wchar_t WORDS_XML_FILE[] = L"Words.xml";

#include "Settings.h"

#include "ElementDescMnr.h"
extern CElementDescMnr _EDMnr;

CSettings::CSettings()
    : m_need_restart(false)
{
}

CSettings::~CSettings()
{
}

void CSettings::Init()
{
	TCHAR filepath[MAX_PATH];
	DWORD pathlen = ::GetModuleFileName(_Module.GetModuleInstance(), filepath, MAX_PATH);
	TCHAR * appname;
	if (pathlen == 0)
		appname = L"FictionBook Editor";
	else
	{
		CString tmp = U::GetFullPathName(filepath);
		int pos = tmp.ReverseFind(_T('\\'));
		if (pos >= 0)
			tmp.Delete(0, pos + 1);
		pos = tmp.ReverseFind(_T('.'));
		if (pos >= 0)
		{
			const TCHAR * cp = tmp;
			cp += pos;
			if (_tcsicmp(cp, _T(".exe")) == 0 || _tcsicmp(cp, _T(".dll")) == 0)
				tmp.Delete(pos, tmp.GetLength() - pos);
		}
		if (tmp.IsEmpty())
			appname = L"FictionBook Editor";
		else
		{
			lstrcpyn(filepath, tmp, MAX_PATH);
			appname = L"FictionBook Editor" /*filepath*/;
		}
	}
	m_key_path = L"Software\\FBETeam\\";
	m_key_path += appname;
	m_key.Create(HKEY_CURRENT_USER, m_key_path);
}

void CSettings::Close()
{
	m_key.Close();
}

// ISerializable interface
int CSettings::GetProperties(std::vector<CString> & properties)
{
	properties.push_back(KEEP_ENCODING_KEY);
	properties.push_back(DEFAULT_ENCODING_KEY);
	properties.push_back(SEARCH_OPTIONS_KEY);
	properties.push_back(COLOR_BG_KEY);
	properties.push_back(COLOR_FG_KEY);
	properties.push_back(FONT_SIZE_KEY);
	properties.push_back(XML_SRC_WRAP_KEY);
	properties.push_back(XML_SRC_SYNTAX_HL_KEY);
	properties.push_back(XML_SRC_TAG_HL_KEY);
	properties.push_back(XML_SRC_SHOW_EOL_KEY);
	properties.push_back(XML_SRC_SHOW_SPACE_KEY);
	properties.push_back(FAST_MODE_KEY);
	properties.push_back(FONT_KEY);
	properties.push_back(SRC_FONT_KEY);
	properties.push_back(VIEW_STATUS_BAR_KEY);
	properties.push_back(VIEW_DOCUMENT_TREE_KEY);
	properties.push_back(SPLITTER_POS_KEY);
	properties.push_back(TOOLBARS_SETTINGS_KEY);
	properties.push_back(RESTORE_FILE_POS_KEY);
	properties.push_back(INTERFACE_LANG_KEY);
	properties.push_back(SCRIPTS_FOLDER_KEY);
	// SeNS
	properties.push_back(USESPELLER_CHECK_KEY);
	properties.push_back(HIGHLIGHT_CHECK_KEY);
	properties.push_back(CUSTOM_DICT_KEY);
	properties.push_back(CUSTOM_DICT_CODEPAGE_KEY);
	properties.push_back(NBSPCHAR_KEY);
	properties.push_back(CHANGE_KEYBD_CHECK_KEY);
	properties.push_back(KEYB_LAYOUT_KEY);
	properties.push_back(SHOW_LINE_NUMBERS_KEY);
	properties.push_back(IMAGE_TYPE_KEY);
	properties.push_back(JPEG_QUALITY_KEY);

	properties.push_back(INSIMAGE_ASKING);
	properties.push_back(INS_CLEAR_IMAGE);
	properties.push_back(WINDOW_POSITION);
	properties.push_back(WORDS_DLG_POSITION);
	properties.push_back(SHOW_WORDS_EXCLUSIONS);
	properties.push_back(m_desc.GetClassName());
	properties.push_back(m_tree_items.GetClassName());

	return properties.size();
}

bool CSettings::GetPropertyValue(const CString & sProperty, CProperty & property)
{
	if (sProperty == KEEP_ENCODING_KEY)
	{
		property = GetStringedProperty(&m_keep_encoding, KEY_BOOL);
		return true;
	}
	else if (sProperty == DEFAULT_ENCODING_KEY)
	{
		property = m_default_encoding;
		return true;
	}
	else if (sProperty == SEARCH_OPTIONS_KEY)
	{
		property = GetStringedProperty(&m_search_options, KEY_INT);
		return true;
	}
	else if (sProperty == COLOR_BG_KEY)
	{
		property = GetStringedProperty(&m_collorBG, KEY_ULONG);
		return true;
	}
	else if (sProperty == COLOR_FG_KEY)
	{
		property = GetStringedProperty(&m_collorFG, KEY_ULONG);
		return true;
	}
	else if (sProperty == FONT_SIZE_KEY)
	{
		property = GetStringedProperty(&m_font_size, KEY_INT);
		return true;
	}
	else if (sProperty == XML_SRC_WRAP_KEY)
	{
		property = GetStringedProperty(&m_xml_src_wrap, KEY_BOOL);
		return true;
	}
	else if (sProperty == XML_SRC_SYNTAX_HL_KEY)
	{
		property = GetStringedProperty(&m_xml_src_syntaxHL, KEY_BOOL);
		return true;
	}
	else if (sProperty == XML_SRC_TAG_HL_KEY)
	{
		property = GetStringedProperty(&m_xml_src_tagHL, KEY_BOOL);
		return true;
	}
	else if (sProperty == XML_SRC_SHOW_EOL_KEY)
	{
		property = GetStringedProperty(&m_xml_src_showEOL, KEY_BOOL);
		return true;
	}
	else if (sProperty == XML_SRC_SHOW_SPACE_KEY)
	{
		property = GetStringedProperty(&m_xml_src_showSpace, KEY_BOOL);
		return true;
	}
	else if (sProperty == FAST_MODE_KEY)
	{
		property = GetStringedProperty(&m_fast_mode, KEY_BOOL);
		return true;
	}
	else if (sProperty == FONT_KEY)
	{
		property = m_font;
		return true;
	}
	else if (sProperty == SRC_FONT_KEY)
	{
		property = m_srcfont;
		return true;
	}
	else if (sProperty == VIEW_STATUS_BAR_KEY)
	{
		property = GetStringedProperty(&m_view_status_bar, KEY_BOOL);
		return true;
	}
	else if (sProperty == VIEW_DOCUMENT_TREE_KEY)
	{
		property = GetStringedProperty(&m_view_doc_tree, KEY_BOOL);
		return true;
	}
	else if (sProperty == SPLITTER_POS_KEY)
	{
		property = GetStringedProperty(&m_splitter_pos, KEY_INT);
		return true;
	}
	else if (sProperty == TOOLBARS_SETTINGS_KEY)
	{
		property = m_toolbars_settings;
		return true;
	}
	else if (sProperty == RESTORE_FILE_POS_KEY)
	{
		property = GetStringedProperty(&m_restore_file_position, KEY_BOOL);
		return true;
	}
	else if (sProperty == INTERFACE_LANG_KEY)
	{
		property = GetStringedProperty(&m_interface_lang_id, KEY_INT);
		return true;
	}
	else if (sProperty == SCRIPTS_FOLDER_KEY)
	{
		property = m_scripts_folder;
		return true;
	}
	// added SeNS
	else if (sProperty == USESPELLER_CHECK_KEY)
	{
		property = GetStringedProperty(&m_usespell_check, KEY_BOOL);
		return true;
	}
	else if (sProperty == HIGHLIGHT_CHECK_KEY)
	{
		property = GetStringedProperty(&m_highlght_check, KEY_BOOL);
		return true;
	}
	else if (sProperty == CUSTOM_DICT_KEY)
	{
		property = m_custom_dict;
		return true;
	}
	else if (sProperty == CUSTOM_DICT_CODEPAGE_KEY)
	{
		property = GetStringedProperty(&m_custom_dict_codepage, KEY_INT);
		return true;
	}
	else if (sProperty == NBSPCHAR_KEY)
	{
		property = m_nbsp_char;
		return true;
	}
	else if (sProperty == CHANGE_KEYBD_CHECK_KEY)
	{
		property = GetStringedProperty(&m_change_kbd_layout_check, KEY_BOOL);
		return true;
	}
	else if (sProperty == KEYB_LAYOUT_KEY)
	{
		property = GetStringedProperty(&m_keyb_layout, KEY_INT);
		return true;
	}
	else if (sProperty == SHOW_LINE_NUMBERS_KEY)
	{
		property = GetStringedProperty(&m_show_line_numbers, KEY_BOOL);
		return true;
	}
	else if (sProperty == IMAGE_TYPE_KEY)
	{
		property = GetStringedProperty(&m_image_type, KEY_INT);
		return true;
	}
	else if (sProperty == JPEG_QUALITY_KEY)
	{
		property = GetStringedProperty(&m_jpeg_quality, KEY_INT);
		return true;
	}
	///
	else if (sProperty == INSIMAGE_ASKING)
	{
		property = GetStringedProperty(&m_insimage_ask, KEY_BOOL);
		return true;
	}
	else if (sProperty == INS_CLEAR_IMAGE)
	{
		property = GetStringedProperty(&m_ins_clear_image, KEY_BOOL);
		return true;
	}
	else if (sProperty == WORDS_DLG_POSITION)
	{
		CString temp;
		temp.Format(L"%u;%u;%u;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld",
		            m_words_dlg_placement.length,
		            m_words_dlg_placement.flags,
		            m_words_dlg_placement.showCmd,
		            m_words_dlg_placement.ptMinPosition.x,
		            m_words_dlg_placement.ptMinPosition.y,
		            m_words_dlg_placement.ptMaxPosition.x,
		            m_words_dlg_placement.ptMaxPosition.y,
		            m_words_dlg_placement.rcNormalPosition.bottom,
		            m_words_dlg_placement.rcNormalPosition.left,
		            m_words_dlg_placement.rcNormalPosition.top,
		            m_words_dlg_placement.rcNormalPosition.right);
		property = temp;
		return true;
	}
	else if (sProperty == SHOW_WORDS_EXCLUSIONS)
	{
		property = GetStringedProperty(&m_show_words_excls, KEY_BOOL);
		return true;
	}
	else if (sProperty == WINDOW_POSITION)
	{
		CString temp;
		temp.Format(L"%u;%u;%u;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld",
		            m_wnd_placement.length,
		            m_wnd_placement.flags,
		            m_wnd_placement.showCmd,
		            m_wnd_placement.ptMinPosition.x,
		            m_wnd_placement.ptMinPosition.y,
		            m_wnd_placement.ptMaxPosition.x,
		            m_wnd_placement.ptMaxPosition.y,
		            m_wnd_placement.rcNormalPosition.bottom,
		            m_wnd_placement.rcNormalPosition.left,
		            m_wnd_placement.rcNormalPosition.top,
		            m_wnd_placement.rcNormalPosition.right);
		property = temp;
		return true;
	}
	else if (sProperty == m_desc.GetClassName())
	{
		property = (ISerializable *)&m_desc;
		property.SetFactory(&m_desc);
		return true;
	}
	else if (sProperty == m_tree_items.GetClassName())
	{
		property = (ISerializable *)&m_tree_items;
		property.SetFactory(&m_tree_items);
		return true;
	}

	return false;
}

bool CSettings::SetPropertyValue(const CString & sProperty, CProperty & sValue)
{
	if (sProperty == KEEP_ENCODING_KEY)
	{
		m_keep_encoding = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == DEFAULT_ENCODING_KEY)
	{
		m_default_encoding = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == SEARCH_OPTIONS_KEY)
	{
		m_search_options = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == COLOR_BG_KEY)
	{
		m_collorBG = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == COLOR_FG_KEY)
	{
		m_collorFG = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == FONT_SIZE_KEY)
	{
		m_font_size = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == XML_SRC_WRAP_KEY)
	{
		m_xml_src_wrap = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == XML_SRC_SYNTAX_HL_KEY)
	{
		m_xml_src_syntaxHL = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == XML_SRC_TAG_HL_KEY)
	{
		m_xml_src_tagHL = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == XML_SRC_SHOW_EOL_KEY)
	{
		m_xml_src_showEOL = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == XML_SRC_SHOW_SPACE_KEY)
	{
		m_xml_src_showSpace = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == FAST_MODE_KEY)
	{
		m_fast_mode = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == FONT_KEY)
	{
		m_font = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == SRC_FONT_KEY)
	{
		m_srcfont = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == VIEW_STATUS_BAR_KEY)
	{
		m_view_status_bar = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == VIEW_DOCUMENT_TREE_KEY)
	{
		m_view_doc_tree = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == SPLITTER_POS_KEY)
	{
		m_splitter_pos = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == TOOLBARS_SETTINGS_KEY)
	{
		m_toolbars_settings = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == RESTORE_FILE_POS_KEY)
	{
		m_restore_file_position = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == INTERFACE_LANG_KEY)
	{
		m_interface_lang_id = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == SCRIPTS_FOLDER_KEY)
	{
		m_scripts_folder = sValue.GetStringValue();
		return true;
	}
	// SeNS
	else if (sProperty == USESPELLER_CHECK_KEY)
	{
		m_usespell_check = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == HIGHLIGHT_CHECK_KEY)
	{
		m_highlght_check = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == CUSTOM_DICT_KEY)
	{
		m_custom_dict = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == CUSTOM_DICT_CODEPAGE_KEY)
	{
		m_custom_dict_codepage = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == NBSPCHAR_KEY)
	{
		m_nbsp_char = sValue.GetStringValue();
		return true;
	}
	else if (sProperty == CHANGE_KEYBD_CHECK_KEY)
	{
		m_change_kbd_layout_check = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == KEYB_LAYOUT_KEY)
	{
		m_keyb_layout = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == SHOW_LINE_NUMBERS_KEY)
	{
		m_show_line_numbers = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == IMAGE_TYPE_KEY)
	{
		m_image_type = StrToInt(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == JPEG_QUALITY_KEY)
	{
		m_jpeg_quality = StrToInt(sValue.GetStringValue());
		return true;
	}
	///
	else if (sProperty == INSIMAGE_ASKING)
	{
		m_insimage_ask = StrToBool(sValue.GetStringValue());
		return true;
	}
	else if (sProperty == INS_CLEAR_IMAGE)
	{
		m_ins_clear_image = StrToBool(sValue);
		return true;
	}
	else if (sProperty == SHOW_WORDS_EXCLUSIONS)
	{
		m_show_words_excls = StrToBool(sValue);
		return true;
	}
	else if (sProperty == WORDS_DLG_POSITION)
	{
		CString str = sValue.GetStringValue();
		int n = 0, curPos = 0;

		while (str.Tokenize(L";", curPos) != L"")
			n++;

		CString * tokens = new CString[n];
		curPos = n = 0;

		CString temp;
		while ((temp = str.Tokenize(L";", curPos)) != L"")
		{
			tokens[n] = temp;
			n++;
		}

		if (n == 11)
		{
			m_words_dlg_placement.length = StrToInt(tokens[0]);
			m_words_dlg_placement.flags = StrToInt(tokens[1]);
			m_words_dlg_placement.showCmd = StrToInt(tokens[2]);
			m_words_dlg_placement.ptMinPosition.x = StrToInt(tokens[3]);
			m_words_dlg_placement.ptMinPosition.y = StrToInt(tokens[4]);
			m_words_dlg_placement.ptMaxPosition.x = StrToInt(tokens[5]);
			m_words_dlg_placement.ptMaxPosition.y = StrToInt(tokens[6]);
			m_words_dlg_placement.rcNormalPosition.bottom = StrToInt(tokens[7]);
			m_words_dlg_placement.rcNormalPosition.left = StrToInt(tokens[8]);
			m_words_dlg_placement.rcNormalPosition.top = StrToInt(tokens[9]);
			m_words_dlg_placement.rcNormalPosition.right = StrToInt(tokens[10]);
		}

		delete[] tokens;

		return true;
	}
	else if (sProperty == WINDOW_POSITION)
	{
		CString str = sValue.GetStringValue();
		int n = 0, curPos = 0;

		while (str.Tokenize(L";", curPos) != L"")
			n++;

		CString * tokens = new CString[n];
		curPos = n = 0;

		CString temp;
		while ((temp = str.Tokenize(L";", curPos)) != L"")
		{
			tokens[n] = temp;
			n++;
		}

		if (n == 11)
		{
			m_wnd_placement.length = StrToInt(tokens[0]);
			m_wnd_placement.flags = StrToInt(tokens[1]);
			m_wnd_placement.showCmd = StrToInt(tokens[2]);
			m_wnd_placement.ptMinPosition.x = StrToInt(tokens[3]);
			m_wnd_placement.ptMinPosition.y = StrToInt(tokens[4]);
			m_wnd_placement.ptMaxPosition.x = StrToInt(tokens[5]);
			m_wnd_placement.ptMaxPosition.y = StrToInt(tokens[6]);
			m_wnd_placement.rcNormalPosition.bottom = StrToInt(tokens[7]);
			m_wnd_placement.rcNormalPosition.left = StrToInt(tokens[8]);
			m_wnd_placement.rcNormalPosition.top = StrToInt(tokens[9]);
			m_wnd_placement.rcNormalPosition.right = StrToInt(tokens[10]);
		}

		delete[] tokens;

		return true;
	}
	else if (sProperty == m_desc.GetClassName())
	{
		DESCSHOWINFO * pdesc = (DESCSHOWINFO *)(sValue.GetObject());
		m_desc.elements = pdesc->elements;
		sValue.GetFactory()->Destroy(pdesc);

		return true;
	}
	else if (sProperty == m_tree_items.GetClassName())
	{
		TREEITEMSHOWINFO * pti = (TREEITEMSHOWINFO *)(sValue.GetObject());
		m_tree_items.items = pti->items;
		sValue.GetFactory()->Destroy(pti);

		return true;
	}

	return false;
}

bool CSettings::HasMultipleInstances()
{
	return false;
}

CString CSettings::GetClassName()
{
	return L"Settings";
}

CString CSettings::GetID()
{
	return L"0";
}

// IObjectFactory interface
ISerializable * CSettings::Create()
{
	return new CSettings;
}

void CSettings::Destroy(ISerializable * obj)
{
	delete obj;
}

void CSettings::Save()
{
	CString fullpath = U::GetSettingsDir() + SETTINGS_XML_FILE;
	CXMLSerializer ser(fullpath, L"FBE", false);

	ser.Serialize(this);
}

void CSettings::Load()
{
	CString fullpath = U::GetSettingsDir() + SETTINGS_XML_FILE;
	CXMLSerializer ser(fullpath, L"FBE", true);

	std::vector<void *> objs;
	SetDefaults();
	if (!ser.Deserialize(this, this))
		Save();
}

bool CSettings::ViewStatusBar() const
{
	return m_view_status_bar;
}

bool CSettings::ViewDocumentTree() const
{
	return m_view_doc_tree;
}

bool CSettings::NeedRestart() const
{
	return m_need_restart;
}

DWORD CSettings::GetSearchOptions() const
{
	return m_search_options;
}
DWORD CSettings::GetFontSize() const
{
	return m_font_size;
}
CString CSettings::GetFont() const
{
	return m_font;
}
CString CSettings::GetSrcFont() const
{
	return m_srcfont;
}
DWORD CSettings::GetSplitterPos() const
{
	return m_splitter_pos;
}
CString CSettings::GetToolbarsSettings() const
{
	return m_toolbars_settings;
}
CString CSettings::GetKeyPath() const
{
	return m_key_path;
}

const CRegKey & CSettings::GetKey() const
{
	return m_key;
}

// SeNS
DWORD CSettings::GetCustomDictCodepage() const
{
	return m_custom_dict_codepage;
}

CString CSettings::GetNBSPChar() const
{
	return m_nbsp_char;
}

CString CSettings::GetOldNBSPChar() const
{
	if (m_old_nbsp.Compare(L"\u00A0") == 0)
		return CString(L"&nbsp;");
	else
		return m_old_nbsp;
}

DWORD CSettings::GetKeybLayout() const
{
	return m_keyb_layout;
}

DWORD CSettings::GetImageType() const
{
	return m_image_type;
}

///
bool CSettings::GetExtElementStyle(const CString & elem) const
{
	std::map<CString, bool>::const_iterator member = m_desc.elements.find(elem);
	if (member == m_desc.elements.end())
		return false;
	else
		return member->second;
}

bool CSettings::GetWindowPosition(WINDOWPLACEMENT & wpl) const
{
	BYTE * temp = new BYTE[sizeof(WINDOWPLACEMENT)];
	::ZeroMemory(temp, sizeof(WINDOWPLACEMENT));

	BYTE * orig = new BYTE[sizeof(WINDOWPLACEMENT)];
	::CopyMemory(orig, &m_wnd_placement, sizeof(WINDOWPLACEMENT));

	bool def = true;

	if (*temp - *orig)
		def = false;

	delete[] temp, orig;

	if (!def)
		wpl = m_wnd_placement;

	return !def;
	/*DWORD sz = sizeof(wpl);
	DWORD type = REG_BINARY;
	return (::RegQueryValueEx(m_key, WINDOW_POSITION,0,&type,(BYTE*)&wpl,&sz)==ERROR_SUCCESS &&
		type==REG_BINARY && sz==sizeof(wpl) && wpl.length==sizeof(wpl));*/
}

bool CSettings::GetWordsDlgPosition(WINDOWPLACEMENT & wpl) const
{
	BYTE * temp = new BYTE[sizeof(WINDOWPLACEMENT)];
	::ZeroMemory(temp, sizeof(WINDOWPLACEMENT));

	BYTE * orig = new BYTE[sizeof(WINDOWPLACEMENT)];
	::CopyMemory(orig, &m_words_dlg_placement, sizeof(WINDOWPLACEMENT));

	bool def = true;

	if (*temp - *orig)
		def = false;

	delete[] temp, orig;

	if (!def)
		wpl = m_words_dlg_placement;

	return !def;
	/*DWORD sz = sizeof(wpl);
	DWORD type = REG_BINARY;
	return (::RegQueryValueEx(m_key, WINDOW_POSITION,0,&type,(BYTE*)&wpl,&sz)==ERROR_SUCCESS &&
	type==REG_BINARY && sz==sizeof(wpl) && wpl.length==sizeof(wpl));*/
}

CString CSettings::GetDefaultEncoding() const
{
	return m_default_encoding;
}

DWORD CSettings::GetColorBG() const
{
	return m_collorBG;
}

DWORD CSettings::GetColorFG() const
{
	return m_collorFG;
}

DWORD CSettings::GetInterfaceLanguageID() const
{
	return m_interface_lang_id;
}

CString CSettings::GetInterfaceLanguageDllName() const
{
	switch (m_interface_lang_id)
	{
	case LANG_RUSSIAN:
		return L"res_rus.dll";
	case LANG_UKRAINIAN:
		return L"res_ukr.dll";
	default:
		return L"";
	}
}

CString CSettings::GetLocalizedGenresFileName() const
{
	switch (m_interface_lang_id)
	{
	case LANG_RUSSIAN:
		return L"genres.rus.txt";
	case LANG_UKRAINIAN:
		return L"genres.ukr.txt";
	default:
		return L"genres.txt";
	}
}

CString CSettings::GetInterfaceLanguageName() const
{
	switch (m_interface_lang_id)
	{
	case LANG_RUSSIAN:
		return L"russian";
	case LANG_UKRAINIAN:
		return L"ukrainian";
	default:
		return L"english";
	}
}

CString CSettings::GetScriptsFolder() const
{
	return m_scripts_folder;
}

CString CSettings::GetDefaultScriptsFolder()
{
	TCHAR filepath[MAX_PATH];
	DWORD pathlen = ::GetModuleFileName(_Module.GetModuleInstance(), filepath, MAX_PATH);
	CString tmp = U::GetFullPathName(filepath);
	int pos = tmp.ReverseFind(_T('\\'));
	if (pos >= 0)
	{
		tmp.Delete(pos, tmp.GetLength() - pos);
		tmp.Append(L"\\");
		tmp.Append(DEFAULT_SCRIPTS_FOLDER);
		tmp.Append(L"\\");
	}

	tmp.MakeLower();

	return tmp;
}

bool CSettings::IsDefaultScriptsFolder()
{
	return GetScriptsFolder() == GetDefaultScriptsFolder();
}

bool CSettings::GetShowWordsExcls() const
{
	return m_show_words_excls;
}

bool CSettings::GetDocTreeItemState(const ATL::CString & item, bool default_state)
{
	std::map<CString, bool>::const_iterator member = m_tree_items.items.find(item);
	if (member == m_tree_items.items.end())
		return default_state;
	else
		return member->second;
}

void CSettings::SetKeepEncoding(bool keep, bool apply)
{
	m_keep_encoding = keep;
	if (apply)
		Save();
}

void CSettings::SetSearchOptions(DWORD opt, bool apply)
{
	m_search_options = opt;
	if (apply)
		Save();
}

void CSettings::SetFontSize(DWORD size, bool apply)
{
	m_font_size = size;
	if (apply)
		Save();
}

void CSettings::SetXmlSrcWrap(bool wrap, bool apply)
{
	m_xml_src_wrap = wrap;
	if (apply)
		Save();
}

void CSettings::SetXmlSrcSyntaxHL(bool hl, bool apply)
{
	m_xml_src_syntaxHL = hl;
	if (apply)
		Save();
}

void CSettings::SetXmlSrcTagHL(bool hl, bool apply)
{
	m_xml_src_tagHL = hl;
	if (apply)
		Save();
}
void CSettings::SetXmlSrcShowEOL(bool eol, bool apply)
{
	m_xml_src_showEOL = eol;
	if (apply)
		Save();
}
void CSettings::SetXmlSrcShowSpace(bool eol, bool apply)
{
	m_xml_src_showSpace = eol;
	if (apply)
		Save();
}
void CSettings::SetFastMode(bool mode, bool apply)
{
	m_fast_mode = mode;
	if (apply)
		Save();
}

void CSettings::SetFont(const CString & font, bool apply)
{
	m_font = font;
	if (apply)
		Save();
}

void CSettings::SetSrcFont(const CString & font, bool apply)
{
	m_srcfont = font;
	if (apply)
		Save();
}

void CSettings::SetViewStatusBar(bool view, bool apply)
{
	m_view_status_bar = view;
	if (apply)
		Save();
}

void CSettings::SetViewDocumentTree(bool view, bool apply)
{
	m_view_doc_tree = view;
	if (apply)
		Save();
}

void CSettings::SetSplitterPos(DWORD pos, bool apply)
{
	m_splitter_pos = pos;
	if (apply)
		Save();
}

void CSettings::SetToolbarsSettings(CString & settings, bool apply)
{
	m_toolbars_settings = settings;
	if (apply)
		Save();
}

void CSettings::SetExtElementStyle(const CString & elem, bool ext, bool apply)
{
	m_desc.elements[elem] = ext;
	if (apply)
		Save();
}

void CSettings::SetWindowPosition(const WINDOWPLACEMENT & wpl, bool /*apply*/)
{
	m_wnd_placement = wpl;
}

void CSettings::SetWordsDlgPosition(const WINDOWPLACEMENT & wpl, bool /*apply*/)
{
	m_words_dlg_placement = wpl;
}

void CSettings::SetDefaultEncoding(const CString & enc, bool apply)
{
	m_default_encoding = enc;
	if (apply)
		Save();
}

void CSettings::SetColorBG(DWORD col, bool apply)
{
	m_collorBG = col;
	if (apply)
		Save();
}

void CSettings::SetColorFG(DWORD col, bool apply)
{
	m_collorFG = col;
	if (apply)
		Save();
}

void CSettings::SetRestoreFilePosition(bool restore, bool apply)
{
	m_restore_file_position = restore;
	if (apply)
		Save();
}

void CSettings::SetInterfaceLanguage(DWORD lang_id, bool apply)
{
	if (m_interface_lang_id != lang_id)
	{
		m_interface_lang_id = lang_id;
		if (apply)
			Save();
	}
}

void CSettings::SetScriptsFolder(const CString fullpath, bool apply)
{
	if (apply)
	{
		if (m_scripts_folder != fullpath)
		{
			m_scripts_folder = fullpath;
		}
	}
}

void CSettings::SetInsImageAsking(bool ask, bool apply)
{
	m_insimage_ask = ask;
	if (apply)
		Save();
}

void CSettings::SetIsInsClearImage(bool clear, bool apply)
{
	m_ins_clear_image = clear;
	if (apply)
		Save();
}

void CSettings::SetShowWordsExcls(bool show, bool apply)
{
	m_show_words_excls = show;
	if (apply)
		Save();
}

void CSettings::SetNeedRestart()
{
	m_need_restart = true;
}

void CSettings::SetDocTreeItemState(const ATL::CString & item, bool state)
{
	m_tree_items.items[item] = state;
	Save();
}

// SeNS
void CSettings::SetUseSpellChecker(const bool value, bool apply)
{
	m_usespell_check = value;
	if (!value)
		SetHighlightMisspells(value, apply);
	if (apply)
		Save();
}

void CSettings::SetHighlightMisspells(const bool value, bool apply)
{
	m_highlght_check = value;
	if (apply)
		Save();
}

void CSettings::SetCustomDict(const ATL::CString & value, bool apply)
{
	m_custom_dict.SetString(value);
	if (apply)
		Save();
}

void CSettings::SetCustomDictCodepage(const DWORD value, bool apply)
{
	m_custom_dict_codepage = value;
	if (apply)
		Save();
}

void CSettings::SetNBSPChar(const ATL::CString & value, bool apply)
{
	if (value.Compare(m_nbsp_char) != 0)
	{
		m_old_nbsp.SetString(m_nbsp_char);
		m_nbsp_char.SetString(value);
		if (apply)
			Save();
	}
}

void CSettings::SetChangeKeybLayout(const bool value, bool apply)
{
	m_change_kbd_layout_check = value;
	if (apply)
		Save();
}

void CSettings::SetKeybLayout(const DWORD_PTR value, bool apply)
{
	m_keyb_layout = value;
	if (apply)
		Save();
}

void CSettings::SetXMLSrcShowLineNumbers(const bool value, bool apply)
{
	m_show_line_numbers = value;
	if (apply)
		Save();
}

void CSettings::SetImageType(const DWORD value, bool apply)
{
	m_image_type = value;
	if (apply)
		Save();
}

void CSettings::SetJpegQuality(const DWORD value, bool apply)
{
	m_jpeg_quality = value;
	if (apply)
		Save();
}

void CSettings::SetViewWidth(int width)
{
	m_viewWidth = width;
}

void CSettings::SetViewHeight(int height)
{
	m_viewHeight = height;
}

int CSettings::GetViewWidth()
{
	return m_viewWidth;
}

int CSettings::GetViewHeight()
{
	return m_viewHeight;
}

void CSettings::SetMainWindow(HWND hwnd)
{
	m_hMainWindow = hwnd;
}

HWND CSettings::GetMainWindow()
{
	return m_hMainWindow;
}

// Predicate for std::sort
class sortComp
{
public:
	bool operator()(void * x, void * y)
	{
		return (reinterpret_cast<WordsItem *>(x)->m_word.Compare(reinterpret_cast<WordsItem *>(y)->m_word) < 0);
	}
};
// Predicate for std::unique
class uniqueComp
{
public:
	bool operator()(void * x, void * y)
	{
		return (reinterpret_cast<WordsItem *>(x)->m_word.Compare(reinterpret_cast<WordsItem *>(y)->m_word) == 0);
	}
};

//
void CSettings::LoadWords()
{
	CXMLSerializer ser(U::GetSettingsDir() + WORDS_XML_FILE, L"FBE", true);

	WordsItem word;
	std::vector<void *> objects;
	ser.Deserialize(&word, objects);

	// changed by SeNS: do it quickly
	std::sort(objects.begin(), objects.end(), sortComp());
	objects.erase(std::unique(objects.begin(), objects.end(), uniqueComp()), objects.end());
	for (unsigned int i = 0; i < objects.size(); ++i)
		m_words.push_back(*reinterpret_cast<WordsItem *>(objects[i]));
}

void CSettings::SaveWords()
{
	// changed by SeNS: extremely slow serialization replaced by fast and simple code
	MSXML2::IXMLDOMDocument2Ptr pXMLDoc;
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(DOMDocument));
	if (!FAILED(hr))
	{
		CString xml(L"<FBE>\n\t<Words>\n");
		// store all words
		for (unsigned int i = 0; i < m_words.size(); i++)
		{
			xml += L"\t\t<Word>\n\t\t\t<Value>" + m_words[i].m_word + L"</Value>\n";
			CString count;
			count.Format(L"%d", m_words[i].m_count);
			xml += L"\t\t\t<Counted>" + count + L"</Counted>\n\t\t</Word>";
		}
		xml += L"\t</Words>\n</FBE>";
		pXMLDoc->loadXML(xml.AllocSysString());

		MSXML2::IXMLDOMElementPtr pXMLRootElem = pXMLDoc->GetdocumentElement();
		MSXML2::IXMLDOMProcessingInstructionPtr pXMLProcessingNode = pXMLDoc->createProcessingInstruction(L"xml", L" version='1.0' encoding='UTF-8'");

		_variant_t vtObject;
		vtObject.vt = VT_DISPATCH;
		vtObject.pdispVal = pXMLRootElem;
		vtObject.pdispVal->AddRef();
		pXMLDoc->insertBefore(pXMLProcessingNode, vtObject);

		CString fileName(U::GetSettingsDir() + WORDS_XML_FILE);
		pXMLDoc->save(fileName.AllocSysString());
	}
}

void CSettings::SetDefaults()
{
	m_keep_encoding = true;
	m_default_encoding = DEFAULT_ENCODING;
	m_search_options = 0;
	m_collorBG = CLR_DEFAULT;
	m_collorFG = CLR_DEFAULT;
	m_font_size = 12;
	m_xml_src_wrap = true;
	m_xml_src_syntaxHL = true;
	m_xml_src_tagHL = true;
	m_xml_src_showEOL = false;
	m_xml_src_showSpace = false;
	m_fast_mode = false;
	m_font = DEFAULT_FONT;
	m_srcfont = DEFAULT_SRCFONT;
	m_view_status_bar = true;
	m_view_doc_tree = true;
	m_splitter_pos = 200;
	m_toolbars_settings = L"";
	m_restore_file_position = false;
	m_interface_lang_id = PRIMARYLANGID(GetSystemDefaultLangID());
	m_scripts_folder = GetDefaultScriptsFolder();
	m_insimage_ask = true;
	m_ins_clear_image = false;
	m_show_words_excls = true;
	// added by SeNS
	m_usespell_check = true;
	m_highlght_check = true;
	m_custom_dict = L"custom.dic";
	m_custom_dict_codepage = 1251;
	m_nbsp_char = L"\u00A0";
	m_change_kbd_layout_check = false;
	m_show_line_numbers = false;
	m_image_type = 1;
	m_jpeg_quality = 75;

	::ZeroMemory(&m_wnd_placement, sizeof(WINDOWPLACEMENT));
	m_desc.SetDefaults();
}

DESCSHOWINFO::DESCSHOWINFO()
{
	SetDefaults();
}

// Default fields showing in description

void DESCSHOWINFO::SetDefaults()
{
	elements[L"ci_all"] = true;
	elements[L"sti_all"] = false;
	elements[L"di_id"] = true;
	elements[L"id"] = true;
	elements[L"ti_kw"] = true;
	elements[L"ti_nic_mail_web"] = true;
	elements[L"ti_genre_match"] = true;
}

int DESCSHOWINFO::GetProperties(std::vector<CString> & properties)
{
	std::map<CString, bool>::iterator iter = elements.begin();
	while (iter != elements.end())
	{
		properties.push_back(iter->first);
		iter++;
	}

	return properties.size();
}

bool DESCSHOWINFO::GetPropertyValue(const CString & sProperty, CProperty & property)
{
	std::map<CString, bool>::iterator iter = elements.begin();
	while (iter != elements.end())
	{
		if (iter->first == sProperty)
		{
			property = GetStringedProperty(&elements[iter->first], KEY_BOOL);
			return true;
		}
		else
			iter++;
	}

	return false;
}

bool DESCSHOWINFO::SetPropertyValue(const CString & sProperty, CProperty & sValue)
{
	std::map<CString, bool>::iterator iter = elements.begin();
	while (iter != elements.end())
	{
		if (iter->first == sProperty)
		{
			iter->second = StrToBool(sValue.GetStringValue());
			return true;
		}
		else
			iter++;
	}

	return false;
}

bool DESCSHOWINFO::HasMultipleInstances()
{
	return false;
}

CString DESCSHOWINFO::GetClassName()
{
	return L"Description";
}

CString DESCSHOWINFO::GetID()
{
	return L"";
}

ISerializable * DESCSHOWINFO::Create()
{
	return new DESCSHOWINFO;
}

void DESCSHOWINFO::Destroy(ISerializable * obj)
{
	delete obj;
}

TREEITEMSHOWINFO::TREEITEMSHOWINFO()
{
	SetDefaults();
}

// Default fields showing in description
void TREEITEMSHOWINFO::SetDefaults()
{
	_EDMnr.InitStandartEDs();
	int edCount = _EDMnr.GetStEDsCount();
	for (int i = 0; i < edCount; ++i)
	{
		CElementDescriptor * ed = _EDMnr.GetStED(i);
		items[ed->GetCaption()] = ed->ViewInTree();
	}
}

int TREEITEMSHOWINFO::GetProperties(std::vector<CString> & properties)
{
	std::map<CString, bool>::iterator iter = items.begin();
	while (iter != items.end())
	{
		properties.push_back(iter->first);
		iter++;
	}

	return properties.size();
}

bool TREEITEMSHOWINFO::GetPropertyValue(const CString & sProperty, CProperty & property)
{
	std::map<CString, bool>::iterator iter = items.begin();
	while (iter != items.end())
	{
		if (iter->first == sProperty)
		{
			property = GetStringedProperty(&items[iter->first], KEY_BOOL);
			return true;
		}
		else
			iter++;
	}

	return false;
}

bool TREEITEMSHOWINFO::SetPropertyValue(const CString & sProperty, CProperty & sValue)
{
	std::map<CString, bool>::iterator iter = items.begin();
	while (iter != items.end())
	{
		if (iter->first == sProperty)
		{
			iter->second = StrToBool(sValue.GetStringValue());
			return true;
		}
		else
			iter++;
	}

	return false;
}

bool TREEITEMSHOWINFO::HasMultipleInstances()
{
	return false;
}

CString TREEITEMSHOWINFO::GetClassName()
{
	return L"TreeItems";
}

CString TREEITEMSHOWINFO::GetID()
{
	return L"";
}

ISerializable * TREEITEMSHOWINFO::Create()
{
	return new TREEITEMSHOWINFO;
}

void TREEITEMSHOWINFO::Destroy(ISerializable * obj)
{
	delete obj;
}

WordsItem::WordsItem()
{
}

WordsItem::WordsItem(CString word, int count)
    : m_word(word), m_count(count)
{
}

int WordsItem::GetProperties(std::vector<CString> & properties)
{
	properties.push_back(L"Value");
	properties.push_back(L"Counted");
	return static_cast<int>(properties.size());
}

bool WordsItem::GetPropertyValue(const CString & sProperty, CProperty & property)
{
	if (sProperty == L"Value")
	{
		property = m_word;
		return true;
	}
	else if (sProperty == L"Counted")
	{
		CString counted;
		counted.Format(L"%d", m_count);
		property = counted;
		return true;
	}
	return false;
}

bool WordsItem::SetPropertyValue(const CString & sProperty, CProperty & property)
{
	if (sProperty == L"Value")
	{
		m_word = property;
		return true;
	}
	else if (sProperty == L"Counted")
	{
		m_count = StrToInt(property.GetStringValue());
		return true;
	}
	return false;
}

bool WordsItem::HasMultipleInstances()
{
	return true;
}

CString WordsItem::GetClassName()
{
	return L"Word";
}

CString WordsItem::GetID()
{
	return L"";
}

ISerializable * WordsItem::Create()
{
	return new WordsItem;
}

void WordsItem::Destroy(ISerializable * obj)
{
	delete obj;
}
