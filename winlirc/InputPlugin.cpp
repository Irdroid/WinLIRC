#include "stdafx.h"
#include "InputPlugin.h"
#include "winlirc.h"
#include "irconfig.h"

#include <algorithm>
#include <functional>

using namespace std::placeholders;

InputPlugin::InputPlugin()
{ }

void InputPlugin::listDllFiles()
{
	FileFinder cFileFind;
	bool found = cFileFind.FindFile(_T(".\\*.dll"));

	if (!found)
	{
		MessageBox(_T("No valid dlls found."));
	}
	else
	{
		do
		{
			CString const fileName = cFileFind.GetFileName();
			if (checkDllFile(fileName))
				m_cboxInputPlugin.AddString(fileName);

			found = cFileFind.FindNextFile();
		} while (found);
	}
}

Dll InputPlugin::checkDllFile(CString file)
{
	return Dll(file);
}

bool InputPlugin::checkRecording(Dll const& dll) const
{
	return dll.dllFile && dll.dllFile.getProcAddress("getHardware") != nullptr;
}

void InputPlugin::enableWindows(bool canRecord)
{
	m_setupButton.EnableWindow(m_dllFile.hasGuiFunction && m_dllFile.hasGuiFunction());
	m_configPath.EnableWindow(canRecord);
	m_createConfigButton.EnableWindow(canRecord);
	m_browseButton.EnableWindow(canRecord);
}

void InputPlugin::unloadDll()
{
	m_dllFile = Dll();
}

// InputPlugin message handlers

LRESULT InputPlugin::OnCbnSelchangeInputPlugin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	unloadDll();

	DoDataExchange(TRUE);

	CString m_cboxInputPluginStr; m_cboxInputPlugin.GetLBText(m_cboxInputPluginIndex, m_cboxInputPluginStr);
	CString const file = _T(".\\") + m_cboxInputPluginStr;
	Dll dll = checkDllFile(file);
	bool const canRecord = checkRecording(dll);

	if (!dll)
		MessageBox(_T("Invalid dll file"), _T("Error"), 0);

	m_dllFile = std::move(dll);
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
	CFileDialog fileDlg(
		TRUE,
		_T("cf"),
		m_configPathStr,
		OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_ENABLESIZING,
		_T("WinLIRC configuration files (*.cf)\0*.cf\0All files (*.*)\0*.*\0"), *this);

	if (fileDlg.DoModal() == IDOK)
	{
		m_configPathStr = fileDlg.m_szFileName;
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
	if (m_dllFile.loadSetupGuiFunction) {

		this->EnableWindow(FALSE);
		m_dllFile.loadSetupGuiFunction();
		this->EnableWindow(TRUE);
		this->SetFocus();
	}
	return 0;
}

LRESULT InputPlugin::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange(FALSE);

	listDllFiles();
	m_dllFile = Dll(config.plugin);

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
