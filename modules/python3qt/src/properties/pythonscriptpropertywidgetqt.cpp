/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/python3qt/properties/pythonscriptpropertywidgetqt.h>

#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/properties/scriptproperty.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/moduleutils.h>
#include <modules/python3/pyanyconverter.h>
#include <modules/python3/python3module.h>
#include <modules/python3qt/properties/pythoneditordockwidget.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/lineeditqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>

#include <any>
#include <string>

#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QToolButton>
#include <QWidget>

namespace inviwo {

class PropertyEditorWidget;

PythonScriptPropertyWidgetQt::PythonScriptPropertyWidgetQt(ScriptProperty* property)
    : PropertyWidgetQt(property), property_(property), lineEdit_{new LineEditQt()} {

    setFocusPolicy(lineEdit_->focusPolicy());
    setFocusProxy(lineEdit_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    hLayout->addWidget(new EditableLabelQt(this, property_));

    hWidgetLayout_ = new QHBoxLayout();

    {
        hWidgetLayout_->setContentsMargins(0, 0, 0, 0);
        auto widget = new QWidget();
        widget->setLayout(hWidgetLayout_);
        auto sp = widget->sizePolicy();
        sp.setHorizontalStretch(3);
        widget->setSizePolicy(sp);
        hLayout->addWidget(widget);
    }

    {
        QSizePolicy sp = lineEdit_->sizePolicy();
        sp.setHorizontalStretch(3);
        lineEdit_->setSizePolicy(sp);
        hWidgetLayout_->addWidget(lineEdit_);

        connect(lineEdit_, &LineEditQt::editingFinished, this,
                &PythonScriptPropertyWidgetQt::setPropertyValue);
        connect(lineEdit_, &LineEditQt::editingCanceled, [this]() {
            QSignalBlocker blocker(lineEdit_);
            updateFromProperty();
            lineEdit_->clearFocus();
        });
    }

    addEditor();
    installPythonBackend();
    updateFromProperty();
}

PythonScriptPropertyWidgetQt::~PythonScriptPropertyWidgetQt() = default;

void PythonScriptPropertyWidgetQt::setPropertyValue() {
    std::string valueStr = utilqt::fromQString(lineEdit_->text());
    property_->setInitiatingWidget(this);
    util::exceptionGuard([&]() { property_->setSource(valueStr); });
    property_->clearInitiatingWidget();
}

bool PythonScriptPropertyWidgetQt::hasEditorWidget() const { return true; }

PropertyEditorWidget* PythonScriptPropertyWidgetQt::getEditorWidget() {
    if (!editor_) {
        initEditor();
    }
    return editor_.get();
}

void PythonScriptPropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<PythonEditorDockWidget>(property_);
}

void PythonScriptPropertyWidgetQt::addEditor() {
    auto* edit = new QToolButton();
    edit->setIcon(QIcon(":/svgicons/edit.svg"));
    edit->setToolTip("Edit script");
    hWidgetLayout_->addWidget(edit);
    connect(edit, &QToolButton::clicked, this, [this]() {
        if (!editor_) initEditor();
        editor_->updateFromProperty();
        editor_->setVisible(true);
    });
}

void PythonScriptPropertyWidgetQt::installPythonBackend() {
    auto* app = util::getInviwoApplication(property_);
    const auto& converter = util::getModuleByTypeOrThrow<Python3Module>(app).getPyAnyConverter();

    property_->setBackend([&converter](const std::string& source,
                                       const std::vector<std::any>& args) -> std::any {
        namespace py = pybind11;
        const py::gil_scoped_acquire guard{};

        py::dict globals = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

        py::list pyArgs;
        for (const auto& arg : args) {
            pyArgs.append(converter.toPyObject(arg));
        }
        globals["__args__"] = pyArgs;

        try {
            py::exec(source, globals, globals);
        } catch (const py::error_already_set& e) {
            throw Exception(IVW_CONTEXT_CUSTOM("PythonScriptPropertyWidgetQt"),
                            "Python script error: {}", e.what());
        }

        if (globals.contains("__result__")) {
            return converter.toAny(globals["__result__"]);
        }

        return std::any{};
    });
}

void PythonScriptPropertyWidgetQt::updateFromProperty() {
    const QSignalBlocker blocker(lineEdit_);
    lineEdit_->setText(utilqt::toQString(property_->getSource()));
    lineEdit_->setCursorPosition(0);
}

}  // namespace inviwo
