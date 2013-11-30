#pragma once

#include "UniqueHandle.h"

/// Traits for handles returned by FindFirsFile, FindFirstFileEx,
/// FindFirstFileTransacted.
struct FileFindTraits
{
	typedef HANDLE HandleType;
	static HandleType invalidValue() { return INVALID_HANDLE_VALUE; }
	static void close(HandleType h) { ::FindClose(h); }
};

class FileFinder : private UniqueHandle<FileFindTraits>
{
	typedef UniqueHandle<FileFindTraits> Base;
public:

	bool FindFile(CString const& fileName)
	{
		reset(::FindFirstFile(fileName, &findData_));
		return *this;
	}

	bool FindNextFile()
	{
		return ::FindNextFile(get(), &findData_) != FALSE;
	}

	//CString GetFilePath() const { return findData_.}
	CString GetFileName() const { return findData_.cFileName; }

private:
	WIN32_FIND_DATA findData_;
};
