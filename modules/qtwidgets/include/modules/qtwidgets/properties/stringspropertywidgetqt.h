/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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
#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/properties/propertywidget.h>          // for PropertyWidget
#include <inviwo/core/util/stdextensions.h>                 // for make_array
#include <inviwo/core/util/zip.h>                           // for zip
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                // for fromQString, toQString
#include <modules/qtwidgets/lineeditqt.h>                   // for LineEditQt
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <array>       // for array
#include <cstddef>     // for size_t
#include <functional>  // for function
#include <string>      // for string

#include <QHBoxLayout>     // for QHBoxLayout
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSizePolicy>     // for QSizePolicy, QSizePolicy::Min...
#include <QWidget>         // for QWidget

class QHBoxLayout;
namespace inviwo {
class Property;
template <size_t N>
class StringsProperty;
}  // namespace inviwo

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API PropertyWidgetDelegate : public PropertyWidget {
public:
    PropertyWidgetDelegate(Property* p, std::function<void()> func)
        : PropertyWidget(p), func{func} {}

    virtual void updateFromProperty() override { func(); }
    std::function<void()> func;
};

/**
 * \brief PropertyWidget for StringsProperties
 */
template <size_t N>
class StringsPropertyWidgetQt : public PropertyWidgetQt {
public:
    StringsPropertyWidgetQt(StringsProperty<N>* property);
    virtual ~StringsPropertyWidgetQt() = default;

    virtual void updateFromProperty() override;

private:
    StringsProperty<N>* property_;
    EditableLabelQt* label_;
    std::array<LineEditQt*, N> lineEdits_;
    std::array<PropertyWidgetDelegate, N> delegates_;
    QHBoxLayout* hWidgetLayout_;
};

template <size_t N>
StringsPropertyWidgetQt<N>::StringsPropertyWidgetQt(StringsProperty<N>* property)
    : PropertyWidgetQt(property)
    , property_{property}
    , label_{new EditableLabelQt(this, property_)}
    , lineEdits_{util::make_array<N>([]([[maybe_unused]] auto i) { return new LineEditQt(); })}
    , delegates_{util::make_array<N>([&](auto i) {
        auto p = &(property_->strings[i]);
        return PropertyWidgetDelegate(p, [p, l = lineEdits_[i]]() {
            QSignalBlocker blocker(l);
            l->setText(utilqt::toQString(p->get()));
            l->setCursorPosition(0);
            l->clearFocus();
        });
    })} {

    setFocusPolicy(lineEdits_[0]->focusPolicy());
    setFocusProxy(lineEdits_[0]);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);
    hLayout->addWidget(label_);

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

    for (auto&& [lineEdit, prop] : util::zip(lineEdits_, property_->strings)) {
        QSizePolicy sp = lineEdit->sizePolicy();
        sp.setHorizontalStretch(0);
        sp.setHorizontalPolicy(QSizePolicy::Minimum);
        lineEdit->setSizePolicy(sp);
        lineEdit->setMinimumWidth(10);
        hWidgetLayout_->addWidget(lineEdit);
        lineEdit->setPlaceholderText(utilqt::toQString(fmt::format("<{}>", prop.getDisplayName())));

        connect(lineEdit, &LineEditQt::editingFinished, this, [this, p = &prop, l = lineEdit]() {
            std::string valueStr = utilqt::fromQString(l->text());
            p->setInitiatingWidget(this);
            p->set(valueStr);
            p->clearInitiatingWidget();
        });

        connect(lineEdit, &LineEditQt::editingCanceled, this, [p = &prop, l = lineEdit]() {
            // undo textual changes by resetting the contents of the line edit
            QSignalBlocker blocker(l);
            l->setText(utilqt::toQString(p->get()));
            l->setCursorPosition(0);
            l->clearFocus();
        });
    }

    for (auto& delegate : delegates_) {
        delegate.updateFromProperty();
    }
    updateFromProperty();
}

template <size_t N>
void StringsPropertyWidgetQt<N>::updateFromProperty() {}

}  // namespace inviwo
