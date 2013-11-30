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
 * Modifications based on LIRC 0.6.x Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "resource.h"
#include "remote.h"
#include "globals.h"
#include "server.h" //so we can send SIGHUP
#include "InputPlugin.h"

HICON loadIcon(LPCTSTR iconId)
{
	return LoadIcon(::GetModuleHandle(NULL), iconId);
}

HICON loadIcon(int iconId)
{
	return loadIcon(MAKEINTRESOURCE(iconId));
}

Cdrvdlg::Cdrvdlg()
	: ti(IDR_TRAYMENU)
{
	m_ircode_edit	= _T("");
	m_remote_edit	= _T("");
	m_reps_edit		= 0;
}

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg message handlers

void Cdrvdlg::ExitLirc()
{
	ti.SetIcon(0);
	DestroyWindow();
	PostQuitMessage(0);
}

LRESULT Cdrvdlg::OnPowerBroadcast(WPARAM wPowerEvent,LPARAM lP)
{
	LRESULT retval = TRUE;

	switch (wPowerEvent)
	{
		case PBT_APMQUERYSUSPEND:
			{
				//can we suspend?
				//bit 0 false => automated or emergency power down 
				//UI iff bit 0 is set
				BOOL bUIEnabled=(lP & 0x00000001)!=0;
				retval = TRUE;
				break;
			}
		case PBT_APMSUSPEND:
			{
				driver.deinit(); //if we RequestDeviceWakeup instead we could wake on irevents
				retval = TRUE;		//if the RI pin was connected to the receiving device
				break;
			}

			//wake up from a critical power shutdown
		case PBT_APMRESUMECRITICAL:
			//standard wake up evrnt; UI is on
			break;
		case PBT_APMRESUMESUSPEND:
			//New for Win98;NT5
			//unattended wake up; no user interaction possible; screen
			//is probably off.
			break;
		case PBT_APMRESUMEAUTOMATIC:

			//system power source has changed, or 
			//battery life has just got into the near-critical state

			if (config.showTrayIcon)
				ti.SetIcon(loadIcon(IDI_LIRC_INIT), _T("WinLIRC / Initializing"));

			Sleep(1000);

			if (!driver.init())
			{
				WL_DEBUG("InitPort failed\n");
				if (config.showTrayIcon)
					ti.SetIcon(loadIcon(IDI_LIRC_ERROR), _T("WinLIRC / Initialization Error"));
				retval = false;
				break;
			}

			if (config.showTrayIcon)
				ti.SetIcon(loadIcon(IDI_LIRC_OK), _T("WinLIRC / Ready"));

			retval = TRUE;
			break;
		case PBT_APMPOWERSTATUSCHANGE:
			retval = TRUE;
			break;
		default:
			retval=TRUE;
			break;
		}

		return retval;
	}

LRESULT Cdrvdlg::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	if(AllowTrayNotification)
		return ti.OnTrayNotification(uID, lEvent);
	else
		return 0;
}

LRESULT Cdrvdlg::OnToggleWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ToggleWindow();
	return 0;
}

void Cdrvdlg::ToggleWindow()
{
	if(IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_SHOW);
		SetForegroundWindow(*this);
	}
	UpdateWindow();
}

void Cdrvdlg::OnHideme() 
{
	if(IsWindowVisible())
		ToggleWindow();
}

void Cdrvdlg::GoGreen()
{
	if(!config.showTrayIcon) {
		return;
	}

	if(SetTimer(1,250,NULL)) {
		ti.SetIcon(loadIcon(IDI_LIRC_RECV), _T("WinLIRC / Received Signal"));
	}
}
void Cdrvdlg::GoBlue()
{
	if (config.showTrayIcon)
	{
		if (SetTimer(1, 250, NULL))
			ti.SetIcon(loadIcon(IDI_LIRC_SEND), _T("WinLIRC / Sent Signal"));
	}
}

int Cdrvdlg::OnTimer(UINT_PTR nIDEvent) {

	if (nIDEvent == 1)
	{
		KillTimer(1);
		ti.SetIcon(loadIcon(IDI_LIRC_OK), _T("WinLIRC / Ready"));
		return 0;
	}
	return 1;
}

