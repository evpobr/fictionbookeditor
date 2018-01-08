#pragma once

#include "Settings.h"
#include "FBE.h"

extern CSettings _Settings;

static int modalResultCode;

class ExternalHelper : public CComObjectRoot,
                       public IDispatchImpl<IExternalHelper, &IID_IExternalHelper>
{
  public:
	DECLARE_NO_REGISTRY()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(ExternalHelper)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IExternalHelper)
	END_COM_MAP()

	// IExternalHelper
	STDMETHODIMP BeginUndoUnit(IDispatch * obj, BSTR name);
	STDMETHODIMP EndUndoUnit(IDispatch * obj);
	STDMETHODIMP get_inflateBlock(IDispatch * obj, BOOL * ifb);
	STDMETHODIMP put_inflateBlock(IDispatch * obj, BOOL ifb);
	STDMETHODIMP GenrePopup(IDispatch * obj, LONG x, LONG y, BSTR * name);
	STDMETHODIMP DescShowMenu(IDispatch * obj, LONG x, LONG y, BSTR * element_id);
	STDMETHODIMP GetStylePath(BSTR * name);
	STDMETHODIMP GetBinarySize(BSTR data, int * length);
	STDMETHODIMP InflateParagraphs(IDispatch * elem);
	STDMETHODIMP GetUUID(BSTR * uid);
	STDMETHODIMP GetNBSP(BSTR * nbsp);
	STDMETHODIMP MsgBox(BSTR message);
	STDMETHODIMP AskYesNo(BSTR message, BOOL * pVal);
	STDMETHODIMP SaveBinary(BSTR path, BSTR data, BOOL prompt, BOOL * ret);
	STDMETHODIMP GetExtendedStyle(BSTR elem, BOOL * ext);
	STDMETHODIMP IsFastMode(BOOL * ext);
	STDMETHODIMP DescShowElement(BSTR elem, BOOL show);
	STDMETHODIMP SetStyleEx(IDispatch * /*doc*/, IDispatch * elem, BSTR style);
	STDMETHODIMP GetImageDimsByPath(BSTR path, BSTR * dims);
	STDMETHODIMP GetImageDimsByData(VARIANT * data, BSTR * dims);
	STDMETHODIMP GetViewWidth(int * width);
	STDMETHODIMP GetViewHeight(int * height);
	STDMETHODIMP GetProgramVersion(BSTR * ver);
	STDMETHODIMP InputBox(BSTR prompt, BSTR title, BSTR value, BSTR * input);
	STDMETHODIMP GetModalResult(int * modalResult);
	STDMETHODIMP SetStatusBarText(BSTR text);
};
