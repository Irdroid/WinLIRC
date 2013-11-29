// InputPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "winlirc.h"
#include "InputPlugin.h"
#include "irconfig.h"


// InputPlugin dialog

IMPLEMENT_DYNAMIC(InputPlugin, CDialog)

InputPlugin::InputPlugin(CWnd* pParent /*=NULL*/)
	: CDialog(InputPlugin::IDD, pParent)
{
	m_hasGuiFunction		= NULL;
	m_loadSetupGuiFunction	= NULL;
	m_dllFile				= NULL;
}

InputPlugin::~InputPlugin()
{
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
			if (checkDllFile(cFileFind.GetFilePath()))
				m_cboxInputPlugin.AddString(cFileFind.GetFileName());
		}
	}
}

bool InputPlugin::checkDllFile(CString file) {

	//==========
	HMODULE tmp;
	//==========

	tmp = LoadLibrary(file);

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

	tmp = LoadLibrary(file);

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

	m_dllFile = LoadLibrary(file);

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


void InputPlugin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_INPUT_PLUGIN, m_cboxInputPlugin);
	DDX_Text(pDX,    IDC_CMB_INPUT_PLUGIN, m_cboxInputPluginStr);
	DDX_CBIndex(pDX, IDC_CMB_INPUT_PLUGIN, m_cboxInputPluginIndex);
	DDX_Control(pDX, IDC_BN_PLUGIN_SETUP, m_setupButton);
	DDX_Control(pDX, IDC_EDIT_CONFIG_PATH, m_configPath);
	DDX_Text(pDX,    IDC_EDIT_CONFIG_PATH, m_configPathStr);
	DDX_Control(pDX, IDC_CHK_DISABLE_KEY_REPEATS, m_disableKeyRepeats);
	DDX_Check(pDX,   IDC_CHK_DISABLE_KEY_REPEATS, m_disableKeyRepeatsInt);
	DDX_Control(pDX, IDC_EDIT_DISABLE_FIRST_REPEATS, m_disableFirstRepeats);
	DDX_Text(pDX,    IDC_EDIT_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsInt);
	DDX_Control(pDX, IDC_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsLabel);
	DDX_Check(pDX,   IDC_CHK_LOCAL_CONNECTIONS_ONLY, m_allowLocalConnectionsOnly);
	DDX_Check(pDX,   IDC_CHK_DISABLE_TRAY_ICON, m_disableSystemTrayIcon);
	DDX_Control(pDX, IDC_BN_CREATE_CONFIG, m_createConfigButton);
	DDX_Control(pDX, IDC_BN_BROWSE, m_browseButton);
}

BEGIN_MESSAGE_MAP(InputPlugin, CDialog)
	ON_CBN_SELCHANGE(IDC_CMB_INPUT_PLUGIN, &InputPlugin::OnCbnSelchangeInputPlugin)
	ON_BN_CLICKED(IDOK, &InputPlugin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BN_BROWSE, &InputPlugin::OnBnClickedBrowse)
	ON_BN_CLICKED(IDCANCEL, &InputPlugin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BN_PLUGIN_SETUP, &InputPlugin::OnBnClickedPluginSetup)
	ON_BN_CLICKED(IDC_CHK_DISABLE_KEY_REPEATS, &InputPlugin::OnBnClickedDisableKeyRepeats)
	ON_BN_CLICKED(IDC_BN_CREATE_CONFIG, &InputPlugin::OnBnClickedCreateConfig)
END_MESSAGE_MAP()


// InputPlugin message handlers

void InputPlugin::OnCbnSelchangeInputPlugin()
{
	unloadDll();

	UpdateData(TRUE);

	CString const file = _T(".\\") + m_cboxInputPluginStr;
	bool const validFile	= checkDllFile(file);
	bool const canRecord	= checkRecording(file);

	if (!validFile)
		MessageBox(_T("Invalid dll file"), _T("Error"), 0);

	loadDll(file);
	enableWindows(canRecord);
}

void InputPlugin::OnBnClickedOk()
{
	UpdateData(TRUE);

	//
	// some basic error checking
	//

	if (!m_configPathStr.IsEmpty())
	{
		if (!PathFileExists(m_configPathStr))
		{
			MessageBox(	_T("The configuration filename is invalid.\n")
				_T("Please try again."),_T("Configuration Error"));
			return;
		}
	}

	config.remoteConfig = m_configPathStr;
	config.plugin = m_cboxInputPluginStr;
	config.disableRepeats = (m_disableKeyRepeatsInt == BST_CHECKED);
	config.disableFirstKeyRepeats = m_disableFirstRepeatsInt;
	config.localConnectionsOnly = (m_allowLocalConnectionsOnly == BST_CHECKED);
	config.showTrayIcon = (m_disableSystemTrayIcon != BST_CHECKED);

	config.writeINIFile();

	OnOK();
}

void InputPlugin::OnBnClickedBrowse()
{
	UpdateData(TRUE);
	CFileDialog fileDlg(TRUE, NULL, m_configPathStr, OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_ENABLESIZING, NULL, this, 0, TRUE);

	if (fileDlg.DoModal() == IDOK)
	{
		m_configPathStr = fileDlg.GetPathName();
		UpdateData(FALSE);
	}
}

void InputPlugin::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void InputPlugin::OnBnClickedPluginSetup()
{
	if(m_loadSetupGuiFunction) {

		this->EnableWindow(FALSE);
		m_loadSetupGuiFunction();
		this->EnableWindow(TRUE);
		this->SetFocus();
	}
}

BOOL InputPlugin::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	UpdateData(FALSE);
	return TRUE;

}

void InputPlugin::OnBnClickedDisableKeyRepeats()
{
	UpdateData(TRUE);
	bool const enable = (m_disableKeyRepeatsInt != BST_CHECKED);
	m_disableFirstRepeats.EnableWindow(enable);
	m_disableFirstRepeatsLabel.EnableWindow(enable);
}

void InputPlugin::OnBnClickedCreateConfig()
{
	UpdateData(TRUE);

	if (m_configPathStr.IsEmpty())
	{
		m_configPathStr = _T("..\\config.cf");
	}

	if (m_cboxInputPluginStr.IsEmpty())
	{
		MessageBox(_T("No valid plugins selected."));
		return;
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

}
