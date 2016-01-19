#include "stdafx.h"
#include "IconExtractor.h"

#include "FBShell.h"

// IExtractImage
HRESULT CIconExtractor::GetLocation(wchar_t *file,DWORD filelen,DWORD *prio,
				   const SIZE *sz,
				   DWORD depth,DWORD *flags)
{
  m_desired_size=*sz;
  m_desired_depth=depth;


  *flags|=IEIFLAG_CACHE;

  lstrcpynW(file,m_filename,filelen);

  if (*flags & IEIFLAG_ASYNC) {
    *prio = 1;
    return E_PENDING;
  }

  return S_OK;
}

HRESULT CIconExtractor::Extract(HBITMAP *hBmp) {
  // load image if available
  CString     type = _T("");
  int	      datalen = 0;
  void	      *data=NULL;
  if (!LoadObject(m_filename,type,data,datalen))
    return E_FAIL;

  CComPtr<IStream> spImageStream;
  HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &spImageStream);
  if (SUCCEEDED(hr))
  {
	  ULONG cbRead = 0;
	  hr = spImageStream->Write(data, datalen, &cbRead);
	  free(data);
	  if (SUCCEEDED(hr))
	  {
		  CImage image;
		  image.Load(spImageStream);

		  float imageWidth = (float)image.GetWidth();
		  float imageHeight = (float)image.GetHeight();
		  float scale = 0.0f;

		  if (imageWidth <= imageHeight)
			  scale = (float)m_desired_size.cy / (float)image.GetHeight();
		  else
			  scale = (float)m_desired_size.cx / (float)image.GetWidth();

		  float thumbWidth = (float)imageWidth * scale;
		  float thumbHeight = (float)imageHeight * scale;

		  CImage thumb;
		  thumb.Create((int)thumbWidth, (int)thumbHeight, m_desired_depth);

		  image.Draw(thumb.GetDC(), CRect(0, 0, (int)thumbWidth, (int)thumbHeight), Gdiplus::InterpolationMode::InterpolationModeHighQuality);

		  thumb.ReleaseDC();

		  *hBmp = thumb.Detach();

		  hr = S_OK;
	  }
  }

  return hr;

}

