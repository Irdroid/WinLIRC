#pragma once

#include "UniqueHandle.h"
#include <utility>

/// Traits for handles returned by CreateEvent.
struct EventTraits
{
	typedef HANDLE HandleType;

	static HandleType invalidValue() { return nullptr; }
	static void close(HandleType h) { ::CloseHandle(h); }
};

class Event : public UniqueHandle<EventTraits>
{
	typedef UniqueHandle<EventTraits> Base;

public:

	explicit Event(
		BOOL bInitiallyOwn = FALSE,
		BOOL bManualReset = FALSE,
		LPCTSTR lpszName = NULL,
		LPSECURITY_ATTRIBUTES lpsaAttribute = NULL
		)
		: Base(::CreateEvent(lpsaAttribute, bManualReset, bInitiallyOwn, lpszName))
	{ }

	Event(Event&& src)
		: Base(std::move(src))
	{ }

	Event& operator=(Event&& rhs)
	{
		Base::operator=(std::move(rhs));
		return *this;
	}

	void SetEvent() const { ::SetEvent(get()); }
	void ResetEvent() const { ::ResetEvent(get()); }
};
