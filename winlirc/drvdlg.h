/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 */

#pragma once

#include "globals.h"

#include "winlirc.h"
#include "trayicon.h"
#include "irdriver.h"
#include "irconfig.h"

class CIRDriver;
class CIRConfig;

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg dialog

class Cdrvdlg
	: public CDialogImpl<Cdrvdlg>
	, public CWinDataExchange<Cdrvdlg>
{
public:
	Cdrvdlg();
	
	bool initialized;
	bool AllowTrayNotification;
	bool DoInitializeDaemon();
	bool InitializeDaemon();
	void GoGreen();
	void GoBlue();	//turns the tray icon blue to indicate a transmission
	
// Dialog Data
	enum { IDD = IDD_DRVDLG };
	CComboBox	m_IrCodeEditCombo;
	CComboBox	m_remote_DropDown;
	CString	m_ircode_edit;
	CString	m_remote_edit;
	int	m_reps_edit;

	CTrayIcon ti;
	CIRDriver driver;

// Implementation
protected:

	// Generated message map functions
	void ExitLirc();
	LRESULT OnToggleWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void ToggleWindow();
	void OnConfig();
	void OnHideme();
	LRESULT OnExitLirc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	int OnTimer(UINT_PTR nIDEvent);
	BOOL OnInitDialog();
	void OnSendcode();
	BOOL OnCopyData(HWND pWnd, COPYDATASTRUCT* pCopyDataStruct);
	void OnDropdownIrcodeEdit();
	LRESULT OnPowerBroadcast(WPARAM uPowerEvent, LPARAM lP);
	void UpdateRemoteComboLists();
	void UpdateIrCodeComboLists();
	LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);


	BEGIN_DDX_MAP(Cdrvdlg)
		DDX_CONTROL_HANDLE(IDC_IRCODE_EDIT, m_IrCodeEditCombo);
		DDX_CONTROL_HANDLE(IDC_REMOTE_EDIT, m_remote_DropDown);
		DDX_TEXT(IDC_IRCODE_EDIT, m_ircode_edit);
		DDX_TEXT_LEN(IDC_IRCODE_EDIT, m_ircode_edit, 64);
		DDX_TEXT(IDC_REMOTE_EDIT, m_remote_edit);
		DDX_TEXT_LEN(IDC_REMOTE_EDIT, m_remote_edit, 64);
		DDX_INT(IDC_REPS_EDIT, m_reps_edit);
		DDX_INT_RANGE(IDC_REPS_EDIT, m_reps_edit, 0, 600);
	END_DDX_MAP()


	BEGIN_MSG_MAP(Cdrvdlg)
		MESSAGE_HANDLER(WM_CREATE, MessageHandler)//ON_WM_CREATE()
		MESSAGE_HANDLER(WM_INITDIALOG, MessageHandler)
		MESSAGE_HANDLER(WM_TRAY, MessageHandler)//ON_MESSAGE(WM_TRAY, OnTrayNotification)
		MESSAGE_HANDLER(WM_POWERBROADCAST, MessageHandler)//ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
		MESSAGE_HANDLER(WM_TIMER, MessageHandler)//ON_WM_TIMER()
		MESSAGE_HANDLER(WM_COPYDATA, MessageHandler)//ON_WM_COPYDATA()

		COMMAND_ID_HANDLER(ID_TOGGLEWINDOW, OnToggleWindow)
		COMMAND_ID_HANDLER(ID_EXITLIRC, OnExitLirc)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExitLirc)//ON_BN_CLICKED(IDC_CONFIG, OnConfig)
		COMMAND_HANDLER(IDC_CONFIG, BN_CLICKED, CommandHandler)//ON_BN_CLICKED(IDC_CONFIG, OnConfig)
		COMMAND_HANDLER(IDC_HIDEME, BN_CLICKED, CommandHandler)//ON_BN_CLICKED(IDC_HIDEME, OnHideme)
		COMMAND_HANDLER(IDC_SENDCODE, BN_CLICKED, CommandHandler)//ON_BN_CLICKED(IDC_SENDCODE, OnSendcode)
		COMMAND_HANDLER(IDC_IRCODE_EDIT, CBN_DROPDOWN, CommandHandler)//ON_CBN_DROPDOWN(IDC_IRCODE_EDIT, OnDropdownIrcodeEdit)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
};
