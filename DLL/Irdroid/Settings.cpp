/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
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
 * Copyright (C) 2010 Ian Curtis
 */

#include "Settings.h"
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>

Settings::Settings() {

	loadSettings();
}

void Settings::setComPort(int port) {

	comPort = port;

	if(comPort<1) comPort = 1;
}

int Settings::getComPort() {

	return comPort;
}

void Settings::saveSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	TCHAR temp[8];
	FILE  *file;	
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	//
	// if our ini files doesn't exist try and create it
	//
	file = _tfopen(currentDirectory,_T("r"));

	if(!file) {
		file = _tfopen(currentDirectory,_T("w"));
		if(file) fclose(file);
	}
	else {
		fclose(file);
	}
	
	_sntprintf(temp, _countof(temp), _T("%i"), comPort);
	WritePrivateProfileString(_T("IrdroidPlugin"),_T("ComPort"),temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	comPort = GetPrivateProfileInt(_T("IrdroidPlugin"),_T("ComPort"),1,currentDirectory);

	// make sure that comPort is in range 0..255
	if (comPort < 0 || 255 < comPort) {
		comPort = 0;
	}
}