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

#include <array>        // for array
#include <exception>    // for exception
#include <string>       // for operator+, string, basic_string
#include <string_view>  // for string_view
#include <ranges>
#include <fmt/format.h>

using namespace std::string_view_literals;

namespace inviwo {

PythonInterpreter::PythonInterpreter() : embedded_{false}, isInit_(false) {
    namespace py = pybind11;

    if (isInit_) {
        throw ModuleInitException("Python already initialized");
    }

#ifdef WIN32
    // Set environment variable before initializing python to ensure that we do not crash if using
    // conda. See https://github.com/inviwo/inviwo/issues/1178
    _putenv_s("CONDA_PY_ALLOW_REG_PATHS", "1");
#endif

    if (!Py_IsInitialized()) {
        log::info("Python version: {}", Py_GetVersion());

#ifdef WIN32
        static constexpr std::array paths = {""sv, "/DLLs"sv, "/Lib"sv, "/Lib/site-packages"sv};
        const auto prefix = fmt::format("tools/python{}", build::python::version.major);
#else
        static constexpr std::array paths = {""sv, "/lib-dynload"sv, "/site-packages"sv};
        const auto prefix = fmt::format("lib/python{}.{}", build::python::version.major,
                                        build::python::version.minor);
#endif

        const auto basedir = [&]() -> std::optional<std::string> {
            // Try finding a python installation in the install tree
            if (std::ranges::all_of(paths, [&](std::string_view path) {
                    return std::filesystem::is_directory(filesystem::findBasePath() /
                                                         fmt::format("{}{}", prefix, path));
                })) {
                return filesystem::findBasePath().generic_string();

                // Try finding a python installation from vcpkg in the build tree
            } else if (std::ranges::all_of(paths, [&](std::string_view path) {
                           return std::filesystem::is_directory(
                               fmt::format("{}/{}/{}{}", build::vcpkg::installDir,
                                           build::vcpkg::triplet, prefix, path));
                       })) {
                return fmt::format("{}/{}", build::vcpkg::installDir, build::vcpkg::triplet);

            } else {
                return std::nullopt;
            }
        }();

        {
            PyPreConfig preconfig;
            if (basedir) {
                PyPreConfig_InitIsolatedConfig(&preconfig);
            } else {
                PyPreConfig_InitPythonConfig(&preconfig);
            }

            preconfig.parse_argv = 0;
            preconfig.utf8_mode = true;
            Py_PreInitialize(&preconfig);
        }

        PyConfig config;

        if (basedir) {
            PyConfig_InitIsolatedConfig(&config);
            config.parse_argv = 0;
            config.install_signal_handlers = 0;
            config.user_site_directory = 0;
            config.site_import = 0;
            config.module_search_paths_set = 1;

            for (auto path : paths) {
                const auto fullpath =
                    util::toWstring(fmt::format("{}/{}{}", *basedir, prefix, path));
                PyWideStringList_Append(&config.module_search_paths, fullpath.c_str());
            }

            PyConfig_SetBytesString(&config, &config.prefix, basedir->c_str());
            PyConfig_SetBytesString(&config, &config.exec_prefix, basedir->c_str());

        } else {
            PyConfig_InitPythonConfig(&config);
            config.parse_argv = 0;
            config.install_signal_handlers = 0;
        }

        if (char* venvPath = std::getenv("VIRTUAL_ENV")) {

            // Relevant documentation:
            // https://stackoverflow.com/questions/77881387/
            // how-to-embed-python-3-8-in-a-c-application-while-using-a-virtual-environment
            // https://docs.python.org/3/library/site.html
#ifdef WIN32
            auto venvExecutable = fmt::format("{}/Scripts/python.exe", venvPath);
#else
            auto venvExecutable = fmt::format("{}/bin/python", venvPath);
#endif
            PyConfig_SetBytesString(&config, &config.executable, venvExecutable.c_str());
        }

        try {
            py::initialize_interpreter(&config, 0, nullptr, false);
        } catch (const std::exception& e) {
            throw ModuleInitException(e.what());
        }

        isInit_ = true;
        embedded_ = true;

        if (!Py_IsInitialized()) {
            throw ModuleInitException("Python is not Initialized");
        }

        auto binDir = filesystem::getExecutablePath().parent_path();
        pyutil::addModulePath(binDir);

#if defined(__APPLE__)
        // When deployed on maxos we assume the following structure
        // inviwo.app/
        // └── Contents/
        //     ├── Info.plist
        //     ├── PkgInfo
        //     ├── _CodeSignature/
        //     ├── MacOS/
        //     │   ├── inviwo                  (main executable)
        //     ├── Resources/                  This should be the returned by findBasePath
        //     │   ├── data/                   (application data)
        //     │   ├── licenses/               (license files)
        //     │   ├── modules/                (Inviwo modules)
        //     │   └── tools/                  (utility tools)
        //     ├── PlugIns/                    (Qt plugins)
        //     ├── cmake/                      (CMake configuration files)
        //     ├── include/                    (Header files)
        //     ├── lib/                        (Dynamic libraries, and python modules)
        //     └── share/                      (Shared data)

        // We will find the python modules in the lib folder
        if (std::filesystem::is_directory(filesystem::findBasePath() / ".." / "lib")) {
            pyutil::addModulePath(filesystem::findBasePath() / ".." / "lib");
        } else {
            // When developing the python modules will next to the inviwo.app folder.
            // On OSX the path returned by getExecutablePath includes folder-paths inside the
            // app-binary
            pyutil::addModulePath(binDir / "../../../");
        }

        if (std::filesystem::is_directory(inviwo::build::python::sitelib)) {
            pyutil::addModulePath(inviwo::build::python::sitelib);
        }

        if (std::filesystem::is_directory(filesystem::findBasePath() / ".." / "site-packages")) {
            pyutil::addModulePath(filesystem::findBasePath() / ".." / "site-packages");
        }

#endif

        try {
            py::exec(R"(
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
)",
                     py::globals());

            tstate_ = PyEval_SaveThread();

        } catch (const py::error_already_set& e) {
            throw ModuleInitException(
                SourceContext{}, "Error while initializing the Python Interpreter\n{}", e.what());
        }
    }
}

PythonInterpreter::~PythonInterpreter() {
    namespace py = pybind11;
    if (embedded_) {
        PyEval_RestoreThread(tstate_);
        py::finalize_interpreter();
    }
}

void PythonInterpreter::importModule(std::string_view moduleName) {
    namespace py = pybind11;
    SafeCStr str(moduleName);

    auto dict = py::globals();
    dict[str] = py::module::import(str);
}

bool PythonInterpreter::runString(std::string_view code) {
    SafeCStr str(code);

    auto ret = PyRun_SimpleString(str);
    return ret == 0;
}

}  // namespace inviwo
