#ifndef CBROWSEFOLDERDIALOG_H
#define CBROWSEFOLDERDIALOG_H

#include <tchar.h>

class CBrowseFolderDialog : public CWnd
{
public:

	CBrowseFolderDialog(UINT ulFlags = 0, LPCTSTR lpTitle = NULL, int nRootFolder = 0);  
	virtual ~CBrowseFolderDialog(); 

	virtual void SetInfo(UINT ulFlags = 0, LPCTSTR lpTitle = NULL, int nRootFolder = 0);

	virtual void GetSelectStr(CString& str, BOOL fFullPath = TRUE);

	virtual int GetSelectStr(LPTSTR lpStr, int nMax, BOOL fFullPath = TRUE);

	virtual int DoModal(LPCTSTR lpSelect);      
	virtual int DoModal(int nSelectFolder);     
	virtual int DoModal();                      

protected:

	static int CALLBACK CallBackFunc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData); 
	virtual void Init();                                  
	virtual int  Modal();                                 
	virtual BOOL GetPIDL(int nFolder,LPITEMIDLIST *pidl); 
	virtual void FreePIDL(LPITEMIDLIST pidl);             

	
	virtual BOOL OnInitDialog(){return TRUE;}                 
	virtual void OnChangedSel(HWND hwnd,LPITEMIDLIST pList){} 
	virtual void OnFailedValidate(LPCTSTR lpStr){}            

	LPITEMIDLIST m_rootPidl;          
	LPITEMIDLIST m_selectPidl;        
	int          m_nRootFolder;       
	int          m_nSelectFolder;     

	TCHAR m_szFirstPath[MAX_PATH];     
	TCHAR m_szSelectedPath[MAX_PATH];  
	TCHAR m_szDisplayName[MAX_PATH];   

	UINT m_ulFlags;                   
	TCHAR m_szTitle[256];              

	//{{AFX_MSG(CBrowseFolderDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
		
#endif  // __BROWSEFOLDERDIALOG_H__
