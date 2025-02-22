# GCC & Clang support 128-bit integers on 64-bit CPUs. However
# support for 128-bit integers in the C++ standard library is
# currently (2021) only enabled if the GNU extensions are enabled.
# The GNU extensions are usually enabled by default but will be
# disabled if the user compiles with the -std=c++* option. You
# should use -std=gnu++* instead of -std=c++* if you want to enable
# 128-bit integer support in primecount (or simply omit both
# -std=c++* and -std=gnu++*).

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}/include")

check_cxx_source_compiles("
    #include "int128_t.hpp"
    #include <limits>
    #include <type_traits>
    int main() {
        using namespace primecount;
        static_assert(std::numeric_limits<uint128_t>::max() != 0, \"\");
        static_assert(std::is_integral<int128_t>::value, \"\");
        static_assert(std::is_integral<uint128_t>::value, \"\");
        static_assert(std::is_signed<int128_t>::value, \"\");
        static_assert(std::is_unsigned<uint128_t>::value, \"\");
        static_assert(std::is_unsigned<typename std::make_unsigned<int128_t>::type>::value, \"\");
        return 0;
    }" int128)

if(NOT int128)
    # Let's try it again with C++11, this usually works for old
    # GCC/Clang versions that use C++03 by default. We add the
    # -std=gnu++11 option before the user's CXXFLAGS to ensure
    # we don't overwrite any of the user's flags.
    if(CMAKE_CXX11_EXTENSION_COMPILE_OPTION)
        set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX11_EXTENSION_COMPILE_OPTION} ${CMAKE_CXX_FLAGS}")
    elseif(CMAKE_CXX11_STANDARD_COMPILE_OPTION)
        set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX11_STANDARD_COMPILE_OPTION} ${CMAKE_CXX_FLAGS}")
    endif()

    check_cxx_source_compiles("
        #include "int128_t.hpp"
        #include <limits>
        #include <type_traits>
        int main() {
            using namespace primecount;
            static_assert(std::numeric_limits<uint128_t>::max() != 0, \"\");
            static_assert(std::is_integral<int128_t>::value, \"\");
            static_assert(std::is_integral<uint128_t>::value, \"\");
            static_assert(std::is_signed<int128_t>::value, \"\");
            static_assert(std::is_unsigned<uint128_t>::value, \"\");
            static_assert(std::is_unsigned<typename std::make_unsigned<int128_t>::type>::value, \"\");
            return 0;
        }" int128_with_cpp11)

    if(NOT int128_with_cpp11)
        set(DISABLE_INT128 "DISABLE_INT128")

        # Print a warning message if the user has specified the -std=c++*
        # option and 128-bit integers are disabled because this of this
        # option. We try to overwrite -std=c++* by -std=gnu++11 and check
        # if this would enable 128-bit integers.
        if(CMAKE_CXX11_EXTENSION_COMPILE_OPTION AND "${CMAKE_CXX_FLAGS}" MATCHES "-std=c\\+\\+[0-9]+")
            set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX11_EXTENSION_COMPILE_OPTION}")
            set(CMAKE_REQUIRED_QUIET TRUE)

            check_cxx_source_compiles("
                #include "int128_t.hpp"
                #include <limits>
                #include <type_traits>
                int main() {
                    using namespace primecount;
                    static_assert(std::numeric_limits<uint128_t>::max() != 0, \"\");
                    static_assert(std::is_integral<int128_t>::value, \"\");
                    static_assert(std::is_integral<uint128_t>::value, \"\");
                    static_assert(std::is_signed<int128_t>::value, \"\");
                    static_assert(std::is_unsigned<uint128_t>::value, \"\");
                    static_assert(std::is_unsigned<typename std::make_unsigned<int128_t>::type>::value, \"\");
                    return 0;
                }" int128_with_gnu11)

            if(int128_with_gnu11)
                message(WARNING "Your usage of -std=c++* disables 128-bit integer support in "
                                "primecount, primecount will only support numbers <= 2^63. "
                                "Use -std=gnu++* instead to enable 128-bit integer support "
                                "(or omit both -std=c++* and -std=gnu++*).")
            endif()
        endif()
    endif()
endif()

cmake_pop_check_state()
