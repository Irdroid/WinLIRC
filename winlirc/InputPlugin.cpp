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

void InputPlugin::listDllFiles() {

	//==========================
	CFileFind	cFileFind;
	CString		searchFile;
	BOOL		found;
	BOOL		foundMatch;			// match with the ini config
	bool		canRecord;
	CString		temp;
	int			i;
	int			matchIndex;
	//==========================

	searchFile	= _T(".\\*.dll");
	found		= cFileFind.FindFile(searchFile);
	foundMatch	= FALSE;
	i			= 0;
	matchIndex	= 0;
	canRecord	= false;

	if(!found) {

		MessageBox(_T("No valid dlls found."));

		return;
	}

	while(found) {

		found = cFileFind.FindNextFile();

		if(checkDllFile(cFileFind.GetFilePath())) {

			m_cboxInputPlugin.AddString(cFileFind.GetFileName());
		
			if(cFileFind.GetFileName() == config.plugin) {
				m_cboxInputPlugin.SetCurSel(i);
				foundMatch	= TRUE;
				matchIndex	= i;
				canRecord	= checkRecording(cFileFind.GetFilePath());
			}
			
			i++;
		}
	}

	m_cboxInputPlugin.SetCurSel(matchIndex);
	m_cboxInputPlugin.GetLBText(matchIndex,temp);

	temp = _T(".\\") + temp;

	loadDll(temp);
	enableWindows(canRecord);
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
	DDX_Control(pDX, IDC_COMBO1, m_cboxInputPlugin);
	DDX_Text(pDX,    IDC_COMBO1, m_cboxInputPluginStr);
	DDX_Control(pDX, IDC_BUTTON1, m_setupButton);
	DDX_Control(pDX, IDC_EDIT1, m_configPath);
	DDX_Text(pDX,    IDC_EDIT1, m_configPathStr);
	DDX_Control(pDX, IDC_CHECK1, m_disableKeyRepeats);
	DDX_Check(pDX,   IDC_CHECK1, m_disableKeyRepeatsInt);
	DDX_Control(pDX, IDC_EDIT3, m_disableFirstRepeats);
	DDX_Text(pDX,    IDC_EDIT3, m_disableFirstRepeatsInt);
	DDX_Control(pDX, IDC_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsLabel);
	DDX_Check(pDX,   IDC_CHECK2, m_allowLocalConnectionsOnly);
	DDX_Check(pDX,   IDC_CHECK3, m_disableSystemTrayIcon);
	DDX_Control(pDX, IDC_BUTTON3, m_createConfigButton);
	DDX_Control(pDX, IDC_BUTTON2, m_browseButton);
}

BEGIN_MESSAGE_MAP(InputPlugin, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &InputPlugin::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &InputPlugin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON2, &InputPlugin::OnBnClickedButton2)
	ON_BN_CLICKED(IDCANCEL, &InputPlugin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &InputPlugin::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &InputPlugin::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON3, &InputPlugin::OnBnClickedButton3)
END_MESSAGE_MAP()


// InputPlugin message handlers

void InputPlugin::OnCbnSelchangeCombo1() {

	//======================
	int		cursorSelection;
	CString file;
	bool	validFile;
	bool	canRecord;
	//======================

	unloadDll();

	cursorSelection = m_cboxInputPlugin.GetCurSel();

	m_cboxInputPlugin.GetLBText(m_cboxInputPlugin.GetCurSel(),file);

	file		= _T(".\\") + file;
	validFile	= checkDllFile(file);
	canRecord	= checkRecording(file);

	if(!validFile) MessageBox(_T("Invalid dll file"),_T("Error"),0);

	loadDll(file);
	enableWindows(canRecord);
}

void InputPlugin::OnBnClickedOk() {

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
	config.plugin = m_cboxInputPluginStr; //m_cboxInputPlugin.GetWindowText(config.plugin);
	config.disableRepeats = (m_disableKeyRepeatsInt == BST_CHECKED);
	config.disableFirstKeyRepeats = m_disableFirstRepeatsInt;
	config.localConnectionsOnly = (m_allowLocalConnectionsOnly == BST_CHECKED);
	config.showTrayIcon = (m_disableSystemTrayIcon != BST_CHECKED);

	config.writeINIFile();

	OnOK();
}

void InputPlugin::OnBnClickedButton2()
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

void InputPlugin::OnBnClickedButton1()
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

	m_configPathStr = config.remoteConfig;
	m_disableFirstRepeatsInt = config.disableFirstKeyRepeats;

	if(config.disableRepeats)
	{
		m_disableKeyRepeatsInt = BST_CHECKED;
		m_disableFirstRepeats.EnableWindow(FALSE);
		m_disableFirstRepeatsLabel.EnableWindow(FALSE);
	}

	if(config.localConnectionsOnly) {
		m_allowLocalConnectionsOnly = BST_CHECKED;
	}

	if(!config.showTrayIcon) {
		m_disableSystemTrayIcon = BST_CHECKED;
	}

	UpdateData(FALSE);
	return TRUE;

}

void InputPlugin::OnBnClickedCheck1()
{
	UpdateData(TRUE);
	bool const enable = (m_disableKeyRepeatsInt != BST_CHECKED);
	m_disableFirstRepeats.EnableWindow(enable);
	m_disableFirstRepeatsLabel.EnableWindow(enable);
}

void InputPlugin::OnBnClickedButton3()
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
