#pragma once
#include "variadic.h"

#if defined(AC_CPP_DEBUG)
#define LOXOGRAPH_DEBUG_ENABLED
#endif

#ifdef LOXOGRAPH_DEBUG_ENABLED
#define LOXOGRAPH_NODISCARD_MSG(_name_)                                        \
  [[nodiscard("discarding a return value of `" #_name_                         \
              "` is strongly discouraged")]]
#include <spdlog/spdlog.h>
#define LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_, ...)                           \
  spdlog::_level_(_msg_, __VA_ARGS__);
#define LOXOGRAPH_DEBUG_LOGGING_SETUP(_level_, _msg_, ...)                     \
  spdlog::set_level(spdlog::level::debug);                                     \
  spdlog::set_pattern("[%^%l%$] %v");                                          \
  LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_)
#else
#define LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_, ...)
#endif
#ifdef __RESHARPER__
/// @note resharper can't get through this @code __PRETTY_FUNCTION__ @endcode
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#ifdef __clang__
#define LOXOGRAPH_FORCEINLINE [[clang::always_inline]]
#define LOXOGRAPH_DEBUG_BREAK __builtin_debugtrap();
#define LOXOGRAPH_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(__GNUC__)
#define LOXOGRAPH_FORCEINLINE [[gnu::always_inline]]
#define LOXOGRAPH_DEBUG_BREAK __builtin_trap();
#define LOXOGRAPH_DEBUG_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define LOXOGRAPH_FORCEINLINE [[msvc::forceinline]]
#define LOXOGRAPH_DEBUG_BREAK __debugbreak();
#define LOXOGRAPH_DEBUG_FUNCTION_NAME __FUNCSIG__
#else
#include <csignal>
#define LOXOGRAPH_FORCEINLINE inline
#define LOXOGRAPH_DEBUG_BREAK raise(SIGTRAP);
#define LOXOGRAPH_DEBUG_FUNCTION_NAME __func__
#endif
#ifdef LOXOGRAPH_DEBUG_ENABLED
#define LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                       \
  switch (0)                                                                   \
  case 0:                                                                      \
  default:
#if __has_include(<source_location>)
#include <source_location>
#define LOXOGRAPH_FILENAME (std::source_location::current().file_name())
#define LOXOGRAPH_FUNCTION_NAME LOXOGRAPH_DEBUG_FUNCTION_NAME
#define LOXOGRAPH_LINE (std::source_location::current().line())
#define LOXOGRAPH_COLUMN (std::source_location::current().column())
#endif
#if __has_include(<stacktrace>)
#include <format>
#include <stacktrace>
#define LOXOGRAPH_STACKTRACE std::format("\n{}", std::stacktrace::current())
#else
#define LOXOGRAPH_STACKTRACE ("<no further information>")
#endif
#define LOXOGRAPH_RUNTIME_DEBUG_RAISE LOXOGRAPH_DEBUG_BREAK
#define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_SINGLE(x)                               \
  spdlog::critical("in file {0}, line {2} column {3},\n"                       \
                   "           function {1},\n"                                \
                   "           Constraints not satisfied:\n"                   \
                   "           Expect `{4}` to be true.\n"                     \
                   "Stacktrace:{5}",                                           \
                   LOXOGRAPH_FILENAME, LOXOGRAPH_FUNCTION_NAME,                \
                   LOXOGRAPH_LINE, LOXOGRAPH_COLUMN, #x,                       \
                   LOXOGRAPH_STACKTRACE);
#define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_BINARY(x, y)                            \
  spdlog::critical("in file {0}, line {2} column {3},\n"                       \
                   "           function {1},\n"                                \
                   "           Constraints not satisfied:\n"                   \
                   "           Expect `{4}` equals to `{5}`,\n"                \
                   "             but actually `{4}` appears to be {6},\n"      \
                   "             and `{5}` appears to be {7}."                 \
                   "Stacktrace:\n{8}",                                         \
                   LOXOGRAPH_FILENAME, LOXOGRAPH_FUNCTION_NAME,                \
                   LOXOGRAPH_LINE, LOXOGRAPH_COLUMN, #x, #y, x, y,             \
                   LOXOGRAPH_STACKTRACE);
#define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_1(x)                                    \
  LOXOGRAPH_PRINT_ERROR_MSG_IMPL_SINGLE(x)
#define LOXOGRAPH_PRINT_ERROR_MSG_IMPL_2(x, y)                                 \
  LOXOGRAPH_PRINT_ERROR_MSG_IMPL_BINARY(x, y)
// flush the stderr to make sure the error message to be shown before SIGTRAP
// was raised.
#define LOXOGRAPH_PRINT_ERROR_MSG(...)                                         \
  do {                                                                         \
    LOXOGRAPH_PRINT_ERROR_MSG_IMPL(__VA_ARGS__, 2, 1)(__VA_ARGS__);            \
    LOXOGRAPH_RUNTIME_DEBUG_RAISE                                              \
  } while (false);
#define LOXOGRAPH_PRINT_ERROR_MSG_IMPL(_1, _2, N, ...)                         \
  LOXOGRAPH_PRINT_ERROR_MSG_IMPL_##N
#define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)                             \
  LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                             \
  if ((x) == (y))                                                              \
    ;                                                                          \
  else {                                                                       \
    LOXOGRAPH_PRINT_ERROR_MSG(x, y)                                            \
  }
