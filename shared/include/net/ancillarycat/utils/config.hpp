#pragma once
#include <version>
#include <filesystem>
#include "details/variadic.h"

/// @note I use Source Code Annotation sometimes; but libstdc++ doesn't support
#if __has_include(<sal.h>)
#  include <sal.h>
#else
#  define _In_
#  define _Inout_
#  define _Out_
#  define _Outptr_
#  define _Outptr_result_maybenull_
#endif

#if defined(AC_CPP_DEBUG)
/// @def AC_UTILS_DEBUG_ENABLED
/// @note only defined in debug mode; never define it when submitting the code
/// to codecrafters or it'll mess up the test.
/// #AC_CPP_DEBUG was defined in the CMakeLists.txt, which can be turned on by
/// set the environment variable `AC_CPP_DEBUG` to `ON`.
/// @attention debug mode needs external libraries to work properly, namely,
/// `fmt`, `spdlog`, and potentially `gtest` and `Google Benchmark`.
/// release mode carries no dependencies and only requires C++23.
#  define AC_UTILS_DEBUG_ENABLED
/// @note use fmt::print, fmt::println when compiling with clang-cl.exe will
/// cause some wired error: Critical error detected c0000374
/// A breakpoint instruction (__debugbreak() statement or a similar call) was
/// executed, which related to heap corruption. The program will terminate.
/// @attention now it's ok again, why? maybe I called vcvarsall.bat?
// #if !(defined(__clang__) && defined(_MSC_VER))
#  define AC_UTILS_USE_FMT_FORMAT
#endif
/// @note GNU on Windows seems failed to perform linking for `stacktrace` and
/// `spdlog`.
#if __has_include(<stacktrace>) && defined(AC_UTILS_DEBUG_ENABLED) && defined(_WIN32)
#  include <stacktrace>
#  if __has_include(<fmt/format.h>)
/// how can fmt cannot format std::filesystem::path? let's fix that
#    include <fmt/format.h>
#    include <filesystem>
template <>
struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(const std::filesystem::path &p, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    return fmt::formatter<std::string>::format(p.string(), ctx);
  }
};
#    define AC_UTILS_STACKTRACE                                                \
      (::std::format("\n{}", ::std::stacktrace::current()))
#  else
template <>
struct ::fmt::formatter<::std::stacktrace> : ::fmt::formatter<::std::string> {
  template <typename FormatContext>
  auto format(const ::std::stacktrace &st, FormatContext &ctx)
      -> decltype(ctx.out()) {
    std::string result;
    for (const auto &entry : st) {
      result += fmt::format("{} {} {} {}\n",
                            entry.description(),
                            entry.native_handle(),
                            entry.source_file(),
                            entry.source_line());
    }
    return ::fmt::format_to(ctx, "{}", result);
  }
};
#    define AC_UTILS_STACKTRACE                                                \
      ::fmt::format("\n{}", ::std::stacktrace::current())
#  endif
#else
#  define AC_UTILS_STACKTRACE ("<no further information>")
#endif
#ifdef AC_UTILS_DEBUG_ENABLED
#  define AC_UTILS_NODISCARD_MSG(_name_)                                       \
    [[nodiscard("discarding a return value of `" #_name_                       \
                "` is strongly discouraged")]]
