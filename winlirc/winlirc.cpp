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
 */

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "server.h"
#include "guicon.h"

CAppModule _Module;
Cserver* server = nullptr;
Cdrvdlg* dlg = nullptr;

int WINAPI _tWinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE /*hPrevInstance*/,
	_In_  LPTSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	::CoInitialize(NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	HRESULT hRes = _Module.Init(NULL, hInstance);
#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	dlg		= NULL;
	server	= NULL;

	// set current directory for plugins from exe path

	{
		//=====================
		CString	fullPath;
		int		indexOfLastSep;
		//=====================

		GetModuleFileName(NULL, fullPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
		indexOfLastSep = fullPath.ReverseFind(_T('\\'));
		
		SetCurrentDirectory(fullPath.Left(indexOfLastSep) + _T("\\plugins\\"));
	}

	config.readINIFile();

	//
	// command line stuff
	//

	if(_tcsstr(lpCmdLine,_T("/e")) || _tcsstr(lpCmdLine,_T("/E"))) {
		config.exitOnError = TRUE;
	}

	if(_tcsstr(lpCmdLine,_T("/t")) || _tcsstr(lpCmdLine,_T("/T"))) {
		config.showTrayIcon = FALSE;
	}

	Cserver server_;
	server = &server_;

	if(!CreateMutex(0,FALSE,_T("WinLIRC Multiple Instance Lockout")) || GetLastError()==ERROR_ALREADY_EXISTS) {

		HWND const winlirc = FindWindow(NULL, _T("WinLIRC"));
		if (!winlirc)
		{
			MessageBox(NULL, _T("WinLIRC is already running"), _T("WinLIRC"), MB_OK);
		}
		else
		{
			// bring it to the top

			HWND const last = GetLastActivePopup(winlirc);
			if (!IsWindowVisible(winlirc))
				ShowWindow(winlirc, SW_SHOW);

			SetForegroundWindow(winlirc);
			SetForegroundWindow(last);
		}
		return FALSE;
	}

	//
	//Process initialization and sanity checks
	//
	if(SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)==0 || SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE)==0)
	{
		MessageBox(NULL,_T("Could not set thread priority."),_T("WinLIRC"),MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	if (!server->init())
	{
		MessageBox(NULL, _T("Could not start server."), _T("WinLIRC"), MB_OK | MB_ICONERROR);
		server = NULL;
		return FALSE;
	}
	
	WL_DEBUG("Creating main dialog...\n");

	{
		Cdrvdlg dlg_;
		dlg = &dlg_;

		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);

		if (dlg_.Create(NULL) == NULL)
		{
			ATLTRACE(_T("Main dialog creation failed!\n"));
			return 0;
		}
		dlg_.ShowWindow(SW_HIDE);
		dlg_.UpdateWindow();
		theLoop.Run();
		_Module.RemoveMessageLoop();
	}

	_Module.Term();
	::CoUninitialize();
	return 0;
}
