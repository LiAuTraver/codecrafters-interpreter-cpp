// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception



// The subset of "C headers" [tab:c.headers] corresponding to
// the "C++ headers for C library facilities" [tab:headers.cpp.c]
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>

#ifdef _WIN32
// <intrin.h> defines some types outside of `extern "C"` or `extern "C++"`.
#include <intrin.h>
#endif // _WIN32


#pragma warning(push)
#pragma warning(disable : 5244) // '#include <meow>' in the purview of module 'std' appears erroneous.

// "C++ library headers" [tab:headers.cpp]
#include <algorithm>
#if _HAS_STATIC_RTTI || !defined(_WIN32)
#include <any>
#endif // _HAS_STATIC_RTTI
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <execution>
#if __cplusplus >= 202203L
#include <expected>
#endif // __cplusplus >= 202203L
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#if __cplusplus >= 202203L
#if __has_include(<mdspan>)
#include <mdspan>
/// @note ^^^^^^ GNU still doesn't support this header
#endif // __has_include(<mdspan>)
#endif // __cplusplus >= 202203L
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#if __cplusplus >= 202203L
#include <print>
#endif // __cplusplus >= 202203L
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#if __cplusplus >= 202203L
#include <spanstream>
#endif // __cplusplus >= 202203L
#include <sstream>
#include <stack>
#if __cplusplus >= 202203L
#include <stacktrace>
#endif // __cplusplus >= 202203L
#include <stdexcept>
#if __cplusplus >= 202203L
#include <stdfloat>
#endif // __cplusplus >= 202203L
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

// "C++ headers for C library facilities" [tab:headers.cpp.c]
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

#pragma warning(pop)
