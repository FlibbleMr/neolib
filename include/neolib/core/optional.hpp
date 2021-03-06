// optional.hpp
/*
 *  Copyright (c) 2007, 2021 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <optional>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_optional.hpp>

namespace neolib
{
    template<typename T>
    class optional;

    template <typename T>
    struct is_optional<optional<T>> { static constexpr bool value = true; };

    template<typename T>
    class optional : public reference_counted<i_optional<abstract_t<T>>>
    {
        typedef optional<T> self_type;
        // types
    public:
        typedef i_optional<abstract_t<T>> abstract_type;
        typedef std::optional<T> std_type;
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        // construction
    public:
        optional() :
            iData{}
        {
        }
        optional(std::nullopt_t) :
            iData{ std::nullopt }
        {
        }
        optional(value_type const& other) :
            iData{ other }
        {
        }
        optional(abstract_type const& other) :
            iData{}
        {
            *this = other;
        }
        template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U>, sfinae>>
        optional(U const& value) :
            iData{ value }
        {
        }
        // state
    public:
        bool valid() const noexcept override
        {
            return static_cast<bool>(*this);
        }
        bool invalid() const noexcept override
        {
            return !valid();
        }
        explicit operator bool() const noexcept override
        {
            return iData.operator bool();
        }
        // std
    public:
        std_type& to_std_optional()
        {
            return iData;
        }
        const std_type& to_std_optional() const
        {
            return iData;
        }
        // element access
    public:
        reference get() override
        {
            return iData.value();
        }
        const_reference get() const override
        {
            return iData.value();
        }
        reference operator*() override
        {
            return iData.operator*();
        }
        const_reference operator*() const override
        {
            return iData.operator*();
        }
        pointer operator->() override
        {
            return iData.operator->();
        }
        const_pointer operator->() const override
        {
            return iData.operator->();
        }
        // modifiers
    public:
        template <typename... Args>
        reference& emplace(Args&&... args)
        {
            iData.emplace(std::forward<Args>(args)...);
            return get();
        }
        void reset() override
        {
            iData.reset();
        }
        self_type& operator=(std::nullopt_t) noexcept override
        {
            iData = std::nullopt;
            return *this;
        }
        self_type& operator=(const self_type& rhs)
        {
            if (rhs.valid())
                iData = rhs.get();
            else
                iData = std::nullopt;
            return *this;
        }
        self_type& operator=(const abstract_type& rhs) override
        {
            if (rhs.valid())
                iData = T{ rhs.get() };
            else
                iData = std::nullopt;
            return *this;
        }
        self_type& operator=(const abstract_t<T>& value) override
        {
            iData = T{ value };
            return *this;
        }
    private:
        std::optional<T> iData;
    };

    template <typename T>
    inline bool operator==(const optional<T>& lhs, std::nullopt_t) noexcept
    {
        return !lhs.valid();
    }

    template <typename T>
    inline bool operator!=(const optional<T>& lhs, std::nullopt_t) noexcept
    {
        return lhs.valid();
    }

    template <typename T>
    inline bool operator==(const optional<T>& lhs, const optional<T>& rhs)
    {
        if (lhs.valid() != rhs.valid())
            return false;
        if (!lhs.valid())
            return true;
        return lhs.get() == rhs.get();
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator==(const optional<T>& lhs, const U& rhs)
    {
        if (!lhs.valid())
            return false;
        return lhs.get() == rhs;
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator==(const U& lhs, const optional<T>& rhs)
    {
        if (!rhs.valid())
            return false;
        return lhs == rhs.get();
    }

    template <typename T>
    inline bool operator!=(const optional<T>& lhs, const optional<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator!=(const optional<T>& lhs, const U& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator!=(const U& lhs, const optional<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    inline bool operator<(const optional<T>& lhs, const optional<T>& rhs)
    {
        if (lhs.valid() != rhs.valid())
            return lhs.valid() < rhs.valid();
        if (!lhs.valid())
            return false;
        return lhs.get() < rhs.get();
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator<(const optional<T>& lhs, const U& rhs)
    {
        if (!lhs.valid())
            return true;
        return lhs.get() < rhs;
    }

    template <typename T, typename U, typename = std::enable_if_t<!is_optional_v<U>, sfinae>>
    inline bool operator<(const U& lhs, const optional<T>& rhs)
    {
        if (!rhs.valid())
            return false;
        return lhs < rhs.get();
    }

    template <typename T, std::enable_if_t<!std::is_same_v<T, abstract_t<T>>, int> = 0>
    inline bool operator<(const optional<T>& lhs, const abstract_t<T>& rhs)
    {
        if (!lhs.valid())
            return true;
        return lhs.get() < rhs;
    }

    template <typename T, std::enable_if_t<!std::is_same_v<T, abstract_t<T>>, int> = 0>
    inline bool operator<(const abstract_t<T>& lhs, const optional<T>& rhs)
    {
        if (!rhs.valid())
            return false;
        return lhs < rhs.get();
    }

    template <typename T>
    struct optional_type { typedef T type; };
    template <typename T>
    struct optional_type<std::optional<T>> { typedef T type; };
    template <typename T>
    struct optional_type<optional<T>> { typedef T type; };

    template <typename T>
    using optional_t = typename optional_type<T>::type;
}
