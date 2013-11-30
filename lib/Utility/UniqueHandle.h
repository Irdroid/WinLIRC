#pragma once

template <typename HandleTraits>
class UniqueHandle
{
protected:
    typedef HandleTraits Traits;
    typedef typename Traits::HandleType HandleType;

private:
    UniqueHandle(UniqueHandle const&); // not copyable
    void operator=(UniqueHandle const&); // not assignable

public:

    explicit UniqueHandle(HandleType h = Traits::invalidValue())
        : handle_(h)
    { }

    ~UniqueHandle()
    {
        reset();
    }

    UniqueHandle(UniqueHandle&& other)
        : handle_(other.release())
    { }

    UniqueHandle& operator=(UniqueHandle&& rhs)
    {
        reset(rhs.release());
        return *this;
    }

    HandleType get() const { return handle_; }

    struct S { int i; };
    typedef int S::* safe_bool;

    operator safe_bool() const
    {
        return handle_ == Traits::invalidValue()
            ? nullptr
            : &S::i;
    }

    HandleType release()
    {
        HandleType const res = handle_;
        handle_ = Traits::invalidValue();
        return res;
    }

    void reset(HandleType replacement = Traits::invalidValue())
    {
        if (handle_ != replacement)
        {
            if (handle_ != Traits::invalidValue())
                Traits::close(handle_);
            handle_ = replacement;
        }
    }

    void close()
    {
        reset();
    }

private:

    HandleType handle_;
};
