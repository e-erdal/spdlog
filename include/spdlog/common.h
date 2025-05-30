// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/tweakme.h>
#include <spdlog/details/null_mutex.h>

#include <atomic>
#include <chrono>
#include <initializer_list>
#include <memory>
#include <exception>
#include <string>
#include <type_traits>
#include <functional>

#ifdef SPDLOG_COMPILED_LIB
#    undef SPDLOG_HEADER_ONLY
#    if defined(_WIN32) && defined(SPDLOG_SHARED_LIB)
#        ifdef spdlog_EXPORTS
#            define SPDLOG_API __declspec(dllexport)
#        else
#            define SPDLOG_API __declspec(dllimport)
#        endif
#    else // !defined(_WIN32) || !defined(SPDLOG_SHARED_LIB)
#        define SPDLOG_API
#    endif
#    define SPDLOG_INLINE
#else // !defined(SPDLOG_COMPILED_LIB)
#    define SPDLOG_API
#    define SPDLOG_HEADER_ONLY
#    define SPDLOG_INLINE inline
#endif // #ifdef SPDLOG_COMPILED_LIB

#include <spdlog/fmt/fmt.h>

// backward compatibility with fmt versions older than 8
#if FMT_VERSION >= 80000
#    define SPDLOG_FMT_RUNTIME(format_string) fmt::runtime(format_string)
#    if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
#        include <spdlog/fmt/xchar.h>
#    endif
#else
#    define SPDLOG_FMT_RUNTIME(format_string) format_string
#endif

// visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#    define SPDLOG_NOEXCEPT _NOEXCEPT
#    define SPDLOG_CONSTEXPR
#else
#    define SPDLOG_NOEXCEPT noexcept
#    define SPDLOG_CONSTEXPR constexpr
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#    define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#    define SPDLOG_DEPRECATED
#endif

// disable thread local on msvc 2013
#ifndef SPDLOG_NO_TLS
#    if (defined(_MSC_VER) && (_MSC_VER < 1900)) || defined(__cplusplus_winrt)
#        define SPDLOG_NO_TLS 1
#    endif
#endif

#ifndef SPDLOG_FUNCTION
#    define SPDLOG_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

#ifdef SPDLOG_NO_EXCEPTIONS
#    define SPDLOG_TRY
#    define SPDLOG_THROW(ex)                                                                                                               \
        do                                                                                                                                 \
        {                                                                                                                                  \
            printf("spdlog fatal error: %s\n", ex.what());                                                                                 \
            std::abort();                                                                                                                  \
        } while (0)
#    define SPDLOG_CATCH_STD
#else
#    define SPDLOG_TRY try
#    define SPDLOG_THROW(ex) throw(ex)
#    define SPDLOG_CATCH_STD                                                                                                               \
        catch (const std::exception &) {}
#endif

