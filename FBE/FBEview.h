// FBEView.h : interface of the CFBEView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Settings.h"
#include "apputils.h"
#include "resource.h"

extern CSettings _Settings;

void BubbleUp(MSHTML::IHTMLDOMNode * node, const wchar_t * name);

static void CenterChildWindow(CWindow parent, CWindow child)
{
	RECT rcParent, rcChild;
	parent.GetWindowRect(&rcParent);
	child.GetWindowRect(&rcChild);
	int parentW = rcParent.right - rcParent.left;
	;
	int parentH = rcParent.bottom - rcParent.top;
	int childW = rcChild.right - rcChild.left;
	int childH = rcChild.bottom - rcChild.top;
	child.MoveWindow(rcParent.left + parentW / 2 - childW / 2, rcParent.top + parentH / 2 - childH / 2, childW, childH);
}

template <class T, int chgID>
class ATL_NO_VTABLE CHTMLChangeSink : public MSHTML::IHTMLChangeSink
{
public:
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject)
	{
		if (iid == IID_IUnknown || iid == IID_IHTMLChangeSink)
		{
			*ppvObject = this;
			return S_OK;
		}

		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG)
	AddRef()
	{
		return 1;
	}
	STDMETHODIMP_(ULONG)
	Release()
	{
		return 1;
	}

	// IHTMLChangeSink
	STDMETHODIMP raw_Notify()
	{
		T * pT = static_cast<T *>(this);
		pT->EditorChanged(chgID);
		return S_OK;
	}
};

typedef CWinTraits<WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0> CFBEViewWinTraits;

enum
{
	FWD_SINK,
	BACK_SINK,
	RANGE_SINK
};

class CFindDlgBase;

