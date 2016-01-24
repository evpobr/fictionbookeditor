// FictionBook.h : Declaration of the CFictionBook

#pragma once

#include <atlhandlerimpl.h>
#include <atlimage.h>

using namespace ATL;

namespace FictionBook2
{

	class CFictionBook : public CAtlDocumentImpl
	{
	public:
		CFictionBook(void)
		{
		}

		virtual ~CFictionBook(void)
		{
		}

		virtual HRESULT LoadFromStream(IStream* pStream, DWORD grfMode);
		virtual void InitializeSearchContent();

	protected:
		CImage m_imgCoverpage;

		void SetSearchContent(CString& value);
		virtual void OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds);

		HRESULT LoadFB2Document(_In_ IStream *pStream, _COM_Outptr_ IXMLDOMDocument2 **pDoc);
		HRESULT DecodeBase64StringToStream(_In_ CString strBase64, _COM_Outptr_ IStream **pStream);
		HRESULT GetCoverpageHRef(_In_ IXMLDOMDocument2 *pDoc, _Out_ CString &strHRef);
		HRESULT GetBinaryNodeById(_In_ IXMLDOMDocument2 *pDoc, _In_z_ LPCWSTR pszId, _COM_Outptr_ IXMLDOMNode **pNode);
		HRESULT GetBinaryBase64String(_In_ IXMLDOMNode *pNode, _Out_ CString &strBase64);
	};
}