namespace spdlog {

class formatter;

namespace sinks {
class sink;
}

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = std::wstring;
// allow macro expansion to occur in SPDLOG_FILENAME_T
#    define SPDLOG_FILENAME_T_INNER(s) L##s
#    define SPDLOG_FILENAME_T(s) SPDLOG_FILENAME_T_INNER(s)
#else
using filename_t = std::string;
#    define SPDLOG_FILENAME_T(s) s
#endif

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using err_handler = std::function<void(const std::string &err_msg)>;
using string_view_t = fmt::basic_string_view<char>;
using wstring_view_t = fmt::basic_string_view<wchar_t>;
using memory_buf_t = fmt::basic_memory_buffer<char, 250>;
using wmemory_buf_t = fmt::basic_memory_buffer<wchar_t, 250>;

template<class T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

// clang doesn't like SFINAE disabled constructor in std::is_convertible<> so have to repeat the condition from basic_format_string here,
// in addition, fmt::basic_runtime<Char> is only convertible to basic_format_string<Char> but not basic_string_view<Char>
template<class T, class Char = char>
struct is_convertible_to_basic_format_string
    : std::integral_constant<bool,
          std::is_convertible<T, fmt::basic_string_view<Char>>::value || std::is_same<remove_cvref_t<T>, fmt::basic_runtime<Char>>::value>
{};

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#    ifndef _WIN32
#        error SPDLOG_WCHAR_TO_UTF8_SUPPORT only supported on windows
#    endif // _WIN32
#endif     // SPDLOG_WCHAR_TO_UTF8_SUPPORT

template<class T>
struct is_convertible_to_any_format_string : std::integral_constant<bool, is_convertible_to_basic_format_string<T, char>::value ||
                                                                              is_convertible_to_basic_format_string<T, wchar_t>::value>
{};

#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
#endif

#define SPDLOG_LEVEL_TRACE 0
#define SPDLOG_LEVEL_DEBUG 1
#define SPDLOG_LEVEL_INFO 2
#define SPDLOG_LEVEL_WARN 3
#define SPDLOG_LEVEL_ERROR 4
#define SPDLOG_LEVEL_CRITICAL 5
#define SPDLOG_LEVEL_OFF 6

#if !defined(SPDLOG_ACTIVE_LEVEL)
#    define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

// Log level enum
namespace level {
enum level_enum
{
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels
};

#define SPDLOG_LEVEL_NAME_TRACE spdlog::string_view_t("TRACE", 5)
#define SPDLOG_LEVEL_NAME_DEBUG spdlog::string_view_t("DEBUG", 5)
#define SPDLOG_LEVEL_NAME_INFO spdlog::string_view_t("INFO", 4)
#define SPDLOG_LEVEL_NAME_WARNING spdlog::string_view_t("WARN", 7)
#define SPDLOG_LEVEL_NAME_ERROR spdlog::string_view_t("ERROR", 5)
#define SPDLOG_LEVEL_NAME_CRITICAL spdlog::string_view_t("FATAL", 8)
#define SPDLOG_LEVEL_NAME_OFF spdlog::string_view_t("OFF", 3)

#if !defined(SPDLOG_LEVEL_NAMES)
#    define SPDLOG_LEVEL_NAMES                                                                                                             \
        {                                                                                                                                  \
            SPDLOG_LEVEL_NAME_TRACE, SPDLOG_LEVEL_NAME_DEBUG, SPDLOG_LEVEL_NAME_INFO, SPDLOG_LEVEL_NAME_WARNING, SPDLOG_LEVEL_NAME_ERROR,  \
                SPDLOG_LEVEL_NAME_CRITICAL, SPDLOG_LEVEL_NAME_OFF                                                                          \
        }
#endif

#if !defined(SPDLOG_SHORT_LEVEL_NAMES)

#    define SPDLOG_SHORT_LEVEL_NAMES                                                                                                       \
        {                                                                                                                                  \
            "T", "D", "I", "W", "E", "C", "O"                                                                                              \
        }
#endif

SPDLOG_API const string_view_t &to_string_view(spdlog::level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_API const char *to_short_c_str(spdlog::level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_API spdlog::level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT;

} // namespace level

//
// Color mode used by sinks with color support.
//
enum class color_mode
{
    always,
    automatic,
    never
};

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum class pattern_time_type
{
    local, // log localtime
    utc    // log utc
};

//
// Log exception
//
class SPDLOG_API spdlog_ex : public std::exception
{
public:
    explicit spdlog_ex(std::string msg);
    spdlog_ex(const std::string &msg, int last_errno);
    const char *what() const SPDLOG_NOEXCEPT override;

private:
    std::string msg_;
};

[[noreturn]] SPDLOG_API void throw_spdlog_ex(const std::string &msg, int last_errno);
[[noreturn]] SPDLOG_API void throw_spdlog_ex(std::string msg);

struct source_loc
{
    SPDLOG_CONSTEXPR source_loc() = default;
    SPDLOG_CONSTEXPR source_loc(const char *filename_in, int line_in, const char *funcname_in)
        : filename{filename_in}
        , line{line_in}
        , funcname{funcname_in}
    {}

    SPDLOG_CONSTEXPR bool empty() const SPDLOG_NOEXCEPT
    {
        return line == 0;
    }
    const char *filename{nullptr};
    int line{0};
    const char *funcname{nullptr};
};

namespace details {
// make_unique support for pre c++14

#if __cplusplus >= 201402L // C++14 and beyond
using std::make_unique;
#else
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    static_assert(!std::is_array<T>::value, "arrays not supported");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "common-inl.h"
#endif
