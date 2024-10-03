/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeLogging(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<LogLevel>(m, "LogLevel")
        .value("Info", LogLevel::Info)
        .value("Warn", LogLevel::Warn)
        .value("Error", LogLevel::Error);

    py::enum_<LogVerbosity>(m, "LogVerbosity")
        .value("Info", LogVerbosity::Info)
        .value("Warn", LogVerbosity::Warn)
        .value("Error", LogVerbosity::Error)
        .value("Nothing", LogVerbosity::None);

    py::enum_<LogAudience>(m, "LogAudience")
        .value("User", LogAudience::User)
        .value("Developer", LogAudience::Developer);

    py::enum_<MessageBreakLevel>(m, "MessageBreakLevel")
        .value("Off", MessageBreakLevel::Off)
        .value("Error", MessageBreakLevel::Error)
        .value("Warn", MessageBreakLevel::Warn)
        .value("Info", MessageBreakLevel::Info);

    py::class_<Logger>(m, "Logger")
        .def("log", &Logger::log)
        .def("logProcessor", &Logger::logProcessor)
        .def("logNetwork", &Logger::logNetwork)
        .def("logAssertion", &Logger::logAssertion);

    py::class_<LogCentral, Logger>(m, "LogCentral")
        .def(py::init([]() {
            auto lc = std::make_unique<LogCentral>();
            if (!LogCentral::isInitialized()) {
                LogCentral::init(lc.get());
            }
            return lc;
        }))
        .def("registerLogger",
             [](LogCentral* lc, std::shared_ptr<Logger> logger) { lc->registerLogger(logger); })
        .def_property("verbosity", &LogCentral::getVerbosity, &LogCentral::setVerbosity)
        .def_property("logStacktrace", &LogCentral::getLogStacktrace, &LogCentral::setLogStacktrace)
        .def_property("messageBreakLevel", &LogCentral::getMessageBreakLevel,
                      &LogCentral::setMessageBreakLevel)
        .def_static("get", &LogCentral::getPtr, py::return_value_policy::reference)
        .def("log", &LogCentral::log, py::arg("source") = "", py::arg("level") = LogLevel::Info,
             py::arg("audience") = LogAudience::Developer, py::arg("file") = "",
             py::arg("function") = "", py::arg("line") = 0, py::arg("msg") = "")
        .def("logProcessor", &LogCentral::logProcessor, py::arg("processor"),
             py::arg("level") = LogLevel::Info, py::arg("audience") = LogAudience::Developer,
             py::arg("msg") = "", py::arg("file") = "", py::arg("function") = "",
             py::arg("line") = 0)
        .def("logNetwork", &LogCentral::logNetwork, py::arg("level") = LogLevel::Info,
             py::arg("audience") = LogAudience::Developer, py::arg("msg") = "",
             py::arg("file") = "", py::arg("function") = "", py::arg("line") = 0)
        .def("logAssertion", &LogCentral::logAssertion, py::arg("file") = "",
             py::arg("function") = "", py::arg("line") = 0, py::arg("msg") = "");

    py::class_<ConsoleLogger, Logger>(m, "ConsoleLogger")
        .def(py::init<>())
        .def("log", &ConsoleLogger::log);

    py::class_<FileLogger, Logger>(m, "FileLogger")
        .def(py::init<std::string>())
        .def("log", &FileLogger::log);

    m.def(
        "log",
        [](std::string_view source, LogLevel level, LogAudience audience, std::string_view file,
           std::string_view function, int line, std::string_view msg) {
            if (LogCentral::isInitialized()) {
                LogCentral::getPtr()->log(source, level, audience, file, function, line, msg);
            }
        },
        py::arg("source") = "", py::arg("level") = LogLevel::Info,
        py::arg("audience") = LogAudience::Developer, py::arg("file") = "",
        py::arg("function") = "", py::arg("line") = 0, py::arg("msg") = "");

    m.def("logInfo", [](const std::string& msg) { LogInfoCustom("inviwopy", msg); });
    m.def("logWarn", [](const std::string& msg) { LogWarnCustom("inviwopy", msg); });
    m.def("logError", [](const std::string& msg) { LogErrorCustom("inviwopy", msg); });
}

}  // namespace inviwo
