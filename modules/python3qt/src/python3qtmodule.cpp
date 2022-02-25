
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <modules/python3qt/python3qtmodule.h>

#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/fileproperty.h>

#include <modules/python3qt/pythoneditorwidget.h>
#include <modules/python3qt/pythonmenu.h>
#include <modules/python3qt/pythonsyntaxhighlight.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/propertylistwidget.h>

#include <modules/python3qt/properties/pythonfilepropertywidgetqt.h>
#include <modules/python3qt/properties/pythonpropertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>

#include <warn/push>
#include <warn/ignore/all>
#include <QInputDialog>
#include <QCoreApplication>
#include <warn/pop>

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
              py::arg("defaultResponse") = "");
        m.def("update", [this]() {
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents();
            if (abortPythonEvaluation_) {
                abortPythonEvaluation_ = false;
                throw PythonAbortException("Evaluation aborted");
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
            .def("move", [](PropertyListWidget* w, int x, int y) { w->move(x, y); });

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
