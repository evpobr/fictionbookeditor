#ifndef SPELLER_H
#define SPELLER_H

#include <map>
#include <set>
#include <atlutil.h>
#include "utils.h"
#include "ModelessDialog.h"
#include "Splitter.h"

// Hunspell API function prototypes
#ifdef __cplusplus
extern "C" {
#endif
#include <hunspell/hunspell.h>
#ifdef __cplusplus
}
#endif

enum SPELL_RESULT
{
	SPELL_MISSPELL = 0,
	SPELL_OK = 1,
	SPELL_IGNORE,
	SPELL_IGNOREALL,
	SPELL_CHANGE,
	SPELL_CHANGEALL,
	SPELL_ADD,
	SPELL_UNDO,
	SPELL_CANCEL
};

// TODO: hardcoded set of dictionaries should be changed
enum SPELL_LANG
{
	LANG_EN = 0,
	LANG_RU,
	LANG_DE,
	LANG_FR,
	LANG_ES,
	LANG_UA,
	LANG_CZ,
	LANG_BY,
	LANG_BG,
	LANG_PL,
	LANG_IT,
	LANG_NONE
};

struct DICT
{
	Hunhandle* handle;
	SPELL_LANG lang;
	CString name;
	int codepage;
};

// currently supported dictionaries
const DICT dicts[] = { 
	{0, LANG_EN, CString("en_US"), CP_UTF8},	// UTF-8
	{0, LANG_RU, CString("ru_RU"), 20866},		// KOI8-R
	{0, LANG_DE, CString("de_DE"), 28591},		// ISO 8859-1 Latin 1
	{0, LANG_FR, CString("fr_FR"), CP_UTF8},	// UTF-8
	{0, LANG_ES, CString("es_ES"), 28591},		// ISO 8859-1 Latin 1
	{0, LANG_UA, CString("uk_UA"), CP_UTF8},	// UTF-8
	{0, LANG_CZ, CString("cs_CZ"), 28592},		// ISO 8859-1 Latin 2
	{0, LANG_BY, CString("be_BY"), 1251},		// Win-1251
	{0, LANG_BG, CString("bg_BG"), 1251},		// Win-1251
	{0, LANG_PL, CString("pl_PL"), 28592},		// ISO 8859-1 Latin 2
	{0, LANG_IT, CString("it_IT"), 28605},		// ISO 8859-15 Latin 9
	{0, LANG_NONE, CString(""), 0}				// unsupported language
};

typedef CSimpleArray<CString> CStrings;

class CSpeller;

class CSpellDialog: public CModelessDialogImpl<CSpellDialog>
{
public:
	enum { IDD = IDD_SPELL_CHECK };

	CString m_sBadWord;
	CString m_sReplacement;
	CStrings m_strSuggestions;

	CSpellDialog(CSpeller* parent): m_Speller(parent) {}

	BEGIN_MSG_MAP(CSpellDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		COMMAND_ID_HANDLER(IDC_SPELL_IGNORE, OnIgnore)
		COMMAND_ID_HANDLER(IDC_SPELL_IGNOREALL, OnIgnoreAll)
		COMMAND_ID_HANDLER(IDC_SPELL_CHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_SPELL_CHANGEALL, OnChangeAll)
		COMMAND_ID_HANDLER(IDC_SPELL_ADD, OnAdd)
		COMMAND_ID_HANDLER(IDC_SPELL_UNDO, OnUndo)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_SPELL_REPLACEMENT, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_SPELL_SUGG_LIST, LBN_SELCHANGE, OnSelChange)
		COMMAND_HANDLER(IDC_SPELL_SUGG_LIST, LBN_DBLCLK, OnSelDblClick)
	END_MSG_MAP()

	LRESULT UpdateData();

private:
	CSpeller* m_Speller;
	int m_RetCode;

	bool m_WasSuspended;

