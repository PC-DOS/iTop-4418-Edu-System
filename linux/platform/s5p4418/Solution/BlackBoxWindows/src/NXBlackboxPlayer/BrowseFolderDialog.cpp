#include "stdafx.h"
#include "BrowseFolderDialog.h"

BEGIN_MESSAGE_MAP(CBrowseFolderDialog, CWnd)
	//{{AFX_MSG_MAP(CBrowseFolderDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CBrowseFolderDialog::CBrowseFolderDialog(UINT ulFlags, LPCTSTR lpTitle, int nRootFolder)
{
	Init();

	memset(m_szTitle, 0, sizeof(m_szTitle));

	m_ulFlags     = ulFlags;
	m_nRootFolder = nRootFolder;

	if( lpTitle != NULL ) {
		_tcscpy_s(m_szTitle, lpTitle);
	}
}

CBrowseFolderDialog::~CBrowseFolderDialog()
{
	
}


void CBrowseFolderDialog::Init()
{
	m_rootPidl   = NULL;
	m_selectPidl = NULL;
	m_nSelectFolder = 0;

	memset(m_szSelectedPath, 0, sizeof(m_szSelectedPath));	
	memset(m_szFirstPath, 0, sizeof(m_szFirstPath));	
	memset(m_szDisplayName, 0, sizeof(m_szDisplayName));
}


void CBrowseFolderDialog::SetInfo(UINT ulFlags, LPCTSTR lpTitle, int nRootFolder)
{
	m_ulFlags     = ulFlags;
	m_nRootFolder = nRootFolder;

	if( lpTitle != NULL ) {
		_tcscpy_s(m_szTitle, lpTitle);
	}
}


int CALLBACK CBrowseFolderDialog::CallBackFunc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CBrowseFolderDialog *dlg = (CBrowseFolderDialog*)lpData;

	switch( uMsg )
	{
		case BFFM_INITIALIZED:    
			if( dlg->m_szFirstPath[0] != NULL ) {
				::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)dlg->m_szFirstPath);
			} else if( dlg->m_selectPidl != NULL ) {
				::SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)dlg->m_selectPidl);
			}

			dlg->SubclassWindow( hwnd );
			dlg->OnInitDialog();
			break;
		case BFFM_SELCHANGED:     
			dlg->OnChangedSel(hwnd,(LPITEMIDLIST)lParam);
			break;
		case BFFM_VALIDATEFAILED: 
			dlg->OnFailedValidate((LPCTSTR)lParam);
			break;
	}

	return 0;
}


int CBrowseFolderDialog::DoModal()
{	
	Init();

	return Modal();
}


int CBrowseFolderDialog::DoModal(LPCTSTR lpSelect)
{	
	Init();

	if( lpSelect != NULL ) {
		_tcscpy_s(m_szFirstPath, lpSelect);
	}

	return Modal();
}


int CBrowseFolderDialog::DoModal(int nSelectFolder)
{	
	Init();

	m_nSelectFolder = nSelectFolder;

	return Modal();
}


int CBrowseFolderDialog::Modal()
{	
	
	if( GetPIDL(m_nRootFolder,&m_rootPidl) == FALSE ) {
		return -1;
	}
	
	if( GetPIDL(m_nSelectFolder,&m_selectPidl) == FALSE ) {
		return -1;
	}

	BROWSEINFO bi;
	ZeroMemory(&bi,sizeof(bi));

	bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	bi.pszDisplayName = m_szDisplayName;
	bi.pidlRoot  = m_rootPidl;   
	bi.lpszTitle = m_szTitle;    
	bi.ulFlags   = m_ulFlags;    
	bi.lpfn      = CallBackFunc; 
	bi.lParam    = (LPARAM)this;

	LPITEMIDLIST pList = ::SHBrowseForFolder(&bi);

	if( pList == NULL ) {
		FreePIDL(pList);
		return IDCANCEL;
	}

	SHGetPathFromIDList(pList, m_szSelectedPath);
	FreePIDL(pList);

	return IDOK;	
}


BOOL CBrowseFolderDialog::GetPIDL(int nFolder, LPITEMIDLIST *pidl)
{
	if( nFolder == 0 ) {
		*pidl = NULL;
		return TRUE;
	}
	
	LPITEMIDLIST pidlTmp;
	if( SHGetSpecialFolderLocation(AfxGetMainWnd()->GetSafeHwnd(), nFolder, &pidlTmp) == NOERROR ) {
		*pidl = pidlTmp;
		return TRUE;
	} else {
		return FALSE;
	}
}


void CBrowseFolderDialog::FreePIDL(LPITEMIDLIST pidl)
{
	CoTaskMemFree(pidl);
	CoTaskMemFree(m_rootPidl);
	CoTaskMemFree(m_selectPidl);
}


void CBrowseFolderDialog::GetSelectStr(CString& str, BOOL fFullPath)
{
	
	if( fFullPath == TRUE ) {
		str = m_szSelectedPath;
	} else {
		str = m_szDisplayName;
	}
}

int CBrowseFolderDialog::GetSelectStr(LPTSTR lpStr, int nMax, BOOL fFullPath)
{
	int nStrLen;
	int nCopyLen;

	if( fFullPath == TRUE ) {
		nStrLen = _tcslen(m_szSelectedPath);
		if( nStrLen == 0 )
			return 0;

		if( nStrLen > nMax )
			nCopyLen = nMax;
		else
			nCopyLen = nStrLen;

		//_tcsncpy(lpStr, m_szSelectedPath, nCopyLen);
		_tcsncpy_s(lpStr, sizeof(lpStr), m_szSelectedPath, nCopyLen);
		*(lpStr + nCopyLen) = NULL;
	} else {
		nStrLen = _tcslen(m_szDisplayName);
		if( nStrLen == 0 )
			return 0;

		if( nStrLen > nMax )
			nCopyLen = nMax;
		else
			nCopyLen = nStrLen;

		//_tcsncpy(lpStr, m_szDisplayName, nCopyLen);
		_tcsncpy_s(lpStr, sizeof(lpStr), m_szDisplayName, nCopyLen);
		*(lpStr + nCopyLen) = NULL;
	}

	return nCopyLen;
}
