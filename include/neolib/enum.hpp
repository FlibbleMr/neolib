// i_enum.hpp
/*
 *  Copyright (c) 2019 Leigh Johnston.
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
#include <neolib/reference_counted.hpp>
#include <neolib/string.hpp>
#include <neolib/map.hpp>
#include <neolib/i_enum.hpp>

namespace neolib
{
    namespace enum_traits
    {
        template <typename Enum>
        using enum_enumerators_t = map<std::underlying_type<Enum>, string>;
        template <typename Enum>
        const enum_enumerators_t<Enum> enum_enumerators_v;
    }

    using namespace enum_traits;

    template <typename Enum>
    class basic_enum : public reference_counted<i_basic_enum<std::underlying_type_t<Enum>>>
    {
        typedef basic_enum<Enum> self_type;
        typedef reference_counted<i_basic_enum<std::underlying_type_t<Enum>>> base_type;
        // exceptions
    public:
        using base_type::bad_enum_string;
        // types
    public:
        typedef Enum enum_type;
        typedef std::underlying_type_t<Enum> underlying_type;
        // construction/assignment
    public:
        basic_enum() :
            iValue{}
        {
        }
        basic_enum(enum_type aValue) :
            iValue{ aValue }
        {
        }
        self_type* clone() const override
        {
            return new self_type{ *this };
        }
        self_type& assign(const self_type& aRhs) override
        {
            iValue = aRhs.value();
            return *this;
        }
        // state
    public:
        const underlying_type& value() const override
        {
            return iValue;
        }
        underlying_type& value() override
        {
            return iValue;
        }
        underlying_type& set_value(const i_string& aValue) override
        {
            for (auto const& e : enumerators())
                if (e.second == aValue)
                {
                    iValue = e.first;
                    return value();
                }
            throw bad_enum_string();
        }
        // meta
    public:
        void to_string(i_string& aString) const override
        {
            aString = enumerators()[value()];
        }
        const enum_enumerators_t<enum_type>& enumerators() const override
        {
            return enum_enumerators_v<enum_type>;
        }
        // state
    public:
        enum_type iValue;
    };

    template <typename Enum>
    using enum_t = basic_enum<Enum>;
}

using namespace neolib::enum_traits;