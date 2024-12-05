"""
This file contains helper functions for creating tests for the LOXO library.
"""

load("@rules_cc//cc:defs.bzl", "cc_test")

def loxo_add_test(name, src):
    cc_test(
        name = name,
        srcs = [
            src,
            "test_env.hpp",
            "//shared:loxo_driver.cpp",
            "//shared:execution_context.hpp",
        ],
        copts = [
            "/Iexternal/gtest/googletest/include",
            "/Iexternal/gtest/googletest",
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