	CEdit m_BadWord;
	CEdit m_Replacement;
	CListBox m_Suggestions;
	// buttons
	CButton m_IgnoreContinue;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnActivate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnIgnore(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnIgnoreAll(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnChange(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnChangeAll(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnAdd(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnUndo(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelDblClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&);
};

typedef std::pair<long, MSHTML::IHighlightSegmentPtr> HIGHLIGHT;
typedef std::multimap<long, MSHTML::IHighlightSegmentPtr> HIGHLIGHTS;

// tokens to tokenize string to separate words
const CString Tokens(L" .,?–!—…\r\n\t\"«»“”‘’:;<>(){}[]\u00A0\u2003\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u200B\u202F\u205F\u2060\u3000\u2012\u2013\u2014\u00BA\u25A1\u25AB\u25E6\u201e\u201c");
																																														   
class CSpeller																																											   
{																																														   
public:
	CSpeller(CString dictPath = L"..\\dict");
	~CSpeller();
	void AttachDocument(MSHTML::IHTMLDocumentPtr doc);
	void SetFrame(HWND frame) { m_frame = frame; }
	bool Available() { return (m_Dictionaries[m_Lang].handle != NULL); }
	bool Enabled() const { return m_Enabled; }
	void SetDocumentLanguage();
	void SetEnabled (bool Enabled)
	{ 
		if (m_Enabled != Enabled)
		{
			m_Enabled = Enabled;
			if (!m_Enabled)
				ClearAllMarks();
			else
				HighlightMisspells();
		}
	}
	void SetCustomDictionary(const CString& pathToDictionary, UINT codePage)
	{
		m_CustomDictPath = pathToDictionary;
		m_CustomDictCodepage = codePage;
		LoadCustomDict();
	}
	
	void StartDocumentCheck(MSHTML::IMarkupServices2Ptr undoSrv = NULL);
	void ContinueDocumentCheck();
	void EndDocumentCheck(bool bCancel = true);

	// undo support
	void Undo()
	{
		if (m_browser)
		{
			IOleCommandTargetPtr ct(m_browser);
			if (ct)
				ct->Exec(&CGID_MSHTML, IDM_UNDO, 0, NULL, NULL);
		}
	}
	bool GetUndoState()
	{
		bool stat = false;
		if (m_browser)
		{
			static OLECMD cmd[] = {IDM_UNDO};
			IOleCommandTargetPtr ct(m_browser);
			if (ct)
			{
				ct->QueryStatus(&CGID_MSHTML, 1, cmd, NULL);
				stat = (cmd[0].cmdf != 1);
			}
		}
		return stat;
	}
	void BeginUndoUnit(const wchar_t *name)
	{
		ATLASSERT(m_undoSrv);
		m_undoSrv->BeginUndoUnit((wchar_t *)name);
	}
	void EndUndoUnit()
	{
		ATLASSERT(m_undoSrv);
		m_undoSrv->EndUndoUnit();
	}

	void CheckScroll();
	void CheckElement(MSHTML::IHTMLElementPtr elem, long uniqID, bool HTMLChanged);
	void CheckCurrentPage();
	// main function
	SPELL_RESULT SpellCheck(const CString& word);
	CStrings GetSuggestions(CString word);
	void MarkElement(MSHTML::IHTMLElementPtr elem, long uniqID, const CString& word, int pos);
	void ClearMarks(int elemID);
	void ClearAllMarks();
	// SeNS
	void SetHighlightMisspells(bool Enabled)
	{
		if (Enabled != m_HighlightMisspells)
		{
			m_HighlightMisspells = Enabled;
			if (!m_HighlightMisspells) 
				ClearAllMarks();
			else
				HighlightMisspells();
		}
	}
	void HighlightMisspells();
	void AdvanceVersionNumber(int delta)
	{
		::PostMessage(m_frame, WM_COMMAND,MAKELONG(ID_VER_ADVANCE,delta), 0);
	}

	// popup menu service
	void AppendSpellMenu (HMENU menu);
	void Replace(int nIndex);
	void Replace(CString word);
	void IgnoreAll(CString word = CString(""));
	void AddToDictionary();
	void AddToDictionary(const CString& word);

	void AddReplacement(const CString& badWord, const CString& goodWord)
	{
		m_ChangeWords.Add(badWord);
		m_ChangeWordsTo.Add(goodWord);
	}

protected:
	SHD::IWebBrowser2Ptr m_browser;
	MSHTML::IHTMLDocument2Ptr m_doc2;
	MSHTML::IHTMLDocument3Ptr m_doc3;
	MSHTML::IHTMLDocument4Ptr m_doc4;
	MSHTML::IHTMLElementPtr m_fbw_body;
	MSHTML::IHTMLElement2Ptr m_scrollElement;
	MSHTML::IHTMLRenderStylePtr m_irs;
	MSHTML::IMarkupContainer2Ptr m_mkc;
	MSHTML::IMarkupServicesPtr m_ims;
	MSHTML::IDisplayServicesPtr m_ids;
	MSHTML::IHighlightRenderingServicesPtr m_ihrs;

	// for document checking
	MSHTML::IHTMLTxtRangePtr m_prevSelRange;
	MSHTML::IHTMLTxtRangePtr m_selRange;
	MSHTML::IMarkupPointerPtr m_impStart;
	MSHTML::IMarkupPointerPtr m_impEnd;
	MSHTML::IMarkupServices2Ptr m_undoSrv;

	CSpellDialog* m_spell_dlg;

	bool m_Enabled;
	bool m_HighlightMisspells;
	int m_prevY, m_codePage, m_numAphChanged;
	HWND m_frame;
	SPELL_LANG m_Lang;
	CSimpleArray<DICT> m_Dictionaries;
	CStrings m_IgnoreWords;
	CStrings m_ChangeWords;
	CStrings m_ChangeWordsTo;
	CStrings m_CustomDict;
	CStrings m_menuSuggestions;
	std::set<long> m_uniqIDs;
	HIGHLIGHTS m_ElementHighlights;

	CString m_DictPath;
	CString m_CustomDictPath;
	DWORD	m_CustomDictCodepage;
	Hunhandle* LoadDictionary(const CString& dictPath, const CString& dictName);
	Hunhandle* GetDictionary(const CString& word);
	void LoadCustomDict();
	void SaveCustomDict();
	MSHTML::IHTMLTxtRangePtr GetSelWordRange();
	CString GetSelWord();

	CSplitter* splitter;
};

#endif
