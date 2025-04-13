"""
This file contains helper functions for creating tests for the lox project.
"""

load("@rules_cc//cc:defs.bzl", "cc_test")

def lox_add_test(name, src):
    cc_test(
        name = name,
        srcs = [
            "//shared:lox_driver.cpp",
            "//shared:execution_context.hpp",
            "//shared:test_env.hpp",
        ] + src,
        copts = [
            "/Iexternal/gtest/googletest/include",
            "/Iexternal/gtest/googletest",
            "/Ishared",
            "/Ishared/include",
            "/std:c++latest",
            "/Zc:preprocessor",
            "/Zc:__cplusplus",
        ],
        defines = [
            "AC_CPP_DEBUG",
            "LIBLOX_SHARED",
            "AC_SILENCE_CPP_STANDARD_CHECK",
        ],
        includes = ["driver/include"],
        deps = [
            "//driver",
            "@googletest//:gtest_main",
        ],
    )
