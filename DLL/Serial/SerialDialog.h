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

#pragma once

#include "resource.h"
#include "Settings.h"

class SerialDialog
    : public CDialogImpl<SerialDialog>
    , public CWinDataExchange<SerialDialog>
{
public:
    SerialDialog();

    enum { IDD = IDD_SERIALDIALOG };

private:

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedRadiorx(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedRadiodcd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BEGIN_MSG_MAP(SerialDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_RADIORX, BN_CLICKED, OnBnClickedRadiorx)
        COMMAND_HANDLER(IDC_RADIODCD, BN_CLICKED, OnBnClickedRadiorx)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    END_MSG_MAP()

    BEGIN_DDX_MAP(SerialDialog)
        DDX_CONTROL_HANDLE(IDC_PORT, cmbPort_)
        DDX_COMBO_INDEX(IDC_PORT, cmbPortIndex_)

        DDX_CONTROL_HANDLE(IDC_SPEED, cmbSpeed_)
        DDX_COMBO_INDEX(IDC_SPEED, cmbSpeedIndex_)

        DDX_CONTROL_HANDLE(IDC_SENSE, cmbSense_)
        DDX_COMBO_INDEX(IDC_SENSE, cmbSenseIndex_)

        DDX_CONTROL_HANDLE(IDC_VIRTPULSE, editVirtualPulse_)
        DDX_INT(IDC_VIRTPULSE, intVirtualPulse_)
        DDX_INT_RANGE(IDC_VIRTPULSE, intVirtualPulse_, 0, 16777215)

        DDX_CONTROL_HANDLE(IDC_CHECKANIMAX, chkAnimax_)
        DDX_CHECK(IDC_CHECKANIMAX, bAnimax_)

        DDX_CHECK(IDC_INVERTED, inverted)
        DDX_CHECK(IDC_CHECKHARDCARRIER, hardwareCarrier)
        DDX_RADIO(IDC_RADIODTR, transmitterPin)
        DDX_RADIO(IDC_RADIORX, deviceType)
    END_DDX_MAP()

private:

    CComboBox cmbPort_;
    int cmbPortIndex_;

    CComboBox cmbSpeed_;
    int cmbSpeedIndex_;

    CComboBox cmbSense_;
    int cmbSenseIndex_;

    CEdit editVirtualPulse_;
    int intVirtualPulse_;

    CButton chkAnimax_;
    BOOL bAnimax_;

    BOOL inverted;
    BOOL hardwareCarrier;
    int transmitterPin;
    int	deviceType;

    Settings settings;
};
