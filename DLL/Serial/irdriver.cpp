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

#include "stdafx.h"
#include "irdriver.h"
#include "Globals.h"
#include "../Common/LIRCDefines.h"
#include "Transmit.h"
#include "Settings.h"

#include <cassert>

CIRDriver::CIRDriver()
    : hDataReadyEvent(FALSE, TRUE)
{ }

bool CIRDriver::start(HANDLE threadExitEvent)
{
    if (threadExitEvent_ || hPort)
        return false;

    try
    {
        hDataReadyEvent.ResetEvent();
        threadExitEvent_ = WeakHandle<EventTraits>(threadExitEvent);

        dataBuffer = DataBuffer();
        Settings settings;
        hPort = initPort(settings);

        if (hPort)
        {
            irThread_ = std::thread([this, settings]()
            {
                HANDLE const hProcess = ::GetCurrentProcess();
                DWORD const oldPriorityClass = ::GetPriorityClass(hProcess);
                ::SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
                ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                this->threadProc(settings);
                ::SetPriorityClass(hProcess, oldPriorityClass);
            });
            return true;
        }
    }
    catch (...)
    { }

    return false;
}

void CIRDriver::stop()
{
    if (threadExitEvent_)
    {
        ::SetEvent(threadExitEvent_.get());
        if (irThread_.joinable())
            irThread_.join();
        hPort.close();
    }
}

CIRDriver::~CIRDriver()
{
    stop();
}

UniqueHandle<SerialPortTraits> CIRDriver::initPort(Settings& settings) const
{
    settings.loadSettings();

    UniqueHandle<SerialPortTraits> hSerialPort(CreateFile(
        settings.portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        0));

    if (hSerialPort)
    {
        DCB dcb;
        if (!GetCommState(hSerialPort.get(), &dcb))
        {
            //DEBUG("GetCommState failed.\n");
            hSerialPort.close();
        }
        else
        {
            dcb.fDtrControl = settings.animax
                ? DTR_CONTROL_ENABLE  //set DTR high, the animax receiver needs this for power
                : DTR_CONTROL_DISABLE; // set the transmit LED to off initially.
            dcb.fRtsControl = RTS_CONTROL_ENABLE;

            dcb.BaudRate = _tstoi(settings.speed);

            if (!SetCommState(hSerialPort.get(), &dcb))
            {
                //DEBUG("SetCommState failed.\n");
                hSerialPort.close();
            }
            else
            {
                SetTransmitPort(hSerialPort.get(), settings.transmitterType);

                if (settings.sense == -1)
                {
                    /* Wait for receiver to settle (since we just powered it on) */
                    Sleep(1000);
                    DWORD state;
                    if (GetCommModemStatus(hSerialPort.get(), &state))
                    {
                        settings.sense = (state & MS_RLSD_ON) ? 1 : 0;
                        //DEBUG("Sense set to %d\n",sense);
                    }
                }
            }
        }
    }
    return hSerialPort;
}

void CIRDriver::resetPort()
{
	//DEBUG("Resetting port\n");
    hPort.close();
}

