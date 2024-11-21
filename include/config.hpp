#pragma once
#include "internal/variadic.h"
#ifdef LIBLOXOGRAPH_SHARED
#  ifdef _WIN32
#    ifdef driver_EXPORTS
#      define LOXOGRAPH_API __declspec(dllexport)
#    else
#      define LOXOGRAPH_API __declspec(dllimport)
#    endif
#  else
#    define LOXOGRAPH_API __attribute__((visibility("default")))
#  endif
#  define LOXOGRAPH_INLINE
#else
#  define LOXOGRAPH_API
#  define LOXOGRAPH_INLINE inline
#endif

#if defined(AC_CPP_DEBUG)
/// @def LOXOGRAPH_DEBUG_ENABLED
/// @note only defined in debug mode; never define it when submitting the code
/// to codecrafters or it'll mess up the test.
/// #AC_CPP_DEBUG was defined in the CMakeLists.txt, which can be turned on by
/// set the environment variable `AC_CPP_DEBUG` to `ON`.
/// @attention debug mode needs external libraries to work properly, namely,
/// `fmt`, `spdlog`, and potentially `gtest` and `Google Benchmark`.
/// release mode carries no dependencies and only requires C++20.
#  define LOXOGRAPH_DEBUG_ENABLED
/// @note use fmt::print, fmt::println when compiling with clang-cl.exe will
/// cause some wired error: Critical error detected c0000374
/// A breakpoint instruction (__debugbreak() statement or a similar call) was
/// executed, which related to heap corruption. The program will terminate.
/// @attention now it's ok again, why? maybe I called vcvarsall.bat?
// #if !(defined(__clang__) && defined(_MSC_VER))
#  define LOXOGRAPH_USE_FMT_FORMAT
// #endif
#endif
/// @note GNU on Windows seems failed to perform linking for `stacktrace` and
/// `spdlog`.
#if __has_include(                                                             \
    <stacktrace>) && defined(LOXOGRAPH_DEBUG_ENABLED) && defined(_WIN32)
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
#    define LOXOGRAPH_STACKTRACE                                               \
      ::std::format("\n{}", ::std::stacktrace::current())
#  else
#    include <fmt/format.h>
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
#    define LOXOGRAPH_STACKTRACE                                               \
      ::fmt::format("\n{}", ::std::stacktrace::current())
#  endif
#else
#  define LOXOGRAPH_STACKTRACE ("<no further information>")
#endif
#ifdef LOXOGRAPH_DEBUG_ENABLED
#  define LOXOGRAPH_NODISCARD_MSG(_name_)                                      \
    [[nodiscard("discarding a return value of `" #_name_                       \
                "` is strongly discouraged")]]
