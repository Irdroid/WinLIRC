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
#include "irconfig.h"
#include "config.h"
#include "drvdlg.h"
#include "server.h"

unsigned int DaemonThread(void* drv) {
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_IDLE);
    static_cast<CIRDriver*>(drv)->DaemonThreadProc();
    return 0;
}
	
CIRDriver::CIRDriver()
	: daemonThreadEvent(FALSE, TRUE)
	, dll()
{ }

CIRDriver::~CIRDriver()
{
	unloadPlugin();

	if(daemonThreadHandle.joinable()) {
		daemonThreadHandle.join();
	}
}

bool CIRDriver::loadPlugin(CString plugin) {

	//
	//make sure we have cleaned up first
	//

	std::unique_lock<std::mutex> l(dllLock);
	unloadPlugin();

	dll = Dll(plugin);
	return dll;
}

void CIRDriver::unloadPlugin() {

	//
	// make sure we have cleaned up
	//
	deinit();

	// daemon thread should not be dead now.
	ASSERT(!daemonThreadHandle.joinable());

	dll = Dll();
}

BOOL CIRDriver::init()
{
	//
	// safe to call deinit first
	//
	deinit();

	// daemon thread should be dead now.
	ASSERT(!daemonThreadHandle.joinable());

	if (dll.initFunction)
	{
		// Make sure that the event is not set. Otherwise the plugin
		// will stop as soon as it has started.
		ResetEvent(daemonThreadEvent.get());
		if (dll.initFunction(daemonThreadEvent.get()))
		{

			//printf("started thread ..\n");
			try
			{
				daemonThreadHandle = std::thread([this]() { DaemonThread(this); });
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		else
		{
			deinit();
		}
	}

	return FALSE;
}

void CIRDriver::deinit() {

	KillThread(daemonThreadHandle, daemonThreadEvent);
	// daemon thread should be dead now.
	ASSERT(!daemonThreadHandle.joinable());

	if(dll.deinitFunction) {
		dll.deinitFunction();
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	std::unique_lock<std::mutex> l(dllLock);
	if(dll.sendFunction) {
		return dll.sendFunction(remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out) {

	std::unique_lock<std::mutex> l(dllLock);
	if(dll.decodeFunction) {
		return dll.decodeFunction(remote,out);
	}

	return 0;
}

int	CIRDriver::setTransmitters(unsigned int transmitterMask) {

	std::unique_lock<std::mutex> l(dllLock);
	if(dll.setTransmittersFunction) {
		return dll.setTransmittersFunction(transmitterMask);
	}

	return 0;
}

void CIRDriver::DaemonThreadProc(void) const {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	//==========================
	char message[PACKET_SIZE+1];
	//==========================

	while(WaitForSingleObject(daemonThreadEvent.get(), 0) == WAIT_TIMEOUT) {

		std::unique_lock<std::mutex> l(dllLock);
		ASSERT(dll.decodeFunction != NULL);
		if(dll.decodeFunction(global_remotes,message)) {
			l.unlock();

			//======================
			UINT64	keyCode;
			INT		repeat;
			CHAR	command[128];
			CHAR	remoteName[128];
			//======================

			if(config.disableRepeats) {

				if(sscanf(message,"%I64x %x",&keyCode,&repeat)==2) {
				
					if(repeat) continue;
				}
			}
			else if(config.disableFirstKeyRepeats>0) {
				
				if(sscanf(message,"%I64x %x %s %s",&keyCode,&repeat,command,remoteName)==4) {
				
					if(repeat) {

						if(repeat<=config.disableFirstKeyRepeats) continue;
						else {
							sprintf(message,"%016llx %02x %s %s\n",keyCode,repeat-config.disableFirstKeyRepeats,command,remoteName);
						}
					}
				}
			}

			::dlg->GoGreen();
			::server->sendToClients(message);
		}

	}
}

