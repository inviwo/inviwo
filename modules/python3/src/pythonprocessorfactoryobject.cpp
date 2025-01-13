/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/python3/pythonprocessorfactoryobject.h>

#include <pybind11/pybind11.h>  // for globals, module_, module
#include <pybind11/cast.h>      // for object::cast, object_api::ope...
#include <pybind11/eval.h>      // for eval, exec, eval_expr
#include <pybind11/pytypes.h>   // for object, error_already_set
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/common/inviwoapplication.h>           // for InviwoApplication
#include <inviwo/core/network/networkutils.h>               // for replaceProcessor
#include <inviwo/core/network/processornetwork.h>           // for ProcessorNetwork
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorfactoryobject.h>  // for ProcessorFactoryObject
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo, operator!=
#include <inviwo/core/util/exception.h>                     // for Exception
#include <inviwo/core/util/fileobserver.h>                  // for FileObserver
#include <inviwo/core/util/filesystem.h>                    // for getFileNameWithoutExtension
#include <inviwo/core/util/logcentral.h>                    // for LogCentral
#include <inviwo/core/util/sourcecontext.h>                 // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stringconversion.h>              // for trim
#include <inviwo/core/util/utilities.h>                     // for stripIdentifier

#include <array>        // for array
#include <exception>    // for exception
#include <fstream>      // for operator<<, basic_ostream
#include <sstream>      // for basic_stringstream<>::string_...
#include <string_view>  // for string_view
#include <type_traits>  // for remove_reference<>::type
#include <utility>      // for move
#include <vector>       // for vector

