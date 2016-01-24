// FictionBook.cpp : Implementation of CFictionBook

#include "stdafx.h"
#include <propkey.h>
#include <atlgdi.h>
#include "FictionBook.h"

using namespace FictionBook2;

HRESULT CFictionBook::LoadFromStream(IStream* pStream, DWORD grfMode)
{
	HRESULT hr = S_FALSE;
	// Load from stream your document data
	try
	{
		if (!pStream)
			AtlThrow(E_INVALIDARG);
		if (!m_imgCoverpage.IsNull())
			AtlThrow(HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED));


		CComPtr<IXMLDOMDocument2> spDoc = nullptr;
		hr = LoadFB2Document(pStream, &spDoc);
		if (SUCCEEDED(hr))
		{
			CString strHRef;
			hr = GetCoverpageHRef(spDoc, strHRef);
			if (SUCCEEDED(hr))
			{
				CComPtr<IXMLDOMNode> spNode;
				hr = GetBinaryNodeById(spDoc, strHRef, &spNode);
				if (SUCCEEDED(hr))
				{
					CString strBase64;
					hr = GetBinaryBase64String(spNode, strBase64);
					if (SUCCEEDED(hr))
					{
						CComPtr<IStream> spCoverpageStream = nullptr;
						hr = DecodeBase64StringToStream(strBase64, &spCoverpageStream);
						if (SUCCEEDED(hr))
						{
							hr = m_imgCoverpage.Load(spCoverpageStream);
						}
					}
				}
			}
		}
	}
	catch (CAtlException &ex)
	{
		hr = ex;
	}

	return hr;
}

void CFictionBook::InitializeSearchContent()
{
	// initialise search content from document's data as the following value
	CString value = _T("test;content;");
	SetSearchContent(value);
}

void CFictionBook::SetSearchContent(CString& value)
{
	// Assigns search content to PKEY_Search_Contents key
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

void CFictionBook::OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds)
{
		float imageWidth = (float)m_imgCoverpage.GetWidth();
		float imageHeight = (float)m_imgCoverpage.GetHeight();
		float scale = 0.0f;

		if (imageWidth <= imageHeight)
			scale = (float)lprcBounds->bottom - lprcBounds->top / (float)m_imgCoverpage.GetHeight();
		else
			scale = (float)lprcBounds->right - lprcBounds->left / (float)m_imgCoverpage.GetWidth();

		float thumbWidth = (float)imageWidth * scale;
		float thumbHeight = (float)imageHeight * scale;
}

HRESULT FictionBook2::CFictionBook::LoadFB2Document(IStream * pStream, IXMLDOMDocument2 ** pDoc)
{
	HRESULT hr;

	try
	{
		if (!pStream)
			AtlThrow(E_INVALIDARG);

		CComPtr<IXMLDOMDocument2> spDoc = nullptr;

		hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument.6.0");
		if FAILED(hr)
			AtlThrow(hr);

		hr = spDoc->put_async(VARIANT_FALSE);
		if FAILED(hr)
			AtlThrow(hr);
		hr = spDoc->put_validateOnParse(VARIANT_FALSE);
		if FAILED(hr)
			AtlThrow(hr);
		spDoc->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
		if FAILED(hr)
			AtlThrow(hr);
		spDoc->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(L"xmlns:fb=\"http://www.gribuser.ru/xml/fictionbook/2.0\"  xmlns:l=\"http://www.w3.org/1999/xlink\""));
		if FAILED(hr)
			AtlThrow(hr);

		CComVariant varStream(pStream);
		VARIANT_BOOL isSuccessful = VARIANT_FALSE;
		hr = spDoc->load(varStream, &isSuccessful);
		if FAILED(hr)
			AtlThrow(hr);

		*pDoc = spDoc.Detach();
	}
	catch (CAtlException &ex)
	{
		*pDoc = nullptr;
		hr = ex;
	}

	return hr;
}

