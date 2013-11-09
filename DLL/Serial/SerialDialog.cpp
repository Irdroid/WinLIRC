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

#include "stdafx.h"
#include "SerialDialog.h"

#include "../Common/LIRCDefines.h"
#include "Transmit.h"


namespace
{
    /// Fills a combobox with values.
    /// @param values - null-separated list of string to be added to \a cmb.
    void fillCombo(CComboBox& cmb, LPCTSTR values)
    {
        while (_tcslen(values) > 0)
        {
            cmb.AddString(values);
            values += _tcslen(values) + 1;
        }
    }

    /// Returns an index of specified string. If the string is not found 0 is returned.
    int selectString(CComboBox& cmb, LPCTSTR str)
    {
        int const x = cmb.FindStringExact(0, str);
        return (x == CB_ERR) ? 0 : x;
    }
}

// SerialDialog dialog

SerialDialog::SerialDialog()
    : cmbSpeedIndex_(0)
    , cmbSenseIndex_(0)
    , intVirtualPulse_(0)
    , bAnimax_(FALSE)
    , inverted(FALSE)
    , hardwareCarrier(FALSE)
    , transmitterPin(-1)
    , deviceType(-1)
{ }

// SerialDialog message handlers

LRESULT SerialDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DoDataExchange(FALSE);


    TCHAR const speeds[] = _T("1200\0002400\0004800\0009600\00014400\00019200\00038400\00056000\00057600\000115200\000128000\000256000\000");
    fillCombo(cmbSpeed_, speeds);

    TCHAR const senses[] = _T("Autodetect\0000 (Low)\0001 (High)\000");
    fillCombo(cmbSense_, senses);

    settings.loadSettings();

    cmbPort_.InitList(settings.port);

    //cmbPortIndex_  = selectString(cmbPort_, settings.port);
    cmbSpeedIndex_ = selectString(cmbSpeed_, settings.speed);
    cmbSenseIndex_ = settings.sense + 1;

    bAnimax_         = settings.animax;
    hardwareCarrier  = settings.transmitterType & HARDCARRIER;
    transmitterPin   = (settings.transmitterType & TXTRANSMITTER)>>1;
    inverted         = (settings.transmitterType & INVERTED)>>2;
    intVirtualPulse_ = settings.virtualPulse;
    deviceType       = settings.deviceType;

    DoDataExchange(FALSE);

    return TRUE;
}

LRESULT SerialDialog::OnBnClickedRadiorx(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);
	cmbSense_.EnableWindow(deviceType);
    editVirtualPulse_.EnableWindow(!deviceType);
	chkAnimax_.EnableWindow(deviceType);
    return 0;
}

LRESULT SerialDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DoDataExchange(TRUE);
    settings.transmitterType = (inverted<<2)|(transmitterPin<<1)|hardwareCarrier;

    int sense = cmbSenseIndex_;
    if (sense == 1 || sense == 2)
        --sense;
    else
        sense = -1;
    settings.sense = sense;

    settings.port = cmbPort_.GetPortNum(); // cmbPort_.GetLBText(cmbPortIndex_, settings.port);
    cmbPort_.GetFileName(settings.portName);
    cmbSpeed_.GetLBText(cmbSpeedIndex_, settings.speed);
    settings.deviceType   = deviceType;
    settings.virtualPulse = intVirtualPulse_;
    settings.animax       = bAnimax_;

    settings.saveSettings();
    EndDialog(wID);
    return 0;
}

LRESULT SerialDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
