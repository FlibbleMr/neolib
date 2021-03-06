// jar.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <vector>
#include <set>
#include <mutex>
#include <atomic>
#include <neolib/core/vector.hpp>
#include <neolib/core/mutex.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_jar.hpp>

namespace neolib
{
    template <typename CookieType>
    class basic_cookie_ref_ptr
    {
    public:
        typedef CookieType cookie_type;
        static constexpr cookie_type no_cookie = cookie_type{};
    public:
        basic_cookie_ref_ptr() :
            iConsumer{ nullptr },
            iCookie{ no_cookie }
        {
        }
        basic_cookie_ref_ptr(i_basic_cookie_consumer<cookie_type>& aConsumer, cookie_type aCookie) :
            iConsumer{ &aConsumer },
            iCookie{ aCookie }
        {
            add_ref();
        }
        ~basic_cookie_ref_ptr()
        {
            release();
        }
        basic_cookie_ref_ptr(basic_cookie_ref_ptr const& aOther) :
            iConsumer{ aOther.iConsumer },
            iCookie{ aOther.iCookie }
        {
            add_ref();
        }
        basic_cookie_ref_ptr(basic_cookie_ref_ptr&& aOther) :
            iConsumer{ aOther.iConsumer },
            iCookie{ aOther.iCookie }
        {
            add_ref();
            aOther.release();
        }
    public:
        basic_cookie_ref_ptr& operator=(basic_cookie_ref_ptr const& aOther)
        {
            if (&aOther == this)
                return *this;
            basic_cookie_ref_ptr temp{ std::move(*this) };
            iConsumer = aOther.iConsumer;
            iCookie = aOther.iCookie;
            add_ref();
            return *this;
        }
        basic_cookie_ref_ptr& operator=(basic_cookie_ref_ptr&& aOther)
        {
            if (&aOther == this)
                return *this;
            basic_cookie_ref_ptr temp{ std::move(*this) };
            iConsumer = aOther.iConsumer;
            iCookie = aOther.iCookie;
            add_ref();
            aOther.release();
            return *this;
        }
    public:
        bool operator==(basic_cookie_ref_ptr const& aRhs) const
        {
            return iConsumer == aRhs.iConsumer && iCookie == aRhs.iCookie;
        }
        bool operator!=(basic_cookie_ref_ptr const& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(basic_cookie_ref_ptr const& aRhs) const
        {
            return std::tie(iConsumer, iCookie) < std::tie(aRhs.iConsumer, aRhs.iCookie);
        }
    public:
        bool valid() const
        {
            return have_consumer() && have_cookie();
        }
        bool expired() const
        {
            return !valid();
        }
        cookie_type cookie() const
        {
            return iCookie;
        }
        void reset() const
        {
            iConsumer = nullptr;
            iCookie = no_cookie;
        }
    private:
        void add_ref() const
        {
            if (!valid())
                return;
            consumer().add_ref(cookie());
        }
        void release() const
        {
            if (!valid())
                return;
            consumer().release(cookie());
            reset();
        }
        bool have_consumer() const
        {
            return iConsumer != nullptr;
        }
        i_basic_cookie_consumer<cookie_type>& consumer() const
        {
            return *iConsumer;
        }
        bool have_cookie() const
        {
            return iCookie != no_cookie;
        }
    private:
        mutable i_basic_cookie_consumer<cookie_type>* iConsumer;
        mutable cookie_type iCookie;
    };

