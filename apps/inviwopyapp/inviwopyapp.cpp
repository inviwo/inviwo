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

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/moduleregistration.h>
#include <inviwo/core/network/processornetwork.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTimer>
#include <QObject>
#include <QFile>
#include <warn/pop>

namespace py = pybind11;

PYBIND11_MODULE(inviwopyapp, m) {
    using namespace inviwo;

    auto inviwopy = py::module::import("inviwopy");
    auto inviwoApplicationClass = static_cast<py::object>(inviwopy.attr("InviwoApplication"));
    py::class_<InviwoApplicationQt, InviwoApplication>(m, "InviwoApplicationQt",
                                                       py::multiple_inheritance{})
        .def(py::init([]() {
            auto app = new InviwoApplicationQt("inviwo");
            app->setStyleSheetFile(":/stylesheets/inviwo.qss");

            auto win = new QMainWindow();
            win->setObjectName("InviwoMainWindow");
            app->setMainWindow(win);
            win->hide();

            return app;
        }))
        .def("run",
             [](InviwoApplicationQt* app) {
                 auto timer = new QTimer(app);
                 QObject::connect(timer, &QTimer::timeout, [app]() {
                     try {
                         py::exec("lambda x: 1");
                     } catch (...) {
                         LogInfoCustom("InviwoPyApp", "Aborted Qt event loop");
                         app->quit();
                     }
                 });
                 timer->start(100);

                 app->exec();
             })
        .def("update", [](InviwoApplicationQt* app) { app->processEvents(); })
        .def("registerModules",
             [](InviwoApplicationQt* app) { app->registerModules(inviwo::getModuleList()); })
        .def("registerRuntimeModules",
             [](InviwoApplicationQt* app) { app->registerModules(RuntimeModuleLoading{}); });

    m.add_object("py", inviwopy);
    m.doc() = "Python inviwo application";
}