namespace inviwo {

PythonProcessorFactoryObject::PythonProcessorFactoryObject(InviwoApplication* app,
                                                           const std::filesystem::path& file)
    : PythonProcessorFactoryObjectBase(load(file)), FileObserver(app), app_{app} {
    startFileObservation(file);
}

std::shared_ptr<Processor> PythonProcessorFactoryObject::create(InviwoApplication*) const {
    namespace py = pybind11;
    const auto& pi = getProcessorInfo();

    try {
        auto main = py::module::import("__main__");
        py::object proc =
            main.attr(name_.c_str())(util::stripIdentifier(pi.displayName), pi.displayName);
        return proc.cast<std::shared_ptr<Processor>>();

    } catch (std::exception& e) {
        throw Exception(IVW_CONTEXT_CUSTOM("Python"),
                        "Failed to create processor {} from script: {}\n{}", name_, file_,
                        e.what());
    }
}

void PythonProcessorFactoryObject::fileChanged(const std::filesystem::path&) {
    try {
        auto data = load(file_);
        name_ = data.name;
        log::info("Reloaded python processor: '{}' file: {}", name_, file_);
        if (getProcessorInfo() != data.info) {
            log::error("ProcessorInfo changes in '{}' will not be reflected", name_);
        }
        reloadProcessors();

    } catch (const std::exception& e) {
        log::error("Error reloading file: '{}' Message:\n{}", file_, e.what());
    }
}

void PythonProcessorFactoryObject::reloadProcessors() {
    auto net = app_->getProcessorNetwork();
    auto processors = net->getProcessors();  // Save a list of the processors here since we will be
                                             // modifying the list below by replacing them
    for (auto p : processors) {
        if (p->getClassIdentifier() == getProcessorInfo().classIdentifier) {
            log::info(R"(Updating python processor: "{}" id: "{}")", name_, p->getIdentifier());
            if (auto replacement = static_cast<std::shared_ptr<Processor>>(create(app_))) {
                util::replaceProcessor(net, replacement, p);
            }
        }
    }
}

namespace {

pybind11::object compileAndRun(const std::string& expr, const std::string& name,
                               pybind11::object global = pybind11::globals(),
                               pybind11::object local = pybind11::object()) {
    if (!local) {
        local = global;
    }
    pybind11::detail::ensure_builtins_in_globals(global);

    PyObject* byteCode = Py_CompileStringObject(expr.c_str(), pybind11::cast(name).ptr(),
                                                Py_file_input, nullptr, -1);
    if (!byteCode) {
        throw pybind11::error_already_set();
    }
    PyObject* result = PyEval_EvalCode(byteCode, global.ptr(), local.ptr());
    Py_XDECREF(byteCode);
    if (!result) {
        throw pybind11::error_already_set();
    }
    return pybind11::reinterpret_steal<pybind11::object>(result);
}

}  // namespace

PythonProcessorFactoryObjectData PythonProcessorFactoryObject::load(
    const std::filesystem::path& file) {
    namespace py = pybind11;

    auto ifs = std::ifstream(file);
    std::stringstream ss;
    ss << ifs.rdbuf();
    const auto script = std::move(ss).str();

    constexpr std::string_view nameLabel{"# Name: "};

    const auto name = [&]() {
        if (script.compare(0, nameLabel.size(), nameLabel) == 0) {
            const auto endl = script.find_first_of('\n');
            return trim(script.substr(nameLabel.size(), endl - nameLabel.size()));
        } else {
            return file.stem().string();
        }
    }();

    try {
        compileAndRun(script, file.generic_string());
    } catch (const py::error_already_set& e) {
        if (e.matches(PyExc_ModuleNotFoundError)) {
            const auto missingModule = e.value().attr("name").cast<std::string>();
            static bool printedNoteOnce = false;
            std::string_view note =
                printedNoteOnce
                    ? ""
                    : "\n\nNote: Inviwo will not access user site-package folders. Make sure to "
                      "install the packages site-wide or add\n"
                      "your user site-package folder to the environment variable `PYTHONPATH`,\n"
                      "for example \"PYTHONPATH=%appdata%\\Python\\Python311\\site-packages\".";
            printedNoteOnce = true;

            throw Exception(
                IVW_CONTEXT_CUSTOM("Python"),
                "Failed to load python processor: '{}' due to missing module: '{}'. File: {}{}",
                name, missingModule, file, note);
        } else if (e.matches(PyExc_SyntaxError)) {
            const auto filename = e.value().attr("filename").cast<std::string>();
            auto lineno = e.value().attr("lineno").cast<int>();
            const auto offset = e.value().attr("offset").cast<int>();
            const auto text = e.value().attr("text").cast<std::string>();

            if (filename == "<string>") {
                e.value().attr("filename") = file.generic_string();
                // HACK: It seems that when we have a <string> file like this that the
                // lineno is off by one. At least on mac, and python 3.10. No idea why.
                lineno -= 1;
                e.value().attr("lineno") = lineno;
            }

            throw Exception(IVW_CONTEXT_CUSTOM("Python"), "{}\n{:<6}:{}\n{:>{}}", e.what(), lineno,
                            rtrim(text), "^", offset + 7);

        } else {
            throw Exception(IVW_CONTEXT_CUSTOM("Python"),
                            "Failed to load python processor: '{}'. File: {}\n{}", name, file,
                            e.what());
        }
    } catch (const std::exception& e) {
        throw Exception(IVW_CONTEXT_CUSTOM("Python"),
                        "Failed to load python processor: '{}'. File: {}\n{}", name, file,
                        e.what());
    }

    if (!py::globals().contains(name.c_str())) {
        throw Exception(IVW_CONTEXT_CUSTOM("Python"),
                        "Failed to find python processor: '{}' in pythons object register", name);
    }

    try {
        py::object proc = py::eval<py::eval_expr>(name + ".processorInfo()");
        auto p = proc.cast<ProcessorInfo>();
        return {p, name, file};
    } catch (const std::exception& e) {
        throw Exception(IVW_CONTEXT_CUSTOM("Python"),
                        "Failed to get ProcessorInfo for python processor: '{}'. File: {}\n{}",
                        name, file, e.what());
    }
}

PythonProcessorFactoryObjectBase::PythonProcessorFactoryObjectBase(
    PythonProcessorFactoryObjectData data)
    : ProcessorFactoryObject(data.info, data.name), name_(data.name), file_(data.file) {}

}  // namespace inviwo