    namespace detail
    {
        template<typename T> struct is_smart_ptr : std::false_type {};
        template<typename T> struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};
        template<typename T> struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};
        template<typename T> struct is_smart_ptr<ref_ptr<T>> : std::true_type {};
        template<typename T>
        inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;
    }

    template <typename T, typename Container = vector<T>, typename CookieType = cookie, typename MutexType = null_mutex>
    class basic_jar : public reference_counted<i_basic_jar<abstract_t<T>, abstract_t<Container>, CookieType>>
    {
    public:
        typedef CookieType cookie_type;
    public:
        typedef T value_type;
        typedef Container container_type;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename container_type::iterator iterator;
        typedef MutexType mutex_type;
    private:
        typedef typename container_type::size_type reverse_index_t;
        typedef std::vector<reverse_index_t> reverse_indices_t;
        typedef std::vector<cookie_type> cookies_t;
    private:
        static constexpr cookie_type INVALID_COOKIE = invalid_cookie<cookie_type>;
        static constexpr reverse_index_t INVALID_REVERSE_INDEX = static_cast<reverse_index_t>(~reverse_index_t{});
    public:
        basic_jar() : iNextAvailableCookie{}
        {
        }
    public:
        bool empty() const override
        {
            return items().empty();
        }
        std::size_t size() const override
        {
            return items().size();
        }
        bool contains(cookie_type aCookie) const override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return aCookie < reverse_indices().size() && reverse_indices()[aCookie] != INVALID_REVERSE_INDEX;
        }
        const_iterator find(cookie_type aCookie) const override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[aCookie]);
            return end();
        }
        iterator find(cookie_type aCookie) override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[aCookie]);
            return end();
        }
        const value_type& operator[](cookie_type aCookie) const override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (aCookie >= reverse_indices().size())
                throw cookie_invalid();
            auto reverseIndex = reverse_indices()[aCookie];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            return items()[reverseIndex];
        }
        value_type& operator[](cookie_type aCookie) override
        {
            return const_cast<value_type&>(to_const(*this)[aCookie]);
        }
        const value_type& at_index(std::size_t aIndex) const override
        {
            return items().at(aIndex);
        }
        value_type& at_index(std::size_t aIndex) override
        {
            return items().at(aIndex);
        }
        cookie_type insert(abstract_t<value_type> const& aItem) override
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, aItem);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        template <typename... Args>
        cookie_type emplace(Args&&... aArgs)
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, std::forward<Args>(aArgs)...);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        iterator add(cookie_type aCookie, abstract_t<value_type> const& aItem) override
        {
            return add<const abstract_t<value_type>&>(aCookie, aItem);
        }
        iterator erase(const_iterator aItem) override
        {
            return remove(*aItem);
        }
        template <typename... Args>
        iterator add(cookie_type aCookie, Args&&... aArgs)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (reverse_indices().size() <= aCookie)
                reverse_indices().insert(reverse_indices().end(), (aCookie + 1) - reverse_indices().size(), INVALID_REVERSE_INDEX);
            if (reverse_indices()[aCookie] != INVALID_REVERSE_INDEX)
                throw cookie_already_added();
            std::optional<iterator> result;
            if constexpr (!detail::is_smart_ptr_v<value_type>)
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && std::is_abstract_v<typename value_type::element_type>)
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && !std::is_abstract_v<typename value_type::element_type>)
                result = items().insert(items().end(), value_type{ new typename value_type::element_type{std::forward<Args>(aArgs)...} });
            try
            {
                allocated_cookies().insert(allocated_cookies().end(), aCookie);
            }
            catch (...)
            {
                items().pop_back();
                throw;
            }
            reverse_indices()[aCookie] = items().size() - 1;
            return *result;
        }
        iterator remove(abstract_t<value_type> const& aItem) override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return remove(item_cookie(aItem));
        }
        iterator remove(cookie_type aCookie) override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (aCookie >= reverse_indices().size())
                throw cookie_invalid();
            auto& reverseIndex = reverse_indices()[aCookie];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            if (reverseIndex < items().size() - 1)
            {
                auto& item = items()[reverseIndex];
                std::swap(item, items().back());
                auto& cookie = allocated_cookies()[reverseIndex];
                std::swap(cookie, allocated_cookies().back());
                reverse_indices()[cookie] = reverseIndex;
            }
            items().pop_back();
            allocated_cookies().pop_back();
            iterator result = std::next(items().begin(), reverseIndex);
            reverseIndex = INVALID_REVERSE_INDEX;
            return_cookie(aCookie);
            return result;
        }
    public:
        cookie_type item_cookie(abstract_t<value_type> const& aItem) const override
        {
            if constexpr (!std::is_pointer_v<value_type>)
                return allocated_cookies()[&static_cast<value_type const&>(aItem) - &items()[0]];
            else
                throw no_pointer_value_type_cookie_lookup();
        }
        cookie_type next_cookie() override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (!free_cookies().empty())
            {
                auto nextCookie = free_cookies().back();
                free_cookies().pop_back();
                return nextCookie;
            }
            auto nextCookie = ++iNextAvailableCookie;
            if (nextCookie == INVALID_COOKIE)
                throw cookies_exhausted();
            assert(std::find(free_cookies().begin(), free_cookies().end(), nextCookie) == free_cookies().end());
            return nextCookie;
        }
        void return_cookie(cookie_type aCookie) override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            free_cookies().push_back(aCookie);
        }
    public:
        mutex_type& mutex() const
        {
            return iMutex;
        }
        const_iterator cbegin() const override
        {
            return items().begin();
        }
        const_iterator begin() const override
        {
            return cbegin();
        }
        iterator begin() override
        {
            return items().begin();
        }
        const_iterator cend() const override
        {
            return items().end();
        }
        const_iterator end() const override
        {
            return cend();
        }
        iterator end() override
        {
            return items().end();
        }
    public:
        void clear() override
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            iNextAvailableCookie = 0ul;
            allocated_cookies().clear();
            free_cookies().clear();
            items().clear();
            reverse_indices().clear();
        }
        const container_type& items() const override
        {
            return iItems;
        }
        container_type& items() override
        {
            return iItems;
        }
    private:
        const cookies_t& allocated_cookies() const
        {
            return iAllocatedCookies;
        }
        cookies_t& allocated_cookies()
        {
            return iAllocatedCookies;
        }
        const cookies_t& free_cookies() const
        {
            return iFreeCookies;
        }
        cookies_t& free_cookies()
        {
            return iFreeCookies;
        }
        const reverse_indices_t& reverse_indices() const
        {
            return iReverseIndices;
        }
        reverse_indices_t& reverse_indices()
        {
            return iReverseIndices;
        }
    private:
        mutable mutex_type iMutex;
        mutable std::atomic<cookie_type> iNextAvailableCookie;
        cookies_t iAllocatedCookies;
        container_type iItems;
        mutable cookies_t iFreeCookies;
        reverse_indices_t iReverseIndices;
    };

    typedef basic_cookie_ref_ptr<cookie> cookie_ref_ptr;
    typedef basic_cookie_ref_ptr<small_cookie> small_cookie_ref_ptr;

    template <typename T, typename MutexType = null_mutex>
    using jar = basic_jar<T, vector<T>, cookie, MutexType>;
    template <typename T, typename MutexType = null_mutex>
    using small_jar = basic_jar<T, vector<T>, small_cookie, MutexType>;
}