#  include <spdlog/spdlog.h>
#  define LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_, ...)                         \
    ::spdlog::_level_(_msg_, ##__VA_ARGS__);
#  ifdef GTEST_API_
// set the pattern with prefix `loxo` in yellow color.
#    define LOXOGRAPH_DEBUG_LOGGING_SETUP(_level_, _msg_, ...)                 \
      ::spdlog::set_level(spdlog::level::_level_);                             \
      ::spdlog::set_pattern("[\033[33mloxo:\033[0m %^%5l%$] %v");              \
      LOXOGRAPH_DEBUG_LOGGING(_level_,                                         \
                              "\033[33m" _msg_ " with gtest."                  \
                              "\033[0m",                                       \
                              ##__VA_ARGS__)
#  else
#    define LOXOGRAPH_DEBUG_LOGGING_SETUP(_level_, _msg_, ...)                 \
      ::spdlog::set_level(spdlog::level::_level_);                             \
      ::spdlog::set_pattern("[%^%l%$] %v");                                    \
      LOXOGRAPH_DEBUG_LOGGING(_level_,                                         \
                              "\033[33m" _msg_ "."                             \
                              "\033[0m",                                       \
                              ##__VA_ARGS__)
#  endif
#else
#  define LOXOGRAPH_DEBUG_LOGGING(...)
#endif
#if defined(__RESHARPER__) || not defined(__PRETTY_FUNCTION__)
/// @note it seems that resharper languege server can't get through this
/// __PRETTY_FUNCTION__ macro when compiler was set to clang++.
// NOLINTNEXTLINE
#  define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#ifdef __clang__
#  define LOXOGRAPH_FORCEINLINE [[clang::always_inline]]
#  define LOXOGRAPH_DEBUG_BREAK __builtin_debugtrap();
#  define LOXOGRAPH_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(__GNUC__)
#  define LOXOGRAPH_FORCEINLINE [[gnu::always_inline]]
#  define LOXOGRAPH_DEBUG_BREAK __builtin_trap();
#  define LOXOGRAPH_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define LOXOGRAPH_FORCEINLINE [[msvc::forceinline]]
#  define LOXOGRAPH_DEBUG_BREAK __debugbreak();
#  define LOXOGRAPH_DEBUG_FUNCTION_NAME __FUNCSIG__
#else
#  include <csignal>
#  define LOXOGRAPH_FORCEINLINE inline
#  define LOXOGRAPH_DEBUG_BREAK raise(SIGTRAP);
#  define LOXOGRAPH_DEBUG_FUNCTION_NAME __func__
#endif
#ifdef LOXOGRAPH_DEBUG_ENABLED
#  define LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                     \
    switch (0)                                                                 \
    case 0:                                                                    \
    default:
/// @note like `stacktrace`, `source_location` is not fully supported
/// sometimes.
#  define LOXOGRAPH_FILENAME (::std::source_location::current().file_name())
#  define LOXOGRAPH_FUNCTION_NAME LOXOGRAPH_DEBUG_FUNCTION_NAME
#  define LOXOGRAPH_LINE (::std::source_location::current().line())
#  define LOXOGRAPH_COLUMN (::std::source_location::current().column())
#  define LOXOGRAPH_RUNTIME_DEBUG_RAISE LOXOGRAPH_DEBUG_BREAK
#  define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_SINGLE(x)                             \
    spdlog::critical("in file {0}, line {2} column {3},\n"                     \
                     "           function {1},\n"                              \
                     "           Constraints not satisfied:\n"                 \
                     "           Expect `{4}` to be true.\n"                   \
                     "Stacktrace:{5}",                                         \
                     LOXOGRAPH_FILENAME,                                       \
                     LOXOGRAPH_FUNCTION_NAME,                                  \
                     LOXOGRAPH_LINE,                                           \
                     LOXOGRAPH_COLUMN,                                         \
                     #x,                                                       \
                     LOXOGRAPH_STACKTRACE);
#  define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_BINARY(x, y)                          \
    spdlog::critical("in file {0}, line {2} column {3},\n"                     \
                     "           function {1},\n"                              \
                     "           Constraints not satisfied:\n"                 \
                     "           Expect `{4}` equals to `{5}`,\n"              \
                     "             but actually `{4}` appears to be {6},\n"    \
                     "             and `{5}` appears to be {7}.\n"             \
                     "Stacktrace:\n{8}",                                       \
                     LOXOGRAPH_FILENAME,                                       \
                     LOXOGRAPH_FUNCTION_NAME,                                  \
                     LOXOGRAPH_LINE,                                           \
                     LOXOGRAPH_COLUMN,                                         \
                     #x,                                                       \
                     #y,                                                       \
                     x,                                                        \
                     y,                                                        \
                     LOXOGRAPH_STACKTRACE);
#  define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_1(x)                                  \
    LOXOGRAPH_PRINT_ERROR_MSG_IMPL_SINGLE(x)
#  define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_2(x, y)                               \
    LOXOGRAPH_PRINT_ERROR_MSG_IMPL_BINARY(x, y)
#  define LOXOGRAPH_PRINT_ERROR_MSG(...)                                       \
    do {                                                                       \
      LOXOGRAPH_PRINT_ERROR_MSG_IMPL(__VA_ARGS__, 2, 1)(__VA_ARGS__);          \
      LOXOGRAPH_RUNTIME_DEBUG_RAISE                                            \
    } while (false);
#  define LOXOGRAPH_PRINT_ERROR_MSG_IMPL(_1, _2, N, ...)                       \
    LOXOGRAPH_PRINT_ERROR_MSG_IMPL_##N
#  define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)                           \
    LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                           \
    if ((x) == (y))                                                            \
      ;                                                                        \
    else {                                                                     \
      LOXOGRAPH_PRINT_ERROR_MSG(x, y)                                          \
    }
#  define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_SATISFY(x)                            \
    LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                           \
    if (x)                                                                     \
      ;                                                                        \
    else {                                                                     \
      LOXOGRAPH_PRINT_ERROR_MSG(x)                                             \
    } //! <- do NOT add entraneous semicolon here, it'll confuse the
      //! preprocessor sometimes.
#  ifdef LOXOGRAPH_USE_BOOST_CONTRACT
#    include <boost/contract.hpp>
#    define LOXOGRAPH_PRECONDITION_IMPL_1(x)                                   \
      boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                         \
          waver_boost_check_precondition_should_be_true) =                     \
          boost::contract::function().precondition(                            \
              [&]() -> bool { return (!!(x)); });
#    define LOXOGRAPH_PRECONDITION_IMPL_2(x, y)                                \
      boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                         \
          waver_boost_check_precondition_should_equal) =                       \
          boost::contract::function().precondition(                            \
              [&]() -> bool { return ((x) == (y)); });
#    define LOXOGRAPH_POSTCONDITION_IMPL_1(x)                                  \
      boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                         \
          waver_boost_check_postcondition_should_be_true) =                    \
          boost::contract::function().postcondition(                           \
              [&]() -> bool { return (!!(x)); });
