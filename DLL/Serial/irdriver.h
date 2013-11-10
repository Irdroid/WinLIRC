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
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#pragma once

class Settings;

struct SerialPortTraits
{
    typedef HANDLE HandleType;

    static HandleType invalidValue() { return INVALID_HANDLE_VALUE; }
    static void close(HandleType h) { ::CloseHandle(h); }
};

struct EventTraits
{
    typedef HANDLE HandleType;

    static HandleType invalidValue() { return nullptr; }
    static void close(HandleType h) { ::CloseHandle(h); }
};

struct DataBuffer
{
    DataBuffer()
        : start_(0)
        , end_(0)
    { }

    bool dataReady() const { return start_ != end_; }

    void pushData(UINT val) { data_[end_++] = val; }
    bool popData(UINT& out)
    {
        if (dataReady())
        {
            //yes start_ will wrap around with only 8 bits, that's what we want
            out = data_[start_++];
            return true;
        }
        return false;
    }

    UCHAR start_;
    UCHAR end_;
    UINT data_[256];
};

class CIRDriver
{
public:
    CIRDriver();
    ~CIRDriver();

    bool start(HANDLE threadExitEvent);
    void stop();

    uint32_t readData(uint32_t maxusec);
    bool     dataReady() const;
    bool     getData(UINT& out);
    bool     waitTillDataIsReady(uint32_t maxUSecs) const;

private:

    UniqueHandle<SerialPortTraits> initPort(Settings& settings) const;
    void  resetPort();
    void  setData(UINT data);
    DWORD threadProc(Settings const& settings) const;

    //==========================
    //int32_t devicetype;
    //int32_t virtpulse;
    //==========================
    mutable DataBuffer  dataBuffer; // thread safe!
    //==========================
    UniqueHandle<SerialPortTraits> hPort;
    UniqueHandle<EventTraits> hDataReadyEvent;
    UniqueHandle<EventTraits> threadExitEvent_;
    //==========================

    std::thread irThread_;
};