HRESULT FictionBook2::CFictionBook::DecodeBase64StringToStream(CString strBase64, IStream ** pStream)
{
	HRESULT hr = E_FAIL;

	try
	{
		DWORD cbBinary = 0;
		BOOL bRet = ::CryptStringToBinaryW(strBase64, strBase64.GetLength(), CRYPT_STRING_BASE64, nullptr, &cbBinary, nullptr, nullptr);
		if (!bRet)
			AtlThrow(E_FAIL);
		CHeapPtr<BYTE> spBinary;
		spBinary.Allocate(cbBinary);
		bRet = ::CryptStringToBinaryW(strBase64, strBase64.GetLength(), CRYPT_STRING_BASE64, spBinary, &cbBinary, nullptr, nullptr);
		if (!bRet)
			AtlThrow(E_FAIL);

		// Copy image data to stream
		CComPtr<IStream> spStream = nullptr;
		spStream.Attach(SHCreateMemStream(nullptr, 0));
		if (!spStream)
			AtlThrow(E_FAIL);
		hr = spStream->Write(spBinary, cbBinary, nullptr);
		if FAILED(hr)
			AtlThrow(hr);

		*pStream = spStream.Detach();
	}
	catch (CAtlException &ex)
	{
		*pStream = nullptr;
		hr = ex;
	}

	return hr;
}

HRESULT FictionBook2::CFictionBook::GetCoverpageHRef(IXMLDOMDocument2 * pDoc, CString & strHRef)
{
	HRESULT hr = E_FAIL;
	CComPtr<IXMLDOMNode> spCoverpageImageNode;

	try
	{
		if (!pDoc)
			AtlThrow(hr);

		hr = pDoc->selectSingleNode(CComBSTR(L"//fb:FictionBook/fb:description/fb:title-info/fb:coverpage/fb:image"), &spCoverpageImageNode);
		if ((FAILED(hr)) || (hr == S_FALSE))
			AtlThrow(hr);

		CComPtr<IXMLDOMElement> spElement;
		hr = spCoverpageImageNode.QueryInterface(&spElement);
		if FAILED(hr)
			AtlThrow(hr);

		CComVariant varHRef;
		hr = spElement->getAttribute(CComBSTR(L"l:href"), &varHRef);
		if FAILED(hr)
			AtlThrow(hr);

		strHRef = varHRef;

		if (strHRef[0] == L'#')
			strHRef.Delete(0, 1);
		else
			AtlThrow(E_FAIL);
	}
	catch (CAtlException &ex)
	{
		hr = ex;
	}

	return hr;
}

HRESULT FictionBook2::CFictionBook::GetBinaryNodeById(IXMLDOMDocument2 * pDoc, LPCWSTR pszId, IXMLDOMNode ** pNode)
{
	HRESULT hr = E_FAIL;

	try
	{
		if (!pDoc)
			AtlThrow(E_INVALIDARG);
		if (!pszId)
			AtlThrow(E_INVALIDARG);

		CString strBinaryId;
		CComPtr<IXMLDOMNode> spNode = nullptr;

		strBinaryId.Format(L"//fb:FictionBook/fb:binary[@id='%s']", pszId);

		hr = pDoc->selectSingleNode(CComBSTR(strBinaryId), &spNode);
		if (FAILED(hr) || (hr == S_FALSE))
			AtlThrow(hr);

		*pNode = spNode.Detach();
	}
	catch (CAtlException &ex)
	{
		*pNode = nullptr;
		hr = ex;
	}

	return hr;
}

HRESULT FictionBook2::CFictionBook::GetBinaryBase64String(IXMLDOMNode * pNode, CString & strBase64)
{
	HRESULT hr = E_FAIL;

	try
	{
		if (!pNode)
			AtlThrow(E_INVALIDARG);

		CComBSTR bstrCoverpageData;
		hr = pNode->get_text(&bstrCoverpageData);
		if (FAILED(hr))
			AtlThrow(hr);

		strBase64 = bstrCoverpageData;
	}
	catch (CAtlException &ex)
	{
		strBase64.Empty();
		hr = ex;
	}

	return hr;
}