#  include <spdlog/spdlog.h>
#  define AC_UTILS_DEBUG_LOGGING(_level_, _msg_, ...)                          \
    ::spdlog::_level_(_msg_, ##__VA_ARGS__);
#  define AC_UTILS_DEBUG_BLOCK(...)                                            \
    const auto AC_UTILS_EXPAND_COUNTER(_accat_utils_debug_block_at) =          \
        [&]() -> std::nullptr_t {                                              \
      __VA_ARGS__                                                              \
      return nullptr;                                                          \
    }();
#  define AC_UTILS_DEBUG_ONLY(...) __VA_ARGS__
#  ifdef GTEST_API_
// set the pattern with prefix `loxo` in yellow color.
#    define AC_UTILS_DEBUG_LOGGING_SETUP(_exec_, _level_, _msg_, ...)          \
      ::spdlog::set_level(spdlog::level::_level_);                             \
      ::spdlog::set_pattern("[\033[33m" #_exec_ ":\033[0m %^%5l%$] %v");       \
      AC_UTILS_DEBUG_LOGGING(_level_,                                          \
                             "\033[33m" _msg_ " with gtest."                   \
                             "\033[0m",                                        \
                             ##__VA_ARGS__)
#  else
#    define AC_UTILS_DEBUG_LOGGING_SETUP(_exec_, _level_, _msg_, ...)          \
      ::spdlog::set_level(spdlog::level::_level_);                             \
      ::spdlog::set_pattern("[%^%l%$] %v");                                    \
      AC_UTILS_DEBUG_LOGGING(_level_,                                          \
                             "\033[33m" _msg_ "."                              \
                             "\033[0m",                                        \
                             ##__VA_ARGS__)
#  endif
#else
#  define AC_UTILS_DEBUG_LOGGING(...)
#endif
#if defined(__RESHARPER__) || not defined(__PRETTY_FUNCTION__)
/// @note seems that resharper languege server can't get through
/// this @def __PRETTY_FUNCTION__ macro when compiler was set to clang++.
#  if defined(__FUNCSIG__) // NOLINTNEXTLINE
#    define __PRETTY_FUNCTION__ __FUNCSIG__
#  else // NOLINTNEXTLINE
#    define __PRETTY_FUNCTION__ __func__
#  endif
#endif
#ifdef __clang__
#  define AC_UTILS_FORCEINLINE [[clang::always_inline]]
#  define AC_UTILS_DEBUG_BREAK __builtin_debugtrap();
#  define AC_UTILS_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
// Visual Studio's intellisense cannot recognize `elifdef` yet.
#elif defined(__GNUC__)
#  define AC_UTILS_FORCEINLINE [[gnu::always_inline]]
#  define AC_UTILS_DEBUG_BREAK __builtin_trap();
#  define AC_UTILS_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define AC_UTILS_FORCEINLINE [[msvc::forceinline]]
#  define AC_UTILS_DEBUG_BREAK __debugbreak();
#  define AC_UTILS_DEBUG_FUNCTION_NAME __FUNCSIG__
#  pragma warning(disable : 4716) // must return a value
#  pragma warning(disable : 4530) // /EHsc
#else
#  include <csignal>
#  define AC_UTILS_FORCEINLINE inline
#  define AC_UTILS_DEBUG_BREAK raise(SIGTRAP);
#  define AC_UTILS_DEBUG_FUNCTION_NAME __func__
#endif
#ifdef AC_UTILS_DEBUG_ENABLED
#  include <source_location>
#  define AC_UTILS_AMBIGUOUS_ELSE_BLOCKER                                      \
    switch (0)                                                                 \
    case 0:                                                                    \
    default:
#  define AC_UTILS_FILENAME (::std::source_location::current().file_name())
#  define AC_UTILS_FUNCTION_NAME AC_UTILS_DEBUG_FUNCTION_NAME
#  define AC_UTILS_LINE (::std::source_location::current().line())
#  define AC_UTILS_COLUMN (::std::source_location::current().column())
#  define AC_UTILS_RUNTIME_DEBUG_RAISE AC_UTILS_DEBUG_BREAK
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_SINGLE(x)                              \
    spdlog::critical("in file {0}, line {2} column {3},\n"                     \
                     "           function {1},\n"                              \
                     "           Constraints not satisfied:\n"                 \
                     "           Expect `{4}` to be true.\n"                   \
                     "Stacktrace:{5}",                                         \
                     AC_UTILS_FILENAME,                                        \
                     AC_UTILS_FUNCTION_NAME,                                   \
                     AC_UTILS_LINE,                                            \
                     AC_UTILS_COLUMN,                                          \
                     #x,                                                       \
                     AC_UTILS_STACKTRACE);
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_BINARY(x, y)                           \
    spdlog::critical("in file {0}, line {2} column {3},\n"                     \
                     "           function {1},\n"                              \
                     "           Constraints not satisfied:\n"                 \
                     "           Expect `{4}` equals to `{5}`,\n"              \
                     "             but actually `{4}` appears to be {6},\n"    \
                     "             and `{5}` appears to be {7}.\n"             \
                     "Stacktrace:{8}",                                         \
                     AC_UTILS_FILENAME,                                        \
                     AC_UTILS_FUNCTION_NAME,                                   \
                     AC_UTILS_LINE,                                            \
                     AC_UTILS_COLUMN,                                          \
                     #x,                                                       \
                     #y,                                                       \
                     x,                                                        \
                     y,                                                        \
                     AC_UTILS_STACKTRACE);
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_WITH_MSG(x, y, _msg_)                  \
    spdlog::critical("in file {0}, line {2} column {3},\n"                     \
                     "           function {1},\n"                              \
                     "           Constraints not satisfied:\n"                 \
                     "           Expect `{4}` equals to `{5}`,\n"              \
                     "             but actually `{4}` appears to be {6},\n"    \
                     "             and `{5}` appears to be {7}.\n"             \
                     "Additional message: {8}\n"                               \
                     "Stacktrace:{9}",                                         \
                     AC_UTILS_FILENAME,                                        \
                     AC_UTILS_FUNCTION_NAME,                                   \
                     AC_UTILS_LINE,                                            \
                     AC_UTILS_COLUMN,                                          \
                     #x,                                                       \
                     #y,                                                       \
                     x,                                                        \
                     y,                                                        \
                     _msg_,                                                    \
                     AC_UTILS_STACKTRACE);
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_1(x)                                   \
    AC_UTILS_PRINT_ERROR_MSG_IMPL_SINGLE(x)
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_2(x, y)                                \
    AC_UTILS_PRINT_ERROR_MSG_IMPL_BINARY(x, y)
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL_3(x, y, _msg_)                         \
    AC_UTILS_PRINT_ERROR_MSG_IMPL_WITH_MSG(x, y, _msg_)
#  define AC_UTILS_PRINT_ERROR_MSG(...)                                        \
    do {                                                                       \
      AC_UTILS_PRINT_ERROR_MSG_IMPL(__VA_ARGS__, 3, 2, 1)(__VA_ARGS__);        \
      AC_UTILS_RUNTIME_DEBUG_RAISE                                             \
    } while (false);
#  define AC_UTILS_PRINT_ERROR_MSG_IMPL(_1, _2, _3, N, ...)                    \
    AC_UTILS_PRINT_ERROR_MSG_IMPL_##N
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)                            \
    AC_UTILS_AMBIGUOUS_ELSE_BLOCKER                                            \
    if ((x) == (y))                                                            \
      ;                                                                        \
    else {                                                                     \
      AC_UTILS_PRINT_ERROR_MSG(x, y)                                           \
    }
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_SATISFY(x)                             \
    AC_UTILS_AMBIGUOUS_ELSE_BLOCKER                                            \
    if ((x))                                                                   \
      ;                                                                        \
    else {                                                                     \
      AC_UTILS_PRINT_ERROR_MSG(x)                                              \
    } //! <- do NOT add entraneous semicolon here, it'll confuse the
      //! preprocessor sometimes.
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_WITH_MSG(x, y, _msg_)                  \
    AC_UTILS_AMBIGUOUS_ELSE_BLOCKER                                            \
    if ((x))                                                                   \
      ;                                                                        \
    else {                                                                     \
      AC_UTILS_PRINT_ERROR_MSG(x, y, _msg_)                                    \
    }
#  ifdef AC_UTILS_USE_BOOST_CONTRACT
#    include <boost/contract.hpp>
#    define AC_UTILS_PRECONDITION_IMPL_1(x)                                    \
      boost::contract::check AC_UTILS_EXPAND_COUNTER(                          \
          accat_utils_boost_check_precondition_should_be_true) =               \
          boost::contract::function().precondition(                            \
              [&]() -> bool { return (!!(x)); });
#    define AC_UTILS_PRECONDITION_IMPL_2(x, y)                                 \
      boost::contract::check AC_UTILS_EXPAND_COUNTER(                          \
          accat_utils_boost_check_precondition_should_equal) =                 \
          boost::contract::function().precondition(                            \
              [&]() -> bool { return ((x) == (y)); });
#    define AC_UTILS_POSTCONDITION_IMPL_1(x)                                   \
      boost::contract::check AC_UTILS_EXPAND_COUNTER(                          \
          accat_utils_boost_check_postcondition_should_be_true) =              \
          boost::contract::function().postcondition(                           \
              [&]() -> bool { return (!!(x)); });
#    define AC_UTILS_POSTCONDITION_IMPL_2(x, y)                                \
      boost::contract::check AC_UTILS_EXPAND_COUNTER(                          \
          accat_utils_boost_check_postcondition_should_equal) =                \
          boost::contract::function().postcondition(                           \
              [&]() -> bool { return ((x) == (y)); });
#  endif

#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_3(x, y, _msg_)                         \
    AC_UTILS_RUNTIME_REQUIRE_IMPL_WITH_MSG(x, y, _msg_)
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_2(x, y)                                \
    AC_UTILS_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL_1(x)                                   \
    AC_UTILS_RUNTIME_REQUIRE_IMPL_SATISFY(x)
#  define AC_UTILS_RUNTIME_REQUIRE_IMPL(...)                                   \
    AC_UTILS_VFUNC(AC_UTILS_RUNTIME_REQUIRE_IMPL, __VA_ARGS__)

#  ifdef AC_UTILS_USE_BOOST_CONTRACT
#    define AC_UTILS_RUNTIME_ASSERT(...)                                       \
      AC_UTILS_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#    define AC_UTILS_PRECONDITION(...)                                         \
      AC_UTILS_VFUNC(AC_UTILS_PRECONDITION_IMPL, __VA_ARGS__)
#    define AC_UTILS_POSTCONDITION(...)                                        \
      AC_UTILS_VFUNC(AC_UTILS_POSTCONDITION_IMPL, __VA_ARGS__)
#  else
#    define AC_UTILS_RUNTIME_ASSERT(_arg1_, ...)                               \
      AC_UTILS_RUNTIME_REQUIRE_IMPL(_arg1_, ##__VA_ARGS__);
#    define AC_UTILS_PRECONDITION(...)                                         \
      AC_UTILS_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#    define AC_UTILS_POSTCONDITION(...)                                        \
      AC_UTILS_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#  endif
#  define AC_UTILS_NOEXCEPT_IF(...) // nothing
#  define AC_UTILS_NOEXCEPT         // nothing
#else
// if debug was turned off, do nothing.
#  define AC_UTILS_RUNTIME_ASSERT(...)
#  define AC_UTILS_PRECONDITION(...)
#  define AC_UTILS_POSTCONDITION(...)
#  define AC_UTILS_NODISCARD_MSG(...)
#  define AC_UTILS_DEBUG_LOGGING_SETUP(...)
#  define AC_UTILS_DEBUG_BLOCK(...)
#  define AC_UTILS_DEBUG_ONLY(...)
#  define AC_UTILS_NOEXCEPT_IF(...) noexcept(__VA_ARGS__)
#  define AC_UTILS_NOEXCEPT noexcept
#  define AC_UTILS_DBG_BREAK
#endif
/// @def AC_SPDLOG_INITIALIZATION(_exec_, _log_level_) initializes the spdlog
/// framework
/// @note only call it once in the whole exec; never call it twice.
#define AC_SPDLOG_INITIALIZATION(_exec_, _log_level_)                          \
  [[maybe_unused]] /* AC_UTILS_API */                                          \
  static           /* <- msvc can't get through this.*/                        \
      const auto AC_SPDLOG_INITIALIZATION =                                    \
          [](void) /* static constexpr <- msvc can't get through this.*/       \
      -> ::std::nullptr_t {                                                    \
    ::std::cout << ::std::unitbuf;                                             \
    ::std::cerr << ::std::unitbuf;                                             \
    ::std::cout << ::std::flush;                                               \
    ::std::cerr << ::std::flush;                                               \
    AC_UTILS_DEBUG_LOGGING(info,                                               \
                           "\033[36mspdlog framework initialized.\033[0m");    \
    AC_UTILS_DEBUG_LOGGING_SETUP(_exec_, _log_level_, "Debug mode enabled");   \
    return nullptr;                                                            \
  }();

#if (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL) && !defined(__clang__)
/// @brief MSVC traditional preprocessor
/// @def dbg(_level_, _msg_, ...)
/// @note MSVC can't get through this:
///        error C2563: mismatch in formal parameter list
///       thus can't just simply write #dbg(...) to replace this macro.
/// @see
/// https://learn.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview
#  pragma message(                                                             \
      "MSVC traditional preprocessor was used. no additional debug info will be provided. to enable MSVC's new preprocessor, add compiler flag `/Zc:preprocessor`.")
#  define dbg(_level_, _msg_, ...)                                             \
    AC_UTILS_DEBUG_LOGGING(_level_, _msg_, ##__VA_ARGS__)
#  define contract_assert(...)
#  define precondition(...)
#  define postcondition(...)
#else
/// @note MSVC cross-platform compatible preprocessor acts like clang and gcc.
#  define contract_assert(...) AC_UTILS_RUNTIME_ASSERT(__VA_ARGS__)
#  define precondition(...) AC_UTILS_PRECONDITION(__VA_ARGS__)
#  define postcondition(...) AC_UTILS_POSTCONDITION(__VA_ARGS__)
#  define dbg(...) AC_UTILS_DEBUG_LOGGING(__VA_ARGS__)
#endif
#define dbg_block(...) AC_UTILS_DEBUG_BLOCK(__VA_ARGS__)
#define dbg_only(...) AC_UTILS_DEBUG_ONLY(__VA_ARGS__)
#define dbg_break AC_UTILS_DEBUG_BREAK

// if exception was disabled, do nothing.
#if defined(__cpp_exceptions) && __cpp_exceptions
#  include <stdexcept>
#  define AC_UTILS_TODO_(...)                                                  \
    throw ::std::logic_error(std::format("TODO: " #__VA_ARGS__));
#elif __has_include(<spdlog/spdlog.h>)
#  define AC_UTILS_TODO_(...)                                                  \
    AC_UTILS_DEBUG_LOGGING(critical, "TODO: " #__VA_ARGS__);
#else
#  include <iostream>
#  define AC_UTILS_TODO_(...)                                                  \
    ::std::cerr << std::format("TODO: " #__VA_ARGS__) << ::std::endl;          \
    AC_UTILS_DEBUG_BREAK
#endif
/// @def TODO mimic from kotlin's `TODO` function, which throws an exception and
/// also discoverable by IDE.
#define TODO(...) AC_UTILS_TODO_(__VA_ARGS__)

#if defined(_MSC_VER) && !defined(__clang__)
/// @remark currenty, MSVC's constexpr was really disgusting.
#  define AC_CONSTEXPR20
#  pragma warning(disable : 4244) // conversion from 'int' to 'char' warning
#else
#  define AC_CONSTEXPR20 constexpr
#endif

/// @see
/// https://stackoverflow.com/questions/32432450/what-is-standard-defer-finalizer-implementation-in-c
struct accat_utils_defer_helper_struct_ {};
template <class Fun_> struct accat_utils_deferrer_ { // NOLINT
  Fun_ f_;
  inline constexpr accat_utils_deferrer_(Fun_ f) : f_(f) {}
  inline constexpr ~accat_utils_deferrer_() { f_(); }
};
template <class Fun_>
static inline constexpr auto operator*(accat_utils_defer_helper_struct_,
                                       Fun_ f_) -> accat_utils_deferrer_<Fun_> {
  return {f_};
}
#define AC_DEFER                                                               \
  const auto AC_UTILS_EXPAND_COUNTER(_accat_utils_defer_block_at) =            \
      accat_utils_defer_helper_struct_{} *[&]()

#ifdef defer
#  warning "defer was already defined. please check the code."
#  pragma pop_macro("defer")
#endif
#define defer AC_DEFER

namespace net::ancillarycat::utils {
class Monostate;
/// @brief a concept that checks if the types are variantable for my custom
/// @link Variant @endlink class,
/// where the first type must be @link Monostate @endlink
/// @tparam Types the types to check
template <typename... Types>
concept Variantable = requires {
  std::is_same_v<std::tuple_element_t<0, std::tuple<Types...>>, Monostate>;
};
/// @brief represents a value that can be stored in a
/// @link StatusOr @endlink object
/// @tparam Ty the type of the value
/// @remarks similiar to Microsoft's @link std::_SMF_control @endlink class,
/// which was used in @link std::optional @endlink
template <typename Ty>
concept Storable = true;
// FIXME: not working
// std::conjunction_v<std::is_default_constructible<Ty>,
//                                       std::is_nothrow_destructible<Ty>,
//                                       std::is_nothrow_constructible<Ty>>;

template <Variantable... Types> class Variant;
class Status;
template <Storable Ty> class StatusOr;
class file_reader;
using string = ::std::string;
using string_view = ::std::string_view;
using path = ::std::filesystem::path;
using ifstream = ::std::ifstream;
using ostringstream = ::std::ostringstream;
using namespace ::std::string_view_literals;
using namespace ::std::string_literals;
} // namespace net::ancillarycat::utils
