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

#ifndef IRDRIVER_H
#define IRDRIVER_H

#include "globals.h"
#include "winlirc.h"

#include "../lib/Utility/Module.h"

struct Dll
{
	Dll(CString file = CString())
		: dllFile(file, ~0)
		, initFunction(nullptr)
		, deinitFunction(nullptr)
		, hasGuiFunction(nullptr)
		, loadSetupGuiFunction(nullptr)
		, sendFunction(nullptr)
		, decodeFunction(nullptr)
		, setTransmittersFunction(nullptr)
	{
		if (dllFile)
		{
			initFunction = dllFile.getProc<InitFunction>("init");
			deinitFunction = dllFile.getProc<DeinitFunction>("deinit");
			hasGuiFunction = dllFile.getProc<HasGuiFunction>("hasGui");
			loadSetupGuiFunction = dllFile.getProc<LoadSetupGuiFunction>("loadSetupGui");
			sendFunction = dllFile.getProc<SendFunction>("sendIR");
			decodeFunction = dllFile.getProc<DecodeFunction>("decodeIR");
			setTransmittersFunction = dllFile.getProc<SetTransmittersFunction>("setTransmitters");
		}
	}

	Dll(Dll&& src)
	{
		*this = std::move(src);
	}

	Dll& operator=(Dll& rhs)
	{
		if (this != &rhs)
		{
			dllFile = std::move(rhs.dllFile);
			initFunction = rhs.initFunction;
			deinitFunction = rhs.deinitFunction;
			hasGuiFunction = rhs.hasGuiFunction;
			loadSetupGuiFunction = rhs.loadSetupGuiFunction;
			sendFunction = rhs.sendFunction;
			decodeFunction = rhs.decodeFunction;
			setTransmittersFunction = rhs.setTransmittersFunction;
		}

		return *this;
	}

	struct S { int i; };
	typedef int S::*BoolType;

	operator BoolType() const
	{
		bool res = dllFile && initFunction && deinitFunction && hasGuiFunction && loadSetupGuiFunction && sendFunction && decodeFunction;
		return res ? &S::i : nullptr;
	}

	typedef int(*InitFunction)(HANDLE);
	typedef void(*DeinitFunction)();
	typedef int(*HasGuiFunction)();
	typedef void(*LoadSetupGuiFunction)();
	typedef int(*SendFunction)(ir_remote *remote, ir_ncode *code, int repeats);
	typedef int(*DecodeFunction)(ir_remote *remote, char *out);
	typedef int(*SetTransmittersFunction)(unsigned int transmitterMask);

	Module		dllFile;
	InitFunction			initFunction;
	DeinitFunction			deinitFunction;
	HasGuiFunction			hasGuiFunction;
	LoadSetupGuiFunction	loadSetupGuiFunction;
	SendFunction			sendFunction;
	DecodeFunction			decodeFunction;
	SetTransmittersFunction setTransmittersFunction;
};

class CIRDriver
{

public:
	CIRDriver();
   ~CIRDriver();

	bool	loadPlugin		(CString plugin);
	void	unloadPlugin	();
	BOOL	init			();
	void	deinit			();
	int		sendIR			(struct ir_remote *remote,struct ir_ncode *code, int repeats);
	int		decodeIR		(struct ir_remote *remote, char *out);
	int		setTransmitters	(unsigned int transmitterMask);

	void	DaemonThreadProc() const;

private:

	/// Protects access to the functions imported from plug-in dll, and the
	/// DLL handle.
	mutable std::mutex dllLock;
	Dll dll;

	//==============================
	Event		daemonThreadEvent;
	std::thread	daemonThreadHandle;
	//==============================
};

#endif
