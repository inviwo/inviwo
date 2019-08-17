/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwopy/pylogging.h>

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

void exposeLogging(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<Logger, std::shared_ptr<Logger>>(m, "Logger")
        .def("log", &Logger::log)
        .def("logProcessor", &Logger::logProcessor)
        .def("logNetwork", &Logger::logNetwork)
        .def("logAssertion", &Logger::logAssertion);

    py::class_<LogCentral, Logger, std::shared_ptr<LogCentral>>(m, "LogCentral")
        .def(py::init([]() {
            auto lc = std::make_unique<LogCentral>();
            if (!LogCentral::isInitialized()) {
                LogCentral::init(lc.get());
            }
            return lc;
        }))
        .def("registerLogger",
             [](LogCentral* lc, std::shared_ptr<Logger> logger) { lc->registerLogger(logger); })
        .def_static("get", &LogCentral::getPtr, py::return_value_policy::reference);

    py::class_<ConsoleLogger, Logger, std::shared_ptr<ConsoleLogger>>(m, "ConsoleLogger")
        .def(py::init<>())
        .def("log", &ConsoleLogger::log);

    py::class_<FileLogger, Logger, std::shared_ptr<FileLogger>>(m, "FileLogger")
        .def(py::init<std::string>())
        .def("log", &FileLogger::log);
}

}  // namespace inviwo
