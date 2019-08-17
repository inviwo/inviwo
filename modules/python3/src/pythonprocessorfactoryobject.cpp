/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>

#include <inviwo/core/network/networkutils.h>

#include <iostream>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>

namespace inviwo {

PythonProcessorFactoryObject::PythonProcessorFactoryObject(InviwoApplication* app,
                                                           const std::string& file)
    : PythonProcessorFactoryObjectBase(load(file)), FileObserver(app), app_{app} {

    startFileObservation(file);
}

std::unique_ptr<Processor> PythonProcessorFactoryObject::create(InviwoApplication*) {
    namespace py = pybind11;
    const auto pi = getProcessorInfo();

    try {
        py::object proc = py::eval<py::eval_expr>(name_ + "(\"" + pi.displayName + "\", \"" +
                                                  pi.displayName + "\")");
        auto p = std::unique_ptr<Processor>(proc.cast<Processor*>());
        proc.release();
        return p;

    } catch (std::exception& e) {
        throw Exception(
            "Failed to create processor " + name_ + " from script: " + file_ + ".\n" + e.what(),
            IVW_CONTEXT_CUSTOM("Python"));
    }
}

void PythonProcessorFactoryObject::fileChanged(const std::string&) {
    try {
        auto data = load(file_);
        name_ = data.name;
        LogInfo("Reloaded python processor: \"" << name_ << "\" file: " << file_);
        if (getProcessorInfo() != data.info) {
            LogError("ProcessorInfo changes in \"" + name_ + "\" will not be reflected");
        }
        reloadProcessors();

    } catch (const std::exception& e) {
        LogError("Error reloading file: " << file_ << " Message:\n" << e.what());
    }
}

void PythonProcessorFactoryObject::reloadProcessors() {
    auto net = app_->getProcessorNetwork();
    auto processors = net->getProcessors();
    for (auto p : processors) {
        if (p->getClassIdentifier() == getProcessorInfo().classIdentifier) {
            LogInfo("Updating python processor: \"" << name_ << "\" id: \"" << p->getIdentifier()
                                                    << "\"");
            if (auto replacement = create(app_)) {
                util::replaceProcessor(net, std::move(replacement), p);
            }
        }
    }
}

PythonProcessorFactoryObjectData PythonProcessorFactoryObject::load(const std::string& file) {
    namespace py = pybind11;

    auto ifs = filesystem::ifstream(file);
    std::stringstream ss;
    ss << ifs.rdbuf();
    const auto script = std::move(ss).str();

    const auto nameLabel = std::string{"# Name: "};

    const auto name = [&]() {
        if (script.compare(0, nameLabel.size(), nameLabel) == 0) {
            const auto endl = script.find_first_of('\n');
            return trim(script.substr(nameLabel.size(), endl - nameLabel.size()));
        } else {
            return filesystem::getFileNameWithoutExtension(file);
        }
    }();

    try {
        py::exec(script);
    } catch (const std::exception& e) {
        throw Exception(
            "Failed to load processor " + name + " from script: " + file + ".\n" + e.what(),
            IVW_CONTEXT_CUSTOM("Python"));
    }

    if (!py::globals().contains(name.c_str())) {
        throw Exception(
            "Failed to find python processor \"" + name + "\" in pythons object register",
            IVW_CONTEXT_CUSTOM("Python"));
    }

    try {
        py::object proc = py::eval<py::eval_expr>(name + ".processorInfo()");
        auto p = proc.cast<ProcessorInfo>();
        proc.release();
        return {p, name, file};
    } catch (const std::exception& e) {
        throw Exception("Failed to get processor info for processor " + name +
                            " from script: " + file + ".\n" + e.what(),
                        IVW_CONTEXT_CUSTOM("Python"));
    }
}

PythonProcessorFactoryObjectBase::PythonProcessorFactoryObjectBase(
    PythonProcessorFactoryObjectData data)
    : ProcessorFactoryObject(data.info), name_(data.name), file_(data.file) {}

}  // namespace inviwo
