#pragma once

#include "dll.h"
#include "resource.h"

#include "../lib/Utility/Module.h"

class InputPlugin
	: public CDialogImpl<InputPlugin>
	, public CWinDataExchange<InputPlugin>
{
public:
	InputPlugin();   // standard constructor

	enum { IDD = IDD_INPUT_PLUGIN };

private:
	void listDllFiles();
	/// Returns a module handle is DLL file exists and exports required functions.
	Dll checkDllFile(CString file);
	bool checkRecording(Dll const& file) const;
	void enableWindows(bool canRecord);					// enable windows based upon selection
	void unloadDll();

	Dll					m_dllFile;

private:

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCbnSelchangeInputPlugin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPluginSetup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDisableKeyRepeats(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCreateConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CComboBox	m_cboxInputPlugin;
	CString		m_cboxInputPluginStr;
	int			m_cboxInputPluginIndex;
	CButton		m_setupButton;
	CEdit		m_configPath;
	CString		m_configPathStr;
	CButton		m_disableKeyRepeats;
	INT			m_disableKeyRepeatsInt;
	CEdit		m_disableFirstRepeats;
	int			m_disableFirstRepeatsInt;
	CStatic		m_disableFirstRepeatsLabel;
	INT			m_allowLocalConnectionsOnly;
	INT			m_disableSystemTrayIcon;
	CButton		m_createConfigButton;
	CButton		m_browseButton;

	BEGIN_MSG_MAP(InputPlugin)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_HANDLER(IDC_CMB_INPUT_PLUGIN, CBN_SELCHANGE, OnCbnSelchangeInputPlugin)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDC_BN_BROWSE, BN_CLICKED, OnBnClickedBrowse)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_BN_PLUGIN_SETUP, BN_CLICKED, OnBnClickedPluginSetup)
		COMMAND_HANDLER(IDC_CHK_DISABLE_KEY_REPEATS, BN_CLICKED, OnBnClickedDisableKeyRepeats)
		COMMAND_HANDLER(IDC_BN_CREATE_CONFIG, BN_CLICKED, OnBnClickedCreateConfig)
	END_MSG_MAP()

	// Works once way only.
	void ddx_combo_text(UINT nID, CString& text, BOOL bSaveAndValidate)
	{
		if (bSaveAndValidate)
		{
			CComboBox ctrl(GetDlgItem(nID));
			ctrl.GetLBText(ctrl.GetCurSel(), text);
		}
	}

	void OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
	{
		MessageBox(_T("DDX Error"));
	}

	void OnDataValidateError(UINT nCtrlID, BOOL /*bSave*/, _XData& /*data*/)
	{
		MessageBox(_T("Validation Error"));
	}

	BEGIN_DDX_MAP(InputPlugin)
		DDX_CONTROL_HANDLE(IDC_CMB_INPUT_PLUGIN, m_cboxInputPlugin);
		static_cast<InputPlugin*>(this)->ddx_combo_text(IDC_CMB_INPUT_PLUGIN, m_cboxInputPluginStr, bSaveAndValidate);
		DDX_COMBO_INDEX(IDC_CMB_INPUT_PLUGIN, m_cboxInputPluginIndex);
		DDX_CONTROL_HANDLE(IDC_BN_PLUGIN_SETUP, m_setupButton);
		DDX_CONTROL_HANDLE(IDC_EDIT_CONFIG_PATH, m_configPath);
		DDX_TEXT(IDC_EDIT_CONFIG_PATH, m_configPathStr);
		DDX_CONTROL_HANDLE(IDC_CHK_DISABLE_KEY_REPEATS, m_disableKeyRepeats);
		DDX_CHECK(IDC_CHK_DISABLE_KEY_REPEATS, m_disableKeyRepeatsInt);
		DDX_CONTROL_HANDLE(IDC_EDIT_DISABLE_FIRST_REPEATS, m_disableFirstRepeats);
		DDX_INT(IDC_EDIT_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsInt);
		DDX_CONTROL_HANDLE(IDC_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsLabel);
		DDX_CHECK(IDC_CHK_LOCAL_CONNECTIONS_ONLY, m_allowLocalConnectionsOnly);
		DDX_CHECK(IDC_CHK_DISABLE_TRAY_ICON, m_disableSystemTrayIcon);
		DDX_CONTROL_HANDLE(IDC_BN_CREATE_CONFIG, m_createConfigButton);
		DDX_CONTROL_HANDLE(IDC_BN_BROWSE, m_browseButton);
	END_DDX_MAP()
};
