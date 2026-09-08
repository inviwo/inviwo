#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import pathlib
import unittest

SCRIPT = pathlib.Path(__file__).with_name("collect-vcpkg-deps-from-depend-info.py")
SPEC = importlib.util.spec_from_file_location("collect_vcpkg_deps_from_depend_info", SCRIPT)
assert SPEC is not None
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class CollectVcpkgDepsFromDependInfoTest(unittest.TestCase):
    def test_extracts_sorted_unique_bare_port_names(self) -> None:
        output = """
vcpkg-cmake:
vcpkg-cmake-config:host:
zlib: vcpkg-cmake:host
qtbase[core,gui,widgets]: freetype, libjpeg-turbo, libpng, zlib
sqlite3[json1]:host: vcpkg-cmake:host, vcpkg-cmake-config:host
zlib: vcpkg-cmake:host
"""

        self.assertEqual(
            MODULE.parse_dependency_names(output),
            ["qtbase", "sqlite3", "vcpkg-cmake", "vcpkg-cmake-config", "zlib"],
        )

    def test_ignores_non_package_log_lines(self) -> None:
        output = """
Computing installation plan...
Detecting compiler hash for triplet x64-linux...
Elapsed time to handle fmt:x64-linux: 1 ms
fmt: vcpkg-cmake, vcpkg-cmake-config
"""

        self.assertEqual(MODULE.parse_dependency_names(output), ["fmt"])


if __name__ == "__main__":
    unittest.main()
