
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <modules/python3qt/python3qtmodule.h>  // for Python3QtModule

// Note: Need to put any python includes first since qt defines some "slots" macro
// which python also uses from some structs
#include <pybind11/cast.h>           // for arg
#include <pybind11/detail/common.h>  // for pybind11
#include <pybind11/detail/descr.h>   // for operator+
#include <pybind11/pybind11.h>       // for class_, module_, init
#include <pybind11/pytypes.h>        // for str, none, reinterp...
#include <pybind11/embed.h>

#include <inviwo/core/common/inviwoapplication.h>      // IWYU pragma: keep
#include <inviwo/core/common/inviwomodule.h>           // for InviwoModule
#include <inviwo/core/properties/fileproperty.h>       // for FileProperty
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>     // for StringProperty
#include <inviwo/core/util/exception.h>                // for Exception, Exceptio...
#include <inviwo/core/util/glmvec.h>                   // for ivec2
#include <inviwo/core/util/settings/settings.h>        // for Settings
#include <inviwo/core/util/sourcecontext.h>            // for IVW_CONTEXT
#include <inviwo/core/network/processornetwork.h>
#include <modules/python3qt/properties/pythonfilepropertywidgetqt.h>  // for PythonFilePropertyW...
#include <modules/python3qt/properties/pythonpropertywidgetqt.h>      // for PythonPropertyWidgetQt
#include <modules/python3qt/pythonmenu.h>                             // for PythonMenu
#include <modules/python3qt/pythonsyntaxhighlight.h>                  // for PythonSyntaxHighlight
#include <modules/qtwidgets/inviwoqtutils.h>                          // for toGLM, getApplicati...
#include <modules/qtwidgets/propertylistwidget.h>                     // for PropertyListWidget

#include <inviwo/qt/applicationbase/qtapptools.h>

#include <atomic>       // for atomic
#include <exception>    // for exception
#include <memory>       // for make_unique, unique...
#include <string>       // for string
#include <string_view>  // for string_view

#include <QByteArray>        // for QByteArray
#include <QCoreApplication>  // for QCoreApplication
#include <QFlags>            // for QFlags
#include <QInputDialog>      // for QInputDialog
#include <QLineEdit>         // for QLineEdit, QLineEdi...
#include <QMainWindow>       // for QMainWindow
#include <QString>           // for QString
#include <Qt>                // for MSWindowsFixedSizeD...
#include <QTimer>
#include <glm/vec2.hpp>  // for vec<>::(anonymous)

