// ThumbnailHandler.cpp : Implementation of CThumbnailHandler

#include "stdafx.h"
#include "ThumbnailHandler.h"


// CThumbnailHandler

HRESULT CThumbnailHandler::LoadFB2Document(IStream * pStream, _COM_Outptr_ IXMLDOMDocument2 **pDoc)
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

HRESULT CThumbnailHandler::DecodeBase64StringToStream(CString strBase64, IStream ** pStream)
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

HRESULT CThumbnailHandler::GetCoverpageHRef(IXMLDOMDocument2 * pDoc, CString & strHRef)
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

HRESULT CThumbnailHandler::GetBinaryNodeById(IXMLDOMDocument2 * pDoc, LPCWSTR pszId, IXMLDOMNode ** pNode)
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

HRESULT CThumbnailHandler::GetBinaryBase64String(IXMLDOMNode * pNode, CString & strBase64)
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

HRESULT CThumbnailHandler::Initialize(IStream * pstream, DWORD grfMode)
{
	HRESULT hr;
	try
	{
		if (!m_imgCoverpage.IsNull())
			AtlThrow(HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED));

		CComPtr<IXMLDOMDocument2> spDoc = nullptr;
		hr = LoadFB2Document(pstream, &spDoc);
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
						CComPtr<IStream> spStream = nullptr;
						hr = DecodeBase64StringToStream(strBase64, &spStream);
						if (SUCCEEDED(hr))
							hr = m_imgCoverpage.Load(spStream);
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

HRESULT CThumbnailHandler::GetThumbnail(UINT cx, HBITMAP * phbmp, WTS_ALPHATYPE * pdwAlpha)
{
	HRESULT hr = S_FALSE;
	float imageWidth = (float)m_imgCoverpage.GetWidth();
	float imageHeight = (float)m_imgCoverpage.GetHeight();
	float scale = 0.0f;

	if ((int)imageWidth <= (int)imageHeight)
		scale = (float)cx / (float)m_imgCoverpage.GetHeight();
	else
		scale = (float)cx / (float)m_imgCoverpage.GetWidth();


	float thumbWidth = (float)imageWidth * scale;
	float thumbHeight = (float)imageHeight * scale;

	CImage thumb;
	BOOL bRet = thumb.Create((int)thumbWidth, (int)thumbHeight, 32);
	if (bRet)
	{
		bRet = m_imgCoverpage.Draw(
			thumb.GetDC(),
			CRect(0, 0, (int)thumbWidth, (int)thumbHeight),
			Gdiplus::InterpolationMode::InterpolationModeHighQuality);

		if (bRet)
		{
			thumb.ReleaseDC();
			*phbmp = thumb.Detach();
			*pdwAlpha = WTSAT_UNKNOWN;
			hr = S_OK;
		}
	}

	return hr;
}
