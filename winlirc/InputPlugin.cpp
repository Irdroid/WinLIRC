// InputPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "winlirc.h"
#include "InputPlugin.h"
#include "irconfig.h"

static HMODULE loadLibrary(CString const& lib)
{
	UINT const backup = ::SetErrorMode(~0);
	HMODULE const res = ::LoadLibrary(lib);
	::SetErrorMode(backup);
	return res;
}

// InputPlugin dialog

InputPlugin::InputPlugin()
{
	m_hasGuiFunction		= NULL;
	m_loadSetupGuiFunction	= NULL;
	m_dllFile				= NULL;
}

void InputPlugin::listDllFiles()
{
	CFileFind	cFileFind;
	BOOL found = cFileFind.FindFile(_T(".\\*.dll"));

	if (!found)
	{
		MessageBox(_T("No valid dlls found."));
	}
	else
	{
		while (found)
		{
			found = cFileFind.FindNextFile();
			if (checkDllFile(cFileFind.GetFileName()))
				m_cboxInputPlugin.AddString(cFileFind.GetFileName());
		}
	}
}

bool InputPlugin::checkDllFile(CString file) {

	//==========
	HMODULE tmp;
	//==========

	tmp = loadLibrary(file);

	if(!tmp) return false;

	if(!GetProcAddress(tmp,"init"))			{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"deinit"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"hasGui"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"loadSetupGui")) { FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"sendIR"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"decodeIR"))		{ FreeLibrary(tmp); return false; }

	FreeLibrary(tmp);

	return true;
}

bool InputPlugin::checkRecording(CString file) {

	//==========
	HMODULE tmp;
	//==========

	tmp = loadLibrary(file);

	if(!tmp) return false;

	if(!GetProcAddress(tmp,"getHardware")) { 
		FreeLibrary(tmp); return false; 
	}

	FreeLibrary(tmp);

	return true;
}

void InputPlugin::enableWindows(bool canRecord)
{
	m_setupButton.EnableWindow(m_hasGuiFunction && m_hasGuiFunction());
	m_configPath.EnableWindow(canRecord);
	m_createConfigButton.EnableWindow(canRecord);
	m_browseButton.EnableWindow(canRecord);
}

void InputPlugin::loadDll(CString file) {

	m_dllFile = loadLibrary(file);

	if(!m_dllFile) return;

	m_hasGuiFunction		= (HasGuiFunction)			GetProcAddress(m_dllFile,"hasGui");
	m_loadSetupGuiFunction	= (LoadSetupGuiFunction)	GetProcAddress(m_dllFile,"loadSetupGui");
}

void InputPlugin::unloadDll() {

	//
	// make sure we have cleaned up
	//
	m_hasGuiFunction		= NULL;
	m_loadSetupGuiFunction	= NULL;

	FreeLibrary(m_dllFile);

	m_dllFile				= NULL;
}

// InputPlugin message handlers

LRESULT InputPlugin::OnCbnSelchangeInputPlugin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	unloadDll();

	DoDataExchange(TRUE);

	CString m_cboxInputPluginStr; m_cboxInputPlugin.GetLBText(m_cboxInputPluginIndex, m_cboxInputPluginStr);
	CString const file = _T(".\\") + m_cboxInputPluginStr;
	bool const validFile	= checkDllFile(file);
	bool const canRecord	= checkRecording(file);

	if (!validFile)
		MessageBox(_T("Invalid dll file"), _T("Error"), 0);

	loadDll(file);
	enableWindows(canRecord);
	return 0;
}

LRESULT InputPlugin::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);

	//
	// some basic error checking
	//

	if (!m_configPathStr.IsEmpty())
	{
		if (!PathFileExists(m_configPathStr))
		{
			MessageBox(	_T("The configuration filename is invalid.\n")
				_T("Please try again."),_T("Configuration Error"));
			return 0;
		}
	}

	config.remoteConfig = m_configPathStr;
	config.plugin = m_cboxInputPluginStr;
	config.disableRepeats = (m_disableKeyRepeatsInt == BST_CHECKED);
	config.disableFirstKeyRepeats = m_disableFirstRepeatsInt;
	config.localConnectionsOnly = (m_allowLocalConnectionsOnly == BST_CHECKED);
	config.showTrayIcon = (m_disableSystemTrayIcon != BST_CHECKED);

	config.writeINIFile();

	EndDialog(IDOK);
	return 0;
}

LRESULT InputPlugin::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);
	CFileDialog fileDlg(TRUE, NULL, m_configPathStr, OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_ENABLESIZING, NULL, *this);

	if (fileDlg.DoModal() == IDOK)
	{
		fileDlg.GetFilePath(m_configPathStr.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
		DoDataExchange(FALSE);
	}
	return 0;
}

LRESULT InputPlugin::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT InputPlugin::OnBnClickedPluginSetup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_loadSetupGuiFunction) {

		this->EnableWindow(FALSE);
		m_loadSetupGuiFunction();
		this->EnableWindow(TRUE);
		this->SetFocus();
	}
	return 0;
}

LRESULT InputPlugin::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange(FALSE);

	listDllFiles();

	m_cboxInputPluginIndex = m_cboxInputPlugin.FindStringExact(0, config.plugin);
	if (m_cboxInputPluginIndex == CB_ERR)
		m_cboxInputPluginIndex = 0;
	m_configPathStr = config.remoteConfig;
	m_disableFirstRepeatsInt = config.disableFirstKeyRepeats;
	m_disableKeyRepeatsInt = config.disableRepeats ? BST_CHECKED : BST_UNCHECKED;
	m_disableFirstRepeats.EnableWindow(!config.disableRepeats);
	m_disableFirstRepeatsLabel.EnableWindow(!config.disableRepeats);
	m_allowLocalConnectionsOnly = config.localConnectionsOnly ? BST_CHECKED : BST_UNCHECKED;
	m_disableSystemTrayIcon = config.showTrayIcon ? BST_UNCHECKED : BST_CHECKED;

	DoDataExchange(FALSE);
	return TRUE;

}

LRESULT InputPlugin::OnBnClickedDisableKeyRepeats(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);
	bool const enable = (m_disableKeyRepeatsInt != BST_CHECKED);
	m_disableFirstRepeats.EnableWindow(enable);
	m_disableFirstRepeatsLabel.EnableWindow(enable);
	return 0;
}

LRESULT InputPlugin::OnBnClickedCreateConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);

	if (m_configPathStr.IsEmpty())
	{
		m_configPathStr = _T("..\\config.cf");
	}

	if (m_cboxInputPluginStr.IsEmpty())
	{
		MessageBox(_T("No valid plugins selected."));
		return 0;
	}

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi = { 0 };

	CString commandLine = _T("..\\IRRecord -d ") + m_cboxInputPluginStr + _T(" \"") + m_configPathStr + _T("\"");

	BOOL const processCreated = CreateProcess(
		NULL,								// No module name (use command line)
		commandLine.GetBuffer(0),			// Command line
		NULL,								// Process handle not inheritable
		NULL,								// Thread handle not inheritable
		FALSE,								// Set handle inheritance to FALSE
		0,									// No creation flags
		NULL,								// Use parent's environment block
		NULL,								// Use parent's starting directory 
		&si,								// Pointer to STARTUPINFO structure
		&pi );								// Pointer to PROCESS_INFORMATION structure

	if(processCreated) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		MessageBox(_T("IRRecord.exe missing"));
	}
	return 0;
}