// IExtractImage2
HRESULT	CIconExtractor::GetDateStamp(FILETIME *tm) {
  HANDLE  hFile=::CreateFile(m_filename,FILE_READ_ATTRIBUTES,0,NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return HRESULT_FROM_WIN32(::GetLastError());
  ::GetFileTime(hFile,NULL,NULL,tm);
  ::CloseHandle(hFile);
  return S_OK;
}

///////////////////////////////////////////////////////////
// SAX xml content handler (I use SAX instead of DOM for speed)
class CIconExtractor::ContentHandlerImpl :
  public CComObjectRoot,
  public ISAXContentHandler
{
public:
  enum ParseMode {
    NONE,
    COVERPAGE,
    DATA
  };

  // construction
  ContentHandlerImpl()
	  : m_mode(NONE), m_ok(false), m_cover_id(_T("")), m_cover_type(_T("")), m_strBase64Image(_T("")), m_nDestLen(0), m_bDest(NULL)
  {
  }

  ~ContentHandlerImpl() {
  }

  DECLARE_NO_REGISTRY()

  BEGIN_COM_MAP(ContentHandlerImpl)
    COM_INTERFACE_ENTRY(ISAXContentHandler)
  END_COM_MAP()

  // ISAXContentHandler
  STDMETHOD(characters)(const wchar_t *chars,int nch);
  STDMETHOD(endDocument)() { return S_OK; }
  STDMETHOD(startDocument)() { return S_OK; }
  STDMETHOD(endElement)(const wchar_t *nsuri,int nslen, const wchar_t *name,int namelen,
	  const wchar_t *qname,int qnamelen);
  STDMETHOD(startElement)(const wchar_t *nsuri,int nslen, const wchar_t *name,int namelen,
	  const wchar_t *qname,int qnamelen,ISAXAttributes *attr);
  STDMETHOD(ignorableWhitespace)(const wchar_t *spc,int spclen) { return S_OK; }
  STDMETHOD(endPrefixMapping)(const wchar_t *prefix,int len) { return S_OK; }
  STDMETHOD(startPrefixMapping)(const wchar_t *prefix,int plen, const wchar_t *uri,int urilen) { return S_OK; }
  STDMETHOD(processingInstruction)(const wchar_t *targ,int targlen, const wchar_t *data,int datalen) { return S_OK; }
  STDMETHOD(skippedEntity)(const wchar_t *name,int namelen) { return S_OK; }
  STDMETHOD(putDocumentLocator)(ISAXLocator *loc) { return S_OK; }

  // data access
  void	  *Detach() {
	  if (m_bDest)
		  return m_bDest.Detach();
	  else
		  return NULL;
  }
  int	  Length() { return m_nDestLen; }
  CString Type() { return m_cover_type; }
  bool	  Ok() { return m_ok; }

private:
  ParseMode	  m_mode;
  bool		  m_ok;

  CString	  m_cover_id;
  CString	  m_cover_type;
  CString	m_strBase64Image;
  int		m_nDestLen;
  CHeapPtr<BYTE> m_bDest;

};

bool    CIconExtractor::LoadObject(const wchar_t *filename,CString& type,void *&data,int& datalen)
{
  ContentHandlerPtr	      ch = NULL;
  if (FAILED(CreateObject(ch)))
    return IStreamPtr();

  ISAXXMLReaderPtr    rdr = NULL;
  if (FAILED(rdr.CreateInstance(L"MSXML2.SAXXMLReader.6.0")))
    return IStreamPtr();

  rdr->putContentHandler(ch);

  rdr->parseURL(filename);

  type=ch->Type();
  datalen=ch->Length();
  data=ch->Detach();

  return true;
}

HRESULT	CIconExtractor::ContentHandlerImpl::endElement(const wchar_t *nsuri,int nslen,
	const wchar_t *name,int namelen,
	const wchar_t *qname,int qnamelen)
{
  // all elements must be in a fictionbook namespace
  if (!StrEQ(FBNS,(wchar_t*)nsuri,nslen))
    return E_FAIL;

  switch (m_mode) {
  case NONE:
    if (StrEQ(L"description", (wchar_t*)name,namelen) && m_cover_id.IsEmpty())
	return E_FAIL;
    break;
  case COVERPAGE:
    if (StrEQ(L"coverpage", (wchar_t*)name,namelen)) {
      if (m_cover_id.IsEmpty())
	return E_FAIL;
      m_mode=NONE;
    }
    break;
  case DATA:
    // if we got here and have some bits left in our buffer then we have malformed
    // base64 data
	  DWORD cbBinary = 0;
	  HRESULT hr = S_OK;
	  if (!CryptStringToBinaryW(m_strBase64Image, m_strBase64Image.GetLength(), CRYPT_STRING_BASE64, NULL, &cbBinary, 0, NULL))
		  hr = E_FAIL;

	  if (SUCCEEDED(hr))
	  {
		  m_nDestLen = cbBinary;
		  m_bDest.Allocate(m_nDestLen);
		  if (!CryptStringToBinaryW(m_strBase64Image, m_strBase64Image.GetLength(), CRYPT_STRING_BASE64, m_bDest, &cbBinary, 0, NULL))
			  hr = E_FAIL;

		  m_nDestLen = cbBinary;

		  if (FAILED(hr))
			  m_bDest.Free();

		  m_mode = NONE;
	  }

	  return hr;
  }

  return S_OK;
}

HRESULT	CIconExtractor::ContentHandlerImpl::startElement(const wchar_t *nsuri,int nslen,
	const wchar_t *name,int namelen,
	const wchar_t *qname,int qnamelen,
	ISAXAttributes *attr)
{
  // all elements must be in a fictionbook namespace
  if (!StrEQ(FBNS, (wchar_t*)nsuri,nslen))
    return E_FAIL;

  switch (m_mode) {
  case NONE:
    if (StrEQ(L"coverpage", (wchar_t*)name,namelen))
      m_mode=COVERPAGE;
    else if (StrEQ(L"binary", (wchar_t*)name,namelen)) {
      if (m_cover_id.IsEmpty()) // invalid file
	return E_FAIL;
      if (m_cover_id!=GetAttr(attr,L"id"))
	return S_OK;

      m_cover_type=GetAttr(attr,L"content-type");

      m_mode=DATA;
    }
    break;
  case COVERPAGE:
    if (StrEQ(L"image", (wchar_t*)name,namelen)) {
      CString	tmp(GetAttr(attr,L"href",XLINKNS));
      if (tmp.GetLength()>1 && tmp[0]==_T('#')) {
	m_cover_id=tmp;
	m_cover_id.Delete(0);
	m_mode=NONE;
      }
    }
    break;
  }

  return S_OK;
}

static BYTE	g_base64_table[256]={
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,62,65,65,65,63,52,53,54,55,56,57,
58,59,60,61,65,65,65,64,65,65,65,0,1,2,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,65,65,65,
65,65,65,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65
};

HRESULT	CIconExtractor::ContentHandlerImpl::characters(const wchar_t *chars,int nch) {
  if (m_mode!=DATA)
    return S_OK;

  // process base64 data and append to m_data

  if ((chars != NULL) && (nch > 0))
  m_strBase64Image.Append((LPCWSTR)chars, nch);


  return S_OK;
}