#pragma once

/// Non-owning handle. Purpose of this is to shield users of handles
/// from specific knowledge of how to detect is handle is valid or not.
template <typename HandleTraits>
class WeakHandle
{
	typedef HandleTraits Traits;
	typedef typename Traits::HandleType HandleType;

public:

	explicit WeakHandle(HandleType h = Traits::invalidValue())
		: handle_(h)
	{ }

	HandleType get() const { return handle_; }

	typedef HandleType WeakHandle::* safe_bool;

	operator safe_bool() const
	{
		return handle_ == Traits::invalidValue()
			? nullptr
			: &WeakHandle::handle_;
	}

private:

	HandleType handle_;
};