namespace inviwo {

namespace {
pybind11::object prompt(std::string title, std::string message, std::string defaultResponse = "") {

    bool ok;
    QString text = QInputDialog::getText(nullptr, title.c_str(), message.c_str(), QLineEdit::Normal,
                                         defaultResponse.c_str(), &ok,
                                         Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    if (ok && !text.isEmpty()) {
        return pybind11::str(text.toLocal8Bit().constData());
    } else if (ok) {
        return pybind11::str("");
    }
    return pybind11::none();
}
}  // namespace

Python3QtModule::Python3QtModule(InviwoApplication* app)
    : InviwoModule(app, "Python3Qt")
    , abortPythonEvaluation_{false}
    , menu_(std::make_unique<PythonMenu>(this, app)) {
    namespace py = pybind11;

    try {
        auto inviwopy = py::module::import("inviwopy");
        auto m = inviwopy.def_submodule("qt", "Qt dependent stuff");

        m.def("prompt", &prompt, py::arg("title"), py::arg("message"),
              py::arg("defaultResponse") = "")
            .def("update",
                 [this]() {
                     qApp->processEvents();
                     qApp->sendPostedEvents();
                     if (abortPythonEvaluation_) {
                         abortPythonEvaluation_ = false;
                         throw PythonAbortException("Evaluation aborted");
                     }
                 })
            .def("c", &utilqt::configureInviwoQtApp)
            .def("logQtMessages",
                 [](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
                     utilqt::logQtMessages(type, context, msg);
                 })
            .def("configureFileSystemObserver", &utilqt::configureFileSystemObserver)
            .def("configurePostEnqueueFront", &utilqt::configurePostEnqueueFront)
            .def("configurePoolResizeWait", &utilqt::configurePoolResizeWait)
            .def("setStyleSheetFile", &utilqt::setStyleSheetFile)
            .def("execWithTimer",
                 []() {
                     auto timer = new QTimer(qApp);
                     QObject::connect(timer, &QTimer::timeout, []() {
                         try {
                             py::exec("lambda x: 1");
                         } catch (...) {
                             LogInfoCustom("InviwoPyApp", "Aborted Qt event loop");
                             qApp->quit();
                         }
                     });
                     timer->start(100);

                     qApp->exec();
                 })
            .def(
                "exit", [](int i) { qApp->exit(i); }, py::arg("exitCode") = 0)
            .def(
                "waitForNetwork",
                [](InviwoApplication* app, int maxJobs) {
                    qApp->processEvents();
                    app->waitForPool();
                    do {
                        qApp->processEvents();
                        app->processFront();
                    } while (app->getProcessorNetwork()->runningBackgroundJobs() > maxJobs);
                },
                py::arg("inviwoApplication"), py::arg("maxJobs") = 0)

            .def("address",
                 [](ProcessorWidget* w) {
                     if (auto qw = dynamic_cast<QWidget*>(w)) {
                         return reinterpret_cast<std::intptr_t>(static_cast<void*>(qw));
                     } else {
                         throw Exception("invalid object");
                     }
                 })
            .def("address",
                 [](PropertyWidget* w) {
                     if (auto qw = dynamic_cast<QWidget*>(w)) {
                         return reinterpret_cast<std::intptr_t>(static_cast<void*>(qw));
                     } else {
                         throw Exception("invalid object");
                     }
                 })
            .def("address", [](PropertyEditorWidget* w) {
                if (auto qw = dynamic_cast<QWidget*>(w)) {
                    return reinterpret_cast<std::intptr_t>(static_cast<void*>(qw));
                } else {
                    throw Exception("invalid object");
                }
            });

        py::class_<PropertyListWidget>(m, "PropertyListWidget")
            .def(py::init([](InviwoApplication* app) {
                auto mainwin = utilqt::getApplicationMainWindow();
                auto plw = new PropertyListWidget(mainwin, app);
                plw->setFloating(true);
                return plw;
            }))
            .def("addProcessorProperties", &PropertyListWidget::addProcessorProperties)
            .def("removeProcessorProperties", &PropertyListWidget::removeProcessorProperties)
            .def_property("visible", &PropertyListWidget::isVisible,
                          &PropertyListWidget::setVisible)
            .def_property("floating", &PropertyListWidget::isFloating,
                          &PropertyListWidget::setFloating)
            .def_property(
                "dimensions", [](const PropertyListWidget& w) { return utilqt::toGLM(w.size()); },
                [](PropertyListWidget& w, ivec2 dim) { w.resize(dim.x, dim.y); })
            .def_property(
                "position", [](const PropertyListWidget& w) { return utilqt::toGLM(w.pos()); },
                [](PropertyListWidget& w, ivec2 pos) { w.move(pos.x, pos.y); })

            .def("removeAndDeleteProcessorProperties",
                 &PropertyListWidget::removeAndDeleteProcessorProperties)

            .def("addPropertyWidgets", &PropertyListWidget::addPropertyWidgets)
            .def("removePropertyWidgets", &PropertyListWidget::removePropertyWidgets)
            .def("removeAndDeletePropertyWidgets",
                 &PropertyListWidget::removeAndDeletePropertyWidgets)

            .def("show", &PropertyListWidget::show)
            .def("hide", &PropertyListWidget::hide)
            .def("move", [](PropertyListWidget* w, int x, int y) { w->move(x, y); })
            .def("address", [](PropertyListWidget* w) {
                return reinterpret_cast<std::intptr_t>(static_cast<void*>(w));
            });

    } catch (const std::exception& e) {
        throw ModuleInitException(e.what(), IVW_CONTEXT);
    }

    registerSettings(std::make_unique<PythonSyntaxHighlight>());

    registerPropertyWidget<PythonFilePropertyWidgetQt, FileProperty>(
        PropertySemantics::PythonEditor);
    registerPropertyWidget<PythonPropertyWidgetQt, StringProperty>(PropertySemantics::PythonEditor);
}

Python3QtModule::~Python3QtModule() = default;

void Python3QtModule::abortPythonEvaluation() { abortPythonEvaluation_ = true; }

PythonAbortException::PythonAbortException(const std::string& message, ExceptionContext context)
    : Exception(message, context) {}

}  // namespace inviwo