class CFBEView : public CWindowImpl<CFBEView, CAxWindow, CFBEViewWinTraits>,
                 public IDispEventSimpleImpl<0, CFBEView, &DIID_DWebBrowserEvents2>,
                 public IDispEventSimpleImpl<0, CFBEView, &DIID_HTMLDocumentEvents2>,
                 public IDispEventSimpleImpl<0, CFBEView, &DIID_HTMLTextContainerEvents2>,
                 public CHTMLChangeSink<CFBEView, RANGE_SINK>
{
protected:
	typedef IDispEventSimpleImpl<0, CFBEView, &DIID_DWebBrowserEvents2> BrowserEvents;
	typedef IDispEventSimpleImpl<0, CFBEView, &DIID_HTMLDocumentEvents2> DocumentEvents;
	typedef IDispEventSimpleImpl<0, CFBEView, &DIID_HTMLTextContainerEvents2> TextEvents;
	typedef CHTMLChangeSink<CFBEView, FWD_SINK> ForwardSink;
	typedef CHTMLChangeSink<CFBEView, BACK_SINK> BackwardSink;
	typedef CHTMLChangeSink<CFBEView, RANGE_SINK> RangeSink;

	HWND m_frame;

public:
	CString m_file_name, m_file_path;
	// changed by SeNS
	DWORD m_dirtyRangeCookie;
	MSHTML::IMarkupServices2Ptr m_mk_srv;

protected:
	SHD::IWebBrowser2Ptr m_browser;
	MSHTML::IHTMLDocument2Ptr m_hdoc;
	MSHTML::IMarkupContainer2Ptr m_mkc;

	int m_ignore_changes;
	int m_enable_paste;

	bool m_normalize : 1;
	bool m_complete : 1;
	bool m_initialized : 1;

	MSHTML::IHTMLElementPtr m_cur_sel;
	MSHTML::IHTMLInputTextElementPtr m_cur_input;
	_bstr_t m_cur_val;
	bool m_form_changed;
	bool m_form_cp;

	CString m_nav_url;

	static _ATL_FUNC_INFO DocumentCompleteInfo;
	static _ATL_FUNC_INFO BeforeNavigateInfo;
	static _ATL_FUNC_INFO EventInfo;
	static _ATL_FUNC_INFO VoidEventInfo;
	static _ATL_FUNC_INFO VoidInfo;

	enum
	{
		FRF_REVERSE = 1,
		FRF_WHOLE = 2,
		FRF_CASE = 4,
		FRF_REGEX = 8
	};

	struct FindReplaceOptions
	{
		CString pattern;
		CString replacement;
		AU::ReMatch match;
		int flags; // IHTMLTxtRange::findText() flags
		bool fRegexp;

		int replNum;

		FindReplaceOptions()
		    : fRegexp(false), flags(0)
		{
		}
	};

	FindReplaceOptions m_fo;
	MSHTML::IHTMLTxtRangePtr m_is_start;

	struct pElAdjacent
	{
		MSHTML::IHTMLElementPtr elem;
		_bstr_t innerText;

		pElAdjacent(MSHTML::IHTMLElementPtr pElem)
		{
			elem = pElem;
			innerText = pElem->innerText;
		}
	};

	friend class CFindDlgBase;
	friend class CViewFindDlg;
	friend class CReplaceDlgBase;
	friend class CViewReplaceDlg;
	friend class CSciFindDlg;
	friend class CSciReplaceDlg;
	friend class FRBase;

	int TextOffset(MSHTML::IHTMLTxtRange * rng, AU::ReMatch rm, CString txt = L"", CString htmlTxt = L"");

	void SelMatch(MSHTML::IHTMLTxtRange * tr, AU::ReMatch rm);
	MSHTML::IHTMLElementPtr SelectionContainerImp();

public:
	CFindDlgBase * m_find_dlg;
	CReplaceDlgBase * m_replace_dlg;

	SHD::IWebBrowser2Ptr Browser();
	MSHTML::IHTMLDocument2Ptr Document();
	bool HasDoc();
	IDispatchPtr Script();
	CString NavURL();
	bool Loaded();
	void Init();
	long GetVersionNumber();
	CAtlList<CString> m_UndoStrings;
	void BeginUndoUnit(const wchar_t * name);
	void EndUndoUnit();

	DECLARE_WND_SUPERCLASS(NULL, CAxWindow::GetWndClassName())

	CFBEView(HWND frame, bool fNorm);
	~CFBEView();

	BOOL PreTranslateMessage(MSG * pMsg);

	BEGIN_MSG_MAP(CFBEView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)

		// editing commands
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnUndo)
		COMMAND_ID_HANDLER(ID_EDIT_REDO, OnRedo)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE2, OnPaste)
		COMMAND_ID_HANDLER(ID_EDIT_BOLD, OnBold)
		COMMAND_ID_HANDLER(ID_EDIT_ITALIC, OnItalic)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnFind)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEXT, OnFindNext)
		COMMAND_ID_HANDLER(ID_EDIT_REPLACE, OnReplace)
		COMMAND_ID_HANDLER(ID_EDIT_STRIK, OnStrik)
		COMMAND_ID_HANDLER(ID_EDIT_SUP, OnSup)
		COMMAND_ID_HANDLER(ID_EDIT_SUB, OnSub)
		COMMAND_ID_HANDLER(ID_EDIT_CODE, OnCode)

		COMMAND_ID_HANDLER(ID_STYLE_LINK, OnStyleLink)
		COMMAND_ID_HANDLER(ID_STYLE_NOTE, OnStyleFootnote)
		COMMAND_ID_HANDLER(ID_STYLE_NOLINK, OnStyleNolink)

		COMMAND_ID_HANDLER(ID_STYLE_NORMAL, OnStyleNormal)
		COMMAND_ID_HANDLER(ID_STYLE_TEXTAUTHOR, OnStyleTextAuthor)
		COMMAND_ID_HANDLER(ID_STYLE_SUBTITLE, OnStyleSubtitle)

		COMMAND_ID_HANDLER(ID_EDIT_ADD_TITLE, OnEditAddTitle)
		COMMAND_ID_HANDLER(ID_EDIT_ADD_BODY, OnEditAddBody)
		COMMAND_ID_HANDLER(ID_EDIT_ADD_EPIGRAPH, OnEditAddEpigraph)
		COMMAND_ID_HANDLER(ID_EDIT_ADD_TA, OnEditAddTA)
		COMMAND_ID_HANDLER(ID_EDIT_CLONE, OnEditClone)
		COMMAND_ID_HANDLER(ID_EDIT_ADD_IMAGE, OnEditAddImage)
		COMMAND_ID_HANDLER(ID_EDIT_ADD_ANN, OnEditAddAnn)
		COMMAND_ID_HANDLER(ID_EDIT_INS_IMAGE, OnEditInsImage)
		COMMAND_ID_HANDLER(ID_EDIT_INS_INLINEIMAGE, OnEditInsImage)

		COMMAND_ID_HANDLER(ID_EDIT_SPLIT, OnEditSplit)
		COMMAND_ID_HANDLER(ID_EDIT_MERGE, OnEditMerge)
		COMMAND_ID_HANDLER(ID_EDIT_REMOVE_OUTER_SECTION, OnEditRemoveOuter)

		COMMAND_ID_HANDLER(ID_EDIT_INS_POEM, OnEditInsPoem)
		COMMAND_ID_HANDLER(ID_EDIT_INS_CITE, OnEditInsCite)
		COMMAND_ID_HANDLER_EX(ID_INSERT_TABLE, OnEditInsertTable)

		COMMAND_ID_HANDLER(ID_VIEW_HTML, OnViewHTML)
		COMMAND_ID_HANDLER(ID_SAVEIMG_AS, OnSaveImageAs)
		COMMAND_RANGE_HANDLER(ID_SEL_BASE, ID_SEL_BASE + 99, OnSelectElement)
	END_MSG_MAP()

	BEGIN_SINK_MAP(CFBEView)
		SINK_ENTRY_INFO(0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete, &DocumentCompleteInfo)
		SINK_ENTRY_INFO(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate, &BeforeNavigateInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONSELECTIONCHANGE, OnSelChange, &VoidEventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONCONTEXTMENU, OnContextMenu, &EventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONCLICK, OnClick, &EventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONKEYDOWN, OnKeyDown, &EventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONFOCUSIN, OnFocusIn, &VoidEventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLTextContainerEvents2, DISPID_HTMLELEMENTEVENTS2_ONPASTE, OnRealPaste, &EventInfo)
		SINK_ENTRY_INFO(0, DIID_HTMLTextContainerEvents2, DISPID_HTMLELEMENTEVENTS2_ONDRAGEND, OnDrop, &VoidEventInfo)
	END_SINK_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

	// editing commands
	LRESULT ExecCommand(int cmd);
	void QueryStatus(OLECMD * cmd, int ncmd);
	CString QueryCmdText(DWORD cmd);

	LRESULT OnUndo(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnCut(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnBold(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnItalic(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStrik(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnSup(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnSub(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnCode(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);

	LRESULT OnFind(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnFindNext(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnReplace(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleLink(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleFootnote(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleNolink(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleNormal(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleTextAuthor(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnStyleSubtitle(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnViewHTML(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnSelectElement(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddTitle(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddEpigraph(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddBody(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddTA(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditClone(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddImage(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditInsImage(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditAddAnn(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditMerge(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditSplit(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditInsPoem(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditInsCite(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnEditRemoveOuter(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);
	LRESULT OnSaveImageAs(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL & bHandled);

	// Modification by Pilgrim
	LRESULT OnEditInsertTable(WORD wNotifyCode, WORD wID, HWND hWndCtl);

	bool CheckCommand(WORD wID);
	bool CheckSetCommand(WORD wID);

	// Searching
	bool CanFindNext();

	void CancelIncSearch();
	void StartIncSearch();
	void StopIncSearch();
	bool DoIncSearch(const CString & str, bool fMore);
	bool DoSearch(bool fMore = true);
	bool DoSearchStd(bool fMore = true);
	bool DoSearchRegexp(bool fMore = true);
	void DoReplace();
	int GlobalReplace(MSHTML::IHTMLElementPtr elem = NULL, CString cntTag = L"P");
	int ToolWordsGlobalReplace(MSHTML::IHTMLElementPtr fbw_body, int * pIndex = NULL, int * globIndex = NULL, bool find = false, CString cntTag = L"P");

	BSTR PrepareDefaultId(const CString & filename);
	void AddImage(const CString & filename, bool bInline = false);

	CString LastSearchPattern();

	int ReplaceAllRe(const CString & re, const CString & str, MSHTML::IHTMLElementPtr elem = NULL, CString cntTag = L"P");
	int ReplaceToolWordsRe(const CString & re,
	                       const CString & str,
	                       MSHTML::IHTMLElementPtr fbw_body,
	                       bool replace = false,
	                       CString cntTag = L"P",
	                       int * pIndex = NULL,
	                       int * globIndex = NULL,
						   int replNum = 0);

	// searching in scintilla
	bool SciFindNext(CScintillaWindow & src, bool fFwdOnly, bool fBarf);

	// utilities
	CString SelPath();
	void GoTo(MSHTML::IHTMLElement * e, bool fScroll = true);
	MSHTML::IHTMLElementPtr SelectionContainer();

	bool GetSelectionInfo(MSHTML::IHTMLElementPtr * begin, MSHTML::IHTMLElementPtr * end, int * begin_char, int * end_char, MSHTML::IHTMLTxtRangePtr range);

	bool SelectionHasTags(wchar_t * elem);
	MSHTML::IHTMLElementPtr SelectionAnchor();
	MSHTML::IHTMLElementPtr SelectionAnchor(MSHTML::IHTMLElementPtr cur);
	MSHTML::IHTMLElementPtr SelectionStructCon();
	MSHTML::IHTMLElementPtr SelectionStructNearestCon();
	MSHTML::IHTMLElementPtr SelectionStructCode();
	MSHTML::IHTMLElementPtr SelectionStructImage();
	MSHTML::IHTMLElementPtr SelectionStructSection();
	MSHTML::IHTMLElementPtr SelectionStructTable();
	MSHTML::IHTMLElementPtr SelectionStructTableCon();
	MSHTML::IHTMLElementPtr SelectionsStyleT();
	MSHTML::IHTMLElementPtr SelectionsStyleTB(_bstr_t & style);
	MSHTML::IHTMLElementPtr SelectionsStyle();
	MSHTML::IHTMLElementPtr SelectionsStyleB(_bstr_t & style);
	MSHTML::IHTMLElementPtr SelectionsColspan();
	MSHTML::IHTMLElementPtr SelectionsColspanB(_bstr_t & colspan);
	MSHTML::IHTMLElementPtr SelectionsRowspan();
	MSHTML::IHTMLElementPtr SelectionsRowspanB(_bstr_t & rowspan);
	MSHTML::IHTMLElementPtr SelectionsAlignTR();
	MSHTML::IHTMLElementPtr SelectionsAlignTRB(_bstr_t & align);
	MSHTML::IHTMLElementPtr SelectionsAlign();
	MSHTML::IHTMLElementPtr SelectionsAlignB(_bstr_t & align);
	MSHTML::IHTMLElementPtr SelectionsVAlign();
	MSHTML::IHTMLElementPtr SelectionsVAlignB(_bstr_t & valign);

	void Normalize(MSHTML::IHTMLDOMNodePtr dom);
	MSHTML::IHTMLDOMNodePtr GetChangedNode();
	void ImgSetURL(IDispatch * elem, const CString & url);

	bool SplitContainer(bool fCheck);
	//  MSHTML::IHTMLDOMNodePtr	  ChangeAttribute(MSHTML::IHTMLElementPtr elem, const wchar_t* attrib, const wchar_t* value);
	bool InsertPoem(bool fCheck);
	bool InsertCite(bool fCheck);
	bool InsertTable(bool fCheck, bool bTitle = true, int nrows = 1);
	long InsertCode();
	bool GoToFootnote(bool fCheck);
	bool GoToReference(bool fCheck);
	MSHTML::IHTMLTxtRangePtr SetSelection(MSHTML::IHTMLElementPtr begin, MSHTML::IHTMLElementPtr end, int begin_pos, int end_pos);
	int GetRelationalCharPos(MSHTML::IHTMLDOMNodePtr node, int pos);
	int GetRealCharPos(MSHTML::IHTMLDOMNodePtr node, int pos);
	int CountNodeChars(MSHTML::IHTMLDOMNodePtr node);
	int GetRangePos(const MSHTML::IHTMLTxtRangePtr range, MSHTML::IHTMLElementPtr & element, int & pos);

	// script calls
	IDispatchPtr Call(const wchar_t * name);
	bool bCall(const wchar_t * name, int nParams, VARIANT * params);
	bool bCall(const wchar_t * name);
	IDispatchPtr Call(const wchar_t * name, IDispatch * pDisp);
	bool bCall(const wchar_t * name, IDispatch * pDisp);

	// binary objects
	_variant_t GetBinary(const wchar_t * id);

	// change notifications
	void EditorChanged(int id);

	// external helper
	static IDispatchPtr CreateHelper();

	// DWebBrowserEvents2
	void __stdcall OnDocumentComplete(IDispatch * pDisp, VARIANT * vtUrl);
	void __stdcall OnBeforeNavigate(IDispatch * pDisp, VARIANT * vtUrl, VARIANT * vtFlags,
	                                VARIANT * vtTargetFrame, VARIANT * vtPostData,
	                                VARIANT * vtHeaders, VARIANT_BOOL * fCancel);

	// HTMLDocumentEvents2
	void __stdcall OnSelChange(IDispatch * evt);
	VARIANT_BOOL __stdcall OnContextMenu(IDispatch * evt);
	VARIANT_BOOL __stdcall OnClick(IDispatch * evt);
	VARIANT_BOOL __stdcall OnKeyDown(IDispatch * evt);
	void __stdcall OnFocusIn(IDispatch * evt);

	// HTMLTextContainerEvents2
	VARIANT_BOOL __stdcall OnRealPaste(IDispatch * evt);
	void __stdcall OnDrop(IDispatch *);

	VARIANT_BOOL __stdcall OnDragDrop(IDispatch *);

	// form changes
	bool IsFormChanged();
	void ResetFormChanged();
	bool IsFormCP();
	void ResetFormCP();

	// extract currently selected text
	_bstr_t Selection();
	bool CloseFindDialog(CFindDlgBase * dlg);
	bool CloseFindDialog(CReplaceDlgBase * dlg);

	// added by SeNS
	long m_elementsNum;
	bool IsHTMLChanged();

private:
	bool ExpandTxtRangeToParagraphs(MSHTML::IHTMLTxtRangePtr & rng, MSHTML::IHTMLElementPtr & begin, MSHTML::IHTMLElementPtr & end) const;
	CString GetClearedRangeText(const MSHTML::IHTMLTxtRangePtr & rng) const;
	// added by SeNS
	int m_startMatch, m_endMatch;
};
