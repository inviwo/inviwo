/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/python3/pythoninterpreter.h>  // for PythonInterpreter

#include <pybind11/cast.h>           // for object_api::operator()
#include <pybind11/detail/common.h>  // for pybind11
#include <pybind11/embed.h>          // for finalize_interpreter, initialize_interpreter
#include <pybind11/eval.h>           // for exec
#include <pybind11/pybind11.h>       // for globals, module_, module
#include <pybind11/pytypes.h>        // for error_already_set, dict, item_accessor

#ifdef WIN32
// For conda workaround
#include <cstdlib>
#endif

#include <inviwo/core/util/exception.h>         // for ModuleInitException
#include <inviwo/core/util/filesystem.h>        // for getExecutablePath, getFileDirectory
#include <inviwo/core/util/logcentral.h>        // for LogCentral
#include <inviwo/core/util/sourcecontext.h>     // for SourceContext
#include <inviwo/core/util/stringconversion.h>  // for toString
#include <inviwo/core/util/safecstr.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwocommondefines.h>
#include <modules/python3/pyutils.h>  // for addModulePath
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <array>        // for array
#include <exception>    // for exception
#include <string>       // for operator+, string, basic_string
#include <string_view>  // for string_view
#include <ranges>
#include <fmt/format.h>

using namespace std::string_view_literals;

