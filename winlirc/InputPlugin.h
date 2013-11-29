#pragma once
#include "afxwin.h"


// InputPlugin dialog

class InputPlugin : public CDialog
{
	DECLARE_DYNAMIC(InputPlugin)

public:
	InputPlugin(CWnd* pParent = NULL);   // standard constructor
	virtual ~InputPlugin();

	enum { IDD = IDD_DIALOG1 };

private:
	void listDllFiles	();
	bool checkDllFile	(CString file);
	bool checkRecording	(CString file);
	void enableWindows	(bool canRecord);					// enable windows based upon selection
	void loadDll		(CString file);
	void unloadDll		();

	typedef int  (*HasGuiFunction)			();
	typedef void (*LoadSetupGuiFunction)	();

	HasGuiFunction			m_hasGuiFunction;
	LoadSetupGuiFunction	m_loadSetupGuiFunction;

	HMODULE					m_dllFile;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton3();

	CComboBox	m_cboxInputPlugin;
	CString		m_cboxInputPluginStr;
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
};
