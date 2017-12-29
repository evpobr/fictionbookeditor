#include "stdafx.h"
#include "ContextMenu.h"

// IShellExtInit
HRESULT	CContextMenu::Initialize(_In_opt_ PCIDLIST_ABSOLUTE pidlFolder,
								 _In_opt_ IDataObject *pdtobj,
								 _In_opt_ HKEY hkeyProgID)
{
	Lock();
	m_files.RemoveAll();
	m_folders = false;
	Unlock();

	if (!pdtobj)
	{
		// we call as a directory background context menu
		CString tmp;
		TCHAR *cp = tmp.GetBuffer(MAX_PATH);
		if (SHGetPathFromIDList(pidlFolder, cp))
		{
			tmp.ReleaseBuffer();
			Lock();
			m_files.Add(tmp);
			Unlock();
			m_folders = true;
		}
		return S_OK;
	}

	HRESULT hr;

	STGMEDIUM stg;
	FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	UINT count;

	if (FAILED(hr = pdtobj->GetData(&etc, &stg)))
		return hr;

	count = ::DragQueryFile((HDROP)stg.hGlobal, (UINT)-1, NULL, 0);

	Lock();

	for (UINT i = 0; i < count; ++i)
	{
		UINT  len = ::DragQueryFile((HDROP)stg.hGlobal, i, NULL, 0);
		if (!len)
			continue;

		CString fname;
		TCHAR *cp = fname.GetBuffer(len + 1);
		len = ::DragQueryFile((HDROP)stg.hGlobal, i, cp, len + 1);
		fname.ReleaseBuffer(len);

		if (fname.Right(4).CompareNoCase(_T(".fb2")) == 0)
		{
			m_files.Add(fname);
		}
		else
		{
			DWORD attr = ::GetFileAttributes(fname);
			if (attr != INVALID_FILE_ATTRIBUTES && attr&FILE_ATTRIBUTE_DIRECTORY)
			{
				m_files.Add(fname);
				m_folders = true;
			}
		}
	}

	Unlock();

	::ReleaseStgMedium(&stg);

	return S_OK;
}

// context menu commands
static struct {
	const wchar_t *verb;
	const wchar_t *text;
	const wchar_t *foldertext;
	const wchar_t *desc;
} g_menu_items[] =
{
  { L"Validate", L"Validate", L"Validate FictionBook Documents", L"Validate selected documents" },
};
#define	NCOMMANDS (sizeof(g_menu_items)/sizeof(g_menu_items[0]))

// IContextMenu
HRESULT	CContextMenu::QueryContextMenu(_In_ HMENU hmenu,
									   _In_ UINT indexMenu,
									   _In_ UINT idCmdFirst,
									   _In_ UINT idCmdLast,
									   _In_ UINT uFlags)
{
	if (!(uFlags & CMF_DEFAULTONLY) && m_files.GetSize() > 0)
	{
		UINT cmd;

		Lock();
		for (cmd = 0; cmd < NCOMMANDS && cmd + idCmdFirst <= idCmdLast; ++cmd)
		{
			::InsertMenu(hmenu, indexMenu + cmd, MF_BYPOSITION | MF_STRING, idCmdFirst + cmd,
						 m_folders ? g_menu_items[cmd].foldertext : g_menu_items[cmd].text);
		}
		Unlock();

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)cmd);
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)0);
}

HRESULT	CContextMenu::GetCommandString(_In_ UINT_PTR idCmd,
									   _In_ UINT uType,
									   _Reserved_ UINT *pReserved,
									   _Out_writes_bytes_((uType & GCS_UNICODE) ? (cchMax * sizeof(wchar_t)) : cchMax) _When_(!(uType & (GCS_VALIDATEA | GCS_VALIDATEW)), _Null_terminated_) CHAR *pszName,
									   _In_ UINT cchMax)
{
	if (idCmd >= NCOMMANDS)
		return E_INVALIDARG;

	switch (uType)
	{
	case GCS_HELPTEXTA:
		::WideCharToMultiByte(CP_ACP, 0, g_menu_items[idCmd].desc, -1, pszName, cchMax, NULL, NULL);
		break;
	case GCS_HELPTEXTW:
		lstrcpynW((wchar_t *)pszName, g_menu_items[idCmd].desc, cchMax);
		break;
	case GCS_VERBA:
		::WideCharToMultiByte(CP_ACP, 0, g_menu_items[idCmd].verb, -1, pszName, cchMax, NULL, NULL);
		break;
	case GCS_VERBW:
		lstrcpynW((wchar_t *)pszName, g_menu_items[idCmd].verb, cchMax);
		break;
	}

	return S_OK;
}

