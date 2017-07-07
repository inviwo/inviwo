
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
#include <modules/python3qt/python3qtmodule.h>
#include <modules/python3qt/pythoneditorwidget.h>
#include <modules/python3/pythoninterpreter.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3qt/pythonmenu.h>

#include <inviwo/core/util/exception.h>

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
                                         defaultResponse.c_str(), &ok);
    if (ok && !text.isEmpty()) {
        return pybind11::str(text.toLocal8Bit().constData());
    } else if (ok) {
        return pybind11::str("");
    }
    return pybind11::none();
}
}

Python3QtModule::Python3QtModule(InviwoApplication* app)
    : InviwoModule(app, "Python3Qt"), menu_(util::make_unique<PythonMenu>(app)) {
    auto module = InviwoApplication::getPtr()->getModuleByType<Python3Module>();
    if (module) {
        auto ivwmodule = module->getInviwopyModule();
        auto m = ivwmodule->def_submodule("qt", "Qt dependent stuff");

        m.def("prompt", &prompt, pybind11::arg("title"), pybind11::arg("message"),
            pybind11::arg("defaultResponse") = "");
        m.def("update", []() { QCoreApplication::instance()->processEvents(); });
    } else {
        throw Exception("Failed to get Python3Module");
    }
}

Python3QtModule::~Python3QtModule() = default;

}  // namespace