bool Cdrvdlg::DoInitializeDaemon()
{
	AllowTrayNotification=false;
	for(;;)
	{
		if(InitializeDaemon()==true)
		{
			AllowTrayNotification=true;
			return true;
		}
		else {
			//printf("failed here :(\n");
		}

		if(config.exitOnError) {
			ti.DisableTrayIcon();
			exit(0);
		}
		
		if(!IsWindowVisible())
			ToggleWindow();

		if(MessageBox(	_T("WinLIRC failed to initialize.\n")
						_T("Would you like to change the configuration\n")
						_T("and try again?"),_T("WinLIRC Error"),MB_OKCANCEL)==IDCANCEL)
			return false;
		
		InputPlugin inputPlugin;// (*this);
		inputPlugin.DoModal();
	}
}

bool Cdrvdlg::InitializeDaemon() {

	//==============
	CString		tmp;
	CWaitCursor foo;
	//==============

	if(config.remoteConfig!="") {

		if(!config.readConfig()) {

			if(!config.exitOnError) MessageBox(	_T("Error loading config file."),_T("Configuration Error"));
			if(config.showTrayIcon) ti.SetIcon(loadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
			return false;
		}
	}

	if(!config.showTrayIcon) {
		ti.DisableTrayIcon();
	}
	
	tmp = _T(".\\");
	tmp = tmp + config.plugin;

	if(driver.loadPlugin(tmp)==false) {
		if(config.showTrayIcon) ti.SetIcon(loadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
		return false;
	}

	if(config.showTrayIcon) {
		ti.SetIcon(loadIcon(IDI_LIRC_INIT),_T("WinLIRC / Initializing"));
	}

	if(driver.init()==false) {
		if(config.showTrayIcon) ti.SetIcon(loadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
		return false;
	}
	
	::server->sendToClients("BEGIN\nSIGHUP\nEND\n");
	if(config.showTrayIcon) ti.SetIcon(loadIcon(IDI_LIRC_OK),_T("WinLIRC / Ready"));
	return true;
}

void Cdrvdlg::OnConfig() 
{
	InputPlugin inputPlugin;
	if (inputPlugin.DoModal(*this) == IDOK)
	{
		driver.deinit();

		AllowTrayNotification = false;
		KillTimer(1);
		if (config.showTrayIcon) ti.SetIcon(loadIcon(IDI_LIRC_ERROR), _T("WinLIRC / Disabled During Configuration"));

		if (!DoInitializeDaemon())
			ExitLirc();

		UpdateRemoteComboLists();
	}
}

LRESULT Cdrvdlg::OnExitLirc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ExitLirc();
	return 0;
}


BOOL Cdrvdlg::OnInitDialog() 
{
	ti.SetNotificationWnd(*this, WM_TRAY);

	if (DoInitializeDaemon() == false)
	{
		DestroyWindow();
		return FALSE;
	}

	m_ircode_edit = "";
	DoDataExchange(FALSE);
	GetDlgItem(IDC_VERSION).SetWindowText(WINLIRC_VERSION);
	UpdateRemoteComboLists();
	return TRUE;
}

void Cdrvdlg::OnSendcode() 
{
	EnableWindow(FALSE);
	DoDataExchange(TRUE);

	//=======================
	struct ir_ncode *codes;
	struct ir_remote *sender;
	//=======================

	m_remote_edit.TrimRight();
	m_ircode_edit.TrimRight();
	m_remote_edit.TrimLeft();
	m_ircode_edit.TrimLeft();

	std::unique_lock<std::mutex> lock(CS_global_remotes);
	USES_CONVERSION;
	char const* const remoteName = T2A(m_remote_edit.GetBuffer());
	sender = get_remote_by_name(global_remotes, remoteName);

	if (sender==NULL) {
		MessageBox(_T("No match found for remote!"));
	}
	else {

		codes = sender->codes;

		while (codes->name!=NULL && _stricmp(remoteName,codes->name)) {
			codes++;
		}

		if (codes==NULL || codes->name==NULL) {
			MessageBox(_T("No match found for ircode!"));
		}
		else {

			if (m_reps_edit < sender->min_repeat) {
				m_reps_edit = sender->min_repeat;		//set minimum number of repeats
				DoDataExchange(FALSE);						//update the display
			}

			//reset toggle masks

			if(has_toggle_mask(sender)) {
				sender->toggle_mask_state = 0;
			}

			if(has_toggle_bit_mask(sender)) {
				sender->toggle_bit_mask_state = (sender->toggle_bit_mask_state^sender->toggle_bit_mask);
			}

			//send code

			if(driver.sendIR(sender,codes,m_reps_edit)) {
				GoBlue();
			}
			else {
				MessageBox(_T("Send failed/not supported."));
			}
		}
	}

	DoDataExchange(FALSE);
	EnableWindow(TRUE);
}

BOOL Cdrvdlg::OnCopyData(HWND pWnd, COPYDATASTRUCT* pCopyDataStruct)
// handles a transmission command recieved from another application
// pCopyDataStruct->lpData should point to a string of the following format
// remotename	ircodename	reps
// where reps is an optional parameter indicating the number of times to repeat the code (default=0)
{
	//================
	CStringA string;
	CStringA response;
	//================

	string  = "SEND_ONCE ";
	string += (LPCSTR) (pCopyDataStruct->lpData);
	
	return ::server->parseSendString(string,response);
}

void Cdrvdlg::UpdateRemoteComboLists()
{
	USES_CONVERSION;
	DoDataExchange(TRUE);
	m_remote_DropDown.ResetContent();

	//Fill remote combo box
	struct ir_remote* sender=global_remotes;
	while (sender!=NULL)
	{
		m_remote_DropDown.AddString(A2T(sender->name));
		sender=sender->next;
	}
	//Set selected item
	if (m_remote_DropDown.SelectString(-1,m_remote_edit) == CB_ERR)
	{
		//Did not find remote selected before, select first
		m_remote_DropDown.SetCurSel(0);
	}
	DoDataExchange(FALSE);

	UpdateIrCodeComboLists();
}

void Cdrvdlg::UpdateIrCodeComboLists()
{
	USES_CONVERSION;
	DoDataExchange(TRUE);
	
	//Retrieve pointer to remote by name
	struct ir_remote* selected_remote = get_remote_by_name(global_remotes,T2A(m_remote_edit.GetBuffer()));

	m_IrCodeEditCombo.ResetContent();

	if (selected_remote)
	{
		ir_ncode* codes = selected_remote->codes;
		while (codes && codes->name!=NULL)
		{
			m_IrCodeEditCombo.AddString(A2T(codes->name));
			codes++;
		}
	}

	if (m_IrCodeEditCombo.SelectString(-1,m_ircode_edit) == CB_ERR)
	{
		m_IrCodeEditCombo.SetCurSel(0);
	}

	DoDataExchange(FALSE);
}

void Cdrvdlg::OnDropdownIrcodeEdit() 
{
	// TODO: Add your control notification handler code here
	UpdateIrCodeComboLists();
}

LRESULT Cdrvdlg::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: return OnInitDialog();
	case WM_TRAY: return OnTrayNotification(wParam, lParam);
	case WM_POWERBROADCAST: return OnPowerBroadcast(wParam, lParam);
	case WM_TIMER: return OnTimer(wParam);
	case WM_COPYDATA: return OnCopyData(reinterpret_cast<HWND>(wParam), reinterpret_cast<COPYDATASTRUCT*>(lParam));
	default: assert(false);  return 0;
	}
}

LRESULT Cdrvdlg::CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch (wID)
	{
	case IDC_CONFIG: OnConfig(); return 0; //, BN_CLICKED //OnConfig)
	case IDC_HIDEME: OnHideme(); return 0; //, BN_CLICKED //OnHideme)
	case IDC_SENDCODE: OnSendcode(); return 0; //, BN_CLICKED //OnSendcode)
	case IDC_IRCODE_EDIT: OnDropdownIrcodeEdit(); return 0; //, CBN_DROPDOWN //OnDropdownIrcodeEdit)
	}
	return 0;
}

