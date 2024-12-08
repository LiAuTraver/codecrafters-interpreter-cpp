"""
This file contains helper functions for creating tests for the LOXO library.
"""

load("@rules_cc//cc:defs.bzl", "cc_test")

def loxo_add_test(name, src):
    cc_test(
        name = name,
        srcs = [
            src,
            "//shared:loxo_driver.cpp",
            "//shared:execution_context.hpp",
            "//shared:test_env.hpp"
        ],
        copts = [
            "/Iexternal/gtest/googletest/include",
            "/Iexternal/gtest/googletest",
            "/Ishared",
            "/Ishared/include",
            "/std:c++latest",
        ],
        defines = [
            "AC_CPP_DEBUG",
            "LIBLOXO_SHARED",
        ],
        includes = ["driver/include"],
        deps = [
            "//driver",
            "@googletest//:gtest_main",
        ],
    )