HRESULT	CContextMenu::InvokeCommand(_In_ CMINVOKECOMMANDINFO *pici)
{
	bool fEx = pici->cbSize >= sizeof(CMINVOKECOMMANDINFOEX);
	bool fUnicode = fEx && (pici->fMask & CMIC_MASK_UNICODE);

	CMINVOKECOMMANDINFOEX *iciex = (CMINVOKECOMMANDINFOEX *)pici;

	UINT cmd = LOWORD(iciex->lpVerb);

	if (fUnicode && HIWORD(iciex->lpVerbW))
	{
		for (cmd = 0; cmd < NCOMMANDS; ++cmd)
		{
			if (lstrcmpW(iciex->lpVerbW, g_menu_items[cmd].verb) == 0)
			{
				break;
			}
		}
	}
	else if (!fUnicode && HIWORD(pici->lpVerb))
	{
		CString	tmp(pici->lpVerb);
		for (cmd = 0; cmd < NCOMMANDS; ++cmd)
		{
			if (tmp == pici->lpVerb)
			{
				break;
			}
		}
	}

	if (cmd >= NCOMMANDS)
		return E_INVALIDARG;

	Lock();

	// get program dir
	CString tmp;
	TCHAR *cp = tmp.GetBuffer(MAX_PATH);
	::GetModuleFileName(_AtlBaseModule.GetModuleInstance(), cp, MAX_PATH);
	tmp.ReleaseBuffer();

	int slash = tmp.ReverseFind(_T('\\'));
	if (slash >= 0)
		tmp.Delete(slash + 1, tmp.GetLength() - slash - 1);

	// dispatch the verb
	if (lstrcmpW(g_menu_items[cmd].verb, L"Validate") == 0)
	{
		tmp += _T("FBV.exe");

		// count needed buffer size
		UINT fsize;
		int i;

		for (i = fsize = 0; i < m_files.GetSize(); ++i)
			fsize += m_files[i].GetLength() + 3; // +space + 2x"

		CString fnames;
		TCHAR *buf = fnames.GetBuffer(fsize);
		TCHAR *cp = buf;

		*cp++ = _T('"');
		memcpy(cp, m_files[0], m_files[0].GetLength() * sizeof(TCHAR));
		cp += m_files[0].GetLength();
		*cp++ = _T('"');

		for (i = 1; i < m_files.GetSize(); ++i)
		{
			*cp++ = _T(' ');
			*cp++ = _T('"');
			memcpy(cp, m_files[i], m_files[i].GetLength() * sizeof(TCHAR));
			cp += m_files[i].GetLength();
			*cp++ = _T('"');
		}

		fnames.ReleaseBuffer(cp - buf);

		PROCESS_INFORMATION	pi;
		STARTUPINFO si;

		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		si.wShowWindow = pici->nShow;
		si.dwFlags = STARTF_USESHOWWINDOW;

		buf = fnames.GetBuffer(fnames.GetLength());

		if (::CreateProcess(tmp, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);
		}
		else
		{
			DWORD code = ::GetLastError();
			CString	msg;
			msg.Format(_T("Can't run %s: "), (const wchar_t *)tmp);
			int	len = msg.GetLength();
			TCHAR *vp = msg.GetBuffer(len + 1024);
			int fl = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, 0, vp + len, 1024, NULL);
			msg.ReleaseBuffer(fl + len);
			AtlTaskDialog(pici->hwnd, IDS_ERROR, (LPCTSTR)msg, (LPCTSTR)NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		}
	}

	Unlock();

	return S_OK;
}
