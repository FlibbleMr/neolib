// os_version.hpp
/* 
 * Parts Copyright (c) 2007 Leigh Johnston.
 * The author makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 */

#include <neolib/neolib.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/app/version.hpp>
#include <neolib/app/application_info.hpp>

namespace neolib
{
    NEOLIB_EXPORT std::string os_name();
    NEOLIB_EXPORT application_info get_application_info(i_application_info const& aAppInfo);
}