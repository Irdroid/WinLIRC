#pragma once

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
	typedef int(*SendFunction)(struct ir_remote *remote, struct ir_ncode *code, int repeats);
	typedef int(*DecodeFunction)(struct ir_remote *remote, char *out);
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