DWORD CIRDriver::threadProc(Settings const& s) const
{
    int32_t sense = s.sense;
	/* Virtually no error checking is done here, because */
	/* it's pretty safe to assume that everything works, */
	/* and we have nowhere to report errors anyway.      */

	/* We use two timers in case the high resolution doesn't   */
	/* last too long before wrapping around (is that true?     */
	/* is it really only a 32 bit timer or a true 64 bit one?) */

	LARGE_INTEGER hr_time, hr_lasttime, hr_freq;	// high-resolution
	time_t lr_time, lr_lasttime;			// low-resolution

    assert(hPort);

	DWORD status;
	GetCommModemStatus(hPort.get(), &status);
	int prev=(status & MS_RLSD_ON) ? 1 : 0;

	/* Initialize timer stuff */
	QueryPerformanceFrequency(&hr_freq);

	/* Get time (both LR and HR) */
	time(&lr_lasttime);
	QueryPerformanceCounter(&hr_lasttime);
	
    Event commIoEvent(FALSE, TRUE);
    if (!commIoEvent)
        return 1;

    OVERLAPPED ov = {0};
    ov.hEvent = commIoEvent.get();

	HANDLE const events[] = { ov.hEvent, threadExitEvent_.get() };

	for(;;)
	{
		/* We want to be notified of DCD or RX changes */
        if(SetCommMask(hPort.get(), s.deviceType ? EV_RLSD : EV_RXCHAR)==0)	
		{
			//DEBUG("SetCommMask returned zero, error=%d\n",GetLastError());
		}
		/* Reset the event */
		ResetEvent(ov.hEvent);
		/* Start waiting for the event */
		DWORD event;
		if(WaitCommEvent(hPort.get(),&event,&ov)==0 && GetLastError()!=997)
		{
			//DEBUG("WaitCommEvent error: %d\n",GetLastError());
		}

		/* Wait for the event to get triggered */
		DWORD const res = WaitForMultipleObjects(2,events,FALSE,INFINITE);
		
		/* Get time (both LR and HR) */
		QueryPerformanceCounter(&hr_time);
		time(&lr_time);
		
		if (res==WAIT_FAILED)
		{
			//DEBUG("Wait failed.\n");
			continue;
		}
		
		if(res==(WAIT_OBJECT_0+1))
		{
			//DEBUG("IRThread terminating\n");
			return 0;
		}
		
		if(res!=WAIT_OBJECT_0)
		{
			//DEBUG("Wrong object\n");
			continue;
		}

		int dcd;
		if (s.deviceType) {
			GetCommModemStatus(hPort.get(), &status);

			dcd = (status & MS_RLSD_ON) ? 1 : 0;

			if(dcd==prev)
			{
				/* Nothing changed?! */
				/* Continue without changing time */
				continue;
			}

			prev=dcd;
		}

		int deltv=(int)(lr_time-lr_lasttime);
		if (s.deviceType && (deltv>15)) {		
			/* More than 15 seconds passed */
			deltv=0xFFFFFF;
			if(!(dcd^sense))
			{
				/* sense had to be wrong */
				sense=sense?0:1;
				//DEBUG("sense was wrong!\n");
			}
		} else
            deltv=(int)(((hr_time.QuadPart-hr_lasttime.QuadPart)*1000000) / hr_freq.QuadPart);
	
		lr_lasttime=lr_time;
		hr_lasttime=hr_time;
		
		int data;				
		if (s.deviceType) {		
			data = (dcd^sense) ? (deltv) : (deltv | 0x1000000);	

            dataBuffer.pushData(data);
			SetEvent(hDataReadyEvent.get());
		} else {
			data = deltv;	

			dataBuffer.pushData(data-100);
            dataBuffer.pushData(s.virtualPulse | 0x1000000);
			SetEvent(hDataReadyEvent.get());
			PurgeComm(hPort.get(),PURGE_RXCLEAR);
		}
	}

    return 0;
}

void CIRDriver::setData(UINT data)
{
    dataBuffer.pushData(data);
}

bool CIRDriver::dataReady() const
{
    return dataBuffer.dataReady();
}

bool CIRDriver::getData(UINT& out)
{
    return dataBuffer.popData(out);
}

uint32_t CIRDriver::readData(uint32_t maxusec)
{
	if (waitTillDataIsReady(maxusec))
    {
	    UINT x = 0;
	    getData(x);
	    return x;
    }
    else
    {
        return 0;
    }
}

/// @retval true if data is ready or waiting for data timed out.
/// @retval false if wait was interrupted by threadExitEvent or there is no threadExitEvent.
bool CIRDriver::waitTillDataIsReady(uint32_t maxUSecs) const
{
    // no event to notify us to stop working?
    if (!threadExitEvent_)
    {
        return false;
    }
    else if (!dataReady())
    {
        ResetEvent(hDataReadyEvent.get());
        DWORD const timeout = maxUSecs
            ? (maxUSecs+500)/1000
            : INFINITE;

        HANDLE const events[] = { hDataReadyEvent.get(), threadExitEvent_.get() };
        DWORD const res = WaitForMultipleObjects(2, events, false, timeout);
        if (res == (WAIT_OBJECT_0+1))
        {
            //DEBUG("Unknown thread terminating (readdata)\n");
            return false;
        }
    }
    return true;
}