#    define LOXOGRAPH_POSTCONDITION_IMPL_2(x, y)                               \
      boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                         \
          waver_boost_check_postcondition_should_equal) =                      \
          boost::contract::function().postcondition(                           \
              [&]() -> bool { return ((x) == (y)); });
#  endif

#  define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_2(x, y)                               \
    LOXOGRAPH_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)
#  define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_1(x)                                  \
    LOXOGRAPH_RUNTIME_REQUIRE_IMPL_SATISFY(x)
#  define LOXOGRAPH_RUNTIME_REQUIRE_IMPL(...)                                  \
    LOXOGRAPH__VFUNC(LOXOGRAPH_RUNTIME_REQUIRE_IMPL, __VA_ARGS__)

#  ifdef LOXOGRAPH_USE_BOOST_CONTRACT
#    define LOXOGRAPH_RUNTIME_ASSERT(...)                                      \
      LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#    define LOXOGRAPH_PRECONDITION(...)                                        \
      LOXOGRAPH__VFUNC(LOXOGRAPH_PRECONDITION_IMPL, __VA_ARGS__)
#    define LOXOGRAPH_POSTCONDITION(...)                                       \
      LOXOGRAPH__VFUNC(LOXOGRAPH_POSTCONDITION_IMPL, __VA_ARGS__)
#  else
#    define LOXOGRAPH_RUNTIME_ASSERT(_arg1_, ...)                              \
      LOXOGRAPH_RUNTIME_REQUIRE_IMPL(_arg1_, ##__VA_ARGS__);
#    define LOXOGRAPH_PRECONDITION(...)                                        \
      LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#    define LOXOGRAPH_POSTCONDITION(...)                                       \
      LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#  endif
#  define LOXOGRAPH_NOEXCEPT_IF(...) // nothing
#  define LOXOGRAPH_NOEXCEPT         // nothing
#else
// if debug was turned off, do nothing.
#  define LOXOGRAPH_RUNTIME_ASSERT(...)
#  define LOXOGRAPH_PRECONDITION(...)
#  define LOXOGRAPH_POSTCONDITION(...)
#  define LOXOGRAPH_NODISCARD_MSG(...)
#  define LOXOGRAPH_DEBUG_LOGGING_SETUP(...)
#  define LOXOGRAPH_NOEXCEPT_IF(...) noexcept(__VA_ARGS__)
#  define LOXOGRAPH_NOEXCEPT noexcept
#endif
/// @def LOXOGRAPH_INITIALIZATION(_log_level_) initializes the spdlog framework
/// @note only call it once in the whole exec; never call it twice.
#define LOXOGRAPH_INITIALIZATION(_log_level_)                                  \
  [[maybe_unused]] /* LOXOGRAPH_API */                                         \
  static           /* <- msvc can't get through this.*/                        \
      inline const auto LOXOGRAPH_INITIALIZATION =                             \
          [](void) static constexpr /* <- msvc can't get through this.*/       \
      -> ::std::nullptr_t {                                                    \
    ::std::cout << ::std::unitbuf;                                             \
    ::std::cerr << ::std::unitbuf;                                             \
    LOXOGRAPH_DEBUG_LOGGING(info,                                              \
                            "\033[36mspdlog framework initialized.\033[0m");   \
    LOXOGRAPH_DEBUG_LOGGING_SETUP(_log_level_, "Debug mode enabled");          \
    return nullptr;                                                            \
  }();

/// @note export macros for convenience
#if (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL) && !defined(__clang__)
/// @brief MSVC traditional preprocessor
/// @def dbg(_level_, _msg_, ...)
/// @note MSVC can't get through this:
///        error C2563: mismatch in formal parameter list
///       thus can't just simply write #dbg(...) to replace this macro.
/// @see
/// https://learn.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=msvc-170
#  define dbg(_level_, _msg_, ...)                                             \
    LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_, ##__VA_ARGS__)
#  define contract_assert(...)
#  define precondition(...)
#  define postcondition(...)
#else
/// @note MSVC cross-platform compatible preprocessor or other compilers
///        (namely GCC and clang)
#  define contract_assert(...) LOXOGRAPH_RUNTIME_ASSERT(__VA_ARGS__)
#  define precondition(...) LOXOGRAPH_PRECONDITION(__VA_ARGS__)
#  define postcondition(...) LOXOGRAPH_POSTCONDITION(__VA_ARGS__)
#  define dbg(...) LOXOGRAPH_DEBUG_LOGGING(__VA_ARGS__)
#endif
#define nodiscard_msg(...) LOXOGRAPH_NODISCARD_MSG(__VA_ARGS__)
