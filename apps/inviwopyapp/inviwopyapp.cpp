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

    m.add_object("py", inviwopy);
    m.doc() = "Python inviwo application";

    auto logger = std::make_shared<inviwo::ConsoleLogger>();
    LogCentral::getPtr()->registerLogger(logger);
    m.attr("consoleLogger") = py::cast(logger);

    int argc = 0;
    auto app = new InviwoApplicationQt(argc, nullptr, "inviwo");
    app->setStyleSheetFile(":/stylesheets/inviwo.qss");

    auto win = new QMainWindow();
    win->setObjectName("InviwoMainWindow");
    app->setMainWindow(win);
    win->hide();

    app->registerModules(inviwo::getModuleList());

    m.def("run",
          [app]() {
              auto timer = new QTimer(app);
              QObject::connect(timer, &QTimer::timeout, [app]() {
                  try {
                      py::exec("lambda x: 1");
                  } catch (...) {
                      LogInfoCustom("inviwopyapp",
                                    "Abort qt event loop, to restart call 'inviwopyapp.run()'");
                      app->quit();
                  }
              });
              timer->start(100);

              app->exec();
          },
          "Run the inviwo event loop");
}
