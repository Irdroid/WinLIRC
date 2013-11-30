#pragma once

#include "HandleTraits.h"
#include "UniqueHandle.h"

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