#define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_SATISFY(x)                              \
  LOXOGRAPH_AMBIGUOUS_ELSE_BLOCKER                                             \
  if (x)                                                                       \
    ;                                                                          \
  else {                                                                       \
    LOXOGRAPH_PRINT_ERROR_MSG(x)                                               \
  }
#ifdef LOXOGRAPH_USE_BOOST_CONTRACT
#include <boost/contract.hpp>
#define LOXOGRAPH_PRECONDITION_IMPL_1(x)                                       \
  boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                             \
      waver_boost_check_precondition_should_be_true) =                         \
      boost::contract::function().precondition(                                \
          [&]() -> bool { return (!!(x)); });
#define LOXOGRAPH_PRECONDITION_IMPL_2(x, y)                                    \
  boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                             \
      waver_boost_check_precondition_should_equal) =                           \
      boost::contract::function().precondition(                                \
          [&]() -> bool { return ((x) == (y)); });
#define LOXOGRAPH_POSTCONDITION_IMPL_1(x)                                      \
  boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                             \
      waver_boost_check_postcondition_should_be_true) =                        \
      boost::contract::function().postcondition(                               \
          [&]() -> bool { return (!!(x)); });
#define LOXOGRAPH_POSTCONDITION_IMPL_2(x, y)                                   \
  boost::contract::check LOXOGRAPH_EXPAND_COUNTER(                             \
      waver_boost_check_postcondition_should_equal) =                          \
      boost::contract::function().postcondition(                               \
          [&]() -> bool { return ((x) == (y)); });
#endif

#define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_2(x, y)                                 \
  LOXOGRAPH_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)
#define LOXOGRAPH_RUNTIME_REQUIRE_IMPL_1(x)                                    \
  LOXOGRAPH_RUNTIME_REQUIRE_IMPL_SATISFY(x)
#define LOXOGRAPH_RUNTIME_REQUIRE_IMPL(...)                                    \
  LOXOGRAPH__VFUNC(LOXOGRAPH_RUNTIME_REQUIRE_IMPL, __VA_ARGS__)

#ifdef LOXOGRAPH_USE_BOOST_CONTRACT
#define LOXOGRAPH_RUNTIME_ASSERT(...)                                          \
  LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#define LOXOGRAPH_PRECONDITION(...)                                            \
  LOXOGRAPH__VFUNC(LOXOGRAPH_PRECONDITION_IMPL, __VA_ARGS__)
#define LOXOGRAPH_POSTCONDITION(...)                                           \
  LOXOGRAPH__VFUNC(LOXOGRAPH_POSTCONDITION_IMPL, __VA_ARGS__)
#else
#define LOXOGRAPH_RUNTIME_ASSERT(...)                                          \
  LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#define LOXOGRAPH_PRECONDITION(...) LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#define LOXOGRAPH_POSTCONDITION(...) LOXOGRAPH_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#endif
#define LOXOGRAPH_NOEXCEPT_IF(...) // nothing(debug mode)
#define LOXOGRAPH_NOEXCEPT         // nothing(debug mode)
#else
// if debug was turned off, do nothing.
#define LOXOGRAPH_RUNTIME_ASSERT(...)
#define LOXOGRAPH_PRECONDITION(...)
#define LOXOGRAPH_POSTCONDITION(...)
#define LOXOGRAPH_NODISCARD_MSG(...)
#define LOXOGRAPH_DEBUG_LOGGING_SETUP(...)
#define LOXOGRAPH_NOEXCEPT_IF(...) noexcept(__VA_ARGS__)
#define LOXOGRAPH_NOEXCEPT noexcept
#endif
#include <iostream>
#define LOXOGRAPH_INITIALIZATION                                               \
  [[maybe_unused]] static inline const auto LOXOGRAPH_INITIALIZATION =         \
      []() -> int {                                                            \
    std::cout << std::unitbuf;                                                 \
    std::cerr << std::unitbuf;                                                 \
    LOXOGRAPH_DEBUG_LOGGING_SETUP(debug, "Debug mode enabled.");               \
    return 0;                                                                  \
  }();

// export macros
#define contract_assert(...) LOXOGRAPH_RUNTIME_ASSERT(__VA_ARGS__)
#define precondition(...) LOXOGRAPH_PRECONDITION(__VA_ARGS__)
#define postcondition(...) LOXOGRAPH_POSTCONDITION(__VA_ARGS__)
/// @note MSVC can't get through this:
///        error C2563: mismatch in formal parameter list
///       thus can't just simply write @code #define dbg(...)
///       LOXOGRAPH_DEBUG_LOGGING(__VA_ARGS__) @endcode
#define dbg(_level_, _msg_, ...)                                               \
  LOXOGRAPH_DEBUG_LOGGING(_level_, _msg_, __VA_ARGS__)
#define streq(...) LOXOGRAPH_TO_STRING(__VA_ARGS__)
#define nodiscard_msg(...) LOXOGRAPH_NODISCARD_MSG(__VA_ARGS__)
#define NOEXCEPT_IF(...) LOXOGRAPH_NOEXCEPT_IF(__VA_ARGS__)
#define NOEXCEPT LOXOGRAPH_NOEXCEPT
