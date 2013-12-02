#pragma once

#include "UniqueHandle.h"
#include <cassert>

/// Traits for handles returned by LoadLibrary, LoadLibraryEx, LoadPackagedLibrary, or GetModuleHandle.
struct ModuleTraits
{
	typedef HMODULE HandleType;

	static HandleType invalidValue() { return nullptr; }
	static void close(HandleType h) { ::FreeLibrary(h); }
};

class Module : public UniqueHandle<ModuleTraits>
{
	typedef UniqueHandle<ModuleTraits> Base;

	Module(Module const&);
	void operator=(Module const&);

public:

	explicit Module(HandleType h = Traits::invalidValue())
		: Base(h)
	{ }

	explicit Module(CString const& path)
		: Base(::LoadLibrary(path))
	{ }

	/// Allows you to select how the system reports errors that occur during load.
	Module(CString const& path, UINT errorMode)
		: Base()
	{
		UINT const backup = ::SetErrorMode(errorMode);
		reset(::LoadLibrary(path));
		::SetErrorMode(backup);
	}

	Module(Module&& src)
		: Base(std::move(src))
	{ }

	Module& operator=(Module&& rhs)
	{
		Base::operator=(std::move(rhs));
		return *this;
	}
	FARPROC getProcAddress(char const* procName) const
	{
		assert(procName != nullptr);
		assert(*this);
		return ::GetProcAddress(get(), procName);
	}

	template <typename ProcType>
	ProcType getProc(char const* procName) const
	{
		return reinterpret_cast<ProcType>(getProcAddress(procName));
	}
};