namespace inviwo {

// when building we assume a layout like:
// <src-dir>
// ├── ...
// ├── modules/                       (inviwo modules)
// │   ├── ...
// │   ├── python3/                   (python module)
// │   │   ├── processors/            (python processor)
// │   │   └── scripts/               (python scripts)
// │   ├── ...
// │   └── ...
// └── ...
//
//
// <build-dirt>
// ├── bin/                                     (binaries)
// │   └── <Config>
// │       ├── inviwo.exe                      (Windows, inviwo executable)
// │       ├── inviwo.app                      (Macos, inviwo executable)
// │       │   └── Contents/
// │       │       └── MacOS/
// │       │           └── inviwo              (inviwo executable)
// │       ├── inviwopy.cpython-3XX-arch.xxx   (inviwo python modules)
// │       ├── ...
// │       ├── inviwo-unittest-python3
// │       └── ...
// ├── ...
// ├── vcpkg_installed/  (Non-windows)
// │   └── <triplet>/
// │       ├── bin/
// │       ├── include/
// │       ├── lib/
// │       │   ├── libpython3.12.dylib
// │       │   └── python3.XX/             (python basedir)
// │       │       ├── lib-dynload/        (python lib-dynload)
// │       │       └── site-packages/      (python site-packages)
// │       ├── share/
// │       └── tools/
// │           └── python3/
// │               └── python3             (python executable)
// │
// ├── vcpkg_installed/  (Windows)
// │   └── <triplet>/
// │       ├── bin/
// │       │   ├── python3.dll
// │       │   └── python3XX.dll
// │       ├── include/
// │       ├── lib/
// │       │   └── python3.lib
// │       ├── share/
// │       └── tools/
// │           └── python3/                (python basedir)
// │               ├── python3             (python executable)
// │               ├── DLLs                (python dlls)
// │               ├── Lib
// │               │   └── site-packages/  (python site-packages)
// │               └── Scripts
// └── ...
//
//
// When deployed on windows we assume the following structure
// inviwo                               This should be returned by filesystem::findBasePath
// ├── bin
// │   ├── ...
// │   ├── inviwo.exe                  (main executable)
// │   ├── inviwopy.cp3XX-win_amd64    (inviwo python modules)
// │   ├── python3.dll
// │   ├── python3XX.dll
// │   └── ...
// ├── cmake
// ├── data
// ├── include
// ├── lib
// ├── licenses
// ├── modules
// │   ├── ...
// │   ├── python3/                   (python module)
// │   │   ├── processors/            (python processor)
// │   │   └── scripts                (python scripts)
// │   └── ...
// ├── share
// └── tools
//     └── python3/                  (python basedir)
//         ├── DLLs                  (python dlls)
//         └── Lib
//             └── site-packages/    (python site-packages)
//
//
// When deployed on MacOS we assume the following structure
// inviwo.app/
// └── Contents/
//     ├── Info.plist
//     ├── PkgInfo
//     ├── _CodeSignature/
//     ├── MacOS/
//     │   └── inviwo                         (main executable)
//     ├── Resources/                         This should be returned by filesystem::findBasePath
//     │   ├── data/                          (application data)
//     │   ├── licenses/                      (license files)
//     │   ├── modules/                       (Inviwo modules)
//     │   │   ├── ...
//     │   │   ├── python3/                   (python module)
//     │   │   │   ├── processors/            (python processor)
//     │   │   │   └── scripts                (python scripts)
//     │   │   └── ...
//     │   └── tools/                         (utility tools)
//     ├── PlugIns/                           (Qt plugins)
//     ├── cmake/                             (CMake configuration files)
//     ├── include/                           (Header files)
//     ├── lib/                               (Dynamic libraries, and python modules)
//     │   ├── inviwopy.cpython-3XX-darwin.so (inviwo python modules)
//     │   └── python3.XX/                    (python basedir)
//     │       ├── lib-dynload/               (python lib-dynload)
//     │       └── site-packages/             (python site-packages)
//     └── share/                             (Shared data)

namespace {
struct PythonPaths {
    std::vector<std::string> paths;
    std::string prefix;
};

std::pair<PythonPaths, PythonPaths> getPythonPaths() {

    if constexpr (build::platform == build::Platform::Windows) {
        const auto install = []() {
            static constexpr std::array templates = {
                "{root}/tools/python{major}"sv, "{root}/tools/python{major}/DLLs"sv,
                "{root}/tools/python{major}/Lib"sv,
                "{root}/tools/python{major}/Lib/site-packages"sv, "{root}/bin"sv};
            const auto root = filesystem::findBasePath().generic_string();

            const auto paths =
                templates | std::views::transform([&](std::string_view path) {
                    return fmt::format(fmt::runtime(path), fmt::arg("root", root),
                                       fmt::arg("major", build::python::version.major));
                }) |
                std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();

        const auto build = []() {
            static constexpr std::array templates = {
                "{vcpkg}/tools/python{major}"sv, "{vcpkg}/tools/python{major}/DLLs"sv,
                "{vcpkg}/tools/python{major}/Lib"sv,
                "{vcpkg}/tools/python{major}/Lib/site-packages"sv, "{bin}"sv};
            const auto bin = filesystem::getExecutablePath().parent_path().generic_string();
            const auto vcpkg =
                fmt::format("{}/{}", build::vcpkg::installDir, build::vcpkg::triplet);

            const auto paths =
                templates | std::views::transform([&](std::string_view path) {
                    return fmt::format(fmt::runtime(path), fmt::arg("bin", bin),
                                       fmt::arg("vcpkg", vcpkg),
                                       fmt::arg("major", build::python::version.major));
                }) |
                std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();
        return {install, build};

    } else if constexpr (build::platform == build::Platform::MacOS) {
        const auto install = []() {
            static constexpr std::array templates = {
                "{root}/lib/python{major}.{minor}"sv,
                "{root}/lib/python{major}.{minor}/lib-dynload"sv,
                "{root}/lib/python{major}.{minor}/site-packages"sv, "{root}/lib"sv};
            const auto root = filesystem::findBasePath().parent_path().generic_string();

            auto paths = templates | std::views::transform([&](std::string_view path) {
                             return fmt::format(fmt::runtime(path), fmt::arg("root", root),
                                                fmt::arg("major", build::python::version.major),
                                                fmt::arg("minor", build::python::version.minor));
                         }) |
                         std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();

        const auto build = []() {
            static constexpr std::array templates = {
                "{vcpkg}/lib/python{major}.{minor}"sv,
                "{vcpkg}/lib/python{major}.{minor}/lib-dynload"sv,
                "{vcpkg}/lib/python{major}.{minor}/site-packages"sv, "{bin}"sv};

            auto exePath = filesystem::getExecutablePath();
            if (exePath.generic_string().contains("Contents/MacOS")) {
                // Running from .app bundle
                exePath = exePath.parent_path().parent_path().parent_path();
            }

            const auto bin = exePath.parent_path().generic_string();
            const auto vcpkg =
                fmt::format("{}/{}", build::vcpkg::installDir, build::vcpkg::triplet);

            const auto paths =
                templates | std::views::transform([&](std::string_view path) {
                    return fmt::format(fmt::runtime(path), fmt::arg("bin", bin),
                                       fmt::arg("vcpkg", vcpkg),
                                       fmt::arg("major", build::python::version.major),
                                       fmt::arg("minor", build::python::version.minor));
                }) |
                std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();
        return {install, build};

    } else {  // Linux
        const auto install = []() {
            static constexpr std::array templates = {
                "{root}/lib/python{major}.{minor}"sv,
                "{root}/lib/python{major}.{minor}/lib-dynload"sv,
                "{root}/lib/python{major}.{minor}/site-packages"sv, "{root}/lib"sv};
            const auto root = filesystem::findBasePath().generic_string();

            const auto paths =
                templates | std::views::transform([&](std::string_view path) {
                    return fmt::format(fmt::runtime(path), fmt::arg("root", root),
                                       fmt::arg("major", build::python::version.major),
                                       fmt::arg("minor", build::python::version.minor));
                }) |
                std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();

        const auto build = []() {
            static constexpr std::array templates = {
                "{vcpkg}/lib/python{major}.{minor}"sv,
                "{vcpkg}/lib/python{major}.{minor}/lib-dynload"sv,
                "{vcpkg}/lib/python{major}.{minor}/site-packages"sv, "{bin}"sv};
            const auto bin = filesystem::getExecutablePath().parent_path().generic_string();
            const auto vcpkg =
                fmt::format("{}/{}", build::vcpkg::installDir, build::vcpkg::triplet);

            const auto paths =
                templates | std::views::transform([&](std::string_view path) {
                    return fmt::format(fmt::runtime(path), fmt::arg("bin", bin),
                                       fmt::arg("vcpkg", vcpkg),
                                       fmt::arg("major", build::python::version.major),
                                       fmt::arg("minor", build::python::version.minor));
                }) |
                std::ranges::to<std::vector>();
            return PythonPaths{.paths = paths, .prefix = paths[0]};
        }();
        return {install, build};
    }
}

void initializePythonInterpreter() {
    log::info("Python runtime version: {}, build version {}", Py_GetVersion(),
              build::python::version);

    const auto [installPaths, buildPaths] = getPythonPaths();

    const auto pythonPaths = [&]() -> std::optional<PythonPaths> {
        // Try finding a python installation in the install tree
        if (std::ranges::all_of(installPaths.paths, [&](std::string_view path) {
                return std::filesystem::is_directory(path);
            })) {
            return installPaths;
        }

        // Try finding a python installation in the build tree
        if (std::ranges::all_of(buildPaths.paths, [&](std::string_view path) {
                return std::filesystem::is_directory(path);
            })) {
            return buildPaths;
        }

        return std::nullopt;
    }();

    {
        PyPreConfig preconfig;
        if (pythonPaths) {
            PyPreConfig_InitIsolatedConfig(&preconfig);
        } else {
            PyPreConfig_InitPythonConfig(&preconfig);
        }

        preconfig.parse_argv = 0;
        preconfig.utf8_mode = true;
        Py_PreInitialize(&preconfig);
    }

    PyConfig config;

    if (pythonPaths) {
        log::info("Internal python found");
        PyConfig_InitIsolatedConfig(&config);
        config.parse_argv = 0;
        config.install_signal_handlers = 0;
        config.user_site_directory = 0;
        config.site_import = 0;
        config.module_search_paths_set = 1;

        for (const auto& path : pythonPaths->paths) {
            const auto fullpath = util::toWstring(path);
            PyWideStringList_Append(&config.module_search_paths, fullpath.c_str());
        }

        PyConfig_SetBytesString(&config, &config.prefix, pythonPaths->prefix.c_str());
        PyConfig_SetBytesString(&config, &config.exec_prefix, pythonPaths->prefix.c_str());

    } else {
        log::info("No internal python found, using system python");
        PyConfig_InitPythonConfig(&config);
        config.parse_argv = 0;
        config.install_signal_handlers = 0;
    }

    if (char* venvPath = std::getenv("VIRTUAL_ENV")) {
        // Relevant documentation:
        // https://stackoverflow.com/questions/77881387/
        // how-to-embed-python-3-8-in-a-c-application-while-using-a-virtual-environment
        // https://docs.python.org/3/library/site.html
        const auto venvExecutable = [&]() {
            if constexpr (build::platform == build::Platform::Windows) {
                return fmt::format("{}/Scripts/python.exe", venvPath);
            } else {
                return fmt::format("{}/bin/python", venvPath);
            }
        }();
        PyConfig_SetBytesString(&config, &config.executable, venvExecutable.c_str());
    }

    try {
        pybind11::initialize_interpreter(&config, 0, nullptr, false);
    } catch (const std::exception& e) {
        throw ModuleInitException(e.what());
    }

    if (!Py_IsInitialized()) {
        throw ModuleInitException("Python is not Initialized");
    }

    // On MacOS we need to add the bin folder to the module search path to find our python
    // modules. Windows seems to add this automatically
    if constexpr (build::platform == build::Platform::MacOS) {
        if (!pythonPaths) {
            auto exePath = filesystem::getExecutablePath();
            if (exePath.generic_string().contains("Contents/MacOS")) {
                // Running from .app bundle
                exePath = exePath.parent_path().parent_path().parent_path();
            }
            pyutil::addModulePath(exePath.parent_path());
        }
    }
}

inline constexpr std::string_view loadInviwoPyScript = R"(
import sys
import sysconfig

import traceback

def formatError(e, name):
    pathlist = '\n'.join(f'{i:>2}: {p}' for i, p in enumerate(sys.path))
    config = '\n'.join(f"{k:15}{v}" for k,v in sysconfig.get_config_vars().items())
    tb = ''.join(traceback.format_exception(e))
    helpmsg = 'Note: Inviwo will not access user site-package folders. Make sure to install the packages site-wide or add\n' \
           'your user site-package folder to the environment variable `PYTHONPATH`,\n' \
           'for example "PYTHONPATH=%appdata%\\Python\\Python311\\site-packages".'
    return f"Unexpected error while importing {name}\n{e}{tb}\n{helpmsg}\n\nsys.path:\n{pathlist}\nconfig:\n{config}"

try:
    import numpy
except ModuleNotFoundError as e:
    raise ModuleNotFoundError(formatError(e, "numpy"))
except ImportError as e:
    raise ImportError(formatError(e, "numpy"))

try:
    import inviwopy
except ModuleNotFoundError as e:
    raise ModuleNotFoundError(formatError(e, "inviwopy"))
except ImportError as e:
    raise ImportError(formatError(e, "inviwopy"))


class OutputRedirector:
    def __init__(self, type):
        self.type = type;
        self.buffer = "";

    def flush(self):
        pass

    def fileno(self):
        return None

    def write(self, string):
        self.buffer += string
        
        if len(self.buffer) > 0 and self.buffer[-1] == "\n":
            inviwopy.handlePythonOutput(self.buffer[:-1], self.type)
            self.buffer = ""

sys.stdout = OutputRedirector(0)
sys.stderr = OutputRedirector(1)
)";

}  // namespace

PythonInterpreter::PythonInterpreter() : embedded_{false}, tstate_{nullptr} {

#ifdef WIN32
    // Set environment variable before initializing python to ensure that we do not crash if using
    // conda. See https://github.com/inviwo/inviwo/issues/1178
    _putenv_s("CONDA_PY_ALLOW_REG_PATHS", "1");
#endif

    if (!Py_IsInitialized()) {
        initializePythonInterpreter();

        embedded_ = true;

        try {
            pybind11::exec(loadInviwoPyScript, pybind11::globals());
            tstate_ = PyEval_SaveThread();
        } catch (const pybind11::error_already_set& e) {
            throw ModuleInitException(
                SourceContext{}, "Error while initializing the Python Interpreter\n{}", e.what());
        }
    }
}

PythonInterpreter::~PythonInterpreter() {
    if (embedded_) {
        PyEval_RestoreThread(tstate_);
        pybind11::finalize_interpreter();
    }
}

}  // namespace inviwo
