// i_variant.hpp
/*
 *  Copyright (c) 2021 Leigh Johnston.
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
#include <type_traits>
#include <optional>
#include <variant>
#include <neolib/core/variadic.hpp>

namespace neolib
{
    template <typename... Types>
    class i_variant
    {
        typedef i_variant<Types...> self_type;
    public:
        typedef self_type abstract_type;
    public:
        virtual ~i_variant() = default;
    public:
        virtual std::size_t index() const = 0;
    private:
        virtual void const* ptr() const = 0;
        virtual void* ptr() = 0;
    public:
        template <typename T>
        bool holds_alternative() const
        {
            return index() == variadic::index_v<T, Types...> + 1;
        }
        template <typename T>
        T const* get_if() const
        {
            if (holds_alternative<T>())
                return static_cast<T const*>(ptr());
            return nullptr;
        }
        template <typename T>
        T* get_if()
        {
            if (holds_alternative<T>())
                return static_cast<T const*>(ptr());
            return nullptr;
        }
        template <typename T>
        T const& get() const
        {
            if (holds_alternative<T>())
                return *get_if<T>();
            throw std::bad_variant_access();
        }
        template <typename T>
        T& get()
        {
            if (holds_alternative<T>())
                return *get_if<T>();
            throw std::bad_variant_access();
        }
    };
}
